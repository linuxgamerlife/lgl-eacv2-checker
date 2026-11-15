#include "core/checks.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <iterator>
#include <sstream>
#include <unordered_set>

namespace ec {
namespace {

std::string trim(std::string value) {
    const auto not_space = [](const unsigned char character) { return !std::isspace(character); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), not_space));
    value.erase(std::find_if(value.rbegin(), value.rend(), not_space).base(), value.end());
    return value;
}

std::unordered_set<std::string> parse_comma_separated_names(const std::string& value) {
    std::unordered_set<std::string> names;
    std::istringstream input(value);
    std::string name;
    while (std::getline(input, name, ',')) {
        name = trim(std::move(name));
        if (!name.empty()) {
            names.insert(std::move(name));
        }
    }
    return names;
}

CheckResult unavailable(std::string id, std::string title, std::string evidence,
                        std::string remediation, std::vector<SuggestedCommand> commands = {}) {
    return {std::move(id),
            std::move(title),
            CheckStatus::Unknown,
            "Unable to read the required system information.",
            std::move(evidence),
            std::move(remediation),
            std::move(commands)};
}

} // namespace

std::string parse_os_release_value(const std::string& contents, const std::string& key) {
    std::istringstream lines(contents);
    std::string line;
    while (std::getline(lines, line)) {
        if (line.starts_with(key + "=")) {
            auto value = trim(line.substr(key.size() + 1U));
            if (value.size() >= 2U && value.front() == '"' && value.back() == '"') {
                value = value.substr(1U, value.size() - 2U);
            }
            return value;
        }
    }
    return {};
}

CheckResult check_platform(const FileReader& files) {
    const auto contents = files.read("/etc/os-release");
    if (!contents) {
        return unavailable(
            "platform", "Fedora platform", "/etc/os-release was unavailable.",
            "First, use the commands below to see whether Fedora's identity file exists and which "
            "package owns it.\n\n"
            "If the first command reports that the file is missing, or the second reports that no "
            "package owns it, the Fedora installation may be damaged. Do not create or edit the "
            "file yourself. Repair the Fedora release package before relying on this checker.",
            {{"Check whether the Fedora identity file exists", "ls -l /etc/os-release"},
             {"Show which installed package owns the file", "rpm -qf /etc/os-release"}});
    }
    const auto id = parse_os_release_value(*contents, "ID");
    const auto version = parse_os_release_value(*contents, "VERSION_ID");
    if (id != "fedora") {
        return {"platform",
                "Fedora platform",
                CheckStatus::Fail,
                "This system does not identify itself as Fedora.",
                "Detected ID: " + (id.empty() ? std::string{"not present"} : id),
                "This checker is designed for Fedora. If you are using another Linux "
                "distribution, no setting will make this check pass. Do not edit os-release to "
                "pretend the system is Fedora.\n\n"
                "Install and boot a current Fedora release before using this checker. You may "
                "still read the other results, but treat them as informational."};
    }
    if (version.empty()) {
        return unavailable(
            "platform", "Fedora platform", "Fedora was detected without VERSION_ID.",
            "Use the commands below to view the file and check its package. Do not add VERSION_ID "
            "by hand.\n\n"
            "If RPM reports modified or missing files, reinstall the package name printed by the "
            "second command using Fedora's Software app or DNF.",
            {{"Display the Fedora identity file", "cat /etc/os-release"},
             {"Show which installed package owns the file", "rpm -qf /etc/os-release"},
             {"Check that package for changed or missing files",
              "rpm -V \"$(rpm -qf /etc/os-release)\""}});
    }
    return {"platform", "Fedora platform", CheckStatus::Pass, "Fedora " + version + " detected.",
            "Evidence: ID=fedora and VERSION_ID=" + version + " in /etc/os-release.", std::nullopt};
}

CheckResult check_uefi(const FileReader& files) {
    if (files.exists("/sys/firmware/efi")) {
        return {"uefi", "UEFI boot", CheckStatus::Pass, "The current system boot used UEFI.",
                "Evidence: /sys/firmware/efi exists.", std::nullopt};
    }
    return {"uefi",
            "UEFI boot",
            CheckStatus::Fail,
            "The current boot does not expose UEFI firmware information.",
            "Evidence: /sys/firmware/efi does not exist.",
            "Your current session appears to have started in Legacy BIOS mode.\n\n"
            "1. Save your work and restart the computer.\n"
            "2. Open the firmware setup screen. The key is commonly F2, Delete, F10, or F12 and "
            "is normally shown when the computer starts.\n"
            "3. Find the boot settings. Enable UEFI and disable Legacy Boot or CSM.\n"
            "4. Save the change and choose the entry named Fedora from the UEFI boot menu.\n"
            "5. Open this checker again.\n\n"
            "Important: an older Fedora installation made in Legacy mode may not start after this "
            "change. Back up important files first and use Fedora's bootloader documentation if "
            "there is no Fedora UEFI entry.",
            {{"Confirm how the current session was started",
              "test -d /sys/firmware/efi && echo \"UEFI boot detected\" || echo \"Legacy/BIOS "
              "boot detected\""}}};
}

CheckResult check_lockdown(const FileReader& files) {
    const auto value = files.read("/sys/kernel/security/lockdown");
    if (!value) {
        return unavailable(
            "lockdown", "Kernel lockdown", "/sys/kernel/security/lockdown was unavailable.",
            "Use the commands below to check Secure Boot and the kernel security directory.\n\n"
            "If Secure Boot is disabled, enable it in the computer's firmware settings and boot "
            "the normal Fedora entry. If the directory exists but the lockdown file does not, "
            "make sure you are using Fedora's standard kernel. Running this app as root will not "
            "fix a missing kernel interface.",
            {{"Show whether Secure Boot is enabled", "mokutil --sb-state"},
             {"Check the kernel security directory", "ls -ld /sys/kernel/security"},
             {"Check whether the lockdown status file exists",
              "ls -l /sys/kernel/security/lockdown"},
             {"Show the running kernel version", "uname -r"}});
    }
    if (value->find("[integrity]") != std::string::npos) {
        return {"lockdown", "Kernel lockdown", CheckStatus::Pass,
                "Kernel lockdown is in integrity mode.", "Evidence: [integrity] is selected.",
                std::nullopt};
    }
    return {"lockdown",
            "Kernel lockdown",
            CheckStatus::Fail,
            "Kernel lockdown is not in integrity mode.",
            "Observed lockdown modes: " + trim(*value),
            "1. Copy and run the commands below.\n"
            "2. If Secure Boot is disabled, restart into the firmware settings and enable Secure "
            "Boot. Then boot the normal Fedora entry.\n"
            "3. If Secure Boot is already enabled, the log command may explain why lockdown was "
            "not enabled. Check that you booted Fedora's standard signed kernel rather than a "
            "custom kernel.\n\n"
            "Secure Boot may stop unsigned NVIDIA, DKMS, or other third-party modules from "
            "loading. Sign or replace those modules; do not turn off signature checking just to "
            "make this test pass.",
            {{"Show whether Secure Boot is enabled", "mokutil --sb-state"},
             {"Show the current lockdown mode", "cat /sys/kernel/security/lockdown"},
             {"Show lockdown messages from this boot",
              "journalctl --dmesg --boot=0 --no-pager | grep -i lockdown"}}};
}

CheckResult check_kexec(const FileReader& files) {
    const auto value = files.read("/proc/sys/kernel/kexec_load_disabled");
    if (!value) {
        return unavailable(
            "kexec", "Kexec loading", "The kexec sysctl was unavailable.",
            "Run the commands below as your normal user. They check whether /proc is available and "
            "whether this kernel provides the kexec setting.\n\n"
            "If /proc is mounted but the setting is still missing, this checker cannot verify "
            "kexec on the current kernel. Use a current standard Fedora kernel and try again.",
            {{"Confirm that /proc is available", "mountpoint /proc"},
             {"Check whether the kernel provides the kexec setting",
              "ls -l /proc/sys/kernel/kexec_load_disabled"},
             {"Show the running kernel version", "uname -r"}});
    }
    if (trim(*value) == "1") {
        return {"kexec", "Kexec loading", CheckStatus::Pass, "Kexec loading is disabled.",
                "Evidence: kernel.kexec_load_disabled=1.", std::nullopt};
    }
    return {"kexec",
            "Kexec loading",
            CheckStatus::Fail,
            "Kexec loading is not disabled.",
            "Observed kernel.kexec_load_disabled=" + trim(*value) + ".",
            "1. Run “Check whether Fedora crash recovery is enabled” below.\n\n"
            "• If it prints enabled, Fedora is using kdump to save diagnostic information after "
            "a kernel crash. To continue with this EAC-oriented setup, run “Turn off Fedora crash "
            "recovery”. This does not delete personal files, but Fedora will no longer save a "
            "kernel crash dump. If you need crash dumps for troubleshooting, leave kdump enabled "
            "and accept that this check will not pass.\n\n"
            "• If it prints disabled or not-found, continue to step 2.\n\n"
            "2. Run “Create the setting that disables kexec”. Enter your administrator password "
            "when sudo asks for it.\n\n"
            "3. Save your work and restart the computer from the desktop menu.\n\n"
            "4. After signing in, run “Verify the setting after restart”. It should print:\n"
            "kernel.kexec_load_disabled = 1\n\n"
            "5. Open this checker again.",
            {{"Check whether Fedora crash recovery is enabled",
              "systemctl is-enabled kdump.service"},
             {"Turn off Fedora crash recovery",
              "sudo systemctl disable --now kdump.service"},
             {"Create the setting that disables kexec",
              "printf '%s\\n' 'kernel.kexec_load_disabled = 1' | sudo tee "
              "/etc/sysctl.d/90-lgl-disable-kexec.conf"},
             {"Verify the setting after restart", "sysctl kernel.kexec_load_disabled"}}};
}

CheckResult check_lsms(const FileReader& files) {
    const auto value = files.read("/sys/kernel/security/lsm");
    if (!value) {
        return unavailable(
            "lsm", "Linux Security Modules", "The active LSM list was unavailable.",
            "Use the commands below to check the kernel security filesystem and look for startup "
            "errors.\n\n"
            "If securityfs is available but the lsm file is missing, use a current standard "
            "Fedora kernel. Running this app as root will not add a missing kernel interface.",
            {{"Confirm that the kernel security filesystem is available",
              "mountpoint /sys/kernel/security"},
             {"Check whether the active-LSM file exists", "ls -l /sys/kernel/security/lsm"},
             {"Show LSM messages from this boot",
              "journalctl --dmesg --boot=0 --no-pager | grep -i lsm"}});
    }
    const auto active = parse_comma_separated_names(*value);
    constexpr std::array expected{"landlock", "lockdown", "yama", "bpf"};
    std::vector<std::string> missing;
    std::ranges::copy_if(expected, std::back_inserter(missing),
                         [&active](const auto* name) { return !active.contains(name); });
    const bool legacy_integrity_name = active.contains("integrity");
    const bool fedora_integrity_names = active.contains("ima") && active.contains("evm");
    if (!legacy_integrity_name && !fedora_integrity_names) {
        missing.emplace_back("integrity support (integrity, or both ima and evm)");
    }
    if (missing.empty()) {
        const std::string integrity_evidence =
            legacy_integrity_name ? "integrity" : "ima and evm";
        return {"lsm", "Linux Security Modules", CheckStatus::Pass,
                "All expected security modules are active.",
                "Active LSMs: " + trim(*value) +
                    "\nIntegrity framework evidence: " + integrity_evidence + ".",
                std::nullopt};
    }
    std::string list;
    for (const auto& name : missing) {
        list += (list.empty() ? "" : ", ") + name;
    }
    const auto command_line = files.read("/proc/cmdline");
    const bool has_lsm_override = command_line && command_line->find("lsm=") != std::string::npos;
    std::string guidance;
    std::vector<SuggestedCommand> commands{
        {"Show the active security modules", "cat /sys/kernel/security/lsm"},
        {"Show the options used to start this boot", "cat /proc/cmdline"},
        {"Show LSM messages from this boot",
         "journalctl --dmesg --boot=0 --no-pager | grep -i lsm"}};
    if (has_lsm_override) {
        guidance =
            "The boot options contain a custom lsm= setting. That setting replaces Fedora's "
            "default LSM list and is the likely reason the named modules are missing.\n\n"
            "If you added the lsm= option yourself, remove that same option from your boot "
            "configuration and restart. Do not copy an LSM list from another distribution. "
            "Because changing boot options incorrectly can prevent startup, this checker does "
            "not guess which command created your custom setting.";
    } else {
        guidance =
            "Your boot options do not contain a custom lsm= setting, so there is nothing to "
            "remove from the boot command line.\n\n"
            "1. Run “Update Fedora” below.\n"
            "2. When the update finishes, save your work and restart from the desktop menu.\n"
            "3. At the boot menu, choose the newest normal Fedora kernel, not a rescue or custom "
            "kernel.\n"
            "4. Open this checker again.\n\n"
            "If the same modules are still missing, keep this result as failed rather than adding "
            "an unverified lsm= option.";
        commands.push_back(
            {"Update Fedora and install the latest standard kernel", "sudo dnf upgrade --refresh"});
    }
    return {"lsm",
            "Linux Security Modules",
            CheckStatus::Fail,
            "One or more expected security modules are missing.",
            "Missing: " + list + ". Active LSMs: " + trim(*value),
            std::move(guidance),
            std::move(commands)};
}

CheckResult check_iommu(const FileReader& files) {
    const auto path = files.resolve("/sys/kernel/iommu_groups");
    std::error_code error;
    if (!std::filesystem::exists(path, error) || error) {
        return unavailable(
            "iommu", "IOMMU", "The IOMMU groups directory was unavailable.",
            "1. Run the log command below. Messages containing DMAR usually indicate Intel "
            "hardware; AMD-Vi indicates AMD hardware.\n"
            "2. Restart into the firmware settings and enable Intel VT-d or AMD IOMMU. This is "
            "separate from ordinary CPU virtualization.\n"
            "3. Boot Fedora and check again.\n\n"
            "Only if firmware is enabled and the IOMMU is still inactive, use the matching "
            "Fedora command below. Choose Intel or AMD, never both, and then restart.",
            {{"Show IOMMU messages from this boot",
              "journalctl --dmesg --boot=0 --no-pager | grep -Ei 'DMAR|IOMMU|AMD-Vi'"},
             {"Enable IOMMU in Fedora on an Intel system",
              "sudo grubby --update-kernel=ALL --args=\"intel_iommu=on\""},
             {"Enable IOMMU in Fedora on an AMD system",
              "sudo grubby --update-kernel=ALL --args=\"amd_iommu=on\""}});
    }
    const bool has_group =
        std::filesystem::directory_iterator(path, error) != std::filesystem::directory_iterator{};
    if (error) {
        return unavailable(
            "iommu", "IOMMU", "The IOMMU groups directory could not be read.",
            "Run the command below as your normal user. If it reports Permission denied, do not "
            "change /sys permissions and do not run this app as root. Restart Fedora and retry; "
            "if the error remains, review system security or mount changes that affect /sys.",
            {{"Try to display the IOMMU groups directory",
              "ls -la /sys/kernel/iommu_groups"}});
    }
    if (has_group) {
        return {"iommu", "IOMMU", CheckStatus::Pass, "Active IOMMU groups were detected.",
                "Evidence: /sys/kernel/iommu_groups contains at least one group.", std::nullopt};
    }
    return {"iommu",
            "IOMMU",
            CheckStatus::Unknown,
            "No active IOMMU groups were detected.",
            "The groups directory exists but is empty; this is not enough evidence to distinguish "
            "disabled IOMMU from unsupported hardware.",
            "The folder exists, but Fedora did not report any IOMMU groups.\n\n"
            "1. Use the commands below to confirm the empty result and check the boot log.\n"
            "2. Restart into firmware settings and enable Intel VT-d or AMD IOMMU if available.\n"
            "3. Restart Fedora and run this checker again.\n\n"
            "If firmware is already enabled, use only the grubby command that matches your CPU, "
            "then restart. Some systems genuinely expose no groups, so this check may remain "
            "unknown.",
            {{"List the IOMMU groups detected by Fedora",
              "find /sys/kernel/iommu_groups -mindepth 1 -maxdepth 1 -type d -printf '%f\\n'"},
             {"Show IOMMU messages from this boot",
              "journalctl --dmesg --boot=0 --no-pager | grep -Ei 'DMAR|IOMMU|AMD-Vi'"},
             {"Enable IOMMU in Fedora on an Intel system",
              "sudo grubby --update-kernel=ALL --args=\"intel_iommu=on\""},
             {"Enable IOMMU in Fedora on an AMD system",
              "sudo grubby --update-kernel=ALL --args=\"amd_iommu=on\""}}};
}

std::vector<CheckResult> run_initial_checks(const FileReader& files) {
    std::vector<CheckResult> results;
    results.reserve(6);
    results.push_back(check_platform(files));
    results.push_back(check_uefi(files));
    results.push_back(check_lockdown(files));
    results.push_back(check_kexec(files));
    results.push_back(check_iommu(files));
    results.push_back(check_lsms(files));
    return results;
}

} // namespace ec

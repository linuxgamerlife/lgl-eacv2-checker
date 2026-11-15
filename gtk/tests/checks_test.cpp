#include "core/check_result.h"
#include "core/checks.h"

#include <glib.h>

#include <filesystem>
#include <fstream>
#include <utility>
#include <vector>

namespace {

class Fixture {
  public:
    Fixture() {
        GError* error = nullptr;
        char* path = g_dir_make_tmp("eac-checks-XXXXXX", &error);
        g_assert_no_error(error);
        g_assert_nonnull(path);
        root_ = path;
        g_free(path);
    }
    ~Fixture() {
        std::error_code error;
        std::filesystem::remove_all(root_, error);
    }
    void write(const std::string& relative, const std::string& contents) const {
        const auto path = root_ / relative;
        std::filesystem::create_directories(path.parent_path());
        std::ofstream{path} << contents;
    }
    [[nodiscard]] ec::FileReader reader() const { return ec::FileReader{root_}; }

  private:
    std::filesystem::path root_;
};

class FakeCommandRunner final : public ec::CommandRunner {
  public:
    explicit FakeCommandRunner(std::vector<ec::CommandOutput> outputs)
        : outputs_(std::move(outputs)) {}

    [[nodiscard]] ec::CommandOutput
    run(const std::vector<std::string>& arguments) const override {
        calls.push_back(arguments);
        g_assert_cmpuint(next_, <, outputs_.size());
        return outputs_[next_++];
    }

    mutable std::vector<std::vector<std::string>> calls;

  private:
    std::vector<ec::CommandOutput> outputs_;
    mutable std::size_t next_{0};
};

void platform_passes() {
    Fixture fixture;
    fixture.write("etc/os-release", "NAME=Fedora Linux\nID=fedora\nVERSION_ID=\"43\"\n");
    const auto result = ec::check_platform(fixture.reader());
    g_assert_cmpint(static_cast<int>(result.status), ==, static_cast<int>(ec::CheckStatus::Pass));
    g_assert_true(result.summary.find("43") != std::string::npos);
}

void malformed_platform_does_not_pass() {
    Fixture fixture;
    fixture.write("etc/os-release", "NAME=Something\n");
    const auto result = ec::check_platform(fixture.reader());
    g_assert_cmpint(static_cast<int>(result.status), !=, static_cast<int>(ec::CheckStatus::Pass));
}

void standard_fedora_kernel_passes() {
    Fixture fixture;
    fixture.write("proc/sys/kernel/osrelease", "6.18.9-200.fc43.x86_64\n");
    fixture.write("usr/lib/modules/6.18.9-200.fc43.x86_64/vmlinuz", "kernel image");
    FakeCommandRunner commands({
        {0,
         "kernel-core|kernel-core-6.18.9-200.fc43.x86_64|Fedora Project\n",
         {},
         {}},
        {0, {}, {}, {}},
    });
    const auto result = ec::check_kernel(fixture.reader(), commands);
    g_assert_cmpint(static_cast<int>(result.status), ==, static_cast<int>(ec::CheckStatus::Pass));
    g_assert_cmpuint(commands.calls.size(), ==, 2U);
    g_assert_true(result.details.find("Fedora Project") != std::string::npos);
    g_assert_true(result.details.find("does not prove") != std::string::npos);
    g_assert_false(result.remediation.has_value());
}

void third_party_kernel_warns() {
    Fixture fixture;
    fixture.write("proc/sys/kernel/osrelease", "6.18.9-custom.x86_64\n");
    fixture.write("usr/lib/modules/6.18.9-custom.x86_64/vmlinuz", "kernel image");
    FakeCommandRunner commands({
        {0, "kernel-custom-core|kernel-custom-core-6.18.9-1.x86_64|Example Vendor\n", {}, {}},
    });
    const auto result = ec::check_kernel(fixture.reader(), commands);
    g_assert_cmpint(static_cast<int>(result.status), ==,
                    static_cast<int>(ec::CheckStatus::Warning));
    g_assert_cmpuint(commands.calls.size(), ==, 1U);
}

void changed_fedora_kernel_fails() {
    Fixture fixture;
    fixture.write("proc/sys/kernel/osrelease", "6.18.9-200.fc43.x86_64\n");
    fixture.write("usr/lib/modules/6.18.9-200.fc43.x86_64/vmlinuz", "kernel image");
    FakeCommandRunner commands({
        {0,
         "kernel-core|kernel-core-6.18.9-200.fc43.x86_64|Fedora Project\n",
         {},
         {}},
        {1, "S.5....T.  /usr/lib/modules/6.18.9-200.fc43.x86_64/vmlinuz\n", {}, {}},
    });
    const auto result = ec::check_kernel(fixture.reader(), commands);
    g_assert_cmpint(static_cast<int>(result.status), ==, static_cast<int>(ec::CheckStatus::Fail));
    g_assert_true(result.details.find("S.5....T.") != std::string::npos);
}

void lockdown_requires_selected_integrity() {
    Fixture fixture;
    fixture.write("sys/kernel/security/lockdown", "none [integrity] confidentiality\n");
    const auto result = ec::check_lockdown(fixture.reader());
    g_assert_cmpint(static_cast<int>(result.status), ==, static_cast<int>(ec::CheckStatus::Pass));
}

void fedora_ima_and_evm_satisfy_integrity_support() {
    Fixture fixture;
    fixture.write("sys/kernel/security/lsm",
                  "lockdown,capability,yama,selinux,bpf,landlock,ipe,ima,evm");
    const auto result = ec::check_lsms(fixture.reader());
    g_assert_cmpint(static_cast<int>(result.status), ==, static_cast<int>(ec::CheckStatus::Pass));
    g_assert_true(result.details.find("ima and evm") != std::string::npos);
}

void lsm_names_must_match_complete_tokens() {
    Fixture fixture;
    fixture.write("sys/kernel/security/lsm",
                  "not-lockdown,capability,yama,selinux,not-bpf,landlock,ima,evm");
    const auto result = ec::check_lsms(fixture.reader());
    g_assert_cmpint(static_cast<int>(result.status), ==, static_cast<int>(ec::CheckStatus::Fail));
}

void unknown_dominates_overall() {
    const std::vector<ec::CheckResult> results{
        {"one", "One", ec::CheckStatus::Pass, {}, {}, std::nullopt},
        {"two", "Two", ec::CheckStatus::Unknown, {}, {}, std::nullopt}};
    g_assert_cmpint(static_cast<int>(ec::overall_status(results)), ==,
                    static_cast<int>(ec::OverallStatus::Unknown));
}

void every_nonpassing_initial_check_has_guidance() {
    Fixture fixture;
    FakeCommandRunner commands({});
    const auto results = ec::run_initial_checks(fixture.reader(), commands);
    g_assert_cmpuint(results.size(), ==, 7U);
    for (const auto& result : results) {
        if (result.status != ec::CheckStatus::Pass) {
            g_assert_true(result.remediation.has_value());
            g_assert_false(result.remediation->empty());
        }
        for (const auto& command : result.commands) {
            g_assert_false(command.description.empty());
            g_assert_false(command.command.empty());
            g_assert_false(command.command.starts_with("$ "));
            g_assert_false(command.command.starts_with("# "));
        }
    }
}

} // namespace

int main(int argc, char** argv) {
    g_test_init(&argc, &argv, nullptr);
    g_test_add_func("/checks/platform-pass", platform_passes);
    g_test_add_func("/checks/platform-malformed", malformed_platform_does_not_pass);
    g_test_add_func("/checks/kernel-fedora", standard_fedora_kernel_passes);
    g_test_add_func("/checks/kernel-third-party", third_party_kernel_warns);
    g_test_add_func("/checks/kernel-changed", changed_fedora_kernel_fails);
    g_test_add_func("/checks/lockdown", lockdown_requires_selected_integrity);
    g_test_add_func("/checks/lsm-fedora-integrity", fedora_ima_and_evm_satisfy_integrity_support);
    g_test_add_func("/checks/lsm-exact-names", lsm_names_must_match_complete_tokens);
    g_test_add_func("/checks/overall-unknown", unknown_dominates_overall);
    g_test_add_func("/checks/nonpassing-guidance", every_nonpassing_initial_check_has_guidance);
    return g_test_run();
}

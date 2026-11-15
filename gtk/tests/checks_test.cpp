#include "core/check_result.h"
#include "core/checks.h"

#include <glib.h>

#include <filesystem>
#include <fstream>

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
    const auto results = ec::run_initial_checks(fixture.reader());
    g_assert_cmpuint(results.size(), ==, 6U);
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
    g_test_add_func("/checks/lockdown", lockdown_requires_selected_integrity);
    g_test_add_func("/checks/lsm-fedora-integrity", fedora_ima_and_evm_satisfy_integrity_support);
    g_test_add_func("/checks/lsm-exact-names", lsm_names_must_match_complete_tokens);
    g_test_add_func("/checks/overall-unknown", unknown_dominates_overall);
    g_test_add_func("/checks/nonpassing-guidance", every_nonpassing_initial_check_has_guidance);
    return g_test_run();
}

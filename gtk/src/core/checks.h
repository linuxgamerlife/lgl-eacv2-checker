#pragma once

#include "core/check_result.h"
#include "core/command_runner.h"
#include "core/file_reader.h"

#include <string>
#include <vector>

namespace ec {

[[nodiscard]] CheckResult check_platform(const FileReader& files);
[[nodiscard]] CheckResult check_kernel(const FileReader& files, const CommandRunner& commands);
[[nodiscard]] CheckResult check_uefi(const FileReader& files);
[[nodiscard]] CheckResult check_lockdown(const FileReader& files);
[[nodiscard]] CheckResult check_kexec(const FileReader& files);
[[nodiscard]] CheckResult check_lsms(const FileReader& files);
[[nodiscard]] CheckResult check_iommu(const FileReader& files);
[[nodiscard]] std::vector<CheckResult> run_initial_checks(const FileReader& files,
                                                          const CommandRunner& commands);
[[nodiscard]] std::vector<CheckResult> run_initial_checks(const FileReader& files);

[[nodiscard]] std::string parse_os_release_value(const std::string& contents,
                                                 const std::string& key);

} // namespace ec

#pragma once

#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace ec {

enum class CheckStatus { Pass, Warning, Fail, Unknown };
enum class OverallStatus { Ready, RestartRequired, FirmwareRequired, NotReady, Unknown };

struct SuggestedCommand {
    std::string description;
    std::string command;
};

struct CheckResult {
    CheckResult(std::string id_value, std::string title_value, CheckStatus status_value,
                std::string summary_value, std::string details_value,
                std::optional<std::string> remediation_value = std::nullopt,
                std::vector<SuggestedCommand> commands_value = {})
        : id(std::move(id_value)), title(std::move(title_value)), status(status_value),
          summary(std::move(summary_value)), details(std::move(details_value)),
          remediation(std::move(remediation_value)), commands(std::move(commands_value)) {}

    std::string id;
    std::string title;
    CheckStatus status{CheckStatus::Unknown};
    std::string summary;
    std::string details;
    std::optional<std::string> remediation;
    std::vector<SuggestedCommand> commands;
};

[[nodiscard]] std::string status_label(CheckStatus status);
[[nodiscard]] OverallStatus overall_status(const std::vector<CheckResult>& results);
[[nodiscard]] std::string overall_label(OverallStatus status);

} // namespace ec

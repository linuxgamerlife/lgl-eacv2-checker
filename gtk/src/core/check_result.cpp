#include "core/check_result.h"

#include <algorithm>

namespace ec {

std::string status_label(const CheckStatus status) {
    switch (status) {
    case CheckStatus::Pass:
        return "PASS";
    case CheckStatus::Warning:
        return "WARNING";
    case CheckStatus::Fail:
        return "FAIL";
    case CheckStatus::Unknown:
        return "UNKNOWN";
    }
    return "UNKNOWN";
}

OverallStatus overall_status(const std::vector<CheckResult>& results) {
    if (results.empty() || std::ranges::any_of(results, [](const auto& result) {
            return result.status == CheckStatus::Unknown;
        })) {
        return OverallStatus::Unknown;
    }
    if (std::ranges::any_of(results, [](const auto& result) {
            return result.status == CheckStatus::Fail;
        })) {
        return OverallStatus::NotReady;
    }
    if (std::ranges::any_of(results, [](const auto& result) {
            return result.status == CheckStatus::Warning;
        })) {
        return OverallStatus::RestartRequired;
    }
    return OverallStatus::Ready;
}

std::string overall_label(const OverallStatus status) {
    switch (status) {
    case OverallStatus::Ready:
        return "Ready";
    case OverallStatus::RestartRequired:
        return "Restart or boot configuration required";
    case OverallStatus::FirmwareRequired:
        return "Firmware changes required";
    case OverallStatus::NotReady:
        return "Not ready";
    case OverallStatus::Unknown:
        return "Unable to determine";
    }
    return "Unable to determine";
}

} // namespace ec

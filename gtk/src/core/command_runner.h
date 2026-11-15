#pragma once

#include <string>
#include <vector>

namespace ec {

struct CommandOutput {
    int exit_code{-1};
    std::string standard_output;
    std::string standard_error;
    std::string launch_error;
};

class CommandRunner {
  public:
    virtual ~CommandRunner() = default;
    [[nodiscard]] virtual CommandOutput run(const std::vector<std::string>& arguments) const = 0;
};

class SystemCommandRunner final : public CommandRunner {
  public:
    [[nodiscard]] CommandOutput run(const std::vector<std::string>& arguments) const override;
};

} // namespace ec

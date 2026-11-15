#pragma once

#include <filesystem>

namespace ec {

class Settings {
  public:
    Settings();
    explicit Settings(std::filesystem::path path);

    [[nodiscard]] bool system_title_bar() const;
    void set_system_title_bar(bool enabled);

  private:
    std::filesystem::path path_;
};

} // namespace ec

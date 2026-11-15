#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace ec {

class FileReader {
  public:
    explicit FileReader(std::filesystem::path root = "/");

    [[nodiscard]] std::optional<std::string> read(const std::filesystem::path& path) const;
    [[nodiscard]] bool exists(const std::filesystem::path& path) const;
    [[nodiscard]] std::filesystem::path resolve(const std::filesystem::path& path) const;

  private:
    std::filesystem::path root_;
    static constexpr std::size_t max_read_size_ = 256U * 1024U;
};

} // namespace ec

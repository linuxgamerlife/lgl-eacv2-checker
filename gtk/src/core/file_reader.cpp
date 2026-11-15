#include "core/file_reader.h"

#include <fstream>
#include <sstream>

namespace ec {

FileReader::FileReader(std::filesystem::path root) : root_(std::move(root)) {}

std::filesystem::path FileReader::resolve(const std::filesystem::path& path) const {
    return root_ / path.relative_path();
}

std::optional<std::string> FileReader::read(const std::filesystem::path& path) const {
    std::ifstream input(resolve(path), std::ios::binary);
    if (!input) {
        return std::nullopt;
    }
    std::ostringstream output;
    char buffer[4096]{};
    std::size_t total = 0;
    while (input && total <= max_read_size_) {
        input.read(buffer, static_cast<std::streamsize>(sizeof(buffer)));
        const auto count = input.gcount();
        if (count > 0) {
            total += static_cast<std::size_t>(count);
            if (total > max_read_size_) {
                return std::nullopt;
            }
            output.write(buffer, count);
        }
    }
    return input.bad() ? std::nullopt : std::optional<std::string>{output.str()};
}

bool FileReader::exists(const std::filesystem::path& path) const {
    std::error_code error;
    const bool result = std::filesystem::exists(resolve(path), error);
    return !error && result;
}

} // namespace ec

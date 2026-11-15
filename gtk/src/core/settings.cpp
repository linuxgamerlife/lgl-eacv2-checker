#include "core/settings.h"

#include <glibmm.h>

namespace ec {
namespace {

std::filesystem::path default_path() {
    return std::filesystem::path{Glib::get_user_config_dir()} / "lgl-eacv2-checker" /
           "settings.ini";
}

} // namespace

Settings::Settings() : Settings(default_path()) {}
Settings::Settings(std::filesystem::path path) : path_(std::move(path)) {}

bool Settings::system_title_bar() const {
    try {
        auto file = Glib::KeyFile::create();
        file->load_from_file(path_.string());
        return file->get_boolean("window", "systemTitleBar");
    } catch (const Glib::Error&) {
        return false;
    }
}

void Settings::set_system_title_bar(const bool enabled) {
    try {
        std::error_code error;
        std::filesystem::create_directories(path_.parent_path(), error);
        if (error) {
            return;
        }
        std::filesystem::permissions(path_.parent_path(), std::filesystem::perms::owner_all,
                                     std::filesystem::perm_options::replace, error);
        if (error) {
            return;
        }
        auto file = Glib::KeyFile::create();
        if (std::filesystem::exists(path_)) {
            try {
                file->load_from_file(path_.string());
            } catch (const Glib::Error&) {
                file = Glib::KeyFile::create();
            }
        }
        file->set_boolean("window", "systemTitleBar", enabled);
        file->save_to_file(path_.string());
    } catch (const Glib::Error&) {
        // Preferences are non-critical and retain their safe default on failure.
    }
}

} // namespace ec

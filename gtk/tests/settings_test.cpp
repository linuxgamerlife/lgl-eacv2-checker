#include "core/settings.h"

#include <glib.h>

#include <filesystem>

namespace {

void settings_round_trip() {
    GError* error = nullptr;
    char* temporary = g_dir_make_tmp("eac-settings-XXXXXX", &error);
    g_assert_no_error(error);
    g_assert_nonnull(temporary);
    const std::filesystem::path root{temporary};
    g_free(temporary);

    const auto path = root / "nested" / "settings.ini";
    ec::Settings settings{path};
    g_assert_false(settings.system_title_bar());
    settings.set_system_title_bar(true);
    g_assert_true(settings.system_title_bar());

    std::error_code cleanup_error;
    std::filesystem::remove_all(root, cleanup_error);
}

} // namespace

int main(int argc, char** argv) {
    g_test_init(&argc, &argv, nullptr);
    g_test_add_func("/settings/round-trip", settings_round_trip);
    return g_test_run();
}

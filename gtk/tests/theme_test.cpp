#include "core/theme.h"

#include <glib.h>

namespace {

void luminance_is_bounded() {
    g_assert_true(ec::is_dark_luminance(0.0, 0.0, 0.0));
    g_assert_false(ec::is_dark_luminance(1.0, 1.0, 1.0));
    g_assert_true(ec::is_dark_luminance(-10.0, -1.0, 0.0));
    g_assert_false(ec::is_dark_luminance(10.0, 10.0, 10.0));
}

} // namespace

int main(int argc, char** argv) {
    g_test_init(&argc, &argv, nullptr);
    g_test_add_func("/theme/luminance", luminance_is_bounded);
    return g_test_run();
}

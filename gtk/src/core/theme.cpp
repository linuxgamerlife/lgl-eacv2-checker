#include "core/theme.h"

#include <algorithm>

namespace ec {

bool is_dark_luminance(const double red, const double green, const double blue) {
    const auto clamp = [](const double value) { return std::clamp(value, 0.0, 1.0); };
    const double luminance =
        (0.2126 * clamp(red)) + (0.7152 * clamp(green)) + (0.0722 * clamp(blue));
    return luminance < 0.5;
}

} // namespace ec

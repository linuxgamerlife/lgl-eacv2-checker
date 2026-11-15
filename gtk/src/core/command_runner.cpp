#include "core/command_runner.h"

#include <gio/gio.h>

#include <algorithm>
#include <cstddef>
#include <iterator>

namespace ec {
namespace {

constexpr std::size_t max_output_size = std::size_t{256} * 1024U;

std::string bounded(const gchar* value) {
    if (value == nullptr) {
        return {};
    }
    std::string output{value};
    if (output.size() > max_output_size) {
        output.resize(max_output_size);
        output += "\n[output truncated]";
    }
    return output;
}

} // namespace

CommandOutput SystemCommandRunner::run(const std::vector<std::string>& arguments) const {
    CommandOutput result;
    if (arguments.empty()) {
        result.launch_error = "No command was specified.";
        return result;
    }

    std::vector<const gchar*> argv;
    argv.reserve(arguments.size() + 1U);
    std::ranges::transform(arguments, std::back_inserter(argv),
                           [](const auto& argument) { return argument.c_str(); });
    argv.push_back(nullptr);

    GError* error = nullptr;
    GSubprocess* process =
        g_subprocess_newv(argv.data(),
                          static_cast<GSubprocessFlags>(G_SUBPROCESS_FLAGS_STDOUT_PIPE |
                                                        G_SUBPROCESS_FLAGS_STDERR_PIPE),
                          &error);
    if (process == nullptr) {
        result.launch_error = error == nullptr ? "Unable to start the command." : error->message;
        g_clear_error(&error);
        return result;
    }

    gchar* standard_output = nullptr;
    gchar* standard_error = nullptr;
    const gboolean completed = g_subprocess_communicate_utf8(
        process, nullptr, nullptr, &standard_output, &standard_error, &error);
    result.standard_output = bounded(standard_output);
    result.standard_error = bounded(standard_error);
    if (completed != 0 && g_subprocess_get_if_exited(process) != 0) {
        result.exit_code = g_subprocess_get_exit_status(process);
    } else if (error != nullptr) {
        result.launch_error = error->message;
    }

    g_free(standard_output);
    g_free(standard_error);
    g_clear_error(&error);
    g_object_unref(process);
    return result;
}

} // namespace ec

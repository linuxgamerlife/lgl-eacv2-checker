#include "ui/theme_watcher.h"

#include <adwaita.h>

namespace ec {

void install_application_stylesheet() {
    adw_init();
    auto provider = Gtk::CssProvider::create();
    provider->load_from_resource("/com/linuxgamerlife/ec/style.css");
    auto display = Gdk::Display::get_default();
    if (display) {
        Gtk::StyleContext::add_provider_for_display(
            display, provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
}

ThemeWatcher::ThemeWatcher(Gtk::Window& window) : window_(window) {
    auto* manager = adw_style_manager_get_default();
    dark_handler_ = g_signal_connect(
        manager, "notify::dark",
        G_CALLBACK(+[](GObject*, GParamSpec*, gpointer data) {
            static_cast<ThemeWatcher*>(data)->update_dark_class();
        }),
        this);
    update_dark_class();
}

ThemeWatcher::~ThemeWatcher() {
    if (dark_handler_ != 0U) {
        g_signal_handler_disconnect(adw_style_manager_get_default(), dark_handler_);
    }
}

void ThemeWatcher::update_dark_class() {
    const bool dark = adw_style_manager_get_dark(adw_style_manager_get_default());
    if (dark) {
        window_.add_css_class("dark");
    } else {
        window_.remove_css_class("dark");
    }
}

} // namespace ec

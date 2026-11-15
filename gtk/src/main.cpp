#include "ui/theme_watcher.h"
#include "ui/window.h"

#include <gtkmm.h>

#include <memory>

int main(int argc, char* argv[]) {
    auto application =
        Gtk::Application::create("com.linuxgamerlife.lgl-eacv2-checker");
    std::unique_ptr<ec::Window> window;

    application->signal_startup().connect([application] {
        Glib::set_application_name("LGL EAC v2 Checker");
        Gtk::Window::set_default_icon_name("lgl-eacv2-checker");
        ec::install_application_stylesheet();
        application->add_action("quit", [application] {
            if (auto active = application->get_active_window()) {
                active->set_visible(false);
            }
            application->quit();
        });
        application->set_accel_for_action("app.quit", "<Primary>q");
    });
    application->signal_activate().connect([&application, &window] {
        if (!window) {
            window = std::make_unique<ec::Window>(application);
        }
        window->present();
    });

    const int result = application->run(argc, argv);
    window.reset();
    return result;
}

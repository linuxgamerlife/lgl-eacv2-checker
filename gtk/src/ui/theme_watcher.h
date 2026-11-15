#pragma once

#include <gtkmm.h>

namespace ec {

void install_application_stylesheet();

class ThemeWatcher final {
  public:
    explicit ThemeWatcher(Gtk::Window& window);
    ~ThemeWatcher();

  private:
    void update_dark_class();

    Gtk::Window& window_;
    gulong dark_handler_{0};
};

} // namespace ec

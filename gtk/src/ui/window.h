#pragma once

#include "core/check_result.h"
#include "core/settings.h"
#include "ui/theme_watcher.h"

#include <gtkmm.h>

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace ec {

class Window final : public Gtk::ApplicationWindow {
  public:
    explicit Window(const Glib::RefPtr<Gtk::Application>& application);
    ~Window() override;

  private:
    void build_header();
    void install_actions();
    void run_checks();
    void receive_results();
    void render_results(const std::vector<CheckResult>& results);
    void set_system_title_bar(bool enabled, bool runtime);
    void show_startup_warning();
    void show_about();

    Glib::RefPtr<Gtk::Application> application_;
    Settings settings_;
    Gtk::HeaderBar header_;
    Gtk::Box content_{Gtk::Orientation::VERTICAL};
    Gtk::Box body_{Gtk::Orientation::VERTICAL, 12};
    Gtk::Box result_card_{Gtk::Orientation::VERTICAL, 6};
    Gtk::Label section_label_{"OVERALL READINESS"};
    Gtk::Label overall_;
    Gtk::Label disclaimer_;
    Gtk::ScrolledWindow scroller_;
    Gtk::Box checks_card_{Gtk::Orientation::VERTICAL};
    Gtk::Box checks_list_{Gtk::Orientation::VERTICAL};
    Gtk::Button rerun_{"Run checks again"};
    Gtk::MenuButton menu_button_;
    std::unique_ptr<Gtk::AboutDialog> about_;
    std::unique_ptr<ThemeWatcher> theme_watcher_;
    Glib::RefPtr<Gtk::AlertDialog> startup_warning_;
    Glib::RefPtr<Gio::Cancellable> startup_warning_cancellable_;
    sigc::connection startup_warning_idle_;

    Glib::Dispatcher dispatcher_;
    std::mutex results_mutex_;
    std::vector<CheckResult> pending_results_;
    std::jthread worker_;
    std::atomic<std::uint64_t> generation_{0};
    std::uint64_t pending_generation_{0};
};

} // namespace ec

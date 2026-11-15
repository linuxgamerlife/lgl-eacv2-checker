#include "ui/window.h"

#include "core/checks.h"
#include "ui/check_row.h"

#include <utility>

namespace ec {

Window::Window(const Glib::RefPtr<Gtk::Application>& application) : application_(application) {
    set_application(application_);
    set_title("LGL EAC v2 Checker");
    set_default_size(700, 680);
    set_size_request(420, 480);

    build_header();
    install_actions();

    section_label_.add_css_class("section-label");
    section_label_.set_xalign(0.0F);
    overall_.set_xalign(0.0F);
    overall_.set_wrap(true);
    overall_.add_css_class("title-2");
    disclaimer_.set_text("This checks detectable prerequisites. It cannot confirm that a game or "
                         "publisher supports this system.");
    disclaimer_.set_xalign(0.0F);
    disclaimer_.set_wrap(true);
    disclaimer_.add_css_class("muted");
    disclaimer_.add_css_class("small");
    result_card_.add_css_class("ec-card");
    result_card_.append(section_label_);
    result_card_.append(overall_);
    result_card_.append(disclaimer_);

    checks_card_.add_css_class("ec-card");
    auto* checks_label = Gtk::make_managed<Gtk::Label>("SYSTEM CHECKS");
    checks_label->add_css_class("section-label");
    checks_label->set_xalign(0.0F);
    checks_card_.append(*checks_label);
    checks_card_.append(checks_list_);

    rerun_.add_css_class("ec-primary");
    rerun_.add_css_class("suggested-action");
    rerun_.set_action_name("win.run-checks");
    rerun_.set_tooltip_text("Run all system checks again (Ctrl+R)");
    rerun_.set_halign(Gtk::Align::END);

    body_.add_css_class("body");
    body_.append(result_card_);
    body_.append(checks_card_);
    body_.append(rerun_);
    scroller_.set_child(body_);
    scroller_.set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
    scroller_.set_vexpand(true);
    content_.append(scroller_);
    set_child(content_);

    dispatcher_.connect(sigc::mem_fun(*this, &Window::receive_results));
    theme_watcher_ = std::make_unique<ThemeWatcher>(*this);
    set_system_title_bar(settings_.system_title_bar(), false);
    overall_.set_text("Waiting for confirmation…");
    rerun_.set_sensitive(false);
    startup_warning_idle_ = Glib::signal_idle().connect([this] {
        show_startup_warning();
        return false;
    });
}

Window::~Window() {
    startup_warning_idle_.disconnect();
    if (startup_warning_cancellable_) {
        startup_warning_cancellable_->cancel();
    }
    ++generation_;
    if (worker_.joinable()) {
        worker_.request_stop();
        worker_.join();
    }
}

void Window::build_header() {
    auto* brand = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 10);
    brand->set_margin_end(12);
    auto* icon = Gtk::make_managed<Gtk::Image>();
    icon->set_from_resource("/com/linuxgamerlife/ec/lgl-eacv2-checker.png");
    icon->set_pixel_size(28);
    icon->add_css_class("brand-icon");
    auto* wordmark = Gtk::make_managed<Gtk::Label>("LGL EAC V2 CHECKER");
    wordmark->add_css_class("wordmark");
    brand->append(*icon);
    brand->append(*wordmark);
    header_.pack_start(*brand);
    auto* empty_title = Gtk::make_managed<Gtk::Box>();
    header_.set_title_widget(*empty_title);

    auto menu = Gio::Menu::create();
    auto view_section = Gio::Menu::create();
    view_section->append("Use System Title Bar", "win.system-title-bar");
    menu->append_section(view_section);
    auto info_section = Gio::Menu::create();
    info_section->append("About LGL EAC v2 Checker", "win.about");
    menu->append_section(info_section);
    auto quit_section = Gio::Menu::create();
    quit_section->append("Quit", "app.quit");
    menu->append_section(quit_section);
    menu_button_.set_icon_name("open-menu-symbolic");
    menu_button_.set_tooltip_text("Main menu");
    menu_button_.set_primary(true);
    menu_button_.set_menu_model(menu);
    header_.pack_end(menu_button_);
}

void Window::install_actions() {
    auto run = add_action("run-checks", sigc::mem_fun(*this, &Window::run_checks));
    application_->set_accel_for_action("win.run-checks", "<Primary>r");
    (void)run;

    add_action("about", sigc::mem_fun(*this, &Window::show_about));
    auto title_action =
        Gio::SimpleAction::create_bool("system-title-bar", settings_.system_title_bar());
    title_action->signal_change_state().connect([this, title_action](const Glib::VariantBase& value) {
        const bool enabled = Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(value).get();
        title_action->set_state(value);
        settings_.set_system_title_bar(enabled);
        set_system_title_bar(enabled, true);
    });
    add_action(title_action);
    application_->set_accel_for_action("win.system-title-bar", "<Primary><Shift>m");
}

void Window::run_checks() {
    const auto request = ++generation_;
    overall_.set_text("Checking this boot…");
    rerun_.set_sensitive(false);
    if (worker_.joinable()) {
        worker_.request_stop();
        worker_.join();
    }
    worker_ = std::jthread([this, request](const std::stop_token stop) {
        auto results = run_initial_checks(FileReader{});
        if (stop.stop_requested() || request != generation_.load()) {
            return;
        }
        {
            const std::lock_guard lock{results_mutex_};
            pending_results_ = std::move(results);
            pending_generation_ = request;
        }
        dispatcher_.emit();
    });
}

void Window::receive_results() {
    std::vector<CheckResult> results;
    {
        const std::lock_guard lock{results_mutex_};
        if (pending_generation_ != generation_.load()) {
            return;
        }
        results = pending_results_;
    }
    render_results(results);
    rerun_.set_sensitive(true);
}

void Window::render_results(const std::vector<CheckResult>& results) {
    while (auto* child = checks_list_.get_first_child()) {
        checks_list_.remove(*child);
    }
    for (const auto& result : results) {
        checks_list_.append(*Gtk::make_managed<CheckRow>(result));
    }
    overall_.set_text(overall_label(overall_status(results)));
}

void Window::set_system_title_bar(const bool enabled, const bool runtime) {
    if (runtime) {
        application_->hold();
        set_visible(false);
        Gtk::Widget::unrealize();
    }
    if (enabled) {
        unset_titlebar();
        header_.set_show_title_buttons(false);
        content_.prepend(header_);
    } else {
        if (header_.get_parent() == &content_) {
            content_.remove(header_);
        }
        header_.set_show_title_buttons(true);
        set_titlebar(header_);
    }
    if (runtime) {
        set_application(application_);
        present();
        scroller_.grab_focus();
        application_->release();
    }
}

void Window::show_startup_warning() {
    startup_warning_ = Gtk::AlertDialog::create("Beta software: read before continuing");
    startup_warning_->set_detail(
        "This checker is in beta. Its checks are based on the CachyOS Easy Anti-Cheat "
        "documentation and have been adapted for Fedora. No Easy Anti-Cheat Linux v2 games are "
        "currently available to test these results against, so the checker may be incomplete or "
        "wrong.\n\n"
        "Some suggested changes affect firmware, boot settings, the kernel, and system security. "
        "A mistake may stop Fedora from starting. Continue only if you are comfortable recovering "
        "the system from a TTY or rescue environment. If you are not comfortable doing that, and "
        "would not be comfortable using an LLM through a tool such as OpenCode to help diagnose "
        "a recovery problem, choose Exit now.\n\n"
        "This software is provided under the MIT licence without warranty. By choosing Continue "
        "at My Own Risk and using this application, you acknowledge that you are responsible for "
        "any changes you make and that you use the application at your own risk.");
    startup_warning_->set_buttons({"Exit", "Continue at My Own Risk"});
    startup_warning_->set_cancel_button(0);
    startup_warning_->set_default_button(0);
    startup_warning_->set_modal(true);
    startup_warning_cancellable_ = Gio::Cancellable::create();
    startup_warning_->choose(
        *this,
        [this](const Glib::RefPtr<Gio::AsyncResult>& result) {
            int response = 0;
            try {
                response = startup_warning_->choose_finish(result);
            } catch (const Glib::Error&) {
                response = 0;
            }
            startup_warning_.reset();
            startup_warning_cancellable_.reset();
            if (response == 1) {
                run_checks();
                return;
            }
            set_visible(false);
            application_->quit();
        },
        startup_warning_cancellable_);
}

void Window::show_about() {
    if (!about_) {
        about_ = std::make_unique<Gtk::AboutDialog>();
        about_->set_program_name("LGL EAC v2 Checker");
        about_->set_version(EAC_VERSION);
        about_->set_comments("Checks detectable Fedora boot-security prerequisites for Easy "
                             "Anti-Cheat Linux v2.\n\nBuilt with GTK 4.");
        about_->set_website("https://github.com/linuxgamerlife/lgl-eacv2-checker");
        about_->set_website_label("Project page and issue tracker");
        about_->set_authors({"LinuxGamerLife https://www.youtube.com/@linuxgamerlife"});
        about_->set_license_type(Gtk::License::MIT_X11);
        about_->set_transient_for(*this);
        about_->set_modal(true);
        about_->set_hide_on_close(true);
    }
    about_->present();
}

} // namespace ec

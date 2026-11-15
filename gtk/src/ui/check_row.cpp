#include "ui/check_row.h"

namespace ec {

CheckRow::CheckRow(const CheckResult& result) : Gtk::Box(Gtk::Orientation::VERTICAL, 6) {
    set_margin_start(12);
    set_margin_end(12);
    set_margin_top(10);
    set_margin_bottom(10);

    title_.set_text(result.title);
    title_.set_xalign(0.0F);
    summary_.set_text(result.summary);
    summary_.set_xalign(0.0F);
    summary_.set_wrap(true);
    summary_.add_css_class("muted");
    summary_.add_css_class("small");
    text_.set_hexpand(true);
    text_.append(title_);
    text_.append(summary_);

    status_.set_text(status_label(result.status));
    status_.add_css_class("ec-chip");
    switch (result.status) {
    case CheckStatus::Pass:
        status_.add_css_class("positive");
        break;
    case CheckStatus::Warning:
    case CheckStatus::Unknown:
        status_.add_css_class("neutral");
        break;
    case CheckStatus::Fail:
        status_.add_css_class("negative");
        break;
    }
    status_.set_valign(Gtk::Align::CENTER);

    heading_.append(text_);
    heading_.append(status_);
    append(heading_);

    detail_text_.set_text(result.details);
    detail_text_.set_xalign(0.0F);
    detail_text_.set_wrap(true);
    detail_text_.set_selectable(true);
    detail_text_.add_css_class("check-details");
    detail_text_.add_css_class("small");
    details_box_.append(detail_text_);

    if (result.remediation) {
        guidance_text_.set_text("What to do\n" + *result.remediation);
        guidance_text_.set_xalign(0.0F);
        guidance_text_.set_wrap(true);
        guidance_text_.set_selectable(true);
        guidance_text_.add_css_class("small");
        details_box_.append(guidance_text_);
    }

    if (!result.commands.empty()) {
        auto* command_heading = Gtk::make_managed<Gtk::Label>("COMMANDS");
        command_heading->set_xalign(0.0F);
        command_heading->add_css_class("section-label");
        details_box_.append(*command_heading);

        for (const auto& command : result.commands) {
            auto* description = Gtk::make_managed<Gtk::Label>(command.description);
            description->set_xalign(0.0F);
            description->set_wrap(true);
            description->add_css_class("small");
            details_box_.append(*description);

            auto* row = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 8);
            auto* command_text = Gtk::make_managed<Gtk::Label>(command.command);
            command_text->set_xalign(0.0F);
            command_text->set_wrap(true);
            command_text->set_selectable(true);
            command_text->set_hexpand(true);
            command_text->add_css_class("mono");
            command_text->add_css_class("command");

            auto* copy = Gtk::make_managed<Gtk::Button>();
            copy->set_icon_name("edit-copy-symbolic");
            copy->set_tooltip_text("Copy command");
            copy->set_valign(Gtk::Align::CENTER);
            copy->signal_clicked().connect([text = command.command] {
                if (const auto display = Gdk::Display::get_default()) {
                    display->get_clipboard()->set_text(text);
                }
            });
            row->append(*command_text);
            row->append(*copy);
            details_box_.append(*row);
        }
    }

    details_.set_label("Technical details");
    details_.set_child(details_box_);
    append(details_);
}

} // namespace ec

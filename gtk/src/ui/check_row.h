#pragma once

#include "core/check_result.h"

#include <gtkmm.h>

namespace ec {

class CheckRow final : public Gtk::Box {
  public:
    explicit CheckRow(const CheckResult& result);

  private:
    Gtk::Box heading_{Gtk::Orientation::HORIZONTAL, 8};
    Gtk::Box text_{Gtk::Orientation::VERTICAL, 4};
    Gtk::Label title_;
    Gtk::Label summary_;
    Gtk::Label status_;
    Gtk::Expander details_;
    Gtk::Box details_box_{Gtk::Orientation::VERTICAL, 8};
    Gtk::Label detail_text_;
    Gtk::Label guidance_text_;
};

} // namespace ec

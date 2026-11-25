// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>
#include <string>

#include "base/signal.h"
#include "ui/view.h"

namespace term {
class TerminalView;
}

namespace properdesktop::ui {
class HeaderButton;
}

class FindField;

// Floats over a terminal while searching its output and history. Enter goes to
// the next older match, Shift+Enter to the next newer one. Drawn in the
// terminal's colours; the field itself has no frame, only the bar does.
class FindBar : public properdesktop::ui::View
{
public:
    explicit FindBar(term::TerminalView* terminal);
    ~FindBar() override;

    // The colour scheme's own colours.
    void setColors(properdesktop::ui::Color background, properdesktop::ui::Color text, bool darkMode,
                   properdesktop::ui::Color error, properdesktop::ui::Color accent);

    void open(const std::string& text);
    void close();
    void retranslate();

    std::function<void()> onClosed;

    properdesktop::ui::SizeF sizeHint() const override { return {320, 40}; }
    bool acceptsPointer() const override { return true; }

protected:
    void layout() override;
    void paint(properdesktop::ui::Painter& painter) override;

private:
    void search(bool forwards, bool fromSelection);
    void updateColors();

    term::TerminalView* m_terminal;
    FindField* m_field;
    properdesktop::ui::HeaderButton* m_previous;
    properdesktop::ui::HeaderButton* m_next;
    properdesktop::ui::HeaderButton* m_close;
    properdesktop::base::Subscription m_findResult;
    bool m_notFound = false;
    properdesktop::ui::Color m_background = properdesktop::ui::Color::rgb(0x1A1B26);
    properdesktop::ui::Color m_text = properdesktop::ui::Color::rgb(0xC0CAF5);
    properdesktop::ui::Color m_error = properdesktop::ui::Color::rgb(0xF7768E);
    properdesktop::ui::Color m_accent = properdesktop::ui::Color::rgb(0x7AA2F7);
    bool m_darkMode = true;
};

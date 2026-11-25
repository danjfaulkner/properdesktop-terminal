// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <ghostty/vt.h>

#include "base/signal.h"
#include "base/timer.h"
#include "ui/scroller.h"
#include "ui/view.h"

#include "boxdrawing.h"
#include "cellfont.h"
#include "terminal.h"

namespace term {

class Search;
class Session;

// Shows a session's terminal from a libghostty-vt render state, cells on whole
// device pixels; takes the keys, the pointer, the wheel and pastes for it; and
// keeps the selection, the scrollback position and the cursor.
class TerminalView : public properdesktop::ui::View, public properdesktop::ui::Scroller::Target
{
public:
    TerminalView();
    ~TerminalView() override;

    void setSession(Session* session);
    Session* session() const { return m_session; }

    void setColorScheme(const std::string& name);
    void setTerminalFont(const std::string& family, double pointSize);
    void setBlinkingCursor(bool blinking);
    bool isCursorBlinking() const { return m_cursorBlinkTimer.isActive(); }
    // 0 block, 1 underline, 2 I-beam.
    void setCursorShape(int shape);
    // The bell is passed on through notifyBell, or ignored.
    void setNotifiesBell(bool notifies) { m_notifiesBell = notifies; }
    void setConfirmMultilinePaste(bool confirm) { m_confirmMultilinePaste = confirm; }
    // The default background over the window's; 0 leaves it to the window.
    void setBackgroundOpacity(double opacity);
    // What links under the pointer are underlined in.
    void setLinkColor(properdesktop::ui::Color color);

    int lines() const { return m_lines; }
    int columns() const { return m_columns; }
    const CellMetrics& cellMetrics() const { return m_font.metrics(); }
    // The characters drawn on a line of the viewport, blank cells as spaces,
    // trailing ones dropped; empty for a line the view holds nothing for.
    std::u32string drawnText(int line) const;

    // The first line shown, from 0 at the top of the scrollback.
    int scrollbarValue() const { return int(m_scrollbar.offset); }
    int scrollbarMinimum() const { return 0; }
    int scrollbarMaximum() const;
    void setScrollbarValue(int value);

    bool hasSelection() const;
    std::string selectedText() const;
    void selectAll();
    void clearSelection();
    void copyClipboard();
    void pasteClipboard();
    void pasteSelection();
    // Text for the program, as a paste.
    void pasteText(const std::string& text);
    // The paste multilinePasteRequested held back.
    void confirmPaste();
    bool isSelectedAt(double x, double y) const;
    bool selectWordAt(double x, double y);

    // The link under the point, as it opens; empty for none.
    std::string linkAt(double x, double y) const;

    void find(const std::string& text, bool forwards, bool fromSelection, bool caseSensitive);
    void clearFind();

    properdesktop::base::Signal<> notifyBell;
    properdesktop::base::Signal<const std::string&> multilinePasteRequested;
    properdesktop::base::Signal<> scrollbarParamsChanged;
    properdesktop::base::Signal<> scrollbarValueChanged;
    properdesktop::base::Signal<bool> findResult;
    properdesktop::base::Signal<> sizeChanged;

    // The window's shortcuts see keys first; a right click, or Shift+right
    // click under a program that takes the mouse, asks for the menu; a
    // Control+click on a link asks to open it.
    std::function<bool(const properdesktop::ui::KeyEvent&)> onShortcut;
    std::function<void(double x, double y)> onContextMenu;
    std::function<void(const std::string& url)> onLinkActivated;

    bool acceptsFocus() const override { return true; }
    bool acceptsPointer() const override { return true; }

protected:
    void layout() override;
    void paint(properdesktop::ui::Painter& painter) override;
    void pointerPress(const properdesktop::ui::PointerEvent& event) override;
    void pointerMove(const properdesktop::ui::PointerEvent& event) override;
    void pointerRelease(const properdesktop::ui::PointerEvent& event) override;
    void hoverMove(const properdesktop::ui::PointerEvent& event) override;
    void hoverChanged() override;
    void focusIn(properdesktop::ui::FocusReason reason) override;
    void focusOut() override;
    bool keyPress(const properdesktop::ui::KeyEvent& event) override;
    bool keyRelease(const properdesktop::ui::KeyEvent& event) override;
    bool scroll(const properdesktop::ui::ScrollEvent& event) override;
    bool acceptsTextInput() const override { return true; }
    properdesktop::ui::TextInputState textInputState() const override;
    void textInput(const properdesktop::ui::TextInputEvent& event) override;

private:
    // Scroller::Target: the scrollback in logical pixels.
    properdesktop::ui::Scroller::Axis horizontalAxis() const override;
    properdesktop::ui::Scroller::Axis verticalAxis() const override;
    properdesktop::ui::PointF position() const override;
    void applyPosition(properdesktop::ui::PointF position) override;

    Terminal* terminal() const;
    std::string resolvedPath(const std::string& path) const;
    double scale() const;
    double cellHeightLogical() const;
    // The grid's top left in device pixels from the view's.
    int gridLeft() const;
    int gridTop() const;
    // The viewport cell under a point of the view.
    std::pair<int, int> cellAt(properdesktop::ui::PointF point) const;
    properdesktop::ui::RectF rowRect(int line, int count = 1) const;
    // At the window's scale, or another a frame is drawn at.
    void updateGeometry(double scale = 0);

    // terminalviewpaint.cpp
    void scheduleUpdate();
    void updateRenderState();
    void paintRows(properdesktop::ui::Painter& painter);
    void paintPreedit(properdesktop::ui::Painter& painter);

    // terminalviewinput.cpp
    bool sendKey(const properdesktop::ui::KeyEvent& event);
    void sendKeyRelease(const properdesktop::ui::KeyEvent& event);
    void releaseHeldKey();
    bool handleScrollKey(const properdesktop::ui::KeyEvent& event);
    bool sendMouse(MouseInput::Action action, int button, properdesktop::ui::PointF position, uint32_t modifiers);
    bool programTakesMouse(uint32_t modifiers) const;
    // Without `apply` the gesture only counts the press, leaving the selection.
    void selectionPress(const properdesktop::ui::PointerEvent& event, bool apply = true);
    // The link under the position, as a selection.
    bool linkSelection(properdesktop::ui::PointF position, GhosttySelection* selection) const;
    void selectionDrag(properdesktop::ui::PointF position);
    void selectionRelease(properdesktop::ui::PointF position);
    void startTextDrag();
    void autoscroll();
    GhosttyGridRef refAt(properdesktop::ui::PointF position, bool* ok) const;

    void scrollLines(int lines);
    void scrollChanged(bool byOutput = false);
    void syncScroll();
    void setScrollOffset(double offset);
    void applyCursorStyle();
    void restartCursorBlink();
    void blinkCursor();
    void selectionChanged();
    void sendPaste(const std::string& text, bool typed);
    bool showsLinks(uint32_t modifiers) const;
    void updateLinkHover(properdesktop::ui::PointF position, uint32_t modifiers);
    void clearLinkHover();
    void updatePointerShape();
    bool hasKeyboard() const;
    void reportFocus();

    Session* m_session = nullptr;
    std::vector<properdesktop::base::Subscription> m_sessionConnections;

    std::string m_colorScheme;
    properdesktop::ui::Color m_background;
    properdesktop::ui::Color m_foreground;
    double m_opacity = 1;
    CellFont m_font;
    BoxDrawing m_boxes;
    std::string m_fontFamily = "Monospace";
    double m_fontPointSize = 10;
    int m_cursorShape = 0;
    bool m_blinkingCursor = true;
    std::pair<int, bool> m_appliedCursorStyle{-1, false};
    // Whether the cursor blinks now, by the settings or the program.
    bool m_cursorBlinks = false;

    int m_columns = 1;
    int m_lines = 1;

    GhosttyRenderState m_state = nullptr;
    GhosttyRenderStateRowIterator m_rows = nullptr;
    GhosttyRenderStateRowCells m_cells = nullptr;
    bool m_updatePending = false;
    bool m_held = false;
    properdesktop::base::Timer m_holdTimeout;
    std::pair<int, int> m_lastCursor{-1, -1};

    // The scrollback position and the touchpad's offset within a line,
    // logical pixels, negative or past a line at the ends' stretch.
    Terminal::Scrollbar m_scrollbar;
    double m_scrollOffset = 0;
    bool m_applyingScroll = false;
    std::unique_ptr<properdesktop::ui::Scroller> m_scroller;
    // Rows the scroller's positions lead the scrollback's by: output that
    // moves the view during a scroll moves the scroller's origin with it.
    int64_t m_scrollerShift = 0;
    double m_wheelRemainder = 0;
    properdesktop::ui::PointF m_lastPointer;

    // The cursor's blink: hidden in the off half.
    properdesktop::base::Timer m_cursorBlinkTimer;
    bool m_cursorHidden = false;
    // Text with the blink attribute.
    properdesktop::base::Timer m_textBlinkTimer;
    bool m_textHidden = false;
    bool m_hasBlinkingText = false;

    bool m_reportedFocus = false;
    // The last key pressed and not yet released.
    std::optional<properdesktop::ui::KeyEvent> m_pressedKey;

    bool m_notifiesBell = false;
    bool m_confirmMultilinePaste = true;
    std::string m_pendingPaste;

    // The pointer.
    GhosttySelectionGesture m_gesture = nullptr;
    GhosttySelectionGestureEvent m_pressEvent = nullptr;
    GhosttySelectionGestureEvent m_dragEvent = nullptr;
    GhosttySelectionGestureEvent m_releaseEvent = nullptr;
    GhosttySelectionGestureEvent m_tickEvent = nullptr;
    properdesktop::base::Timer m_autoscroll;
    uint32_t m_buttons = 0;
    uint32_t m_lastModifiers = 0;
    bool m_selecting = false;
    bool m_menuPress = false;
    bool m_mousePress = false;
    std::string m_pressedLink;
    // A press on selected text drags it out once the pointer moves far enough.
    enum class Drag { None, Pending, Dragging };
    Drag m_drag = Drag::None;
    properdesktop::ui::PointF m_dragStart;

    // The link under the pointer, underlined while it is over it.
    struct Segment
    {
        int line;
        int column;
        int length;
    };
    std::vector<Segment> m_linkSegments;
    properdesktop::ui::Color m_linkColor;
    bool m_tracksMouse = false;
    std::pair<int, int> m_hoverCell{-1, -1};

    std::unique_ptr<Search> m_search;

    // The input method's text, shown at the cursor.
    std::u32string m_preedit;

    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

}

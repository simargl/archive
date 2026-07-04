#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Openbox panel (Win7-style behavior) using PyGObject + libwnck.

Updates in this version
- Panel spans 100% of the screen width, pinned to bottom.
- Task buttons use even widths and reflow dynamically as windows open/close.
- Start button on the left; clicking it runs:  toggle-app dlauncher
- Each task button shows the app/window icon + title.
- Right-click menu on tasks: Maximize/Restore, Minimize, Close.
- Left-click on tasks: if minimized -> unminimize+focus; if active -> minimize; else -> focus/raise.
- Clock on the right, formatted as "%b %d, %H:%M".

Dependencies (Debian/Ubuntu)
  sudo apt install python3-gi gir1.2-gtk-3.0 gir1.2-wnck-3.0

Run
  python3 openbox_win7_style_panel.py

Notes
- Requires X11 (libwnck is X11-only). Designed for Openbox on Xorg.
- To keep the code simple, this does not set EWMH struts; maximized windows may overlap.
  (Tell me if you want me to add struts so maximized windows avoid the panel.)
"""

import gi
import locale
import subprocess
from datetime import datetime

# GI requirements
gi.require_version("Gtk", "3.0")
gi.require_version("Wnck", "3.0")
from gi.repository import Gtk, Gdk, GLib, Wnck, Pango
from Xlib import display, X, Xatom

locale.setlocale(locale.LC_TIME, "")  # use system locale for month names

PANEL_HEIGHT = 32
ICON_SIZE = 32  # icon inside task button


def now_str():
    return datetime.now().strftime("%b %d, %H:%M")

def reserve_space_for_panel(window, height):
    """Reserve space at the bottom of the screen for the panel."""
    d = display.Display()
    root = d.screen().root

    # Get the XID of the GTK window
    gdk_window = window.get_window()
    xid = gdk_window.get_xid()

    # Screen geometry
    screen = window.get_screen()
    screen_w = screen.get_width()
    screen_h = screen.get_height()

    # Left, right, top, bottom reserved space
    strut = [0, 0, 0, height]
    # Partial strut (left, right, top, bottom, left_start_y, left_end_y,
    # right_start_y, right_end_y, top_start_x, top_end_x, bottom_start_x, bottom_end_x)
    strut_partial = [
        0, 0, 0, height,
        0, 0, 0, 0, 0, 0,
        0, screen_w-1
    ]

    NET_WM_STRUT = d.intern_atom("_NET_WM_STRUT")
    NET_WM_STRUT_PARTIAL = d.intern_atom("_NET_WM_STRUT_PARTIAL")

    win = d.create_resource_object('window', xid)
    win.change_property(NET_WM_STRUT, Xatom.CARDINAL, PANEL_HEIGHT, strut)
    win.change_property(NET_WM_STRUT_PARTIAL, Xatom.CARDINAL, PANEL_HEIGHT, strut_partial)

    d.sync()


class StartButton(Gtk.Button):
    def __init__(self):
        super().__init__()
        self.set_relief(Gtk.ReliefStyle.NONE)
        self.set_can_focus(False)
        self.get_style_context().add_class("start-button")
        box = Gtk.Box.new(Gtk.Orientation.HORIZONTAL, 6)
        img = Gtk.Image.new_from_icon_name(
            # Common themes usually have one of these; try in order
            "start-here",
            Gtk.IconSize.MENU,
        )
        if img.get_storage_type() == Gtk.ImageType.EMPTY:
            img = Gtk.Image.new_from_icon_name("applications-system", Gtk.IconSize.MENU)
        if img.get_storage_type() == Gtk.ImageType.EMPTY:
            img = Gtk.Image.new_from_icon_name("system-run", Gtk.IconSize.MENU)
        label = Gtk.Label.new("Start")
        label.set_xalign(0)
        box.pack_start(img, False, False, 0)
        box.pack_start(label, False, False, 0)
        self.add(box)
        self.connect("clicked", self.on_clicked)
        self.show_all()

    def on_clicked(self, _btn):
        # Run the user-specified command
        try:
            subprocess.Popen(["toggle-app", "dlauncher"])  # do not wait/block
        except Exception as e:
            print("Failed to execute start command:", e)


class TaskButton(Gtk.Button):
    def __init__(self, win: Wnck.Window, panel):
        super().__init__()
        self.set_relief(Gtk.ReliefStyle.NONE)
        self.set_can_focus(False)
        self.set_hexpand(True)
        self.set_halign(Gtk.Align.FILL)
        self.get_style_context().add_class("task-button")

        self.win = win
        self.panel = panel

        # Content: [icon][label]
        wrapper = Gtk.Box.new(Gtk.Orientation.HORIZONTAL, 6)
        self.icon = Gtk.Image()
        self.icon.set_from_pixbuf(self._get_scaled_icon())
        self.label = Gtk.Label.new(win.get_name())
        self.label.set_ellipsize(Pango.EllipsizeMode.END)
        self.label.set_xalign(0)
        self.set_hexpand(True)
        wrapper.pack_start(self.icon, False, False, 0)
        wrapper.pack_start(self.label, True, True, 0)
        self.add(wrapper)
        self.show_all()

        # Update on window changes
        win.connect("name-changed", self._on_name_changed)
        win.connect("state-changed", self._on_state_changed)
        win.connect("icon-changed", self._on_icon_changed)

        # Mouse handlers
        self.add_events(Gdk.EventMask.BUTTON_PRESS_MASK)
        self.connect("button-press-event", self._on_button_press)

        # Initial state reflect
        self._refresh_label()
        self._refresh_style()

    # ---- Helpers ----
    def _get_scaled_icon(self):
        pix = self.win.get_icon()
        if pix is None:
            return None
        try:
            return pix.scale_simple(ICON_SIZE, ICON_SIZE, Gdk.InterpType.BILINEAR)
        except Exception:
            return pix

    def _on_icon_changed(self, *args):
        pix = self._get_scaled_icon()
        if pix is not None:
            self.icon.set_from_pixbuf(pix)

    def _on_name_changed(self, *args):
        self._refresh_label()

    def _on_state_changed(self, *args):
        self._refresh_style()

    def _refresh_label(self):
        self.label.set_text(self.win.get_name())

    def _refresh_style(self):
        # Mark active/minimized via CSS classes for feedback
        ctx = self.get_style_context()
        for cls in ("active", "minimized"):
            if ctx.has_class(cls):
                ctx.remove_class(cls)
        screen = self.panel.screen
        if screen.get_active_window() == self.win:
            ctx.add_class("active")
        if self.win.is_minimized():
            ctx.add_class("minimized")

    # --- Actions ---
    def activate_or_minimize(self):
        # Left-click behavior
        if self.win.is_minimized():
            self.win.unminimize(Gdk.CURRENT_TIME)
            self.win.activate(Gdk.CURRENT_TIME)
        else:
            if self.panel.screen.get_active_window() == self.win:
                self.win.minimize()
            else:
                if self.win.is_minimized():
                    self.win.unminimize(Gdk.CURRENT_TIME)
                self.win.activate(Gdk.CURRENT_TIME)

    def do_maximize_toggle(self, *_):
        if self.win.is_maximized():
            self.win.unmaximize()
        else:
            self.win.maximize()

    def do_minimize(self, *_):
        if not self.win.is_minimized():
            self.win.minimize()

    def do_close(self, *_):
        self.win.close(Gdk.CURRENT_TIME)

    def _on_button_press(self, _widget, event):
        if event.button == 1:  # left click
            self.activate_or_minimize()
            return True
        if event.button == 3:  # right click
            self._popup_menu(event)
            return True
        return False

    def _popup_menu(self, event):
        menu = Gtk.Menu()

        item_max = Gtk.MenuItem.new_with_label(
            "Restore" if self.win.is_maximized() else "Maximize"
        )
        item_max.connect("activate", self.do_maximize_toggle)
        menu.append(item_max)

        item_min = Gtk.MenuItem.new_with_label("Minimize")
        item_min.connect("activate", self.do_minimize)
        menu.append(item_min)

        item_close = Gtk.MenuItem.new_with_label("Close")
        item_close.connect("activate", self.do_close)
        menu.append(item_close)

        menu.show_all()
        menu.popup_at_pointer(event)


class Panel(Gtk.Window):
    def __init__(self):
        super().__init__(type=Gtk.WindowType.TOPLEVEL)
        self.set_title("Openbox Win7 Panel")
        self.set_decorated(False)
        self.set_resizable(False)
        self.set_accept_focus(False)
        self.set_focus_on_map(False)
        self.stick()
        self.set_keep_above(True)
        self.set_skip_taskbar_hint(True)
        self.set_skip_pager_hint(True)
        self.set_type_hint(Gdk.WindowTypeHint.DOCK)

        self.connect("delete-event", Gtk.main_quit)

        # Monitor geometry & position bottom
        self._height = PANEL_HEIGHT
        self._place_bottom_fullwidth()
        self.connect("screen-changed", lambda *a: self._place_bottom_fullwidth())
        self.connect("size-allocate", lambda *a: self._place_bottom_fullwidth())
        
        monitor = self.get_screen().get_primary_monitor()
        if monitor is None:
            w = self.get_screen().get_width()
            h = self.get_screen().get_height()
        else:
            geo = self.get_screen().get_monitor_geometry(monitor)
            w, h = geo.width, geo.height

        # Layout: [ start | tasklist | clock ]
        outer = Gtk.Box.new(Gtk.Orientation.HORIZONTAL, 6)
        outer.set_border_width(0)
        outer.set_spacing(0)
        outer.set_halign(Gtk.Align.FILL)
        outer.set_hexpand(True)
        outer.set_size_request(w, 32)
        self.add(outer)

        # Start button (fixed min width)
        self.start_btn = StartButton()
        outer.pack_start(self.start_btn, False, False, 0)

        # Tasklist: homogeneous => even widths for all children
        self.task_box = Gtk.Box.new(Gtk.Orientation.HORIZONTAL, 4)
        self.task_box.set_homogeneous(True)  # ensures even widths and dynamic reflow
        outer.pack_start(self.task_box, True, True, 0)

        # Clock (right, fixed)
        self.clock_label = Gtk.Label.new(now_str())
        self.clock_label.set_xalign(1.0)
        self.clock_label.set_width_chars(12)
        self.clock_label.modify_font(Pango.FontDescription("Consolas 14"))
        outer.pack_end(self.clock_label, False, False, 0)

        # CSS for small visual cues
        self._load_css()

        # Wnck screen & signals
        self.screen = Wnck.Screen.get_default()
        self.screen.force_update()
        self.screen.connect("window-opened", self._on_window_opened)
        self.screen.connect("window-closed", self._on_window_closed)
        self.screen.connect("active-window-changed", self._on_active_changed)

        # Initial population
        self.buttons_by_xid = {}
        self._rebuild_tasklist()

        # Clock ticker
        GLib.timeout_add_seconds(1, self._tick_clock)

        self.show_all()

    def _load_css(self):
        provider = Gtk.CssProvider()
        css = b"""
        .task-button {
            padding: 6px 10px;
            border-radius: 8px;
        }
        .task-button.active {
            background: shade(@theme_selected_bg_color, 1.1);
        }
        .task-button.minimized {
            opacity: 0.65;
        }
        .start-button {
            padding: 6px 10px;
            border-radius: 8px;
            font-weight: bold;
        }
        """
        provider.load_from_data(css)
        Gtk.StyleContext.add_provider_for_screen(
            Gdk.Screen.get_default(), provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION
        )

    def _place_bottom_fullwidth(self):
        # Use monitor geometry
        monitor = self.get_screen().get_primary_monitor()
        if monitor is None:
            w = self.get_screen().get_width()
            h = self.get_screen().get_height()
        else:
            geo = self.get_screen().get_monitor_geometry(monitor)
            w, h = geo.width, geo.height

        self.resize(w, PANEL_HEIGHT)
        self.move(0, h - PANEL_HEIGHT)

    # --- Clock ---
    def _tick_clock(self):
        self.clock_label.set_text(now_str())
        return True  # keep timer

    # --- Task list management ---
    def _accept_window(self, win: Wnck.Window) -> bool:
        if win is None:
            return False
        if win.is_skip_tasklist() or win.is_skip_pager():
            return False
        wtype = win.get_window_type()
        if wtype not in (
            Wnck.WindowType.NORMAL,
            Wnck.WindowType.DIALOG,
            Wnck.WindowType.UTILITY,
        ):
            return False
        return True

    def _rebuild_tasklist(self):
        # Clear old
        for child in list(self.task_box.get_children()):
            self.task_box.remove(child)
        self.buttons_by_xid.clear()

        # Add current windows
        for win in self.screen.get_windows():
            if self._accept_window(win):
                self._add_button_for(win)

        self.show_all()

    def _add_button_for(self, win: Wnck.Window):
        xid = win.get_xid()
        if xid in self.buttons_by_xid:
            return
        btn = TaskButton(win, self)
        self.buttons_by_xid[xid] = btn
        self.task_box.pack_start(btn, True, True, 0)

    def _remove_button_for(self, win: Wnck.Window):
        xid = win.get_xid()
        btn = self.buttons_by_xid.pop(xid, None)
        if btn is not None:
            self.task_box.remove(btn)

    # --- Wnck callbacks ---
    def _on_window_opened(self, _screen, win):
        if self._accept_window(win):
            self._add_button_for(win)
            self.show_all()

    def _on_window_closed(self, _screen, win):
        self._remove_button_for(win)

    def _on_active_changed(self, *_):
        # Refresh styles for all buttons to reflect active state
        for btn in self.buttons_by_xid.values():
            btn._refresh_style()


def main():
    panel = Panel()
    panel.show_all()
    reserve_space_for_panel(panel, 32)  # height matches your panel height
    Gtk.main()


if __name__ == "__main__":
    main()

import os
import gi

gi.require_version("Gtk", "3.0")
gi.require_version("GdkPixbuf", "2.0")

from gi.repository import Gtk, GdkPixbuf, Gio, Gdk


class ApplicationViewer(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title="Application Viewer")
        self.set_default_size(600, 500)
        self.set_decorated(False)
        self.set_skip_taskbar_hint(True)
        self.set_skip_pager_hint(True)
        self.set_type_hint(Gdk.WindowTypeHint.DIALOG)

        self.connect("destroy", Gtk.main_quit)
        self.connect("focus-out-event", self.on_focus_out_event)
        self.connect("key-press-event", self.on_key_press)

        # Cache desktop files
        self.desktop_files = [
            f for f in os.listdir("/usr/share/applications")
            if f.endswith(".desktop")
        ]

        # ListStore: icon, display name, desktop filename
        self.liststore = Gtk.ListStore(GdkPixbuf.Pixbuf, str, str)

        for desktop_file in self.desktop_files:
            info = self.get_app_info(desktop_file)
            if not info:
                continue

            icon_name, display_name = info
            self.liststore.append([
                self.load_pixbuf(icon_name),
                display_name,
                desktop_file
            ])

        # Search entry
        self.search_entry = Gtk.SearchEntry()
        self.search_entry.connect("search-changed", self.filter_applications)
        self.search_entry.connect("key-press-event", self.on_search_key_press)

        # IconView
        self.iconview = Gtk.IconView.new()
        self.iconview.set_model(self.liststore)
        self.iconview.set_text_column(1)
        self.iconview.set_pixbuf_column(0)
        self.iconview.set_columns(4)
        self.iconview.set_item_width(80)
        self.iconview.connect("item-activated", self.launch_application)
        self.iconview.connect("button-press-event", self.on_iconview_button_press)

        # Layout
        box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=6)
        box.pack_start(self.search_entry, False, False, 6)

        scrolled = Gtk.ScrolledWindow()
        scrolled.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
        scrolled.add(self.iconview)

        box.pack_start(scrolled, True, True, 0)
        self.add(box)

        self.move(50, 2)

        self.show_all()
        self.search_entry.grab_focus()

    # ---------------- helpers ---------------- #

    def get_app_info(self, desktop_file):
        path = os.path.join("/usr/share/applications", desktop_file)
        try:
            app_info = Gio.DesktopAppInfo.new_from_filename(path)
            if not app_info:
                return None

            # 🔴 Hide NoDisplay=true apps
            if app_info.get_nodisplay():
                return None

            icon = app_info.get_icon()
            icon_name = icon.get_names()[0] if icon else "applications-other"
            display_name = app_info.get_display_name()

            return icon_name, display_name

        except Exception:
            return None

    def load_pixbuf(self, icon_name):
        theme = Gtk.IconTheme.get_default()
        try:
            icon_info = theme.lookup_icon(icon_name, 48, 0)
            return icon_info.load_icon()
        except Exception:
            return GdkPixbuf.Pixbuf.new_from_file_at_size(
                "/usr/share/icons/Adwaita/48x48/devices/computer.png",
                48, 48
            )

    # ---------------- actions ---------------- #

    def launch_application(self, widget, path):
        # path may be Gtk.TreePath OR int
        if isinstance(path, int):
            index = path
        else:
            index = path.get_indices()[0]

        desktop_file = self.liststore[index][2]

        try:
            app_info = Gio.DesktopAppInfo.new_from_filename(
                os.path.join("/usr/share/applications", desktop_file)
            )
            app_info.launch([], None)
            Gtk.main_quit()
        except Exception as e:
            print("Launch error:", e)

    def filter_applications(self, entry):
        text = entry.get_text().lower()
        self.liststore.clear()

        for desktop_file in self.desktop_files:
            info = self.get_app_info(desktop_file)
            if not info:
                continue

            icon_name, display_name = info
            if text in display_name.lower():
                self.liststore.append([
                    self.load_pixbuf(icon_name),
                    display_name,
                    desktop_file
                ])

    # ---------------- events ---------------- #

    def on_search_key_press(self, widget, event):
        if event.keyval == Gdk.KEY_Return:
            if len(self.liststore) == 1:
                path = Gtk.TreePath.new_from_indices([0])
                self.launch_application(self.iconview, path)
                return True
        return False

    def on_key_press(self, widget, event):
        if event.keyval == Gdk.KEY_Escape:
            self.hide()
            return True
        return False

    def on_focus_out_event(self, widget, event):
        Gtk.main_quit()

    def on_iconview_button_press(self, widget, event):
        if event.type == Gdk.EventType.BUTTON_PRESS and event.button == 1:
            result = self.iconview.get_path_at_pos(int(event.x), int(event.y))
            if result:
                self.launch_application(widget, result[0])


if __name__ == "__main__":
    ApplicationViewer()
    Gtk.main()

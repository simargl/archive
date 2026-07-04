import os
import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk, Gio, GLib
import shutil
import time
import threading

class FileManagerWindow(Gtk.Window):
    def __init__(self):
        super(FileManagerWindow, self).__init__(title="Simple File Manager")
        self.set_default_size(800, 600)
        self.maximize()

        # Main vertical box
        vbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=6)
        self.add(vbox)

        # Horizontal box for two panels
        hbox = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=6)
        vbox.pack_start(hbox, True, True, 0)

        # ── Left panel ───────────────────────────────────────
        self.left_store = Gtk.ListStore(str, str, str, str, str)  # name, path, icon, size, mtime
        self.left_tree = Gtk.TreeView(model=self.left_store)
        self.left_tree.get_selection().set_mode(Gtk.SelectionMode.MULTIPLE)

        self._setup_treeview_columns(self.left_tree)
        self.left_tree.connect("button-press-event", self.on_treeview_clicked, True)

        left_scroll = Gtk.ScrolledWindow()
        left_scroll.add(self.left_tree)
        hbox.pack_start(left_scroll, True, True, 0)

        # ── Right panel ──────────────────────────────────────
        self.right_store = Gtk.ListStore(str, str, str, str, str)
        self.right_tree = Gtk.TreeView(model=self.right_store)
        self.right_tree.get_selection().set_mode(Gtk.SelectionMode.MULTIPLE)

        self._setup_treeview_columns(self.right_tree)
        self.right_tree.connect("button-press-event", self.on_treeview_clicked, False)

        right_scroll = Gtk.ScrolledWindow()
        right_scroll.add(self.right_tree)
        hbox.pack_start(right_scroll, True, True, 0)

        # Progress bar
        self.progress_bar = Gtk.ProgressBar()
        self.progress_bar.set_show_text(True)
        self.progress_bar.set_text("Ready")
        vbox.pack_start(self.progress_bar, False, False, 0)

        # Buttons
        button_box = Gtk.Box(spacing=6)
        vbox.pack_start(button_box, False, False, 0)

        buttons = [
            ("View",   self.on_view_clicked),
            ("Open",   self.on_open_clicked),
            ("Copy",   self.on_copy_clicked),
            ("Move",   self.on_move_clicked),
            ("Delete", self.on_delete_clicked),
            ("Refresh",self.on_refresh_clicked),
        ]
        for label, callback in buttons:
            btn = Gtk.Button(label=label)
            btn.connect("clicked", callback)
            button_box.pack_start(btn, True, True, 0)

        # Initial paths
        self.left_path  = os.path.expanduser("~")
        self.right_path = os.path.expanduser("~")
        self.populate_panel(self.left_store,  self.left_path)
        self.populate_panel(self.right_store, self.right_path)

    def _setup_treeview_columns(self, treeview):
        # Name + icon column
        col_name = Gtk.TreeViewColumn("Name")
        rend_pix  = Gtk.CellRendererPixbuf()
        rend_text = Gtk.CellRendererText()
        col_name.pack_start(rend_pix,  False)
        col_name.pack_start(rend_text, True)
        col_name.add_attribute(rend_pix,  "icon-name", 2)
        col_name.add_attribute(rend_text, "text",      0)
        treeview.append_column(col_name)

        # Size
        rend_size = Gtk.CellRendererText()
        col_size = Gtk.TreeViewColumn("Size", rend_size, text=3)
        treeview.append_column(col_size)

        # Modified
        rend_date = Gtk.CellRendererText()
        col_date = Gtk.TreeViewColumn("Modified", rend_date, text=4)
        treeview.append_column(col_date)

    def get_icon_name(self, path):
        if os.path.isdir(path):
            return "folder"
        if not os.path.isfile(path):
            return "text-x-generic"

        try:
            file = Gio.File.new_for_path(path)
            info = file.query_info("standard::content-type", 0, None)
            ctype = info.get_content_type()
            icon  = Gio.content_type_get_icon(ctype)
            if isinstance(icon, Gio.ThemedIcon):
                names = icon.get_names()
                if names:
                    return names[0]
        except Exception:
            pass

        return "text-x-generic"

    @staticmethod
    def human_readable_size(size):
        for unit in ['B', 'KB', 'MB', 'GB', 'TB']:
            if size < 1024:
                return "{} {}".format(round(size, 1), unit) if unit != 'B' else "{} {}".format(size, unit)
            size /= 1024.0
        return "{} PB".format(size)

    @staticmethod
    def format_mtime(mtime):
        return time.strftime("%Y-%m-%d %H:%M", time.localtime(mtime))

    def populate_panel(self, store, path):
        store.clear()

        parent = os.path.dirname(path)
        if os.path.exists(parent) and parent != path:
            store.append(["..", parent, "folder", "", ""])

        try:
            folders = []
            files   = []

            for name in os.listdir(path):
                full = os.path.join(path, name)
                icon = self.get_icon_name(full)

                try:
                    st   = os.stat(full)
                    size = self.human_readable_size(st.st_size) if not os.path.isdir(full) else ""
                    mtim = self.format_mtime(st.st_mtime)
                except Exception:
                    size = mtim = ""

                row = [name, full, icon, size, mtim]

                if os.path.isdir(full):
                    folders.append(row)
                else:
                    files.append(row)

            for row in sorted(folders, key=lambda r: r[0].lower()):
                store.append(row)
            for row in sorted(files,   key=lambda r: r[0].lower()):
                store.append(row)

        except Exception as e:
            print("Populate error:", e)

    def get_selected_paths(self, treeview):
        sel = treeview.get_selection()
        model, tree_paths = sel.get_selected_rows()
        return [model[p][1] for p in tree_paths]

    def on_treeview_clicked(self, treeview, event, is_left):
        if event.button != 1:
            return False

        res = treeview.get_path_at_pos(int(event.x), int(event.y))
        if not res:
            return False

        path, _, _, _ = res
        if not path:
            return False

        model = treeview.get_model()
        fullpath = model[path][1]

        if os.path.isdir(fullpath):
            if is_left:
                self.left_path = fullpath
                self.populate_panel(self.left_store, fullpath)
            else:
                self.right_path = fullpath
                self.populate_panel(self.right_store, fullpath)
        elif os.path.isfile(fullpath):
            self.open_with_default_app(fullpath)

        return True

    def open_with_default_app(self, path):
        try:
            f = Gio.File.new_for_path(path)
            info = f.query_info("standard::content-type", 0, None)
            ctype = info.get_content_type()
            app = Gio.AppInfo.get_default_for_type(ctype, False)
            if app:
                app.launch([f], None)
            else:
                print("No default app for: {}".format(ctype))
        except Exception as e:
            print("Open failed: {}".format(e))

    def update_progress(self, current, total, operation):
        fraction = current / total if total > 0 else 1.0
        GLib.idle_add(self.progress_bar.set_fraction, fraction)
        GLib.idle_add(self.progress_bar.set_text, "{}: {}%".format(operation, int(fraction*100)))

    def safe_copy(self, src, dst):
        """Recursive copy - replacement for shutil.copytree(dirs_exist_ok=True)"""
        if os.path.isfile(src):
            shutil.copy2(src, dst)
            return

        # It's a directory
        if not os.path.exists(dst):
            os.makedirs(dst)

        for item in os.listdir(src):
            s = os.path.join(src, item)
            d = os.path.join(dst, item)
            if os.path.isdir(s):
                self.safe_copy(s, d)
            else:
                shutil.copy2(s, d)

    def copy_file(self, source, dest_parent, idx, total, operation):
        try:
            dest = os.path.join(dest_parent, os.path.basename(source))
            if os.path.isdir(source):
                self.safe_copy(source, dest)
            else:
                shutil.copy2(source, dest)
            GLib.idle_add(self.update_progress, idx + 1, total, operation)
        except Exception as e:
            print("Copy error {} → {}: {}".format(source, dest, e))

    def move_file(self, source, dest_parent, idx, total, operation):
        try:
            dest = os.path.join(dest_parent, os.path.basename(source))
            shutil.move(source, dest)
            GLib.idle_add(self.update_progress, idx + 1, total, operation)
        except Exception as e:
            print("Move error {}: {}".format(source, e))

    def delete_file(self, path, idx, total, operation):
        try:
            if os.path.isfile(path):
                os.remove(path)
            elif os.path.isdir(path):
                shutil.rmtree(path, ignore_errors=True)
            GLib.idle_add(self.update_progress, idx + 1, total, operation)
        except Exception as e:
            print("Delete error {}: {}".format(path, e))

    def on_view_clicked(self, _btn):
        for tv in (self.left_tree, self.right_tree):
            paths = self.get_selected_paths(tv)
            if paths and os.path.isfile(paths[0]):
                self.open_with_default_app(paths[0])
                return

    def on_open_clicked(self, _btn):
        for tv, current_path, store in [
            (self.left_tree,  self.left_path,  self.left_store),
            (self.right_tree, self.right_path, self.right_store),
        ]:
            paths = self.get_selected_paths(tv)
            if not paths:
                continue
            p = paths[0]
            if os.path.isfile(p):
                self.open_with_default_app(p)
            elif os.path.isdir(p):
                if tv == self.left_tree:
                    self.left_path = p
                else:
                    self.right_path = p
                self.populate_panel(store, p)
            return

    def on_copy_clicked(self, _btn):
        sources = self.get_selected_paths(self.left_tree)
        dest = self.right_path
        if not sources:
            sources = self.get_selected_paths(self.right_tree)
            dest = self.left_path

        if not sources:
            return

        total = len(sources)
        self.update_progress(0, total, "Copying")

        for i, src in enumerate(sources):
            if not os.path.exists(src):
                continue
            threading.Thread(
                target=self.copy_file,
                args=(src, dest, i, total, "Copying"),
                daemon=True
            ).start()

        GLib.timeout_add(300, self.check_operation_complete, total, "Copying")

    def on_move_clicked(self, _btn):
        sources = self.get_selected_paths(self.left_tree)
        dest = self.right_path
        if not sources:
            sources = self.get_selected_paths(self.right_tree)
            dest = self.left_path

        if not sources:
            return

        total = len(sources)
        self.update_progress(0, total, "Moving")

        for i, src in enumerate(sources):
            if not os.path.exists(src):
                continue
            threading.Thread(
                target=self.move_file,
                args=(src, dest, i, total, "Moving"),
                daemon=True
            ).start()

        GLib.timeout_add(300, self.check_operation_complete, total, "Moving")

    def on_delete_clicked(self, _btn):
        paths = self.get_selected_paths(self.left_tree) or self.get_selected_paths(self.right_tree)
        if not paths:
            return

        dialog = Gtk.MessageDialog(
            transient_for=self,
            flags=0,
            message_type=Gtk.MessageType.WARNING,
            buttons=Gtk.ButtonsType.YES_NO,
            text="Permanently delete {} item(s)?".format(len(paths))
        )
        resp = dialog.run()
        dialog.destroy()

        if resp != Gtk.ResponseType.YES:
            return

        total = len(paths)
        self.update_progress(0, total, "Deleting")

        for i, p in enumerate(paths):
            threading.Thread(
                target=self.delete_file,
                args=(p, i, total, "Deleting"),
                daemon=True
            ).start()

        GLib.timeout_add(300, self.check_operation_complete, total, "Deleting")

    def check_operation_complete(self, total, operation):
        if self.progress_bar.get_fraction() >= 0.99:
            GLib.idle_add(self.on_refresh_clicked, None)
            GLib.idle_add(self.progress_bar.set_text, "Ready")
            return False
        return True

    def on_refresh_clicked(self, _btn=None):
        self.populate_panel(self.left_store,  self.left_path)
        self.populate_panel(self.right_store, self.right_path)
        self.progress_bar.set_fraction(0.0)
        self.progress_bar.set_text("Ready")


if __name__ == '__main__':
    win = FileManagerWindow()
    win.connect("destroy", Gtk.main_quit)
    win.show_all()
    Gtk.main()

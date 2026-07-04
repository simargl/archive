import gi
import os
import subprocess
import gi.repository.GLib as GLib

gi.require_version("Gtk", "3.0")
from gi.repository import Gtk, GObject

class TaskManager(Gtk.Window):
    def __init__(self):
        super().__init__(title="Gtk+ Task Manager")
        self.set_default_size(900, 600)

        # Vertical Box to pack widgets
        vbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=6)
        self.add(vbox)

        # Create TreeView and ListStore
        # Columns: Memory (MB), PID, User, Command, Status
        self.store = Gtk.ListStore(str, int, str, str, str)
        self.treeview = Gtk.TreeView(model=self.store)

        # Define columns (memory in MB first)
        columns = ["Memory (MB)", "PID", "User", "Command", "Status"]
        for i, title in enumerate(columns):
            renderer = Gtk.CellRendererText()
            column = Gtk.TreeViewColumn(title, renderer, text=i)
            self.treeview.append_column(column)

        # Enable right-click menu
        self.treeview.connect("button-press-event", self.on_right_click)

        # Add TreeView to scrolled window
        scrolled_window = Gtk.ScrolledWindow()
        scrolled_window.set_vexpand(True)
        scrolled_window.add(self.treeview)
        vbox.pack_start(scrolled_window, True, True, 0)

        # Setup periodic update every 2 seconds
        self.update_interval = 2000  # milliseconds
        GLib.timeout_add(self.update_interval, self.refresh_process_list)

        # Initial population
        self.refresh_process_list()

    def refresh_process_list(self, widget=None):
        self.store.clear()

        # Retrieve process info sorted by memory usage in descending order
        try:
            output = subprocess.check_output(['ps', 'aux', '--sort=-%mem'], text=True)
        except subprocess.CalledProcessError as e:
            print("Failed to get process list:", e)
            return True  # Continue the timeout

        lines = output.strip().split('\n')
        # Skip header line
        for line in lines[1:]:
            parts = line.split(None, 10)
            if len(parts) >= 11:
                user = parts[0]
                pid = int(parts[1])
                mem_percent = parts[3]  # %MEM
                rss_kb = parts[5]  # RSS in KB
                command = parts[10]

                # Convert RSS to MB
                try:
                    rss_mb = float(rss_kb) / 1024
                    rss_str = f"{rss_mb:.2f}"
                except (ValueError, ZeroDivisionError):
                    rss_str = "N/A"

                # Append row with Memory in MB first
                self.store.append([rss_str, pid, user, command, mem_percent])

        return True  # Continue calling every interval

    def on_right_click(self, widget, event):
        if event.button == 3:  # Right click
            path_info = widget.get_path_at_pos(int(event.x), int(event.y))
            if path_info is not None:
                path, col, cell_x, cell_y = path_info
                widget.grab_focus()
                widget.set_cursor(path, col, 0)
                self.show_context_menu(event)
            return True  # stop propagation
        return False

    def show_context_menu(self, event):
        menu = Gtk.Menu()

        kill_item = Gtk.MenuItem(label="Kill Process")
        kill_item.connect("activate", self.kill_process_at_cursor)
        menu.append(kill_item)

        menu.show_all()
        menu.popup(None, None, None, None, event.button, event.time)

    def kill_process_at_cursor(self, menu_item):
        selection = self.treeview.get_selection()
        model, treeiter = selection.get_selected()
        if treeiter is None:
            # Get the path at cursor position
            path = self.treeview.get_cursor()[0]
            if path is not None:
                treeiter = model.get_iter(path)
        if treeiter is not None:
            pid = model.get_value(treeiter, 1)
            dialog = Gtk.MessageDialog(
                transient_for=self,
                flags=0,
                message_type=Gtk.MessageType.QUESTION,
                buttons=Gtk.ButtonsType.YES_NO,
                text=f"Kill process {pid}?",
            )
            response = dialog.run()
            dialog.destroy()

            if response == Gtk.ResponseType.YES:
                try:
                    os.kill(pid, 9)
                except PermissionError:
                    self.show_error(f"Permission denied to kill process {pid}")
                except ProcessLookupError:
                    self.show_error(f"Process {pid} does not exist")
                self.refresh_process_list()

    def kill_selected_process(self, button):
        selection = self.treeview.get_selection()
        model, treeiter = selection.get_selected()
        if treeiter is not None:
            pid = model.get_value(treeiter, 1)
            dialog = Gtk.MessageDialog(
                transient_for=self,
                flags=0,
                message_type=Gtk.MessageType.QUESTION,
                buttons=Gtk.ButtonsType.YES_NO,
                text=f"Kill process {pid}?",
            )
            response = dialog.run()
            dialog.destroy()

            if response == Gtk.ResponseType.YES:
                try:
                    os.kill(pid, 9)
                except PermissionError:
                    self.show_error(f"Permission denied to kill process {pid}")
                except ProcessLookupError:
                    self.show_error(f"Process {pid} does not exist")
                self.refresh_process_list()

    def show_error(self, message):
        dialog = Gtk.MessageDialog(
            transient_for=self,
            flags=0,
            message_type=Gtk.MessageType.ERROR,
            buttons=Gtk.ButtonsType.OK,
            text=message,
        )
        dialog.run()
        dialog.destroy()

def main():
    app = TaskManager()
    app.connect("destroy", Gtk.main_quit)
    app.show_all()
    Gtk.main()

if __name__ == "__main__":
    main()

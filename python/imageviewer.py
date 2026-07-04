import gi
import os
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk, GdkPixbuf, Gdk

class ImageViewer(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title="Image Viewer")
        self.set_default_size(800, 600)  # Set the initial window size

        main_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
        self.add(main_box)

        self.image = Gtk.Image()
        main_box.pack_start(self.create_menu(), False, False, 0)
        main_box.pack_start(self.image, True, True, 0)

        self.image_paths = []
        self.current_image_index = 0

        self.connect("key-press-event", self.on_key_press)

    def create_menu(self):
        menubar = Gtk.MenuBar()

        # File menu
        file_menu = Gtk.Menu()
        file_item = Gtk.MenuItem(label="File")
        file_item.set_submenu(file_menu)
        menubar.append(file_item)

        open_item = Gtk.MenuItem(label="Open")
        open_item.connect("activate", self.on_open_clicked)
        file_menu.append(open_item)

        # Help menu
        help_menu = Gtk.Menu()
        help_item = Gtk.MenuItem(label="Help")
        help_item.set_submenu(help_menu)
        menubar.append(help_item)

        about_item = Gtk.MenuItem(label="About")
        about_item.connect("activate", self.on_about_clicked)
        help_menu.append(about_item)

        return menubar

    def on_open_clicked(self, widget):
        dialog = Gtk.FileChooserDialog(
            title="Please choose an image file",
            parent=self,
            action=Gtk.FileChooserAction.OPEN
        )
        dialog.add_buttons(
            Gtk.STOCK_CANCEL,
            Gtk.ResponseType.CANCEL,
            Gtk.STOCK_OPEN,
            Gtk.ResponseType.OK
        )

        filter_image = Gtk.FileFilter()
        filter_image.set_name("Image files")
        filter_image.add_mime_type("image/jpeg")
        filter_image.add_mime_type("image/png")
        filter_image.add_mime_type("image/gif")
        dialog.add_filter(filter_image)

        response = dialog.run()
        if response == Gtk.ResponseType.OK:
            filename = dialog.get_filename()
            self.image_paths = self.get_image_files_from_directory(os.path.dirname(filename))
            self.current_image_index = self.image_paths.index(filename)
            self.load_current_image()

        dialog.destroy()

    def on_about_clicked(self, widget):
        about_dialog = Gtk.AboutDialog()
        about_dialog.set_program_name("Image Viewer")
        about_dialog.set_version("1.0")
        about_dialog.set_authors(["Your Name"])
        about_dialog.set_comments("A simple image viewer")
        about_dialog.run()
        about_dialog.destroy()

    def load_current_image(self):
        filename = self.image_paths[self.current_image_index]
        pixbuf = GdkPixbuf.Pixbuf.new_from_file(filename)
        if (pixbuf.get_width() > 800):
            resized_pixbuf = pixbuf.scale_simple(800, 600, GdkPixbuf.InterpType.BILINEAR)
        else:
            resized_pixbuf = pixbuf
        self.image.set_from_pixbuf(resized_pixbuf)

    def on_key_press(self, widget, event):
        if event.keyval == Gdk.KEY_Left:
            self.show_previous_image()
        elif event.keyval == Gdk.KEY_Right:
            self.show_next_image()

    def show_previous_image(self):
        if self.current_image_index > 0:
            self.current_image_index -= 1
            self.load_current_image()

    def show_next_image(self):
        if self.current_image_index < len(self.image_paths) - 1:
            self.current_image_index += 1
            self.load_current_image()

    def get_image_files_from_directory(self, directory):
        image_paths = []
        for file in os.listdir(directory):
            if file.lower().endswith(('.jpg', '.jpeg', '.png', '.gif')):
                image_paths.append(os.path.join(directory, file))
        return image_paths

if __name__ == "__main__":
    viewer = ImageViewer()
    viewer.connect("destroy", Gtk.main_quit)
    viewer.show_all()
    Gtk.main()

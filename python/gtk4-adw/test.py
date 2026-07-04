import gi
gi.require_version("Gtk", "4.0")
gi.require_version("Adw", "1")

from gi.repository import Gtk, Adw

class MyApp(Adw.Application):
    def __init__(self):
        super().__init__(application_id="com.example.AdwaitaApp")

    def do_activate(self):
        win = MyWindow(application=self)
        win.present()

class MyWindow(Adw.ApplicationWindow):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.set_title("Adwaita GTK4 App")
        self.set_default_size(400, 200)

        # Main vertical layout
        box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)

        # Header bar (as a normal widget!)
        header = Adw.HeaderBar()
        box.append(header)

        # Content
        content = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=12)
        content.set_margin_top(24)
        content.set_margin_bottom(24)
        content.set_margin_start(24)
        content.set_margin_end(24)

        label = Gtk.Label(label="Hello, Libadwaita!")
        button = Gtk.Button(label="Click me")
        button.connect("clicked", self.on_button_clicked)

        content.append(label)
        content.append(button)

        box.append(content)

        self.set_content(box)

    def on_button_clicked(self, button):
        print("Button clicked!")

app = MyApp()

# Proper theme handling (optional)
style_manager = Adw.StyleManager.get_default()
style_manager.set_color_scheme(Adw.ColorScheme.FORCE_DARK)

app.run([])

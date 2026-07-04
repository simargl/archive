import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk, Gdk

class Calculator(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title="Calculator")
        self.set_default_size(300, 400)
        self.set_icon_name("accessories-calculator")  # Set window icon

        grid = Gtk.Grid()
        self.add(grid)

        menubar = Gtk.MenuBar()
        grid.attach(menubar, 0, 0, 4, 1)

        file_menu = Gtk.Menu()
        file_menu_item = Gtk.MenuItem(label="File")
        file_menu_item.set_submenu(file_menu)
        menubar.append(file_menu_item)

        quit_menu_item = Gtk.MenuItem(label="Quit")
        quit_menu_item.connect("activate", self.quit)
        file_menu.append(quit_menu_item)

        help_menu = Gtk.Menu()
        help_menu_item = Gtk.MenuItem(label="Help")
        help_menu_item.set_submenu(help_menu)
        menubar.append(help_menu_item)

        about_menu_item = Gtk.MenuItem(label="About")
        about_menu_item.connect("activate", self.show_about_dialog)
        help_menu.append(about_menu_item)

        self.result_entry = Gtk.Entry()
        self.result_entry.set_editable(False)
        grid.attach(self.result_entry, 0, 1, 4, 1)

        self.expression_entry = Gtk.Entry()
        self.expression_entry.connect("activate", self.calculate)
        grid.attach(self.expression_entry, 0, 2, 4, 1)

        buttons = [
            '7', '8', '9', '/',
            '4', '5', '6', '*',
            '1', '2', '3', '-',
            '0', '.', '=', '+'
        ]

        button_position = 0
        for button_label in buttons:
            button = Gtk.Button(label=button_label)
            button.connect("clicked", self.button_clicked)
            grid.attach(button, button_position % 4, button_position // 4 + 3, 1, 1)
            button.set_hexpand(True)
            button.set_vexpand(True)
            button_position += 1

        self.expression_entry.grab_focus()

    def button_clicked(self, widget):
        button_label = widget.get_label()
        current_expression = self.expression_entry.get_text()

        if button_label == '=':
            self.calculate()
        else:
            self.expression_entry.set_text(current_expression + button_label)

    def calculate(self, *args):
        current_expression = self.expression_entry.get_text()
        try:
            result = eval(current_expression)
            self.result_entry.set_text(str(result))
        except:
            self.result_entry.set_text("Error")

    def quit(self, widget):
        Gtk.main_quit()

    def show_about_dialog(self, widget):
        about_dialog = Gtk.AboutDialog()
        about_dialog.set_transient_for(self)
        about_dialog.set_program_name("Calculator")
        about_dialog.set_version("1.0")
        about_dialog.set_authors(["Your Name"])
        about_dialog.set_comments("A simple calculator application.")
        about_dialog.set_logo_icon_name("accessories-calculator")  # Set About dialog icon
        about_dialog.run()
        about_dialog.destroy()

win = Calculator()
win.connect("destroy", Gtk.main_quit)
win.show_all()
Gtk.main()

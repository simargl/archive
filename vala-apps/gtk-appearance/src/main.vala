/*  Author: simargl <https://github.com/simargl>
 *  License: GPL v3
 */

namespace Appearance {
public class Program: Gtk.Application {
    const string NAME        = "Gtk Appearance";
    const string VERSION     = "1.2.0";
    const string DESCRIPTION = "Utility for adjusting Gtk appearance";
    const string ICON        = "preferences-desktop-theme";
    const string APP_ID      = "org.vala-apps.gtk-appearance";
    const string APP_ID_PREF = "org.vala-apps.gtk-appearance.preferences";
    const string[] AUTHORS   = { "Simargl <https://github.com/simargl>", null };

    Gtk.ApplicationWindow window;

    private const GLib.ActionEntry[] action_entries = {
        { "about",       action_about       },
        { "quit",        action_quit        }
    };

    public Program() {
        Object(application_id: APP_ID, flags: GLib.ApplicationFlags.FLAGS_NONE);
        add_action_entries(action_entries, this);
    }

    public override void startup() {
        base.startup();
        var menu = new GLib.Menu();
        var section = new GLib.Menu();
        section.append("About", "app.about");
        section.append("Quit", "app.quit");
        menu.append_section(null, section);
        set_app_menu(menu);
        add_accelerator("<Primary>Q", "app.quit", null);
        //set_accels_for_action("app.quit", {"<Primary><Shift>Q", "<Primary>Q"});
        action_add_widgets();
    }

    public override void activate() {
        window.present();
    }

    private void action_add_widgets() {
        // Labels
        var label_dark       = new Gtk.Label("Global dark theme");
        var label_theme      = new Gtk.Label("Gtk theme");
        var label_icon       = new Gtk.Label("Icons");
        var label_font       = new Gtk.Label("Default font");
        var label_cursor     = new Gtk.Label("Cursor");
        label_dark.set_halign(Gtk.Align.START);
        label_theme.set_halign(Gtk.Align.START);
        label_icon.set_halign(Gtk.Align.START);
        label_font.set_halign(Gtk.Align.START);
        label_cursor.set_halign(Gtk.Align.START);
        // Buttons
        button_dark       = new Gtk.ComboBoxText();
        button_theme      = new Gtk.ComboBoxText();
        button_icon       = new Gtk.ComboBoxText();
        button_font       = new Gtk.FontButton();
        button_cursor     = new Gtk.ComboBoxText();
        // Get Values
        var settings = new Appearance.Settings();
        settings.get_current_values();
        populate_combo_boxes();
        button_dark.set_active_id(dark.to_string());
        button_theme.set_active_id(theme);
        button_icon.set_active_id(icon);
        button_font.set_font_name(font);
        button_cursor.set_active_id(cursor);
        // Connect signals
        button_dark.changed.connect         (on_button_dark_changed);
        button_theme.changed.connect        (on_button_theme_changed);
        button_icon.changed.connect         (on_button_icon_changed);
        button_font.font_set.connect        (on_button_font_changed);
        button_cursor.changed.connect       (on_button_cursor_changed);
        // Grid
        var grid = new Gtk.Grid();
        grid.set_column_spacing(10);
        grid.set_row_spacing(10);
        grid.set_border_width(20);
        grid.set_column_homogeneous(true);
        grid.attach(label_dark,        0, 0, 3, 1);
        grid.attach(button_dark,       3, 0, 2, 1);
        grid.attach(label_theme,       0, 1, 3, 1);
        grid.attach(button_theme,      3, 1, 2, 1);
        grid.attach(label_icon,        0, 2, 3, 1);
        grid.attach(button_icon,       3, 2, 2, 1);
        grid.attach(label_font,        0, 3, 3, 1);
        grid.attach(button_font,       3, 3, 2, 1);
        grid.attach(label_cursor,      0, 4, 3, 1);
        grid.attach(button_cursor,     3, 4, 2, 1);
        window = new Gtk.ApplicationWindow(this);
        window.window_position = Gtk.WindowPosition.CENTER;
        window.set_title(NAME);
        window.add(grid);
        window.set_default_size(450, 300);
        window.set_icon_name(ICON);
        window.show_all();
        window.delete_event.connect(() => {
            action_quit();
            return true;
        });
    }

    private void populate_combo_boxes() {
        string name;
        // Themes
        try {
            button_theme.append("Adwaita", "Adwaita"); // Adwaita
            var st = Dir.open("/usr/share/themes");
            while ((name = st.read_name()) != null) {
                File file = File.new_for_path("/usr/share/themes/" + name + "/gtk-3.0/gtk.css");
                if (file.query_exists() == true) {
                    button_theme.append(name, name);
                }
            }
            var ut = Dir.open(GLib.Environment.get_home_dir() + "/.themes");
            while ((name = ut.read_name()) != null) {
                File file = File.new_for_path(GLib.Environment.get_home_dir() + "/.themes/" +
                                              name + "/gtk-3.0/gtk.css");
                if (file.query_exists() == true) {
                    button_theme.append(name, name);
                }
            }
        } catch (GLib.Error e) {
            stderr.printf("%s\n", e.message);
        }
        // Icons
        try {
            var si = Dir.open("/usr/share/icons");
            while ((name = si.read_name()) != null) {
                File file = File.new_for_path("/usr/share/icons/" + name + "/index.theme");
                if (name != "default" & name != "hicolor")
                    if (file.query_exists() == true) {
                        button_icon.append(name, name);
                    }
            }
            var ui = Dir.open(GLib.Environment.get_home_dir() + "/.icons");
            while ((name = ui.read_name()) != null) {
                File file = File.new_for_path(GLib.Environment.get_home_dir() + "/.icons/" +
                                              name + "/index.theme");
                if (name != "default" & name != "hicolor")
                    if (file.query_exists() == true) {
                        button_icon.append(name, name);
                    }
            }
        } catch (GLib.Error e) {
            stderr.printf("%s\n", e.message);
        }
        // Cursors
        try {
            var sc = Dir.open("/usr/share/icons");
            while ((name = sc.read_name()) != null) {
                File file = File.new_for_path("/usr/share/icons/" + name + "/cursors");
                if (file.query_exists() == true) {
                    button_cursor.append(name, name);
                }
            }
        } catch (GLib.Error e) {
            stderr.printf("%s\n", e.message);
        }
        // Dark
        button_dark.append("true", "True");
        button_dark.append("false", "False");
    }

    private void on_button_dark_changed() {
        var settings = new Appearance.Settings();
        settings.dark_changed();
    }

    private void on_button_theme_changed() {
        var settings = new Appearance.Settings();
        settings.theme_changed();
    }

    private void on_button_icon_changed() {
        var settings = new Appearance.Settings();
        settings.icon_changed();
    }

    private void on_button_font_changed() {
        var settings = new Appearance.Settings();
        settings.font_changed();
    }

    private void on_button_cursor_changed() {
        var settings = new Appearance.Settings();
        settings.cursor_changed();
    }

    private void action_about() {
        var window = this.get_active_window();
        var about = new Gtk.AboutDialog();
        about.set_program_name(NAME);
        about.set_version(VERSION);
        about.set_comments(DESCRIPTION);
        about.set_logo_icon_name(ICON);
        about.set_icon_name(ICON);
        about.set_authors(AUTHORS);
        about.set_copyright("Copyright \xc2\xa9 2015");
        about.set_website("https://github.com/simargl");
        about.set_property("skip-taskbar-hint", true);
        about.set_transient_for(window);
        about.license_type = Gtk.License.GPL_3_0;
        about.run();
        about.hide();
    }

    private void action_quit() {
        quit();
    }
}

private int main (string[] args) {
    var app = new Program();
    return app.run(args);
}
}

namespace Appearance {
bool dark;
string theme;
string icon;
string font;
string cursor;

Gtk.Settings gtk_settings;
Gtk.ComboBoxText button_dark;
Gtk.ComboBoxText button_theme;
Gtk.ComboBoxText button_icon;
Gtk.FontButton   button_font;
Gtk.ComboBoxText button_cursor;

public class Settings: GLib.Object {
    public void get_current_values() {
        gtk_settings = Gtk.Settings.get_default();
        dark         = gtk_settings.gtk_application_prefer_dark_theme;
        theme        = gtk_settings.gtk_theme_name;
        icon         = gtk_settings.gtk_icon_theme_name;
        font         = gtk_settings.gtk_font_name;
        cursor       = gtk_settings.gtk_cursor_theme_name;
    }

    public void dark_changed() {
        gtk_settings.set("gtk-application-prefer-dark-theme",
                         bool.parse(button_dark.get_active_id()));
        write_changes_gtk3();
    }

    public void theme_changed() {
        gtk_settings.set("gtk-theme-name", button_theme.get_active_text());
        write_changes_gtk3();
        write_changes_gtk2();
        write_changes_openbox();
    }

    public void icon_changed() {
        gtk_settings.set("gtk-icon-theme-name", button_icon.get_active_text());
        write_changes_gtk3();
        write_changes_gtk2();
    }

    public void font_changed() {
        gtk_settings.set("gtk-font-name", button_font.get_font().to_string());
        write_changes_gtk3();
        write_changes_gtk2();
    }

    public void cursor_changed() {
        gtk_settings.set("gtk-cursor-theme-name", button_cursor.get_active_text());
        write_changes_gtk3();
        write_changes_gtk2();
    }

    public void write_changes_gtk3() {
        try {
            // GTK3
            string gtk3file = GLib.Environment.get_user_config_dir() +
                              "/gtk-3.0/settings.ini";
            string gtk3content =
                "[Settings]\n" +
                "gtk-application-prefer-dark-theme=%s\n".printf(
                    button_dark.get_active_id().to_string()) +
                "gtk-dialogs-use-header=false\n" +
                "gtk-theme-name=%s\n".printf(button_theme.get_active_text()) +
                "gtk-icon-theme-name=%s\n".printf(button_icon.get_active_text()) +
                "gtk-font-name=%s\n".printf(button_font.get_font().to_string()) +
                "gtk-cursor-theme-name=%s\n".printf(button_cursor.get_active_text()) +
                "gtk-menu-images=true\n";
            FileUtils.set_contents(gtk3file, gtk3content);
        } catch (FileError e) {
            stderr.printf ("%s\n", e.message);
        }
    }

    public void write_changes_gtk2() {
        try {
            // GTK2
            string gtk2file = GLib.Environment.get_home_dir() + "/.gtkrc-2.0";
            string gtk2content =
                "gtk-theme-name=\"%s\"\n".printf(button_theme.get_active_text()) +
                "gtk-icon-theme-name=\"%s\"\n".printf(button_icon.get_active_text()) +
                "gtk-font-name=\"%s\"\n".printf(button_font.get_font().to_string()) +
                "gtk-cursor-theme-name=\"%s\"\n".printf(button_cursor.get_active_text()) +
                "gtk-toolbar-style=GTK_TOOLBAR_ICONS\n" +
                "gtk-toolbar-icon-size=GTK_ICON_SIZE_SMALL_TOOLBAR\n" +
                "gtk-button-images=0\n" +
                "gtk-menu-images=1\n" +
                "gtk-xft-antialias=1\n" +
                "gtk-xft-hinting=1\n" +
                "gtk-xft-hintstyle=\"hintfull\"\n" +
                "include \"/root/.gtkrc-2.0.mine\"\n";
            FileUtils.set_contents(gtk2file, gtk2content);
        } catch (FileError e) {
            stderr.printf ("%s\n", e.message);
        }
    }

    private void write_changes_openbox() {
        File file;
        file = File.new_for_path(GLib.Environment.get_home_dir() + "/.themes/" +
                                 button_theme.get_active_text() + "/openbox-3");
        if (file.query_exists() == true) {
            write_rc_xml();
        }
        file = File.new_for_path("/usr/share/themes/" + button_theme.get_active_text() +
                                 "/openbox-3");
        if (file.query_exists() == true) {
            write_rc_xml();
        }
    }

    private void write_rc_xml() {
        string rcxml = GLib.Environment.get_home_dir() + "/.config/openbox/rc.xml";
        var file = File.new_for_path(rcxml);
        if (file.query_exists() == true) {
            try {
                Process.spawn_command_line_sync("sh -c 'gtk-appearance-openbox \"%s\"'".printf(
                                                    button_theme.get_active_text()));
            } catch (GLib.Error e) {
                stderr.printf ("%s\n", e.message);
            }
        }
    }
}
}

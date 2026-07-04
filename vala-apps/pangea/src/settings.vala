namespace Pangea {
public class Settings : GLib.Object {
    GLib.Settings settings;

    public void get_settings() {
        settings = new GLib.Settings("org.vala-apps.pangea.preferences");
        width = settings.get_int("width");
        height = settings.get_int("height");
        paned_pos = settings.get_int("paned-pos");
        icon_size = settings.get_int("icon-size");
        thumbnail_size = settings.get_int("thumbnail-size");
        saved_dir = settings.get_string("saved-dir");
        terminal = settings.get_string("terminal");
    }

    public void save_settings() {
        window.get_size(out width, out height);
        paned_pos = pane.get_position();
        settings = new GLib.Settings("org.vala-apps.pangea.preferences");
        settings.set_int("width", width);
        settings.set_int("height", height);
        settings.set_int("icon-size", icon_size);
        settings.set_int("thumbnail-size", thumbnail_size);
        settings.set_int("paned-pos", paned_pos);
        settings.set_string("saved-dir", current_dir);
        GLib.Settings.sync();
    }
}
}

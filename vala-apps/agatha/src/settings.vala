namespace Agatha {
private int     width;
private int     height;
private bool    maximized;
private double  zoom;
private string  last_file;
private int     last_page;

public class Settings: GLib.Object {
    public void get_all() {
        settings = new GLib.Settings(APP_ID_PREF);
        get_width();
        get_height();
        get_maximized();
        get_zoom();
        get_last_file();
        get_last_page();
    }

    // settings get
    public int get_width() {
        return width = settings.get_int("width");
    }

    public int get_height() {
        return height = settings.get_int("height");
    }

    public bool get_maximized() {
        return maximized = settings.get_boolean("maximized");
    }

    public double get_zoom() {
        return zoom = settings.get_double("zoom");
    }

    public string get_last_file() {
        return last_file = settings.get_string("last-file");
    }

    public int get_last_page() {
        return last_page = settings.get_int("last-page");
    }

    // settings set
    public void set_width() {
        settings.set_int("width", width);
    }

    public void set_height() {
        settings.set_int("height", height);
    }

    public void set_maximized() {
        settings.set_boolean("maximized", maximized);
    }

    public void set_zoom() {
        settings.set_double("zoom", zoom);
    }

    public void set_last_file() {
        settings.set_string("last-file", last_file);
    }

    public void set_last_page() {
        settings.set_int("last-page", last_page);
    }

}
}

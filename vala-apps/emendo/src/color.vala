namespace Emendo {
public class Color: Gtk.Dialog {
    Gtk.ColorChooserDialog dialog;
    string selected;

    public void show_dialog() {
        if (notebook.get_n_pages() == 0) {
            return;
        }
        var rgba = Gdk.RGBA();
        dialog = new Gtk.ColorChooserDialog("Select Color", window);
        dialog.set_property("skip-taskbar-hint", true);
        get_start_color();
        if (dialog.run() == Gtk.ResponseType.OK) {
            rgba = dialog.get_rgba();
            int r = (int)Math.round(rgba.red   * 255);
            int g = (int)Math.round(rgba.green * 255);
            int b = (int)Math.round(rgba.blue  * 255);
            selected = "#%02x%02x%02x".printf(r, g, b).up();
            insert_selected_color();
        }
        dialog.close();
    }

    private void get_start_color() {
        Gtk.TextIter sel_st;
        Gtk.TextIter sel_end;
        var tabs = new Emendo.Tabs();
        var view = tabs.get_current_sourceview();
        var buffer = (Gtk.SourceBuffer) view.get_buffer();
        var rgba = Gdk.RGBA();
        if (buffer.has_selection == true) {
            buffer.get_selection_bounds(out sel_st, out sel_end);
            string text = buffer.get_text(sel_st, sel_end, false);
            if (text.length == 7) {
                rgba.parse(text);
                dialog.set_rgba(rgba);
            }
            if (text.length == 6) {
                rgba.parse("#" + text);
                sel_st.backward_char();
                buffer.select_range(sel_st, sel_end);
                dialog.set_rgba(rgba);
            }
        }
    }

    private void insert_selected_color() {
        var tabs = new Emendo.Tabs();
        var view = tabs.get_current_sourceview();
        var buffer = (Gtk.SourceBuffer) view.get_buffer();
        if (buffer.has_selection == true) {
            buffer.delete_selection(true, true);
        }
        buffer.insert_at_cursor(selected, selected.length);
    }

}
}

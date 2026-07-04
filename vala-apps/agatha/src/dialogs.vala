namespace Agatha {
public class Dialogs: Gtk.Dialog {
    private Gtk.SpinButton spinbutton;

    public void show_open() {
        var dialog = new Gtk.FileChooserDialog("Open", window,
                                               Gtk.FileChooserAction.OPEN,
                                               "Cancel", Gtk.ResponseType.CANCEL,
                                               "Open",   Gtk.ResponseType.ACCEPT);
        if (filename != null) {
            dialog.set_current_folder(Path.get_dirname(filename));
        }
        dialog.set_select_multiple(false);
        dialog.set_modal(true);
        dialog.show();
        if (dialog.run() == Gtk.ResponseType.ACCEPT) {
            filename = dialog.get_filename();
            cpage = 1;
            var viewer = new Agatha.Viewer();
            viewer.render_page();
        }
        dialog.destroy();
    }

    public void show_goto() {
        var dialog = new Gtk.Dialog();
        dialog.set_title("Go to page");
        dialog.set_border_width(10);
        dialog.set_property("skip-taskbar-hint", true);
        dialog.set_transient_for(window);
        dialog.set_resizable(false);
        spinbutton = new Gtk.SpinButton.with_range(1, total, 1);
        spinbutton.set_value(cpage);
        spinbutton.set_size_request(250, 0);
        spinbutton.value_changed.connect(() => {
            on_spinbutton_activate(dialog);
        });
        var content = dialog.get_content_area() as Gtk.Box;
        content.pack_start(spinbutton, true, true, 10);
        dialog.add_button("Close", Gtk.ResponseType.CLOSE);
        dialog.delete_event.connect(() => {
            dialog.destroy();
            return true;
        });
        dialog.show_all();
        if (dialog.run() == Gtk.ResponseType.CLOSE) {
            dialog.destroy();
        }
    }

    private void on_spinbutton_activate(Gtk.Dialog d) {
        cpage = spinbutton.get_value_as_int();
        var viewer = new Agatha.Viewer();
        viewer.render_page();
    }

    public void show_about() {
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
}
}

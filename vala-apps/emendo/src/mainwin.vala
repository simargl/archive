namespace Emendo {
private GLib.List<string> files;
private Gtk.ApplicationWindow window;
private Gtk.Notebook notebook;

public class MainWin: Gtk.ApplicationWindow {
    private const GLib.ActionEntry[] action_entries = {
        { "undo",      action_undo      },
        { "redo",      action_redo      },
        { "open",      action_open      },
        { "save",      action_save      },
        { "find",      action_find      },
        { "new",       action_new       },
        { "save-as",   action_save_as   },
        { "save-all",  action_save_all  },
        { "replace",   action_replace   },
        { "wrap",      action_wrap      },
        { "color",     action_color     },
        { "close",     action_close     },
        { "close-all", action_close_all },
        { "pref",      action_pref      },
        { "about",     action_about     },
        { "quit",      action_quit      }
    };

    public void add_main_window(Gtk.Application app) {
        files = new GLib.List<string>();
        var settings = new Emendo.Settings();
        settings.get_all();
        app.add_action_entries(action_entries, app);
        // app menu
        var menu = new GLib.Menu();
        var section = new GLib.Menu();
        section.append("New",            "app.new");
        section.append("Open",           "app.open");
        section.append("Save",           "app.save");
        section.append("Save As...",     "app.save-as");
        section.append("Save All",       "app.save-all");
        menu.append_section(null, section);
        section = new GLib.Menu();
        section.append("Find...",   	    "app.find");
        section.append("Replace...",   	"app.replace");
        section.append("Text Wrap", 		"app.wrap");
        section.append("Select Color", 	"app.color");
        menu.append_section(null, section);
        section = new GLib.Menu();
        section.append("Close",          "app.close");
        section.append("Close All",      "app.close-all");
        menu.append_section(null, section);
        section = new GLib.Menu();
        section.append("Preferences",    "app.pref");
        menu.append_section(null, section);
        section = new GLib.Menu();
        section.append("About",          "app.about");
        section.append("Quit",           "app.quit");
        menu.append_section(null, section);
        app.set_app_menu(menu);
        // window
        notebook = new Gtk.Notebook();
        notebook.expand = true;
        notebook.popup_enable();
        notebook.set_scrollable(true);
        notebook.switch_page.connect(on_notebook_page_switched);
        notebook.page_reordered.connect(on_page_reordered);
        window = new Gtk.ApplicationWindow(app);
        window.window_position = Gtk.WindowPosition.CENTER;
        window.set_default_size(width, height);
        window.set_icon_name(ICON);
        window.set_title(NAME);
        if (maximized == true) {
            window.maximize();
        }
        window.add(notebook);
        window.show_all();
        window.window_state_event.connect((w, e) => {
            maximized = (e.new_window_state & Gdk.WindowState.MAXIMIZED) != 0;
            return false;
        });
        window.delete_event.connect(() => {
            action_quit();
            return false;
        });
    }

    private void on_notebook_page_switched(Gtk.Widget page, uint page_num) {
        var tabs = new Emendo.Tabs();
        string path = tabs.get_path_at_tab((int)page_num);
        window.set_title(path);
    }

    void on_page_reordered (Gtk.Widget page, uint pagenum) {
        // full path is in the tooltip text of Tab Label
        var tabs = new Emendo.Tabs();
        Gtk.Label l = tabs.get_label_at_tab((int)pagenum);
        string path = l.get_tooltip_text();
        // find and update file's position in GLib.List
        for (int i = 0; i < files.length(); i++) {
            if (files.nth_data(i) == path) {
                // remove from files list
                unowned List<string> del_item = files.find_custom (path, strcmp);
                files.remove_link(del_item);
                // insert in new position
                files.insert(path, (int)pagenum);
            }
        }
        for (int i = 0; i < files.length(); i++) {
            print("NEW LIST %s\n", files.nth_data(i));
        }
    }

    public void action_app_quit() {
        window.get_application().quit();
    }

    private void action_undo() {
        var operations = new Emendo.Operations();
        operations.undo_last();
    }

    private void action_redo() {
        var operations = new Emendo.Operations();
        operations.redo_last();
    }

    // buttons
    private void action_open() {
        var dialogs = new Emendo.Dialogs();
        dialogs.show_open();
    }

    private void action_save() {
        var operations = new Emendo.Operations();
        operations.save_current();
    }

    private void action_find() {
        var find = new Emendo.Find();
        find.show_dialog();
    }

    // gear menu
    private void action_new() {
        var nbook = new Emendo.NBook();
        nbook.create_tab("/tmp/untitled");
    }

    private void action_save_as() {
        var dialogs = new Emendo.Dialogs();
        dialogs.show_save();
    }

    private void action_save_all() {
        var operations = new Emendo.Operations();
        operations.save_all();
    }

    private void action_replace() {
        var replace = new Emendo.Replace();
        replace.show_dialog();
    }

    private void action_wrap() {
        var operations = new Emendo.Operations();
        operations.wrap_text();
    }

    private void action_color() {
        var color = new Emendo.Color();
        color.show_dialog();
    }

    private void action_close() {
        var operations = new Emendo.Operations();
        operations.close_tab();
    }

    private void action_close_all() {
        var operations = new Emendo.Operations();
        operations.close_all_tabs();
    }

    // app menu
    private void action_pref() {
        var prefdialog = new Emendo.PrefDialog();
        prefdialog.on_activate();
    }

    private void action_about() {
        var dialogs = new Emendo.Dialogs();
        dialogs.show_about();
    }

    public void action_quit() {
        window.get_size(out width, out height);
        active_tab = notebook.get_current_page();
        var settings = new Emendo.Settings();
        settings.set_width();
        settings.set_height();
        settings.set_maximized();
        settings.set_active_tab();
        GLib.Settings.sync();
        var dialogs = new Emendo.Dialogs();
        dialogs.changes_all();
    }

}
}

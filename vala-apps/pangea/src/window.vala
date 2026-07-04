namespace Pangea {
public class Window: Gtk.ApplicationWindow {
    private const GLib.ActionEntry[] action_entries = {
        { "go-home",       action_go_to_home_directory },
        { "create-folder", action_create_folder        },
        { "create-file",   action_create_file          },
        { "add-bookmark",  action_add_bookmark         },
        { "terminal",      action_terminal             },
        { "preferences",   action_preferences          },
        { "cut",           action_cut                  },
        { "copy",          action_copy                 },
        { "rename",        action_rename               },
        { "delete",        action_delete               },
        { "properties",    action_properties           },
        { "paste",         action_paste                },
        { "about",         action_about                },
        { "quit",          action_quit                 }
    };

    public void add_app_window(Gtk.Application app) {
        app.add_action_entries(action_entries, app);
        var menu = new GLib.Menu();
        var section = new GLib.Menu();
        section.append("Create folder", "app.create-folder");
        section.append("Create file",  "app.create-file");
        menu.append_section(null, section);
        section = new GLib.Menu();
        section.append("Paste", "app.paste");
        menu.append_section(null, section);
        section = new GLib.Menu();
        section.append("Add to Bookmarks", "app.add-bookmark");
        section.append("Open in Terminal", "app.terminal");
        menu.append_section(null, section);
        section = new GLib.Menu();
        section.append("Preferences", "app.preferences");
        menu.append_section(null, section);
        section = new GLib.Menu();
        section.append("About", "app.about");
        section.append("Quit",  "app.quit");
        menu.append_section(null, section);
        app.set_app_menu(menu);
        new Pangea.Settings().get_settings();
        list_dir_history = new GLib.List<string>();
        window = new Gtk.ApplicationWindow(app);
        add_widgets(window);
        connect_signals(window);
        var tmpfile = GLib.File.new_for_path("/tmp/.pangea.lock");
        try {
            FileOutputStream os = tmpfile.create(FileCreateFlags.NONE);
            os.close();
        } catch (Error e) {
            stdout.printf ("Error: %s\n", e.message);
        }
    }

    private void add_widgets(Gtk.ApplicationWindow appwindow) {
        grid_devices = new Gtk.Grid();
        grid_bookmarks = new Gtk.Grid();
        action_add_devices_list();
        action_add_bookmarks_list();
        var scrolled_devices = new Gtk.ScrolledWindow(null, null);
        scrolled_devices.add(grid_devices);
        scrolled_devices.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC);
        scrolled_devices.vexpand = true;
        scrolled_devices.width_request = 180;
        var scrolled_bookmarks = new Gtk.ScrolledWindow(null, null);
        scrolled_bookmarks.add(grid_bookmarks);
        scrolled_bookmarks.set_policy(Gtk.PolicyType.AUTOMATIC,
                                      Gtk.PolicyType.AUTOMATIC);
        scrolled_bookmarks.vexpand = true;
        scrolled_bookmarks.width_request = 180;
        // Dropdown Menu
        combo_path_menu = new Gtk.Menu();
        var combo_path = new Gtk.MenuButton();
        combo_path.set_relief(Gtk.ReliefStyle.NONE);
        combo_path.set_popup(combo_path_menu);
        combo_path.set_image(new Gtk.Image.from_icon_name("go-down",
                             Gtk.IconSize.MENU));
        var button_up = new Gtk.Button();
        button_up.set_always_show_image(true);
        button_up.set_image(new Gtk.Image.from_icon_name("go-up",
                            Gtk.IconSize.MENU));
        button_up.set_relief(Gtk.ReliefStyle.NONE);
        button_up.clicked.connect(() => {
            action_go_to_up_directory();
        });
        var button_home = new Gtk.Button();
        button_home.set_always_show_image(true);
        button_home.set_image(new Gtk.Image.from_icon_name("go-home",
                              Gtk.IconSize.MENU));
        button_home.set_relief(Gtk.ReliefStyle.NONE);
        button_home.clicked.connect(() => {
            action_go_to_home_directory();
        });
        var buttons_grid = new Gtk.Grid();
        buttons_grid.attach(combo_path,          0, 0, 1, 1);
        buttons_grid.attach(button_up,           1, 0, 1, 1);
        buttons_grid.attach(button_home,         2, 0, 1, 1);
        var separator = new Gtk.Separator(Gtk.Orientation.HORIZONTAL);
        pane = new Gtk.Paned (Gtk.Orientation.VERTICAL);
        pane.add1(scrolled_devices);
        pane.add2(scrolled_bookmarks);
        pane.set_position(paned_pos);
        var places_grid = new Gtk.Grid();
        places_grid.attach(buttons_grid,    0, 0, 1, 1);
        places_grid.attach(separator,       0, 1, 1, 1);
        places_grid.attach(pane,            0, 2, 1, 1);
        model = new Gtk.ListStore(4, typeof (Gdk.Pixbuf), typeof (string),
                                  typeof (string), typeof (string));
        view = new Gtk.IconView.with_model(model);
        view.set_pixbuf_column(0);
        view.set_text_column(1);
        view.set_tooltip_column(3);
        view.set_column_spacing(3);
        view.set_item_width(icon_size + 24);
        view.set_activate_on_single_click(true);
        view.set_selection_mode(Gtk.SelectionMode.MULTIPLE);
        view.enable_model_drag_source(Gdk.ModifierType.BUTTON1_MASK, targets,
                                      Gdk.DragAction.COPY);
        view.drag_data_get.connect(drag_source_operation);
        view.drag_data_received.connect(on_drag_data_received);
        Gtk.drag_dest_set(view, Gtk.DestDefaults.ALL, targets, Gdk.DragAction.COPY);
        location_entry = new Gtk.Entry();
        location_entry.expand = false;
        var scrolled = new Gtk.ScrolledWindow(null, null);
        scrolled.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC);
        scrolled.add(view);
        scrolled.expand = true;
        scrolled.width_request = 250;
        statusbar = new Gtk.Statusbar();
        context_id = statusbar.get_context_id("pangea");
        statusbar.push(context_id, "Starting up...");
        var location_grid = new Gtk.Grid();
        location_grid.attach(location_entry, 0, 0, 1, 1);
        location_grid.attach(scrolled,       0, 1, 1, 1);
        var grid = new Gtk.Grid();
        grid.attach(places_grid,    0, 0, 1, 1);
        grid.attach(location_grid,     1, 0, 1, 1);
        grid.attach(statusbar,      0, 1, 2, 1);
        window.add(grid);
        window.set_title(NAME);
        window.set_default_size(width, height);
        window.set_icon_name(ICON);
        window.show_all();
    }

    private void action_add_devices_list() {
        new Pangea.Operations().add_devices_grid();
    }

    private void action_add_bookmarks_list() {
        new Pangea.Operations().add_bookmarks_grid();
    }

    private void drag_source_operation(Gdk.DragContext ctx, Gtk.SelectionData data,
                                       uint info, uint time_) {
        var path = view.get_selected_items().nth_data(0);
        var model = view.get_model();
        Gtk.TreeIter iter;
        model.get_iter(out iter, path);
        GLib.Value filepath;
        model.get_value(iter, 2, out filepath);
        string[] uris = {""};
        uris[0] = filepath.get_string();
        data.set_uris(uris);
    }

    private void on_drag_data_received (Gdk.DragContext ctx, int x, int y,
                                        Gtk.SelectionData data, uint info, uint time) {
        print("received");
    }

    private void connect_signals(Gtk.ApplicationWindow appwindow) {
        view.item_activated.connect(() => {
            (new Pangea.IconView().icon_clicked());
        });
        view.selection_changed.connect(() => {
            (new Pangea.IconView().on_selection_changed());
        });
        view.key_press_event.connect(on_key_press_event);
        view.button_press_event.connect(context_menu_activate);
        location_entry.activate.connect(() => {
            new Pangea.IconView().open_location(GLib.File.new_for_path(
                                                    location_entry.get_text()), true);
        });
        window.delete_event.connect(() => {
            action_quit();
            return true;
        });
    }

    private bool on_key_press_event(Gdk.EventKey event) {
        if (Gdk.keyval_name(event.keyval) == "BackSpace") {
            action_go_to_up_directory();
        }
        if (Gdk.keyval_name(event.keyval) == "Escape") {
            view.unselect_all();
        }
        string match = event.str;
        if (match != "") {
            var model = view.get_model();
            GLib.Value val;
            var paths = new List<Gtk.TreePath>();
            int i = 0;
            while (i < model.iter_n_children(null)) {
                var path = new Gtk.TreePath.from_indices(i, -1);
                model.get_iter(out iter, path);
                model.get_value(iter, 3, out val);
                string strval = val.get_string();
                if (strval.substring(0, 1).down() == match.down()) {
                    paths.append(path);
                }
                i++;
            }
            if (paths.length() == 0) {
                return false;
            }
            var test = view.get_selected_items().nth_data(0);
            if (test == null) {
                view.select_path(paths.first().data);
                view.set_cursor(paths.first().data, null, false);
                view.scroll_to_path(paths.first().data, false, 0, 0);
            } else {
                uint len = paths.length();
                unowned List<Gtk.TreePath> last = paths.nth(len-1);
                paths.delete_link(last);
                foreach(Gtk.TreePath j in paths) {
                    if (j.to_string() == test.to_string()) {
                        j.next();
                        view.unselect_path(test);
                        view.select_path(j);
                        view.set_cursor(j, null, false);
                        view.scroll_to_path(j, false, 0, 0);
                    }
                }
            }
        }
        return false;
    }

    private bool context_menu_activate(Gdk.EventButton event) {
        Gtk.TreePath? path = null;
        if (event.button == 8) {
            action_go_to_up_directory();
        }
        if (event.button == 9) {
            action_go_to_previous_directory();
        }        
        if (event.button == 3) {
            var selection = new GLib.List<string>();
            selection = new Pangea.Operations().get_files_selection();
            uint len_s = selection.length();
            if (len_s == 1) {
                view.unselect_all();
            }
            path = view.get_path_at_pos((int) event.x, (int) event.y);
            if (path != null) {
                view.select_path(path);
                menu = new Pangea.Menu().activate_file_menu();
                menu.popup (null, null, null, event.button, event.time);
            } else {
                view.unselect_all();
                menu = new Pangea.Menu().activate_context_menu();
                menu.popup (null, null, null, event.button, event.time);
            }
        }
        return false;
    }

    private void action_create_folder() {
        new Pangea.Operations().make_new(false);
    }

    private void action_create_file() {
        new Pangea.Operations().make_new(true);
    }

    private void action_add_bookmark() {
        new Pangea.Operations().add_bookmark();
    }

    private void action_go_to_home_directory() {
        string home = GLib.Environment.get_home_dir();
        new Pangea.IconView().open_location(GLib.File.new_for_path(home), true);
    }

    private void action_go_to_up_directory() {
        previous_dir = current_dir;
        new Pangea.IconView().open_location(GLib.File.new_for_path(
                                                GLib.Path.get_dirname(current_dir)), true);
    }

    private void action_go_to_previous_directory() {
        new Pangea.IconView().open_location(GLib.File.new_for_path(previous_dir), true);
    }

    private void action_terminal() {
        new Pangea.Operations().execute_command_async("%s".printf(terminal));
    }

    private void action_preferences() {
        new Pangea.Operations().show_preferences_dialog();
    }

    private void action_cut() {
        new Pangea.Operations().file_cut_activate();
    }

    private void action_copy() {
        new Pangea.Operations().file_copy_activate();
    }

    private void action_rename() {
        new Pangea.Operations().file_rename_activate();
    }

    private void action_delete() {
        new Pangea.Operations().file_delete_activate();
    }

    private void action_properties() {
        new Pangea.Operations().file_properties_activate();
    }

    private void action_paste() {
        new Pangea.Operations().file_paste_activate();
    }

    private void action_about() {
        var about = new Gtk.AboutDialog();
        about.set_program_name(NAME);
        about.set_version(VERSION);
        about.set_comments(DESCRIPTION);
        about.set_logo_icon_name(ICON);
        about.set_authors(AUTHORS);
        about.set_copyright("Copyright \xc2\xa9 2018");
        about.set_website("https://github.com/simargl");
        about.set_property("skip-taskbar-hint", true);
        about.set_transient_for(window);
        about.license_type = Gtk.License.GPL_3_0;
        about.run();
        about.hide();
    }

    private void action_quit() {
        File tmpfile = File.new_for_path("/tmp/.pangea.lock");
        try {
            tmpfile.delete();
        } catch (Error e) {
            stdout.printf("Error: %s\n", e.message);
        }
        new Pangea.Settings().save_settings();
        window.get_application().quit();
    }
}
}

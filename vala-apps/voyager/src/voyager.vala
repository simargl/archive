/*  Author: simargl <https://github.com/simargl>
 *  License: GPL v3
 */

private class Program : Gtk.Application {
    const string NAME        = "Voyager";
    const string VERSION     = "1.4.0";
    const string DESCRIPTION = "Fast and elegant image browser";
    const string ICON        = "eog";
    const string[] AUTHORS   = { "Simargl <https://github.com/simargl>", null };

    Gtk.Image image;
    Gdk.Pixbuf pixbuf;
    Gdk.Pixbuf pixbuf_scaled;
    Gtk.ApplicationWindow window;
    Gtk.TreeView treeview;
    Gtk.ListStore liststore;
    Gtk.ScrolledWindow scrolled_window_image;
    Gtk.ScrolledWindow scrolled_window_treeview;
    int pixbuf_width;
    int pixbuf_height;
    int saved_pixbuf_width;
    int saved_pixbuf_height;
    bool slideshow_active;
    bool list_visible;
    string file;
    string basename;
    private uint timeout_id;
    private const Gtk.TargetEntry[] targets = { {"text/uri-list", 0, 0} };

    int x_start;
    int y_start;
    int x_current;
    int y_current;
    int x_end;
    int y_end;
    bool dragging;

    double hadj_value;
    double vadj_value;
    double zoom = 1.00;

    Gtk.Adjustment hadj;
    Gtk.Adjustment vadj;

    private const GLib.ActionEntry[] action_entries = {
        { "next-image",         action_next_image         },
        { "previous-image",     action_previous_image     },
        { "open",               action_open               },
        { "set-as-wallpaper",   action_set_as_wallpaper   },
        { "toggle-thumbnails",  action_reveal_thumbnails  },
        { "rotate-right",       action_rotate_right       },
        { "rotate-left",        action_rotate_left        },
        { "zoom-in",            action_zoom_in            },
        { "zoom-out",           action_zoom_out           },
        { "zoom-to-fit",        action_zoom_to_fit        },
        { "zoom-actual",        action_zoom_actual        },
        { "slideshow",          action_slideshow          },
        { "edit-with-gimp",     action_edit_with_gimp     },
        { "full-screen-toggle", action_full_screen_toggle },
        { "full-screen-exit",   action_full_screen_exit   },
        { "about",              action_about              },
        { "quit",               action_quit               }
    };

    private Program() {
        Object(application_id: "org.vala-apps.voyager",
               flags: ApplicationFlags.HANDLES_OPEN);
        add_action_entries(action_entries, this);
    }

    public override void startup() {
        base.startup();
        add_accelerator("O",            "app.open", null);
        add_accelerator("W",            "app.set-as-wallpaper", null);
        add_accelerator("T",            "app.toggle-thumbnails", null);
        add_accelerator("Page_Down",    "app.rotate-right", null);
        add_accelerator("Page_Up",      "app.rotate-left", null);
        add_accelerator("Escape",       "app.full-screen-exit", null);
        add_accelerator("KP_Add",       "app.zoom-in", null);
        add_accelerator("KP_Subtract",  "app.zoom-out", null);
        add_accelerator("KP_0",         "app.zoom-actual", null);
        add_accelerator("F11",          "app.full-screen-toggle", null);
        add_accelerator("F10",          "app.show-menu", null);
        add_accelerator("Q",            "app.quit", null);
        /*
        set_accels_for_action("app.open",                  {"O", "<Primary>O"});
        set_accels_for_action("app.set-as-wallpaper",      {"W", "<Primary>W"});
        set_accels_for_action("app.toggle-thumbnails",     {"T", "<Primary>T"});
        set_accels_for_action("app.rotate-right",          {"Page_Down", "<Primary>Page_Down"});
        set_accels_for_action("app.rotate-left",           {"Page_Up", "<Primary>Page_Up"});
        set_accels_for_action("app.full-screen-exit",      {"Escape"});
        set_accels_for_action("app.zoom-in",               {"KP_Add"});
        set_accels_for_action("app.zoom-out",              {"KP_Subtract"});
        set_accels_for_action("app.zoom-actual",           {"KP_0"});
        set_accels_for_action("app.full-screen-toggle",    {"F11"});
        set_accels_for_action("app.show-menu",             {"F10"});
        set_accels_for_action("app.quit",                  {"Q", "<Primary>Q"});
        */
        var menu = new Menu();
        var section = new GLib.Menu();
        section.append("Open",               "app.open");
        section.append("Set as Wallpaper",   "app.set-as-wallpaper");
        section.append("Thumbnails On/Off",  "app.toggle-thumbnails");
        menu.append_section(null, section);
        section = new GLib.Menu();
        section.append("Rotate Right",       "app.rotate-right");
        section.append("Rotate Left",        "app.rotate-left");
        menu.append_section(null, section);
        section = new GLib.Menu();
        section.append("Zoom In",            "app.zoom-in");
        section.append("Zoom Out",           "app.zoom-out");
        section.append("Zoom to Fit",        "app.zoom-to-fit");
        section.append("Actual Size",        "app.zoom-actual");
        menu.append_section(null, section);
        section = new GLib.Menu();
        section.append("Slideshow",          "app.slideshow");
        section.append("Edit With Gimp",     "app.edit-with-gimp");
        menu.append_section(null, section);
        section = new GLib.Menu();
        section.append("About",              "app.about");
        section.append("Quit",               "app.quit");
        menu.append_section(null, section);
        set_app_menu(menu);
        image = new Gtk.Image();
        // TreeView
        liststore = new Gtk.ListStore(3, typeof (Gdk.Pixbuf), typeof (string),
                                      typeof (string));
        treeview = new Gtk.TreeView();
        treeview.set_model(liststore);
        treeview.set_tooltip_column(2);
        treeview.set_headers_visible(false);
        treeview.set_activate_on_single_click(true);
        treeview.row_activated.connect(show_selected_image);
        treeview.insert_column_with_attributes (-1, "Preview",
                                                new Gtk.CellRendererPixbuf(), "pixbuf", 0);
        // ScrolledWindow
        scrolled_window_image = new Gtk.ScrolledWindow(null, null);
        scrolled_window_image.set_policy(Gtk.PolicyType.AUTOMATIC,
                                         Gtk.PolicyType.AUTOMATIC);
        scrolled_window_image.expand = true;
        scrolled_window_image.set_size_request(200, 0);
        scrolled_window_image.add(image);
        scrolled_window_image.scroll_event.connect(button_scroll_event);
        var eventbox_image = new Gtk.EventBox();
        eventbox_image.add(scrolled_window_image);
        eventbox_image.show();
        eventbox_image.add_events(Gdk.EventMask.POINTER_MOTION_MASK
                                  | Gdk.EventMask.BUTTON_PRESS_MASK
                                  | Gdk.EventMask.BUTTON_RELEASE_MASK);
        eventbox_image.button_press_event.connect(button_press_event);
        eventbox_image.motion_notify_event.connect(button_motion_event);
        eventbox_image.button_release_event.connect(button_release_event);
        scrolled_window_treeview = new Gtk.ScrolledWindow(null, null);
        scrolled_window_treeview.set_policy(Gtk.PolicyType.AUTOMATIC,
                                            Gtk.PolicyType.AUTOMATIC);
        scrolled_window_treeview.set_size_request(200, 0);
        scrolled_window_treeview.add(treeview);
        var grid = new Gtk.Grid();
        grid.attach(scrolled_window_treeview, 0, 0, 1, 1);
        grid.attach(eventbox_image, 1, 0, 1, 1);
        Gtk.drag_dest_set(grid, Gtk.DestDefaults.ALL, targets, Gdk.DragAction.COPY);
        grid.drag_data_received.connect(on_drag_data_received);
        // Window
        window = new Gtk.ApplicationWindow(this);
        window.add(grid);
        window.set_title(NAME);
        window.set_default_size(640, 550);
        window.set_icon_name(ICON);
        window.show_all();
        window.delete_event.connect(() => {
            quit();
            return true;
        });
        window.key_press_event.connect(on_key_press_event);
        add_window(window);
        hadj = scrolled_window_image.get_hadjustment();
        vadj = scrolled_window_image.get_vadjustment();
        list_visible = false;
        scrolled_window_treeview.hide();
    }

    public override void activate() {
        window.present();
    }

    public override void open(File[] files, string hint) {
        activate();
        foreach (File f in files) {
            file = f.get_path();
        }
        GLib.Idle.add(() => {
            load_pixbuf_on_start();
            return false;
        });
        list_images(Path.get_dirname(file));
    }

    private bool on_key_press_event(Gdk.EventKey event) {
        if (Gdk.keyval_name(event.keyval) == "Left" || Gdk.keyval_name(event.keyval) == "Up") {
            action_previous_image();
        }
        if (Gdk.keyval_name(event.keyval) == "Right" || Gdk.keyval_name(event.keyval) == "Down") {
            action_next_image();
        }
        return false;
    }

    // Treeview
    private void list_images(string directory) {
        try {
            liststore.clear();
            Environment.set_current_dir(directory);
            var d = File.new_for_path(directory);
            var enumerator = d.enumerate_children(FileAttribute.STANDARD_NAME, 0);
            FileInfo info;
            while((info = enumerator.next_file()) != null) {
                string output = info.get_name();
                var file_check = File.new_for_path(output);
                var file_info = file_check.query_info("standard::content-type", 0, null);
                string content = file_info.get_content_type();
                if ( content.contains("image")) {
                    string fullpath = directory + "/" + output;
                    Gdk.Pixbuf pixbuf = null;
                    Gtk.TreeIter? iter = null;
                    load_thumbnail.begin(fullpath, (obj, res) => {
                        pixbuf = load_thumbnail.end(res);
                        liststore.append(out iter);
                        liststore.set(iter, 0, pixbuf, 1, fullpath, 2, output, -1);
                        if (file == fullpath) {
                            treeview.get_selection().select_iter(iter);
                            Gtk.TreePath path = treeview.get_model().get_path(iter);
                            treeview.scroll_to_cell(path, null, false, 0, 0);
                        }
                    });
                }
            }
            treeview.grab_focus();
        } catch(Error e) {
            stderr.printf("Error: %s\n", e.message);
        }
    }

    private async Gdk.Pixbuf load_thumbnail(string name) {
        Gdk.Pixbuf? pix = null;
        var file = GLib.File.new_for_path(name);
        try {
            GLib.InputStream stream = yield file.read_async();
            pix = yield new Gdk.Pixbuf.from_stream_at_scale_async(stream, 140, 100, true,
                    null);
        } catch (Error e) {
            stderr.printf("%s\n", e.message);
        }
        return pix;
    }

    private void show_selected_image() {
        Gtk.TreeIter iter;
        Gtk.TreeModel model;
        var selection = treeview.get_selection();
        selection.get_selected(out model, out iter);
        model.get(iter, 1, out file);
        load_pixbuf_on_start();
    }

    private void update_title() {
        basename = Path.get_basename(file);
        double izoom = zoom * 100;
        window.set_title("%s (%sx%s) %s%s".printf(basename,
                         pixbuf.get_width().to_string(), pixbuf.get_height().to_string(),
                         Math.round(izoom).to_string(), "%"));
    }

    // load pixbuf with full size
    private void load_pixbuf_full() {
        try {
            pixbuf = new Gdk.Pixbuf.from_file(file);
            pixbuf_scaled = pixbuf;
            zoom = 1.00;
            image.set_from_pixbuf(pixbuf);
            update_title();
        } catch(Error error) {
            stderr.printf("error: %s\n", error.message);
        }
    }

    // load pixbuf with specified size, if width is smaller than 400px - then load at full size
    private void load_pixbuf_with_size(int pixbuf_width, int pixbuf_height) {
        try {
            pixbuf = new Gdk.Pixbuf.from_file(file);
            if (pixbuf.get_width() <= 400) {
                pixbuf_scaled = pixbuf;
                zoom = 1.00;
                image.set_from_pixbuf(pixbuf);
            } else {
                try {
                    pixbuf_scaled = new Gdk.Pixbuf.from_file_at_size(file, pixbuf_width,
                            pixbuf_width);
                    zoom = (double)pixbuf_scaled.get_width() / pixbuf.get_width();
                    image.set_from_pixbuf(pixbuf_scaled);
                } catch(Error error) {
                    stderr.printf("error: %s\n", error.message);
                }
            }
            update_title();
        } catch(Error error) {
            stderr.printf("error: %s\n", error.message);
        }
    }

    // load pixbuf on start
    private void load_pixbuf_on_start() {
        int width, height;
        window.get_size(out width, out height);
        if ((window.get_window().get_state() & Gdk.WindowState.FULLSCREEN) != 0) {
            pixbuf_width = width;
            pixbuf_height = height;
        } else {
            pixbuf_width = scrolled_window_image.get_allocated_width();
            pixbuf_height = scrolled_window_image.get_allocated_height();
        }
        load_pixbuf_with_size(pixbuf_width, pixbuf_height);
    }

    private void zoom_image(bool plus, double large, double small) {
        double curr_width = pixbuf_scaled.get_width();
        double curr_height = pixbuf_scaled.get_height();
        double real_width = pixbuf.get_width();
        double real_height = pixbuf.get_height();
        double zoom_factor;
        if (real_width < 100) {
            zoom_factor = large;
        } else {
            zoom_factor = small;
        }
        if (plus == true) {
            zoom = zoom + zoom_factor;
            pixbuf_scaled = pixbuf.scale_simple((int)(real_width * zoom),
                                                (int)(real_height * zoom), Gdk.InterpType.BILINEAR);
        } else {
            if (curr_height > real_height * 0.02 && curr_width > real_width * 0.02) {
                zoom = zoom - zoom_factor;
                pixbuf_scaled = pixbuf.scale_simple((int)(real_width * zoom),
                                                    (int)(real_height * zoom), Gdk.InterpType.BILINEAR);
            }
        }
        image.set_from_pixbuf(pixbuf_scaled);
        update_title();
    }

    // Mouse EventButton Press
    private bool button_press_event(Gdk.EventButton event) {
        if (file != null) {
            if (event.type == Gdk.EventType.2BUTTON_PRESS && event.button == 1) {
                if ((window.get_window().get_state() & Gdk.WindowState.FULLSCREEN) != 0) {
                    action_full_screen_exit();
                } else {
                    action_full_screen_toggle();
                }
            }
        }
        if (event.button == 1) {
            var device = Gtk.get_current_event_device();
            if(device != null) {
                event.window.get_device_position(device, out x_start, out y_start, null);
                event.window.set_cursor(new Gdk.Cursor.for_display(Gdk.Display.get_default(),
                                        Gdk.CursorType.FLEUR));
            }
            dragging = true;
            hadj_value = hadj.get_value();
            vadj_value = vadj.get_value();
        }
        if (event.button == 2) {
            action_previous_image();
        }
        if (event.button == 3) {
            action_next_image();
        }
        return false;
    }

    // motion
    private bool button_motion_event(Gdk.EventMotion event) {
        if (dragging == true) {
            var device = Gtk.get_current_event_device();
            if (device != null) {
                event.window.get_device_position(device, out x_current, out y_current, null);
            }
            int x_diff = x_start - x_current;
            int y_diff = y_start - y_current;
            hadj.set_value(hadj_value + x_diff);
            vadj.set_value(vadj_value + y_diff);
        }
        return false;
    }

    // release
    private bool button_release_event(Gdk.EventButton event) {
        if (event.type == Gdk.EventType.BUTTON_RELEASE) {
            if (event.button == 1) {
                var device = Gtk.get_current_event_device();
                if(device != null) {
                    event.window.get_device_position(device, out x_end, out y_end, null);
                    event.window.set_cursor(null);
                }
                dragging = false;
            }
        }
        return false;
    }

    // Mouse EventButton Scroll
    private bool button_scroll_event(Gdk.EventScroll event) {
        if (file != null) {
            switch(event.direction) {
            case Gdk.ScrollDirection.UP:
                zoom_image(true, 0.20, 0.04);
                break;
            case Gdk.ScrollDirection.DOWN:
                zoom_image(false, 0.20, 0.04);
                break;
            }
            return true;
        }
        return true;
    }

    // Drag Data
    private void on_drag_data_received(Gdk.DragContext drag_context, int x, int y,
                                       Gtk.SelectionData data, uint info, uint time) {
        foreach(string uri in data.get_uris()) {
            file = uri.replace("file://", "");
            file = Uri.unescape_string(file);
            load_pixbuf_on_start();
            list_images(Path.get_dirname(file));
        }
        Gtk.drag_finish(drag_context, true, false, time);
    }

    private void action_reveal_thumbnails() {
        if (list_visible == false) {
            scrolled_window_treeview.show();
            list_visible = true;
        } else {
            scrolled_window_treeview.hide();
            list_visible = false;
        }
    }

    private void action_next_image() {
        if (file != null) {
            Gtk.TreeIter iter;
            Gtk.TreeModel model;
            var selection = treeview.get_selection();
            selection.get_selected(out model, out iter);
            if (model.iter_next(ref iter)) {
                selection.select_iter(iter);
            } else {
                model.get_iter_first(out iter);
                selection.select_iter(iter);
            }
            treeview.scroll_to_cell(model.get_path(iter), null, false, 0.0f, 0.0f);
            show_selected_image();
        }
    }

    private void action_previous_image() {
        if (file != null) {
            Gtk.TreeIter iter;
            Gtk.TreeModel model;
            int children;
            var selection = treeview.get_selection();
            selection.get_selected(out model, out iter);
            if (model.iter_previous(ref iter)) {
                selection.select_iter(iter);
            } else {
                children = model.iter_n_children(null);
                model.iter_nth_child(out iter, null, children - 1);
                selection.select_iter(iter);
            }
            treeview.scroll_to_cell(model.get_path(iter), null, false, 0.0f, 0.0f);
            show_selected_image();
        }
    }

    private void action_open() {
        Gtk.FileFilter filter = new Gtk.FileFilter();
        filter.add_mime_type("image/*");
        var dialog = new Gtk.FileChooserDialog("Open", window,
                                               Gtk.FileChooserAction.OPEN,
                                               "gtk-cancel", Gtk.ResponseType.CANCEL,
                                               "gtk-open", Gtk.ResponseType.ACCEPT);
        dialog.set_transient_for(window);
        dialog.set_select_multiple(false);
        dialog.set_filter(filter);
        if (file != null) {
            dialog.set_current_folder(Path.get_dirname(file));
        }
        if (dialog.run() == Gtk.ResponseType.ACCEPT) {
            file = dialog.get_filename();
            load_pixbuf_on_start();
            list_images(Path.get_dirname(file));
        }
        dialog.destroy();
    }

    private void action_set_as_wallpaper() {
        if (file != null) {
            bool test_feh = GLib.FileUtils.test("/usr/bin/feh", FileTest.IS_EXECUTABLE);
            if (test_feh == true) {
                try {
                    Process.spawn_command_line_async("feh --bg-scale \'%s\'".printf(file));
                } catch(Error error) {
                    stderr.printf("error: %s\n", error.message);
                }
                return;
            }
            bool test_xsri = GLib.FileUtils.test("/usr/bin/xsri", FileTest.IS_EXECUTABLE);
            bool test_xsri_local = GLib.FileUtils.test("/usr/local/bin/xsri",
                                   FileTest.IS_EXECUTABLE);
            if (test_xsri == true || test_xsri_local == true) {
                try {
                    Process.spawn_command_line_async("xsri --center-x --center-y --scale-width=100 --scale-height=100 \'%s\'".printf(
                                                         file));
                } catch(Error error) {
                    stderr.printf("error: %s\n", error.message);
                }
                return;
            }
            bool test_schemas =
                GLib.FileUtils.test("/usr/share/glib-2.0/schemas/org.gnome.desktop.background.gschema.xml",
                                    FileTest.IS_REGULAR);
            if (test_schemas == true) {
                var gnome_settings = new GLib.Settings("org.gnome.desktop.background");
                gnome_settings.set_string("picture-uri", file);
                GLib.Settings.sync();
                try {
                    Process.spawn_command_line_sync("wpset-shell --set");
                } catch(Error error) {
                    stderr.printf("error: %s\n", error.message);
                }
            } else {
                Gtk.MessageDialog msg = new Gtk.MessageDialog (window, Gtk.DialogFlags.MODAL,
                        Gtk.MessageType.WARNING, Gtk.ButtonsType.OK,
                        "Error: Settings schema 'org.gnome.desktop.background' is not installed");
                msg.response.connect ((response_id) => {
                    switch (response_id) {
                    case Gtk.ResponseType.OK:
                        break;
                    }
                    msg.destroy();
                });
                msg.show ();
            }
        }
    }

    private void action_rotate_right() {
        pixbuf_scaled = pixbuf_scaled.rotate_simple(Gdk.PixbufRotation.CLOCKWISE);
        image.set_from_pixbuf(pixbuf_scaled);
    }

    private void action_rotate_left() {
        pixbuf_scaled = pixbuf_scaled.rotate_simple(
                            Gdk.PixbufRotation.COUNTERCLOCKWISE);
        image.set_from_pixbuf(pixbuf_scaled);
    }

    private void action_zoom_in() {
        if (file != null) {
            zoom_image(true, 0.50, 0.20);
        }
    }

    private void action_zoom_out() {
        if (file != null) {
            zoom_image(false, 0.50, 0.20);
        }
    }

    private void action_zoom_to_fit() {
        if (file != null) {
            load_pixbuf_on_start();
        }
    }

    private void action_zoom_actual() {
        if (file != null) {
            load_pixbuf_full();
        }
    }

    private void action_slideshow() {
        if (file != null) {
            if (slideshow_active == false) {
                timeout_id = GLib.Timeout.add(2000,
                                              (GLib.SourceFunc)action_next_image, 0);
                slideshow_active = true;
            } else {
                if (timeout_id > 0) {
                    GLib.Source.remove(timeout_id);
                }
                slideshow_active = false;
            }
        }
    }

    private void action_edit_with_gimp() {
        if (file != null) {
            bool test_gimp = GLib.FileUtils.test("/usr/bin/gimp", FileTest.IS_EXECUTABLE);
            if (test_gimp == true) {
                try {
                    Process.spawn_command_line_async("gimp \'%s\'".printf(file));
                } catch(Error error) {
                    stderr.printf("error: %s\n", error.message);
                }
            } else {
                Gtk.MessageDialog msg = new Gtk.MessageDialog (window, Gtk.DialogFlags.MODAL,
                        Gtk.MessageType.WARNING, Gtk.ButtonsType.OK, "Error: Gimp is not installed!");
                msg.response.connect ((response_id) => {
                    switch (response_id) {
                    case Gtk.ResponseType.OK:
                        break;
                    }
                    msg.destroy();
                });
                msg.show ();
            }
        }
    }

    private void action_full_screen_toggle() {
        if ((window.get_window().get_state() & Gdk.WindowState.FULLSCREEN) != 0) {
            action_full_screen_exit();
        } else {
            window.fullscreen();
            saved_pixbuf_width = (int)pixbuf_scaled.get_width();
            saved_pixbuf_height = (int)pixbuf_scaled.get_height();
            int screen_width;
            int screen_height;
            var root_window = Gdk.get_default_root_window();
            root_window.get_geometry(null, null, out screen_width, out screen_height);
            load_pixbuf_with_size(screen_width, screen_height);
        }
    }

    private void action_full_screen_exit() {
        if ((window.get_window().get_state() & Gdk.WindowState.FULLSCREEN) != 0) {
            window.unfullscreen();
            load_pixbuf_with_size(saved_pixbuf_width, saved_pixbuf_height);
        }
    }

    private void action_about() {
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
        this.quit();
    }

    private static int main (string[] args) {
        Program app = new Program();
        return app.run(args);
    }
}

namespace Pangea {
public class Operations: GLib.Object {

    public void add_devices_grid() {
        clear_grid(grid_devices);
        var uc = new GUdev.Client(null);
        GLib.List<GUdev.Device> devs = uc.query_by_subsystem("block");
        int i = 0;
        foreach (GUdev.Device d in devs) {
            if (d.get_devtype() == "partition") {
                add_button_to_devices_grid("/mnt/"+d.get_name(), "/dev/"+d.get_name(), i);
                i++;
            }
        }
    }

    void add_button_to_devices_grid(string mount_point, string device,
                                    int position) {
        var button = new Gtk.Button.with_label(device.replace("/dev/",""));
        button.set_halign(Gtk.Align.START);
        button.set_always_show_image(true);
        button.set_relief(Gtk.ReliefStyle.NONE);
        // set device icon
        button.set_image(new Gtk.Image.from_icon_name("drive-harddisk",
                         Gtk.IconSize.MENU));
        if (GLib.File.new_for_path(mount_point).query_exists() == true) {
            string name;
            int c=0;
            try {
                var d = Dir.open(mount_point);
                while ((name = d.read_name()) != null) {
                    c++;
                }
                if (c != 0) {
                    button.set_image(new Gtk.Image.from_icon_name("media-eject",
                                     Gtk.IconSize.MENU));
                }
            } catch (GLib.Error e) {
                stderr.printf("%s\n", e.message);
            }
        }
        // button clicked
        button.button_press_event.connect((w, e) => {
            if (e.button == 1) {
                if (GLib.File.new_for_path(mount_point).query_exists() == false) {
                    execute_command_sync("mkdir %s".printf(mount_point));
                    execute_command_sync("mount %s %s".printf(device, mount_point));
                } else {
                    string name;
                    int c=0;
                    try {
                        var d = Dir.open(mount_point);
                        while ((name = d.read_name()) != null) {
                            c++;
                        }
                        if (c == 0) {
                            execute_command_sync("mount %s %s".printf(device, mount_point));
                            execute_command_sync("notify-send \"Pangea\" \"Mounted %s\"".printf("/dev/" +
                                                 button.get_label()));
                            execute_command_async("mpg123 -q /usr/share/sounds/dialog-information.mp3");
                            button.set_image(new Gtk.Image.from_icon_name("media-eject",
                                             Gtk.IconSize.MENU));
                        }
                    } catch (GLib.Error e) {
                        stderr.printf("%s\n", e.message);
                    }
                }
                new Pangea.IconView().open_location(GLib.File.new_for_path(mount_point), true);
            }
            // middle click unmount
            if (e.button == 2) {
                execute_command_sync("umount %s".printf("/dev/" + button.get_label()));
                string name;
                int c=0;
                try {
                    var d = Dir.open("/mnt/" + button.get_label());
                    while ((name = d.read_name()) != null) {
                        c++;
                    }
                    // mount point is empty
                    if (c == 0) {
                        execute_command_sync("notify-send \"Pangea\" \"Ejected %s\"".printf("/dev/" +
                                             button.get_label()));
                        execute_command_async("mpg123 -q /usr/share/sounds/dialog-information.mp3");
                        button.set_image(new Gtk.Image.from_icon_name("drive-harddisk",
                                         Gtk.IconSize.MENU));
                    } else {
                        execute_command_sync("notify-send \"Pangea\" \"Problem ejecting %s. Device is currently in use.\"".printf("/dev/"
                                             + button.get_label()));
                        execute_command_async("mpg123 -q /usr/share/sounds/dialog-error.mp3");
                    }
                } catch (GLib.Error e) {
                    stderr.printf("%s\n", e.message);
                }
            }
            return false;
        });
        grid_devices.attach(button, 0, position, 1, 1);
    }

    public void clear_grid(Gtk.Grid g) {
        int j = 50;
        while (j != -1) {
            grid_bookmarks.remove_row(j);
            j--;
        }
    }

    public void add_bookmarks_grid() {
        clear_grid(grid_bookmarks);
        string bookmark = GLib.Path.build_filename(
                              GLib.Environment.get_user_config_dir(), "gtk-3.0/bookmarks");
        var bookmarks_file = File.new_for_path(bookmark);
        if (bookmarks_file.query_exists() == true) {
            try {
                var dis = new DataInputStream (bookmarks_file.read ());
                string line;
                int i=0;
                while ((line = dis.read_line (null)) != null) {
                    int l = line.index_of_char(' ');
                    string s = GLib.Path.get_basename(line.slice (0, l));
                    string p = line.slice(0, l).replace("file://", "");
                    add_button_to_bookmarks_grid(p, s, i);
                    i++;
                }
            } catch (Error e) {
                error ("%s", e.message);
            }
        }
    }

    void add_button_to_bookmarks_grid(string path, string label, int position) {
        var button = new Gtk.Button.with_label(label);
        button.set_halign(Gtk.Align.START);
        button.set_always_show_image(true);
        button.set_image(new Gtk.Image.from_icon_name("user-bookmarks",
                         Gtk.IconSize.LARGE_TOOLBAR));
        button.set_relief(Gtk.ReliefStyle.NONE);
        button.button_press_event.connect((w, e) => {
            if (e.button == 1 || e.button == 2) {
                new Pangea.IconView().open_location(GLib.File.new_for_path(path), true);
            }
            if (e.button == 3) {
                var menuitem_remove_bookmark = new Gtk.MenuItem.with_label("Remove");
                menuitem_remove_bookmark.activate.connect(() => {
                    remove_bookmark(button.get_label());
                });
                menu = new Gtk.Menu();
                menu.append(menuitem_remove_bookmark);
                menu.show_all();
                menu.popup (null, null, null, e.button, e.time);
            }
            return false;
        });
        grid_bookmarks.attach(button, 0, position, 1, 1);
        grid_bookmarks.show_all();
    }

    // Bookmark
    public void add_bookmark() {
        GLib.FileOutputStream fos = null;
        string bookmark = GLib.Path.build_filename(
                              GLib.Environment.get_user_config_dir(), "gtk-3.0/bookmarks");
        GLib.File bookmarks_dir = GLib.File.new_for_path(GLib.Path.get_dirname(
                                      bookmark));
        if (bookmarks_dir.query_exists() == false) {
            try {
                bookmarks_dir.make_directory();
            } catch (GLib.Error e) {
                stderr.printf ("%s\n", e.message);
            }
        }
        try {
            fos = File.new_for_path(bookmark).append_to(FileCreateFlags.NONE);
        } catch (GLib.Error e) {
            stderr.printf ("%s\n", e.message);
        }
        string current_uri = GLib.File.new_for_path(current_dir).get_uri() + " " +
                             GLib.Path.get_basename(current_dir) + "\n";
        try {
            fos.write(current_uri.data);
        } catch (GLib.IOError e) {
            stderr.printf ("%s\n", e.message);
        }
        add_bookmarks_grid();
    }

    public void remove_bookmark(string rem) {
        string bookmark = GLib.Path.build_filename(
                              GLib.Environment.get_user_config_dir(), "gtk-3.0/bookmarks");
        var bookmarks_file = File.new_for_path(bookmark);
        try {
            var dis = new DataInputStream (bookmarks_file.read ());
            string line;
            string match = " " + rem;
            while ((line = dis.read_line (null)) != null) {
                if (line.contains(match)) {
                    execute_command_sync("sed -i '/%s/d' %s".printf(match, bookmark));
                    add_bookmarks_grid();
                }
            }
        } catch (Error e) {
            error ("%s", e.message);
        }
    }

    // Make
    public void make_new(bool file) {
        string entry_title = null;
        string typed = null;
        if (file == true) {
            entry_title = "Create File";
        } else {
            entry_title = "Create Folder";
        }
        var dialog = new Gtk.Dialog();
        dialog.set_title(entry_title);
        dialog.set_border_width(10);
        dialog.set_property("skip-taskbar-hint", true);
        dialog.set_transient_for(window);
        dialog.set_resizable(false);
        var entry = new Gtk.Entry();
        entry.set_text("new");
        entry.select_region(0, 0);
        entry.set_position(-1);
        entry.set_size_request(250, 0);
        entry.activate.connect(() => {
            typed = entry.get_text();
            on_make_new_dialog_response(entry, dialog, typed, file);
        });
        var content = dialog.get_content_area() as Gtk.Box;
        content.pack_start(entry, true, true, 10);
        dialog.add_button("Close", Gtk.ResponseType.CLOSE);
        dialog.add_button("Create", Gtk.ResponseType.OK);
        dialog.set_default_response(Gtk.ResponseType.OK);
        dialog.show_all();
        if (dialog.run() == Gtk.ResponseType.OK) {
            typed = entry.get_text();
            on_make_new_dialog_response(entry, dialog, typed, file);
        }
        dialog.destroy();
    }

    private void on_make_new_dialog_response(Gtk.Entry entry, Gtk.Dialog dialog,
            string typed, bool file) {
        if (typed != "")
            if (file == true) {
                execute_command_sync("touch '%s'".printf(GLib.Path.build_filename(current_dir,
                                     typed)));
            } else {
                execute_command_sync("mkdir '%s'".printf(GLib.Path.build_filename(current_dir,
                                     typed)));
            }
        dialog.destroy();
    }

    // Open
    public void file_open_activate() {
        GLib.FileInfo file_info = null;
        var files_open = new GLib.List<string>();
        files_open = get_files_selection();
        if (files_open.length() == 1) {
            GLib.File file_check = GLib.File.new_for_path(files_open.nth_data(0));
            if (file_check.query_file_type(0) == GLib.FileType.DIRECTORY) {
                new Pangea.IconView().open_location(GLib.File.new_for_path(files_open.nth_data(
                                                        0)),
                                                    true);
            } else {
                try {
                    file_info = file_check.query_info("standard::content-type", 0, null);
                } catch (GLib.Error e) {
                    stderr.printf("%s\n", e.message);
                }
                string content = file_info.get_content_type();
                string mime = GLib.ContentType.get_mime_type(content);
                var appinfo = AppInfo.get_default_for_type(mime, false);
                if (appinfo != null) {
                    execute_command_async("%s '%s'".printf(appinfo.get_executable(),
                                                           files_open.nth_data(0)));
                }
            }
        }
    }

    // Open With
    public void file_open_with_activate() {
        GLib.FileInfo file_info = null;
        var files_open_with = new GLib.List<string>();
        files_open_with = get_files_selection();
        if (files_open_with.length() == 1) {
            GLib.File file_check = GLib.File.new_for_path(files_open_with.nth_data(0));
            if (file_check.query_file_type(0) != GLib.FileType.DIRECTORY) {
                try {
                    file_info = file_check.query_info("standard::content-type", 0, null);
                } catch (GLib.Error e) {
                    stderr.printf("%s\n", e.message);
                }
                string content = file_info.get_content_type();
                string mime = GLib.ContentType.get_mime_type(content);
                var dialog = new Gtk.AppChooserDialog.for_content_type(window, 0, mime);
                if (dialog.run() == Gtk.ResponseType.OK) {
                    var appinfo = dialog.get_app_info();
                    if (appinfo != null) {
                        execute_command_async("%s '%s'".printf(appinfo.get_executable(),
                                                               files_open_with.nth_data(0)));
                    }
                }
                dialog.close();
            }
        }
    }

    // Execute
    public void file_execute_activate() {
        var files_execute = new GLib.List<string>();
        files_execute = get_files_selection();
        if (files_execute.length() == 1) {
            string efile = files_execute.nth_data(0);
            if (GLib.FileUtils.test(efile, FileTest.IS_EXECUTABLE) == true) {
                execute_command_async("%s\n".printf(efile));
            }
            string c;
            try {
                c = new Pangea.IconView().get_file_content(efile);
            } catch (GLib.Error e) {
                error("%s\n", e.message);
            }
            if (c == "application/x-desktop") {
                string exec_line;
                var keyfile = new GLib.KeyFile();
                try {
                    keyfile.load_from_file(efile, GLib.KeyFileFlags.NONE);
                } catch (GLib.Error e) {
                    error("%s\n", e.message);
                }
                try {
                    exec_line = keyfile.get_string("Desktop Entry", "Exec");
                    exec_line = exec_line.replace("%F","").replace("%U","").replace("%f",
                                "").replace("%u","");
                } catch (GLib.Error e) {
                    error("%s\n", e.message);
                }
                execute_command_async("%s\n".printf(exec_line));
            }
        }
    }

    // Compress tar gz
    public void file_compress_tar_gz_activate() {
        var file_compress_tar_gz = new GLib.List<string>();
        file_compress_tar_gz = get_files_selection();
        if (file_compress_tar_gz.length() == 1) {
            string tar_gz_file = file_compress_tar_gz.nth_data(0) + "." + "tar.gz";
            string compress_dir = GLib.Path.get_basename(file_compress_tar_gz.nth_data(0));
            execute_command_async("bsdtar -czf \"%s\" \"%s\"".printf(tar_gz_file,
                                  compress_dir));
        }
    }

    // Compress zip
    public void file_compress_zip_activate() {
        var file_compress_zip = new GLib.List<string>();
        file_compress_zip = get_files_selection();
        if (file_compress_zip.length() == 1) {
            string zip_file = file_compress_zip.nth_data(0) + "." + "zip";
            string compress_dir = GLib.Path.get_basename(file_compress_zip.nth_data(0));
            execute_command_async("bsdtar -a -cf \"%s\" \"%s\"".printf(zip_file,
                                  compress_dir));
        }
    }

    // Cut
    public void file_cut_activate() {
        if (files_copy != null) {
            files_copy = null;
            //files_copy.clear();
        }
        files_cut = get_files_selection();
    }

    // Copy
    public void file_copy_activate() {
        if (files_cut!= null) {
            files_cut = null;
            //files_cut.clear();
        }
        files_copy = get_files_selection();
    }

    // Paste
    public void file_paste_activate() {
        if (files_copy != null) {
            foreach(string i in files_copy) {
                GLib.File file2 = File.new_for_path(current_dir + "/" + GLib.Path.get_basename(
                                                        i));
                if (file2.query_exists() == true) {
                    var dialog = new Gtk.Dialog();
                    dialog.set_title("Overwrite?");
                    dialog.set_border_width(10);
                    dialog.set_property("skip-taskbar-hint", true);
                    dialog.set_transient_for(window);
                    dialog.set_resizable(false);
                    var label = new Gtk.Label("Overwrite %s?".printf(GLib.Path.get_basename(i)));
                    var content = dialog.get_content_area() as Gtk.Box;
                    content.pack_start(label, true, true, 10);
                    dialog.add_button("No", Gtk.ResponseType.NO);
                    dialog.add_button("Yes", Gtk.ResponseType.YES);
                    dialog.set_default_response(Gtk.ResponseType.NO);
                    dialog.show_all();
                    if (dialog.run() != Gtk.ResponseType.YES) {
                        dialog.destroy();
                        return;
                    }
                    dialog.destroy();
                }
                execute_command_sync("cp -a '%s' '%s'".printf(i, current_dir));
                stdout.printf("DEBUG: copying '%s' to '%s'\n".printf(i,
                              current_dir + "/" + GLib.Path.get_basename(i)));
            }
        }
        if (files_cut != null) {
            foreach(string i in files_cut) {
                execute_command_sync("mv '%s' '%s'".printf(i, current_dir));
                stdout.printf("DEBUG: moving '%s' to '%s'\n".printf(i,
                              current_dir + "/" + GLib.Path.get_basename(i)));
            }
        }
    }

    // Rename
    public void file_rename_activate() {
        var files_rename = new GLib.List<string>();
        files_rename = get_files_selection();
        string typed = null;
        if (files_rename.length() == 1) {
            var dialog = new Gtk.Dialog();
            dialog.set_title("Rename File/Folder");
            dialog.set_border_width(10);
            dialog.set_property("skip-taskbar-hint", true);
            dialog.set_transient_for(window);
            dialog.set_resizable(false);
            var entry = new Gtk.Entry();
            entry.set_size_request(270, 0);
            string etext = GLib.Path.get_basename(files_rename.nth_data(0));
            entry.set_text(etext);
            entry.activate.connect(() => {
                typed = entry.get_text();
                on_rename_dialog_response(entry, dialog, files_rename.nth_data(0), typed);
            });
            var content = dialog.get_content_area() as Gtk.Box;
            content.pack_start(entry, true, true, 10);
            dialog.add_button("Close", Gtk.ResponseType.CLOSE);
            dialog.add_button("Rename", Gtk.ResponseType.OK);
            dialog.set_default_response(Gtk.ResponseType.OK);
            dialog.show_all();
            if( etext.contains(".") == true ) {
                entry.select_region(0, etext.last_index_of_char('.'));
            }
            if (dialog.run() == Gtk.ResponseType.OK) {
                typed = entry.get_text();
                on_rename_dialog_response(entry, dialog, files_rename.nth_data(0), typed);
            }
            dialog.destroy();
        }
    }

    private void on_rename_dialog_response(Gtk.Entry entry, Gtk.Dialog dialog,
                                           string old_path, string new_name) {
        if (new_name != "") {
            execute_command_sync("mv '%s' '%s'".printf(old_path,
                                 GLib.Path.build_filename(current_dir, new_name)));
        }
        stdout.printf("DEBUG: moving '%s' to '%s'\n".printf(old_path,
                      GLib.Path.build_filename(current_dir, new_name)));
        dialog.destroy();
    }

    // Delete
    public void file_delete_activate() {
        var files_delete = new GLib.List<string>();
        files_delete = get_files_selection();
        if (files_delete.length() == 0) {
            return;
        }
        var dialog = new Gtk.Dialog();
        dialog.set_title("Delete File/Folder");
        dialog.set_border_width(10);
        dialog.set_property("skip-taskbar-hint", true);
        dialog.set_transient_for(window);
        dialog.set_resizable(false);
        var label = new Gtk.Label("Delete %d selected item(s)?".printf(
                                      (int)files_delete.length()));
        var content = dialog.get_content_area() as Gtk.Box;
        content.pack_start(label, true, true, 10);
        dialog.add_button("Close", Gtk.ResponseType.CLOSE);
        dialog.add_button("Delete", Gtk.ResponseType.OK);
        dialog.set_default_response(Gtk.ResponseType.OK);
        dialog.show_all();
        if (dialog.run() == Gtk.ResponseType.OK) {
            foreach(string i in files_delete) {
                on_delete_dialog_response(dialog, i);
            }
        }
        dialog.destroy();
    }

    private void on_delete_dialog_response(Gtk.Dialog dialog, string filename) {
        if (filename != "") {
            execute_command_sync("rm -r '%s'".printf(filename));
        }
        stdout.printf("DEBUG: deleted '%s'\n".printf(filename));
        dialog.destroy();
    }

    // Properties
    public void file_properties_activate() {
        var files_properties = new GLib.List<string>();
        files_properties = get_files_selection();
        if (files_properties.length() == 1) {
            var dialog = new Gtk.Dialog();
            dialog.set_title("File properties");
            dialog.set_border_width(10);
            dialog.set_property("skip-taskbar-hint", true);
            dialog.set_transient_for(window);
            dialog.set_resizable(false);
            dialog.width_request = 450;
            string fullpath = files_properties.nth_data(0);
            // name
            string name = GLib.Path.get_basename(fullpath);
            // location
            string location = GLib.Path.get_dirname(fullpath);
            // size, type, modified
            string size = "";
            string type = "";
            string modified = "";
            try {
                var file_check = GLib.File.new_for_path(fullpath);
                GLib.FileInfo file_info = file_check.query_info("*", 0, null);
                int64 bytes = get_file_size(file_check);
                int64 kb = bytes / 1024;
                int64 mb = bytes / 1048576;
                if ( bytes > 1048576) {
                    size = "%s MB (%s bytes)".printf(mb.to_string(), bytes.to_string());
                } else {
                    size = "%s KB (%s bytes)".printf(kb.to_string(), bytes.to_string());
                }
                string content = file_info.get_content_type();
                type = GLib.ContentType.get_description(content);
                modified = file_info.get_modification_time().to_iso8601();
                modified = modified.slice (0, 10) + " " + modified.slice (11, 19);
            } catch (GLib.Error e) {
                stderr.printf ("%s\n", e.message);
            }
            // ui
            var label_name = new Gtk.Label("");
            label_name.set_markup("<b>Name:</b> %s".printf(name));
            label_name.set_halign(Gtk.Align.START);
            var label_size = new Gtk.Label("");
            label_size.set_markup("<b>Size:</b> %s".printf(size));
            label_size.set_halign(Gtk.Align.START);
            var label_type = new Gtk.Label("");
            label_type.set_markup("<b>Type:</b> %s".printf(type));
            label_type.set_halign(Gtk.Align.START);
            var label_location = new Gtk.Label("");
            label_location.set_markup("<b>Location:</b> %s".printf(location));
            label_location.set_halign(Gtk.Align.START);
            var label_modified = new Gtk.Label("");
            label_modified.set_markup("<b>Modified:</b> %s".printf(modified));
            label_modified.set_halign(Gtk.Align.START);
            var grid = new Gtk.Grid();
            grid.attach(label_name,     0, 0, 1, 1);
            grid.attach(label_size,     0, 1, 1, 1);
            grid.attach(label_type,     0, 2, 1, 1);
            grid.attach(label_location, 0, 3, 1, 1);
            grid.attach(label_modified, 0, 4, 1, 1);
            grid.set_column_spacing(10);
            grid.set_row_spacing(5);
            grid.set_border_width(10);
            grid.set_column_homogeneous(true);
            var container = dialog.get_content_area() as Gtk.Container;
            container.add(grid);
            dialog.add_button("OK", Gtk.ResponseType.OK);
            dialog.set_default_response(Gtk.ResponseType.OK);
            dialog.show_all();
            if (dialog.run() == Gtk.ResponseType.OK) {
                dialog.destroy();
            }
            dialog.destroy();
        }
    }

    public int64 get_file_size(GLib.File file) {
        try {
            GLib.FileInfo fileInfo = file.query_info ("standard::*",
                                     FileQueryInfoFlags.NONE);
            if ( fileInfo.get_file_type () == FileType.DIRECTORY ) {
                int64 size = 0;
                var enumerator = file.enumerate_children ("standard::*",
                                 FileQueryInfoFlags.NOFOLLOW_SYMLINKS);
                GLib.FileInfo childFileInfo;
                while ( (childFileInfo = enumerator.next_file ()) != null ) {
                    size += get_file_size (file.resolve_relative_path (childFileInfo.get_name ()));
                }
                return size;
            } else {
                return fileInfo.get_size ();
            }
        } catch (Error e) {
            error ("%s", e.message);
        }
    }

    public void show_preferences_dialog() {
        var dialog = new Gtk.Dialog();
        dialog.set_title("Preferences");
        dialog.set_border_width(5);
        dialog.set_property("skip-taskbar-hint", true);
        dialog.set_transient_for(window);
        dialog.set_resizable(false);
        dialog.width_request = 360;
        var preferences_label_icon_size =       new Gtk.Label("Icon size");
        preferences_label_icon_size.set_halign(Gtk.Align.START);
        var preferences_label_thumbnail_size =  new Gtk.Label("Thumbnail size");
        preferences_label_thumbnail_size.set_halign(Gtk.Align.START);
        var preferences_spinbutton_icon_size = new Gtk.SpinButton.with_range(
            24, 256, 8);
        preferences_spinbutton_icon_size.adjustment.page_increment = 8;
        preferences_spinbutton_icon_size.set_value(icon_size);
        preferences_spinbutton_icon_size.value_changed.connect(() => {
            icon_size = (int)preferences_spinbutton_icon_size.get_value();
            view.set_item_width(icon_size + 24);
            new Pangea.Settings().save_settings();
            new Pangea.IconView().open_location(GLib.File.new_for_path(current_dir), false);
        });
        var preferences_spinbutton_thumbnail_size = new Gtk.SpinButton.with_range(
            24, 256, 8);
        preferences_spinbutton_thumbnail_size.adjustment.page_increment = 8;
        preferences_spinbutton_thumbnail_size.set_value(thumbnail_size);
        preferences_spinbutton_thumbnail_size.value_changed.connect(() => {
            thumbnail_size = (int)preferences_spinbutton_thumbnail_size.get_value();
            new Pangea.Settings().save_settings();
            new Pangea.IconView().open_location(GLib.File.new_for_path(current_dir), false);
        });
        var grid = new Gtk.Grid();
        grid.attach(preferences_label_icon_size,        0, 0, 1, 1);
        grid.attach(preferences_label_thumbnail_size,   0, 1, 1, 1);
        grid.attach(preferences_spinbutton_icon_size,        1, 0, 1, 1);
        grid.attach(preferences_spinbutton_thumbnail_size,   1, 1, 1, 1);
        grid.set_column_spacing(5);
        grid.set_row_spacing(10);
        grid.set_border_width(5);
        grid.set_column_homogeneous(true);
        var container = dialog.get_content_area() as Gtk.Container;
        container.add(grid);
        dialog.add_button("OK", Gtk.ResponseType.OK);
        dialog.set_default_response(Gtk.ResponseType.OK);
        dialog.show_all();
        if (dialog.run() == Gtk.ResponseType.OK) {
            dialog.destroy();
        }
        dialog.destroy();
    }

    public GLib.List<string> get_files_selection() {
        var list = new GLib.List<string>();
        List<Gtk.TreePath> paths = view.get_selected_items();
        GLib.Value filepath;
        foreach (Gtk.TreePath path in paths) {
            model.get_iter(out iter, path);
            model.get_value(iter, 2, out filepath);
            list.append((string)filepath);
        }
        return list;
    }

    public void execute_command_async(string item_name) {
        try {
            Process.spawn_command_line_async(item_name);
        } catch (GLib.Error e) {
            stderr.printf ("%s\n", e.message);
        }
    }

    public void execute_command_sync(string item_name) {
        try {
            Process.spawn_command_line_sync(item_name);
        } catch (GLib.Error e) {
            stderr.printf ("%s\n", e.message);
        }
    }
}
}

namespace Pangea {
public class IconView : GLib.Object {
    string content;
    bool symlink;
    string target;

    public void open_location(GLib.File file, bool start_monitor) {
        list_dir  = new GLib.List<string>();
        list_file = new GLib.List<string>();
        var list_images = new GLib.List<string>();
        var list_desktop = new GLib.List<string>();
        string name;
        string fullpath;
        current_dir = file.get_path();
        Gdk.Pixbuf pbuf = null;
        if (current_dir != null && file.query_file_type(0) == GLib.FileType.DIRECTORY) {
            try {
                var d = Dir.open(file.get_path());
                model.clear();
                while ((name = d.read_name()) != null) {
                    fullpath = GLib.Path.build_filename(current_dir, name);
                    get_file_content(fullpath);
                    if (content == "inode/directory") {
                        list_dir.append(name);
                    } else {
                        list_file.append(name);
                    }
                }
                list_dir.sort(strcmp);
                list_file.sort(strcmp);
                foreach(string i in list_dir) {
                    fullpath = GLib.Path.build_filename(current_dir, i);
                    Gtk.IconTheme icon_theme = Gtk.IconTheme.get_default();
                    check_if_symlink(fullpath);
                    pbuf = icon_theme.load_icon("folder", icon_size, 0);
                    if (symlink == true) {
                        pbuf = add_emblem(pbuf);
                        target = get_target(fullpath);
                    } else  {
                        target = i.replace("&", "&amp;");
                    }
                    model.append(out iter);
                    model.set(iter, 0, pbuf, 1, i, 2, fullpath, 3, target);
                }
                foreach(string i in list_file) {
                    fullpath = GLib.Path.build_filename(current_dir, i);
                    get_file_content(fullpath);
                    GLib.Icon icon = GLib.ContentType.get_icon(content);
                    Gtk.IconTheme icon_theme = Gtk.IconTheme.get_default();
                    Gtk.IconInfo? icon_info = icon_theme.lookup_by_gicon(icon, icon_size,
                                              Gtk.IconLookupFlags.FORCE_SIZE);
                    check_if_symlink(fullpath);
                    if (icon_info != null) {
                        pbuf = icon_info.load_icon();
                    } else {
                        pbuf = icon_theme.load_icon("text-x-generic", icon_size, 0);
                    }
                    //print("%s\n", content);
                    // thumbnails
                    if (content == "image/jpeg" || content == "image/png"
                            || content == "image/svg+xml" || content == "image/x-xbitmap"
                            || content == "image/x-xpixmap") {
                        list_images.append(fullpath);
                    }
                    if (content == "application/x-desktop") {
                        list_desktop.append(fullpath);
                    }
                    if (symlink == true) {
                        pbuf = add_emblem(pbuf);
                        target= get_target(fullpath);
                    } else  {
                        target = i.replace("&", "&amp;");
                    }
                    model.append(out iter);
                    model.set(iter, 0, pbuf, 1, i, 2, fullpath, 3, target);
                }
                foreach(string i in list_images) {
                    new Pangea.Thumbnails().get_thumbnail_from_file_name.begin(i, (obj, res) => {
                        pbuf = new Pangea.Thumbnails().get_thumbnail_from_file_name.end(res);
                        Gtk.TreeModelForeachFunc replace_image_in_liststore = (m, path, iter) => {
                            GLib.Value cell2;
                            m.get_value(iter, 2, out cell2);
                            if ((string)cell2 == i) {
                                model.set(iter, 0, pbuf);
                            }
                            return false;
                        };
                        model.foreach (replace_image_in_liststore);
                    });
                }
                foreach(string i in list_desktop) {
                    new Pangea.Thumbnails().get_desktop_file_icon.begin(i, (obj, res) => {
                        pbuf = new Pangea.Thumbnails().get_desktop_file_icon.end(res);
                        Gtk.TreeModelForeachFunc replace_image_in_liststore = (m, path, iter) => {
                            GLib.Value cell2;
                            m.get_value(iter, 2, out cell2);
                            if ((string)cell2 == i) {
                                model.set(iter, 0, pbuf);
                            }
                            return false;
                        };
                        model.foreach (replace_image_in_liststore);
                    });
                }
                window.set_title("%s".printf(current_dir));
                location_entry.set_text("%s".printf(current_dir));
                bool already_in_the_list = false;
                foreach (string i in list_dir_history) {
                    if (i == current_dir) {
                        already_in_the_list = true;
                        break;
                    }
                }
                if (already_in_the_list == false) {
                    list_dir_history.append(current_dir);
                    var combo_path_menu_item = new Gtk.MenuItem.with_label(current_dir);
                    combo_path_menu_item.activate.connect (() => {
                        open_location(GLib.File.new_for_path(combo_path_menu_item.get_label()), true);
                    });
                    combo_path_menu.append(combo_path_menu_item);
                    combo_path_menu.show_all();
                }
                uint len = list_dir.length() + list_file.length();
                statusbar.push(context_id, "%s item(s)".printf(len.to_string()));
                GLib.Environment.set_current_dir(current_dir);
                view.grab_focus();
                if (start_monitor == true) {
                    var m = new Pangea.DirectoryMonitor();
                    m.setup_file_monitor();
                }
            } catch (GLib.Error e) {
                stderr.printf("%s\n", e.message);
            }
        }
    }

    private Gdk.Pixbuf add_emblem(Gdk.Pixbuf p) {
        Gtk.IconTheme icon_theme = Gtk.IconTheme.get_default();
        var icon_info = icon_theme.lookup_icon ("emblem-symbolic-link", 16,
                                                Gtk.IconLookupFlags.FORCE_SIZE);
        Gdk.Pixbuf emblem = null;
        try {
            emblem = icon_info.load_icon();
        } catch (GLib.Error e) {
            stderr.printf ("%s\n", e.message);
        }
        int dest_x = icon_size - 16;
        int dest_y = icon_size - 16;
        var emblemed = p.copy();
        emblem.composite(emblemed,
                         dest_x, dest_y,
                         16, 16,
                         dest_x, dest_y,
                         1.0, 1.0,
                         Gdk.InterpType.BILINEAR, 242);
        return emblemed;
    }

    public void icon_clicked() {
        List<Gtk.TreePath> paths = view.get_selected_items();
        GLib.Value filepath;
        foreach (Gtk.TreePath path in paths) {
            model.get_iter(out iter, path);
            model.get_value(iter, 2, out filepath);
            var file_check = File.new_for_path((string)filepath);
            if (file_check.query_file_type(0) == GLib.FileType.DIRECTORY) {
                open_location(GLib.File.new_for_path((string)filepath), true);
            } else {
                try {
                    GLib.FileInfo file_info = file_check.query_info("standard::content-type", 0,
                                              null);
                    string content = file_info.get_content_type();
                    string mime = GLib.ContentType.get_mime_type(content);
                    if (mime == "application/x-bzip-compressed-tar"
                            || mime == "application/x-compressed-tar"
                            || mime == "application/zip"
                            || mime == "application/x-xz-compressed-tar") {
                        string uncomp_dir_s = file_check.get_basename().replace(".tar.bz2",
                                              "").replace(".tar.gz", "").replace(".zip", "").replace(".tar.xz", "");
                        string uncomp_dir_path = GLib.Path.get_dirname((string)filepath) + "/" +
                                                 uncomp_dir_s;
                        var uncomp_dir = File.new_for_path(uncomp_dir_path);
                        if (uncomp_dir.query_exists() == false) {
                            new Pangea.Operations().execute_command_async("mkdir -p \"%s\"".printf(
                                        uncomp_dir_path));
                            new Pangea.Operations().execute_command_async("bsdtar -xf \"%s\" -C \"%s\"".printf((
                                        string)filepath, uncomp_dir_path));
                        }
                        return;
                    }
                    if (content == "application/x-desktop") {
                        new Pangea.Operations().file_execute_activate();
                        return;
                    }
                    var appinfo = AppInfo.get_default_for_type(mime, false);
                    if (appinfo != null) {
                        new Pangea.Operations().execute_command_async("%s '%s'".printf(
                                    appinfo.get_executable(), (string)filepath));
                    } else {
                        var dialog = new Gtk.AppChooserDialog.for_content_type(window, 0, mime);
                        if (dialog.run() == Gtk.ResponseType.OK) {
                            appinfo = dialog.get_app_info();
                            if (appinfo != null) {
                                new Pangea.Operations().execute_command_async("%s '%s'".printf(
                                            appinfo.get_executable(), (string)filepath));
                            }
                        }
                        dialog.close();
                    }
                } catch (GLib.Error e) {
                    stderr.printf ("%s\n", e.message);
                }
            }
        }
    }

    public void on_selection_changed() {
        var selection = new GLib.List<string>();
        selection = new Pangea.Operations().get_files_selection();
        uint len_s = selection.length();
        if (len_s == 0) {
            uint len = list_dir.length() + list_file.length();
            statusbar.push(context_id, "%s item(s)".printf(len.to_string()));
        }
        if (len_s == 1) {
            string name = "";
            string size = "";
            string type = "";
            try {
                string fullpath = selection.nth_data(0);
                var file_check = GLib.File.new_for_path(fullpath);
                if (file_check.query_exists() == false) {
                    return;
                }
                GLib.FileInfo file_info = file_check.query_info("*", 0, null);
                int64 bytes = file_info.get_size();
                int64 kb = bytes / 1024;
                int64 mb = bytes / 1048576;
                if ( bytes > 1048576) {
                    size = "%s MB".printf(mb.to_string());
                } else {
                    size = "%s KB".printf(kb.to_string());
                }
                name = GLib.Path.get_basename(fullpath);
                string content = file_info.get_content_type();
                type = GLib.ContentType.get_description(content);
            } catch (GLib.Error e) {
                stderr.printf ("%s\n", e.message);
            }
            statusbar.push(context_id, "\"%s\" (%s) %s".printf(name, type, size));
        }
        if (len_s > 1) {
            int64 bytes_one = 0;
            int64 bytes = 0;
            string size = "";
            foreach(string i in selection) {
                try {
                    string fullpath = i;
                    var file_check = GLib.File.new_for_path(fullpath);
                    if (file_check.query_exists() == false) {
                        return;
                    }
                    GLib.FileInfo file_info = file_check.query_info("*", 0, null);
                    bytes_one = file_info.get_size();
                    bytes = bytes + bytes_one;
                } catch (GLib.Error e) {
                    stderr.printf ("%s\n", e.message);
                }
            }
            int64 kb = bytes / 1024;
            int64 mb = bytes / 1048576;
            if ( bytes > 1048576) {
                size = "%s MB".printf(mb.to_string());
            } else {
                size = "%s KB".printf(kb.to_string());
            }
            statusbar.push(context_id, "%s items selected (%s)".printf(len_s.to_string(),
                           size));
        }
    }

    public string get_file_content(string filepath) throws GLib.Error {
        var file_check = File.new_for_path(filepath);
        var file_info = file_check.query_info("standard::content-type", 0, null);
        content = file_info.get_content_type();
        return content;
    }

    public bool check_if_symlink(string filepath) throws GLib.Error {
        var file_check = File.new_for_path(filepath);
        var file_info = file_check.query_info("standard::is-symlink", 0, null);
        symlink = file_info.get_is_symlink();
        return symlink;
    }

    public string get_target(string filepath) throws GLib.Error {
        var file_check = File.new_for_path(filepath);
        var file_info = file_check.query_info("standard::symlink-target", 0, null);
        target = "Link → "+file_info.get_symlink_target();
        return target;
    }

}
}

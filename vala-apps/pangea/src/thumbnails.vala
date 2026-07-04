namespace Pangea {
public class Thumbnails: GLib.Object {
    Gdk.Pixbuf? pix = null;

    public async Gdk.Pixbuf get_thumbnail_from_file_name(string name) {
        if (current_dir.contains(".thumbnails") != true) {
            string thumb_dir = GLib.Path.build_filename(GLib.Environment.get_home_dir(),
                               ".thumbnails/normal");
            string checksum = get_file_md5(name);
            string thumb_name = GLib.Path.build_filename(thumb_dir, checksum + ".png");
            var thumb_file = File.new_for_path(thumb_name);
            if (thumb_file.query_exists() == true) {
                //print("%s\n", "DEBUG: image preview found in .thumbnails/normal");
                try {
                    GLib.InputStream stream = yield thumb_file.read_async();
                    pix = yield new Gdk.Pixbuf.from_stream_async(stream, null);
                    if (pix.get_width() > thumbnail_size) {
                        GLib.InputStream s = yield thumb_file.read_async();
                        pix = yield new Gdk.Pixbuf.from_stream_at_scale_async(s, thumbnail_size,
                                thumbnail_size, true, null);
                    }
                } catch (Error e) {
                    stderr.printf("%s\n", e.message);
                }
            } else {
                //print("%s\n", "DEBUG: image preview not found");
                var file = GLib.File.new_for_path(name);
                try {
                    GLib.InputStream stream = yield file.read_async();
                    pix = yield new Gdk.Pixbuf.from_stream_async(stream, null);
                    if (pix.get_width() > thumbnail_size) {
                        GLib.InputStream s = yield file.read_async();
                        pix = yield new Gdk.Pixbuf.from_stream_at_scale_async(s, thumbnail_size,
                                thumbnail_size, true, null);
                    }
                    var check_dir = File.new_for_path(thumb_dir);
                    if (check_dir.query_exists() == false)
                        try {
                            check_dir.make_directory_with_parents();
                        } catch (GLib.Error e) {
                            error ("%s\n", e.message);
                        }
                    pix.save(thumb_name, "png");
                    GLib.FileUtils.chmod(thumb_name, 0600);
                } catch (Error e) {
                    stderr.printf("%s\n", e.message);
                }
            }
        } else {
            pix = load_fallback_icon();
        }
        return pix;
    }

    private Gdk.Pixbuf load_fallback_icon() {
        try {
            Gtk.IconTheme icon_theme = Gtk.IconTheme.get_default();
            pix = icon_theme.load_icon("text-x-generic", icon_size, 0);
        } catch (GLib.Error e) {
            error("%s\n", e.message);
        }
        return pix;
    }

    public async Gdk.Pixbuf get_desktop_file_icon(string name) {
        string icon_name;
        GLib.Icon icon;
        var keyfile = new GLib.KeyFile();
        try {
            keyfile.load_from_file(name, GLib.KeyFileFlags.NONE);
        } catch (GLib.Error e) {
            error("%s\n", e.message);
        }
        try {
            icon_name = keyfile.get_string ("Desktop Entry", "Icon");
        } catch (GLib.Error e) {
            icon_name = "";
            //error("%s\n", e.message);
        }
        try {
            icon = GLib.Icon.new_for_string(icon_name);
        } catch (GLib.Error e) {
            error("%s\n", e.message);
        }
        Gtk.IconTheme icon_theme = Gtk.IconTheme.get_default();
        Gtk.IconInfo? icon_info = icon_theme.lookup_by_gicon(icon, icon_size,
                                  Gtk.IconLookupFlags.FORCE_SIZE);
        if (icon_info != null) {
            try {
                pix = icon_info.load_icon();
            } catch (GLib.Error e) {
                error("%s\n", e.message);
            }
        } else {
            try {
                pix = icon_theme.load_icon("text-x-generic", icon_size, 0);
            } catch (GLib.Error e) {
                error("%s\n", e.message);
            }
        }
        return pix;
    }

    public string get_file_md5(string name) {
        uint8[] contents;
        try {
            FileUtils.get_data(name, out contents);
        } catch (GLib.FileError e) {
            stdout.printf("%s/n", e.message);
        }
        string checksum = GLib.Checksum.compute_for_data(ChecksumType.MD5, contents);
        return checksum;
    }

}
}

namespace Pangea {
public class Menu: GLib.Object {
    public Gtk.Menu activate_file_menu() {
        var file_open = new Gtk.MenuItem.with_label("Open");
        var file_open_with = new Gtk.MenuItem.with_label("Open With...");
        var file_execute = new Gtk.MenuItem.with_label("Execute");
        var context_separator1 = new Gtk.SeparatorMenuItem();
        var file_compress_tar_gz = new Gtk.MenuItem.with_label(
            "Compress (as TAR.GZ)");
        var file_compress_zip = new Gtk.MenuItem.with_label("Compress (as ZIP)");
        var context_separator2 = new Gtk.SeparatorMenuItem();
        var file_cut = new Gtk.MenuItem.with_label("Cut");
        var file_copy = new Gtk.MenuItem.with_label("Copy");
        var file_rename = new Gtk.MenuItem.with_label("Rename");
        var file_delete = new Gtk.MenuItem.with_label("Delete");
        var context_separator3 = new Gtk.SeparatorMenuItem();
        var file_properties = new Gtk.MenuItem.with_label("Properties");
        menu = new Gtk.Menu();
        menu.append(file_open);
        menu.append(file_open_with);
        menu.append(file_execute);
        menu.append(context_separator1);
        menu.append(file_compress_tar_gz);
        menu.append(file_compress_zip);
        menu.append(context_separator2);
        menu.append(file_cut);
        menu.append(file_copy);
        menu.append(file_rename);
        menu.append(file_delete);
        menu.append(context_separator3);
        menu.append(file_properties);
        var op = new Pangea.Operations();
        file_open.activate.connect(() => {
            op.file_open_activate();
        });
        file_open_with.activate.connect(() => {
            op.file_open_with_activate();
        });
        file_execute.activate.connect(() => {
            op.file_execute_activate();
        });
        file_compress_tar_gz.activate.connect(() => {
            op.file_compress_tar_gz_activate();
        });
        file_compress_zip.activate.connect(() => {
            op.file_compress_zip_activate();
        });
        file_cut.activate.connect(() => {
            op.file_cut_activate();
        });
        file_copy.activate.connect(() => {
            op.file_copy_activate();
        });
        file_rename.activate.connect(() => {
            op.file_rename_activate();
        });
        file_delete.activate.connect(() => {
            op.file_delete_activate();
        });
        file_properties.activate.connect(() => {
            op.file_properties_activate();
        });
        menu.show_all();
        return menu;
    }

    public Gtk.Menu activate_context_menu() {
        var context_folder = new Gtk.MenuItem.with_label("Create Folder");
        var context_file = new Gtk.MenuItem.with_label("Create File");
        var context_separator1 = new Gtk.SeparatorMenuItem();
        var context_paste = new Gtk.MenuItem.with_label("Paste");
        var context_separator2 = new Gtk.SeparatorMenuItem();
        var context_bookmark = new Gtk.MenuItem.with_label("Add to Bookmarks");
        var context_terminal = new Gtk.MenuItem.with_label("Open in Terminal");
        var op = new Pangea.Operations();
        context_folder.activate.connect(() => {
            op.make_new(false);
        });
        context_file.activate.connect(() => {
            op.make_new(true);
        });
        context_paste.activate.connect(() => {
            op.file_paste_activate();
        });
        context_bookmark.activate.connect(() => {
            op.add_bookmark();
        });
        context_terminal.activate.connect(() => {
            op.execute_command_async("%s".printf(terminal));
        });
        menu = new Gtk.Menu();
        menu.append(context_folder);
        menu.append(context_file);
        menu.append(context_separator1);
        menu.append(context_paste);
        menu.append(context_separator2);
        menu.append(context_bookmark);
        menu.append(context_terminal);
        menu.show_all();
        return menu;
    }
}
}

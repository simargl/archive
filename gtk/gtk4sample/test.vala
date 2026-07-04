// valac --pkg gtk4 test.vala; ./test

using Gtk;

int main (string[] argv) {
    // Create a new application
    var app = new Gtk.Application ("com.example.GtkApplication", GLib.ApplicationFlags.NON_UNIQUE);
    app.activate.connect (() => {
        var window = new Gtk.ApplicationWindow (app);
        var button = new Gtk.Button.with_label ("Hello, World!");

        button.clicked.connect (() => {
            window.close ();
        });

        window.set_child (button);
        window.present ();
    });
    return app.run (argv);
}

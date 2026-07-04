/*  Author: simargl <https://github.org/simargl>
 *  License: GPL v3
 */

namespace Pangea {
private class Main: Gtk.Application {
    public Main() {
        Object(application_id: "org.vala-apps.pangea",
               flags: GLib.ApplicationFlags.HANDLES_OPEN);
    }

    public override void startup() {
        base.startup();
        add_accelerator("<Alt>Home", "app.go-home", null);
        add_accelerator("<Control>N", "app.create-folder", null);
        add_accelerator("<Control><Shift>N", "app.create-file", null);
        add_accelerator("<Control>B", "app.add-bookmark", null);
        add_accelerator("F4", "app.terminal", null);
        add_accelerator("<Control>P", "app.preferences", null);
        add_accelerator("<Control>X", "app.cut", null);
        add_accelerator("<Control>C", "app.copy", null);
        add_accelerator("F2", "app.rename", null );
        add_accelerator("<Control>V", "app.paste", null);
        add_accelerator("Delete", "app.delete", null );
        add_accelerator("<Control>I", "app.properties", null);
        add_accelerator("<Control>Q", "app.quit", null);
        /*
        app.set_accels_for_action("app.go-home",          {"<Alt>Home"});
        app.set_accels_for_action("app.create-folder",    {"<Control>N"});
        app.set_accels_for_action("app.create-file",      {"<Control><Shift>N"});
        app.set_accels_for_action("app.add-bookmark",     {"<Control>B"});
        app.set_accels_for_action("app.terminal",         {"F4"});
        app.set_accels_for_action("app.preferences",      {"<Control>P"});
        app.set_accels_for_action("app.cut",              {"<Control>X"});
        app.set_accels_for_action("app.copy",             {"<Control>C"});
        app.set_accels_for_action("app.rename",           {"F2"});
        app.set_accels_for_action("app.paste",            {"<Control>V"});
        app.set_accels_for_action("app.delete",           {"Delete"});
        app.set_accels_for_action("app.properties",       {"<Control>I"});
        app.set_accels_for_action("app.quit",             {"<Control>Q"});
        */
        new Pangea.Window().add_app_window(this);
    }

    public override void open(File[] files, string hint) {
        foreach (File f in files) {
            saved_dir = f.get_path();
        }
        get_active_window().present();
        new Pangea.IconView().open_location(GLib.File.new_for_path(saved_dir), true);
    }

    public override void activate() {
        get_active_window().present();
        new Pangea.IconView().open_location(GLib.File.new_for_path(saved_dir), true);
    }

    private static int main (string[] args) {
        Pangea.Main app = new Pangea.Main();
        return app.run(args);
    }
}
}

/*  Author: simargl <https://github.com/simargl>
 *  License: GPL v3
 */

namespace Emendo {
const string NAME        = "Emendo";
const string VERSION     = "3.2.0";
const string DESCRIPTION = "Text editor with syntax highlighting";
const string ICON        = "accessories-text-editor";
const string[] AUTHORS   = { "Simargl <https://github.com/simargl>", "Yosef Or Boczko <yoseforb-at-gmail-dot-com>", null };

private class Application: Gtk.Application {
    public Application() {
        Object(application_id: "org.vala-apps.emendo",
               flags: GLib.ApplicationFlags.HANDLES_OPEN);
    }

    public override void startup() {
        base.startup();
        // accelerators
        add_accelerator("F10", "app.show-menu", null);
        add_accelerator("<Primary>Z", "app.undo", null);
        add_accelerator("<Primary>Y", "app.redo", null);
        add_accelerator("<Primary>O", "app.open", null);
        add_accelerator("<Primary>S", "app.save", null);
        add_accelerator("<Primary>N", "app.new", null);
        add_accelerator("<Primary><Shift>S", "app.save-all", null);
        add_accelerator("<Primary>F", "app.find", null);
        add_accelerator("<Primary>H", "app.replace", null);
        add_accelerator("<Primary>R", "app.wrap", null);
        add_accelerator("F9", "app.color", null);
        add_accelerator("<Primary>P", "app.pref", null);
        add_accelerator("<Primary>W",  "app.close", null);
        add_accelerator("<Primary><Shift>W", "app.close-all", null);
        add_accelerator("<Primary>Q", "app.quit", null);
        /*
        app.set_accels_for_action("app.show-menu",  {"F10"});
        app.set_accels_for_action("app.undo",       {"<Primary>Z"});
        app.set_accels_for_action("app.redo",       {"<Primary>Y"});
        app.set_accels_for_action("app.open",       {"<Primary>O"});
        app.set_accels_for_action("app.save",       {"<Primary>S"});
        app.set_accels_for_action("app.new",        {"<Primary>N"});
        app.set_accels_for_action("app.save-all",   {"<Primary><Shift>S"});
        app.set_accels_for_action("app.find",       {"<Primary>F"});
        app.set_accels_for_action("app.replace",    {"<Primary>H"});
        app.set_accels_for_action("app.wrap",       {"<Primary>R"});
        app.set_accels_for_action("app.color",      {"F9"});
        app.set_accels_for_action("app.pref",       {"<Primary>P"});
        app.set_accels_for_action("app.close",      {"<Primary>W"});
        app.set_accels_for_action("app.close-all",  {"<Primary><Shift>W"});
        app.set_accels_for_action("app.quit",       {"<Primary>Q"});
        */
        var mainwin = new Emendo.MainWin();
        mainwin.add_main_window(this);
        var operations = new Emendo.Operations();
        operations.add_recent_files();
    }

    public override void activate() {
        if (files.length() == 0) {
            string fileopen = GLib.Path.build_filename(GLib.Environment.get_tmp_dir(),
                              "untitled");
            var nbook = new Emendo.NBook();
            nbook.create_tab(fileopen);
        }
        get_active_window().present();
    }

    public override void open(File[] files, string hint) {
        string fileopen = null;
        foreach (File f in files) {
            fileopen = f.get_path();
            var nbook = new Emendo.NBook();
            nbook.create_tab(fileopen);
            var operations = new Emendo.Operations();
            operations.open_file(fileopen);
        }
        get_active_window().present();
    }

    private static int main (string[] args) {
        Emendo.Application app = new Emendo.Application();
        return app.run(args);
    }
}
}

namespace Pangea {
const string NAME        = "Pangea";
const string VERSION     = "1.0.0";
const string DESCRIPTION = "Browse the file system and manage the files";
const string ICON        = "system-file-manager";
const string[] AUTHORS   = { "Simargl <https://github.com/simargl>", null };

Gtk.ApplicationWindow window;
Gtk.IconView view;
Gtk.Entry location_entry;
Gtk.ListStore model;
Gtk.Menu menu;
Gtk.TreeIter iter;
GLib.FileMonitor current_monitor;
GLib.TimeoutSource time;
GLib.MainLoop loop;
Gtk.Statusbar statusbar;
Gtk.Grid grid_devices;
Gtk.Grid grid_bookmarks;
Gtk.Paned pane;
Gtk.Menu combo_path_menu;
uint context_id;
const Gtk.TargetEntry[] targets = { {"text/uri-list",0,0} };

int width;
int height;
int paned_pos;
int icon_size;
int thumbnail_size;
string saved_dir;
string terminal;

string current_dir;
string previous_dir;
GLib.List<string> list_dir;
GLib.List<string> list_file;
GLib.List<string> list_dir_history;
GLib.List<string> files_copy;
GLib.List<string> files_cut;

}

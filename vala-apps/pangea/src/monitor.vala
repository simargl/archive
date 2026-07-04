namespace Pangea {

public class DirectoryMonitor: GLib.Object {

    public void setup_file_monitor() {
        try {
            if (current_monitor != null) {
                current_monitor.cancel();
                current_monitor.changed.disconnect(on_file_changed);
                current_monitor = null;
                time.destroy();
                loop.quit();
            }
            //print("DEBUG: starting FileMonitor in %s\n", current_dir);
            current_monitor = GLib.File.new_for_path(current_dir).monitor_directory(
                                  GLib.FileMonitorFlags.NONE, null);
            current_monitor.changed.connect(on_file_changed);
            loop = new MainLoop();
            File tmpfile = File.new_for_path ("/tmp/.pangea.lock");
            time = new TimeoutSource(500);
            time.set_callback(() => {
                //print("DEBUG: checking for the lock file\n");
                if (tmpfile.query_exists() == false) {
                    //print("DEBUG: lock file not found, break loop\n");
                    loop.quit();
                    return false;
                }
                return true;
            });
            time.attach(loop.get_context());
            loop.run();
        } catch (GLib.Error e) {
            stderr.printf("%s\n", e.message);
        }
    }

    private void on_file_changed(GLib.File src, GLib.File? dest,
                                 GLib.FileMonitorEvent event) {
        //print("DEBUG: %s changed\n", current_dir);
        new Pangea.IconView().open_location(GLib.File.new_for_path(current_dir),
                                            false);
    }

}
}

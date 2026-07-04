namespace Agatha {
public class PageZoom: GLib.Object {
    public void zoom_plus() {
        zoom = zoom + 0.25;
        var viewer = new Agatha.Viewer();
        viewer.render_page();
        var settings = new Agatha.Settings();
        settings.set_zoom();
    }

    public void zoom_minus() {
        if (zoom < 0.25) {
            return;
        }
        zoom = zoom - 0.25;
        var viewer = new Agatha.Viewer();
        viewer.render_page();
        var settings = new Agatha.Settings();
        settings.set_zoom();
    }

}
}

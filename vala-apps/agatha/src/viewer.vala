namespace Agatha {
public class Viewer: Gtk.Application {
    public void render_page() {
        // document
        Poppler.Document document = null;
        try {
            document = new Poppler.Document.from_file(Filename.to_uri(filename), "");
        } catch(Error e) {
            error ("%s", e.message);
        }
        total = document.get_n_pages();
        // page size
        double page_width;
        double page_height;
        var page = document.get_page(cpage - 1);
        page.get_size(out page_width, out page_height);
        // image size
        int width  = (int)(zoom * page_width) ;
        int height = (int)(zoom * page_height);
        // render page to cairo context
        var surface = new Cairo.ImageSurface(Cairo.Format.ARGB32, width, height);
        var context = new Cairo.Context(surface);
        context.scale(zoom, zoom);
        page.render(context);
        // get pixbuf from surface
        Gdk.Pixbuf pixbuf = Gdk.pixbuf_get_from_surface(surface, 0, 0, width, height);
        // set title
        window.set_title(("%s (%d/%d)").printf(GLib.Path.get_basename(filename), cpage,
                                               total));
        // image from pixbuf
        image.set_from_pixbuf(pixbuf);
        // move scrollbar's adjustment
        scrolled.get_vadjustment().set_value(0);
        // save
        last_file = filename;
        last_page = cpage;
    }
}
}

//gcc `pkg-config --cflags gtk+-3.0` -o test test.c `pkg-config --libs gtk+-3.0`

#include <gtk/gtk.h>
#include <gdk/gdk.h>

// Define CSS for transparency
const char *css = "#toplevel { background-color: rgba(0, 255, 255, 0.5); }";

int main(int argc, char **argv) {
    gtk_init(&argc, &argv);

    // Create a new borderless window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Hello World");

    // Remove the title bar and borders
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);

    // Set window name as a property
    g_object_set_data(G_OBJECT(window), "toplevel", NULL);

    // Check if the default screen supports RGBA visuals
    GdkScreen *screen = gtk_window_get_screen(GTK_WINDOW(window));
    GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
    
    if (visual) {
        gtk_widget_set_visual(window, visual);
        gtk_widget_set_app_paintable(window, TRUE);
    } else {
        g_warning("RGBA visual not available. Transparency may not work.");
    }

    // Load CSS for transparency
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    gtk_style_context_add_provider_for_screen(screen, 
        GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    // Create a button and add it to the window
    GtkWidget *button1 = gtk_button_new_with_label("Hello, world!");
    gtk_widget_set_margin_bottom(button1, 50);
    gtk_widget_set_margin_top(button1, 50);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 50);
    gtk_box_pack_start(GTK_BOX(box), button1, TRUE, TRUE, 0);
    
    gtk_container_add(GTK_CONTAINER(window), box);
    gtk_widget_show_all(window);
    
    // Connect the destroy signal
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_main();
    
    return 0;
}

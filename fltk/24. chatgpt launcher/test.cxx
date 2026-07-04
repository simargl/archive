#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_PNG_Image.H>

int main() {
    // Create a window
    Fl_Window *window = new Fl_Window(600, 400, "FLTK Button with PNG Icon");

    // Load the PNG image
    Fl_PNG_Image *png = new Fl_PNG_Image("icon.png");

    // Create a button and set the icon
    Fl_Button *button = new Fl_Button(20, 20, 100, 100);
    button->label("Transmission");
    button->image(png);
    
    // Disable focus for the button
    button->clear_visible_focus();

    // End the window creation
    window->end();
    
    // Display the window
    window->show();

    // Run the FLTK event loop
    return Fl::run();
}

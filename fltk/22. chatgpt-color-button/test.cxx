#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>

class HoverButton : public Fl_Button {
public:
    HoverButton(int x, int y, int w, int h, const char *label = 0)
        : Fl_Button(x, y, w, h, label) {}

    int handle(int event) override {
        switch (event) {
            case FL_ENTER:
                color(FL_LIGHT3);  // Set the color when the button is hovered
                redraw();          // Redraw the button to apply the color change
                return 1;          // Return 1 to indicate that we handled the event
            case FL_LEAVE:
                color(FL_LIGHT1);  // Set the initial button color to blue
                redraw();          // Redraw the button to apply the color change
                return 1;          // Return 1 to indicate that we handled the event
        }
        return Fl_Button::handle(event);  // Call the default handle method for other events
    }
};

void button_callback(Fl_Widget *widget, void *data) {
    HoverButton *button = static_cast<HoverButton *>(widget);
    if (button->label() == "Hello") {
        button->label("World");
    } else {
        button->label("Hello");
    }
}

int main() {
    Fl::scheme("oxy");  // Set the dark theme

    Fl_Window window(300, 200, "FLTK Hello World Example");
    window.color(FL_DARK3);  // Set a dark background color

    HoverButton button(30, 30, 240, 140);
    button.label("Hello");
    button.color(FL_LIGHT1);
    button.callback(button_callback);

    // Disable visible focus dots for the button
    button.clear_visible_focus();

    window.add(&button);

    window.show();
    return Fl::run();
}

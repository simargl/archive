// test.cxx - Fixed version (compiles with fltk-config --compile)

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/fl_draw.H>
#include <cstdlib>          // ← added for exit()
#include <unistd.h>         // ← sometimes needed on Linux for exit()

const int HEADER_HEIGHT = 40;

class DraggableHeader : public Fl_Widget {
    Fl_Window *win;
    int drag_start_x, drag_start_y;
    bool dragging = false;

    void draw() override {
        fl_color(FL_DARK3);
        fl_rectf(x(), y(), w(), h());

        fl_color(FL_WHITE);
        fl_font(FL_HELVETICA_BOLD, 14);
        const char *title = win->label() ? win->label() : "Untitled Window";
        int tw = 0, th = 0;
        fl_measure(title, tw, th);
        fl_draw(title, x() + (w() - tw) / 2, y() + (h() + th) / 2 - 4);
    }

    int handle(int event) override {
        switch (event) {
            case FL_PUSH:
                if (Fl::event_button() == FL_LEFT_MOUSE) {
                    dragging = true;
                    drag_start_x = Fl::event_x_root() - win->x();
                    drag_start_y = Fl::event_y_root() - win->y();
                    return 1;
                }
                break;
            case FL_DRAG:
                if (dragging) {
                    win->position(Fl::event_x_root() - drag_start_x,
                                  Fl::event_y_root() - drag_start_y);
                    return 1;
                }
                break;
            case FL_RELEASE:
                dragging = false;
                return 1;
        }
        return Fl_Widget::handle(event);
    }

public:
    DraggableHeader(int X, int Y, int W, int H, Fl_Window *window)
        : Fl_Widget(X, Y, W, H), win(window) {
        box(FL_FLAT_BOX);
    }
};

class CustomWindow : public Fl_Window {
    void create_header() {
        new DraggableHeader(0, 0, w(), HEADER_HEIGHT, this);

        // Minimize button
        Fl_Button *min_btn = new Fl_Button(w() - 80, 4, 40, HEADER_HEIGHT - 8);
        min_btn->copy_label("_");
        min_btn->labelsize(20);
        min_btn->labelcolor(FL_WHITE);
        min_btn->box(FL_FLAT_BOX);
        min_btn->labelfont(FL_HELVETICA_BOLD);
        min_btn->clear_visible_focus();
        min_btn->callback([](Fl_Widget*, void* v) {
            static_cast<Fl_Window*>(v)->iconize();
        }, this);

        // Close button - fixed: use hide() instead of exit(0)
        Fl_Button *close_btn = new Fl_Button(w() - 40, 4, 40, HEADER_HEIGHT - 8);
        close_btn->copy_label("X");
        close_btn->labelsize(20);
        close_btn->labelcolor(FL_WHITE);
        close_btn->box(FL_FLAT_BOX);
        close_btn->labelfont(FL_HELVETICA_BOLD);
        close_btn->clear_visible_focus();
        close_btn->callback([](Fl_Widget* w, void*) {
            w->window()->hide();   // proper way to close an FLTK window
        });

        Fl_Box *content = new Fl_Box(0, HEADER_HEIGHT, w(), h() - HEADER_HEIGHT,
            "This is the main content area.\n\n"
            "Drag the dark header bar at the top to move the window.\n\n"
            "GTK3-style client-side header bar in FLTK");
        content->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
        content->labelfont(FL_HELVETICA);
        content->labelsize(16);
    }

public:
    CustomWindow(int W, int H, const char *label = "")
        : Fl_Window(W, H + HEADER_HEIGHT, label) {
        color(FL_WHITE);
        border(false);
        begin();
        create_header();
        end();
        resizable(this);
    }

    // Keep the override that removes decorations even after show()
    void show() override {
        Fl_Window::show();
        border(false);
    }
};

int main(int argc, char **argv) {
    CustomWindow win(500, 400, "FLTK GTK3-like Header Bar Demo");
    win.show();                 // ← fixed: no argc/argv needed here
    return Fl::run();
}

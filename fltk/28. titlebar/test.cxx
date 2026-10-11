// g++ test.cxx -o test $(fltk-config --cxxflags) $(fltk-config --ldflags)

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>

class Header : public Fl_Box {
    int drag;
    int dx, dy;
    int hover;

public:
    Header(int X, int Y, int W, int H)
        : Fl_Box(X, Y, W, H),
          drag(0),
          dx(0),
          dy(0),
          hover(0)
    {
    }

    void draw() override
    {
        // Header background
        fl_color(fl_rgb_color(32, 32, 36));
        fl_rectf(x(), y(), w(), h());

        // Close button
        int bx = w() - 46;

        fl_color(
            hover
                ? fl_rgb_color(196, 43, 43)
                : fl_rgb_color(32, 32, 36)
        );

        fl_rectf(bx, 0, 46, h());

        // Title
        fl_color(fl_rgb_color(235, 235, 240));
        fl_font(FL_HELVETICA_BOLD, 15);

        fl_draw(
            "Hello FLTK",
            0, 0, w(), h(),
            FL_ALIGN_CENTER
        );

        // Close icon
        fl_color(FL_WHITE);
        fl_font(FL_HELVETICA, 20);

        fl_draw(
            "×",
            bx, 0, 46, h(),
            FL_ALIGN_CENTER
        );
    }

    int handle(int e) override
    {
        Fl_Window *win = window();

        // Close button starts here
        int bx = w() - 46;

        switch (e)
        {
            // Mouse enters the header
            case FL_ENTER:
            {
                return 1;
            }

            // Mouse moves inside the header
            case FL_MOVE:
            {
                int over_close =
                    Fl::event_x() >= bx &&
                    Fl::event_x() < w() &&
                    Fl::event_y() >= 0 &&
                    Fl::event_y() < h();

                if (over_close != hover)
                {
                    hover = over_close;
                    redraw();
                }

                return 1;
            }

            // Mouse leaves the header
            case FL_LEAVE:
            {
                if (hover)
                {
                    hover = 0;
                    redraw();
                }

                return 1;
            }

            // Mouse button pressed
            case FL_PUSH:
            {
                if (Fl::event_button() != 1)
                    return 1;

                // Clicked close button
                if (Fl::event_x() >= bx)
                {
                    win->hide();
                    return 1;
                }

                // Start window dragging
                drag = 1;

                dx = Fl::event_x_root() - win->x();
                dy = Fl::event_y_root() - win->y();

                return 1;
            }

            // Window is being dragged
            case FL_DRAG:
            {
                if (drag)
                {
                    win->position(
                        Fl::event_x_root() - dx,
                        Fl::event_y_root() - dy
                    );

                    return 1;
                }

                return 1;
            }

            // Mouse button released
            case FL_RELEASE:
            {
                drag = 0;
                return 1;
            }
        }

        return Fl_Box::handle(e);
    }
};


int main()
{
    // Use GTK+ FLTK scheme
    Fl::scheme("gtk+");

    // Main window
    Fl_Window win(
        640,
        400,
        "Hello FLTK"
    );

    // Remove native window border
    win.border(0);

    // Dark background
    win.color(
        fl_rgb_color(24, 24, 28)
    );

    // Custom header
    Header header(
        0,
        0,
        640,
        56
    );

    // Main title
    Fl_Box title(
        0,
        130,
        640,
        45,
        "Hello, World!"
    );

    title.box(FL_NO_BOX);
    title.labelsize(30);
    title.labelfont(FL_HELVETICA_BOLD);
    title.labelcolor(
        fl_rgb_color(245, 245, 250)
    );

    // Subtitle
    Fl_Box subtitle(
        0,
        180,
        640,
        30,
        "A modern dark FLTK application"
    );

    subtitle.box(FL_NO_BOX);
    subtitle.labelsize(15);
    subtitle.labelcolor(
        fl_rgb_color(160, 160, 170)
    );

    // Finish adding widgets
    win.end();

    // Show window
    win.show();

    // Start FLTK event loop
    return Fl::run();
}

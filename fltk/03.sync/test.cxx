// g++ `fltk-config --cxxflags` test.cxx `fltk-config --ldflags` -o test

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Slider.H>

Fl_Window *window;
Fl_Spinner *spinner;
Fl_Slider *slider;

void spinner_changed(Fl_Widget *w, void* userdata)
{
    slider->value(spinner->value());
}

void slider_changed(Fl_Widget *w, void* userdata)
{
    spinner->value(slider->value());
}

int main(int argc, char **argv)
{
    window = new Fl_Window(320, 40);
    window->label("Enter your age");

    spinner = new Fl_Spinner(5, 5, 150, 30, 0);
    spinner->maximum(130);
    spinner->callback(spinner_changed);

    slider = new Fl_Slider(160, 5, 150, 30);
    slider->type(1);
    slider->scrollvalue(1, 1, 1, 130);
    slider->callback(slider_changed);

    window->resizable(window);
    window->end();
    window->show(argc, argv);
    return Fl::run();
}

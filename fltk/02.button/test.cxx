// g++ `fltk-config --cxxflags` test.cxx `fltk-config --ldflags` -o test

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>

Fl_Window *window;
Fl_Button *button;

void button_clicked(Fl_Widget *w, void* userdata)
{
    button->label("Thank you");
}

int main(int argc, char **argv)
{
    Fl::scheme("oxy");    // "gtk+" "plastic" "oxy" and "gleam" are popular schemes 
    Fl::get_system_colors();
    Fl::background(60, 60, 60);
    Fl::foreground(220, 220, 220);
    
    
    window = new Fl_Window(350, 70);
    window->label("First FLTK Program");
    window->border(10);

    button = new Fl_Button(25, 10, 300, 50, "Click me!");
    button->callback(button_clicked);

    window->resizable(button);
    window->end();
    window->show(argc, argv);
    return Fl::run();
}

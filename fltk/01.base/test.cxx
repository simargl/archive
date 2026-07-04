// g++ `fltk-config --cxxflags` test.cxx `fltk-config --ldflags` -o test
// i686-w64-mingw32-g++ test.cxx -o test.exe -I/opt/mingw32/include -L/opt/mingw32/lib -lfltk -mwindows

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>

Fl_Window *window;
Fl_Button *button;

int main(int argc, char **argv)
{
    window = new Fl_Window(200, 200);
    window->end();
    window->show(argc, argv);
    return Fl::run();
}

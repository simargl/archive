// fltk-config --compile test.cxx --use-images

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_PNG_Image.H>

int main() {
    Fl_Window *win = new Fl_Window(500,200,"foo");
    const char *name = "test.png"; 
    Fl_PNG_Image *img = new Fl_PNG_Image(name);
    win->icon(img);
    win->show();
    return(Fl::run());
}

// fltk-config --compile test.cxx

#include <stdlib.h> 
#include <stdio.h> 

#include <FL/Fl.H> 
#include <FL/Fl_Double_Window.H> 
#include <FL/Fl_Box.H> 
#include <FL/Fl_Button.H> 

class mywin : public Fl_Double_Window 
{ 
protected: 
    int handle (int); 
public: 
    mywin(int W, int H, const char *L);
};

mywin::mywin(int W, int H, const char *L=0): Fl_Double_Window(W,H,L) {
    mywin::clear_border();
    mywin::show();
} 

int mywin::handle(int ev) { 
    int xx; 
    int yy; 
    int res = Fl_Double_Window::handle(ev); 
    if (ev == FL_DRAG) { 
        xx = Fl::event_x() - x(); 
        yy = Fl::event_y() - y(); 
        Fl_Double_Window::position(x() + Fl::event_x() - decorated_w()/2, y() + Fl::event_y() - decorated_h()/2);
        fflush(stdout); 
        res = 1;
    } 
    return res;
}

int main(int argc, char **argv) { 
    Fl_Double_Window *win; 
    win = new mywin(555, 330, "test");
    return Fl::run(); 
} 

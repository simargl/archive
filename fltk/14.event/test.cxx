
/************************************************************************/ 
// compile as: fltk-config --compile mouse_move.cxx 
#include <stdlib.h> 
#include <stdio.h> 

#include <FL/Fl.H> 
#include <FL/Fl_Double_Window.H> 
#include <FL/Fl_Box.H> 
#include <FL/Fl_Button.H> 

Fl_Double_Window *main_win = NULL; 
/************************************************************************/ 
class click_box : public Fl_Box 
{ 
protected: 
    int handle (int); 
public: 
    click_box(int X, int Y, int W, int H, const char *L=0) : Fl_Box(X,Y,W,H,L) { } 
}; 
/************************************************************************/ 
int click_box::handle(int ev) { 
    int xx; 
    int yy; 
    int res = Fl_Box::handle(ev); 
    if (ev == FL_PUSH) { 
        xx = Fl::event_x() - x(); 
        yy = Fl::event_y() - y(); 
        printf("Click at (%d, %d)\n", xx, yy); 
        fflush(stdout); 
        res = 1; // we ate this event 
    } 
    else 
    if ((ev == FL_RELEASE) || 
        (ev == FL_ENTER)   || 
        (ev == FL_LEAVE)   ) 
    { 
        res = 1; // we ate this event 
    } 
    else 
    if (ev == FL_DRAG) 
    { 
        xx = Fl::event_x() - x(); 
        yy = Fl::event_y() - y(); 
        printf("Drag at (%d, %d)\n", xx, yy); 
        fflush(stdout); 
        res = 1; // we ate this event 
    } 
    else 
    if (ev == FL_MOVE) 
    { 
        xx = Fl::event_x() - x(); 
        yy = Fl::event_y() - y(); 
        printf("Move at (%d, %d)\n", xx, yy); 
        fflush(stdout); 
        res = 1; // we ate this event 
    } 
    else 
    if (ev == FL_MOUSEWHEEL) { 
        int dx = Fl::event_dx(); 
        int dy = Fl::event_dy(); 
        printf("Got a wheel event (%d, %d)\n", dx, dy); 
        fflush(stdout); 
        res = 1; // we ate this event 
    } 

    return res; // return base widget status 
} /* end of handle() method */ 
/************************************************************************/ 
click_box *test_box; 
/************************************************************************/ 
void quit_cb(Fl_Button *, void *) { 
    main_win->hide(); 
} 
/************************************************************************/ 
int main(int argc, char **argv) { 
    main_win = new Fl_Double_Window(522, 570); 
    main_win->begin(); 

    test_box = new click_box(5, 5, 512, 512); 
    test_box->box(FL_BORDER_BOX); 

    Fl_Button *quit = new Fl_Button(452, 525, 60, 30, "Quit"); 
    quit->box(FL_THIN_UP_BOX); 
    quit->callback((Fl_Callback *)quit_cb); 

    main_win->end(); 
    main_win->show(argc, argv); 
    return Fl::run(); 
} 
/* End of File */ 

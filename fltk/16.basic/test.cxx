#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>

void but_clicked(Fl_Widget *w, void *data) {
    w->label("Thank you");
}

int main(int argc, char *argv[]) {
    // window
    Fl_Window *win = new Fl_Window(400, 300, "Hello World");
    win->box(FL_FLAT_BOX); win->color(51); win->begin();
    
    // button
    Fl_Button *but = new Fl_Button(10, 10, 300, 30, "Click Me");
    but->box(FL_BORDER_BOX); but->color(93); but->callback(but_clicked);

    // run
    win->end(); 
    //win->clear_border();
    win->show();    
    return Fl::run();
}

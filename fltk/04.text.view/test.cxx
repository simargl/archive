// g++ `fltk-config --cxxflags` test.cxx `fltk-config --ldflags` -o test

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <cstdlib>

Fl_Window *window;
Fl_Menu_Bar *menu;
Fl_Text_Buffer *buf;
Fl_Text_Display *text;
Fl_Native_File_Chooser *chooser;

void action_open(Fl_Widget *, void *)
{
    chooser = new Fl_Native_File_Chooser();
    chooser->type(Fl_Native_File_Chooser::BROWSE_FILE);
    if ( chooser->show() ) return;
    buf->loadfile(chooser->filename());
}

void action_quit(Fl_Widget *, void *)
{
    exit(0);
}

int main(int argc, char **argv)
{
    window = new Fl_Window(800, 600);
    window->label("Text File Viewer");
    
    menu = new Fl_Menu_Bar(0, 0, 800, 25);
    menu->add("File/Open", FL_CTRL+'o', action_open);
    menu->add("File/Quit", FL_CTRL+'q', action_quit);

    buf = new Fl_Text_Buffer();

    text = new Fl_Text_Display(0, 25, 800, 600, "");
    text->buffer(buf);
    text->textfont(FL_COURIER);
    text->textsize(20);

    window->resizable(text);
    window->end();
    window->show(argc, argv);
    return Fl::run();
}

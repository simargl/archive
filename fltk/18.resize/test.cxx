#include <cstdio>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>


class Group : public Fl_Group
{
private:
  Fl_Box *b1;
  Fl_Box *b2;

public:
    Group(int x, int y, int w, int h) : Fl_Group(x, y, w, h) 
    {
        b1 = new Fl_Box(10, 10, 240, 20);
        b2 = new Fl_Box(b1->x()+b1->w()+10, 10, 30, 20);    
        b1->box(FL_BORDER_BOX);
        b2->box(FL_BORDER_BOX);            
        resizable(b1);
        end();
    }
    
    void resize(int x, int y, int _w, int h)
    {
        Fl_Group::resize(x, y, _w, h);
        if (_w < 250) {
            b2->hide();
            b1->size(_w-20, b1->h());
        }
        else {
            b2->show();
            b1->size(_w-60, b1->h());
        }
        Fl_Group::init_sizes();
    }
    

};


int main(int argc, char **argv)
{
    Fl_Window *win = new Fl_Window(300, 200);
    Group *group = new Group(0, 0, 300, 200);
    win->add(group);
    win->end();
    win->resizable(group);
    win->show();
    return(Fl::run());
}

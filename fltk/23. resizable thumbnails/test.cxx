#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Pixmap.H>
#include <FL/fl_ask.H>
#include <stdio.h>
//
// Demonstrate simple scrollable thumbnail browser
// erco@netcom.com 08/06/02 -- "Draggable Boxes on Scrollable Desk" demo
// erco@seriss.com 06/29/16 -- Rewritten as thumbnail viewer
//
static const char *cat_xpm[] = {                       // XPM
"50 34 4 1",
"  c black",
"o c #ff9900",
"@ c #ffffff",
"# c None",
"##################################################",
"###      ##############################       ####",
"### ooooo  ###########################  ooooo ####",
"### oo  oo  #########################  oo  oo ####",
"### oo   oo  #######################  oo   oo ####",
"### oo    oo  #####################  oo    oo ####",
"### oo     oo  ###################  oo     oo ####",
"### oo      oo                     oo      oo ####",
"### oo       oo  ooooooooooooooo  oo       oo ####",
"### oo        ooooooooooooooooooooo        oo ####",
"### oo     ooooooooooooooooooooooooooo    ooo ####",
"#### oo   ooooooo ooooooooooooo ooooooo   oo #####",
"####  oo oooooooo ooooooooooooo oooooooo oo  #####",
"##### oo oooooooo ooooooooooooo oooooooo oo ######",
"#####  o ooooooooooooooooooooooooooooooo o  ######",
"###### ooooooooooooooooooooooooooooooooooo #######",
"##### ooooooooo     ooooooooo     ooooooooo ######",
"##### oooooooo  @@@  ooooooo  @@@  oooooooo ######",
"##### oooooooo @@@@@ ooooooo @@@@@ oooooooo ######",
"##### oooooooo @@@@@ ooooooo @@@@@ oooooooo ######",
"##### oooooooo  @@@  ooooooo  @@@  oooooooo ######",
"##### ooooooooo     ooooooooo     ooooooooo ######",
"###### oooooooooooooo       oooooooooooooo #######",
"###### oooooooo@@@@@@@     @@@@@@@oooooooo #######",
"###### ooooooo@@@@@@@@@   @@@@@@@@@ooooooo #######",
"####### ooooo@@@@@@@@@@@ @@@@@@@@@@@ooooo ########",
"######### oo@@@@@@@@@@@@ @@@@@@@@@@@@oo ##########",
"########## o@@@@@@ @@@@@ @@@@@ @@@@@@o ###########",
"########### @@@@@@@     @     @@@@@@@ ############",
"############  @@@@@@@@@@@@@@@@@@@@@  #############",
"##############  @@@@@@@@@@@@@@@@@  ###############",
"################    @@@@@@@@@    #################",
"####################         #####################",
"##################################################",
};

// Manage arranging thumbnails in a viewer
//     Assumes thumbnails are all the same size.
//
class MyThumbnailViewer : public Fl_Scroll {
    int xsep;
    int ysep;

    // Rearrange the children based on new xywh
    void RearrangeChildren(int X, int Y, int W, int H) {
      int nchildren = children() - 2;   // -2: ignore Fl_Scroll's two child scrollbars
      if ( nchildren < 1 ) return;
      int sw = ( scrollbar.visible() ) ? scrollbar.w() : 0;
      // Figure out how many thumbnails we can arrange horizontally
      int thumb_w = (child(0)->w() + xsep);     // overall width of thumbnail, including separation
      int thumb_h = (child(0)->h() + ysep);     // overall height of thumbnail, including separation
      int nw      = (W - sw - xsep) / thumb_w;  // max # thumbnails we can arrage horizontally before wrapping
      int remain  = (W - sw - xsep) % thumb_w;  // remainder of division
      if ( nw < 2 ) nw = 2;                     // minimum number of thumbs
      int left = X + xsep + (remain/2);         // left edge of thumb arrangement
      int top  = Y + ysep;                      // top edge of thumb arrangement
      // Now walk the children, moving them around
      int tx = left;
      int ty = top;
      for ( int i=0; i<nchildren; i++ ) {
          Fl_Widget *w = child(i);
          w->position(tx,ty);
          if ( (i % nw) == (nw - 1) ) {         // wrap?
            ty += thumb_h;                      // advance to next row
            tx = left;                          // reset to left
          } else {                      // no wrap?
            tx += thumb_w;              // just advance to next right position
          }
      }
    }
    void resize(int X, int Y, int W, int H) {
        RearrangeChildren(X,Y,W,H);     // move children around to fit new size
        Fl_Scroll::resize(X,Y,W,H);     // let Fl_Scroll handle the new arrangement
    }
public:
    MyThumbnailViewer(int X, int Y, int W, int H, const char *L=0) : Fl_Scroll(X,Y,W,H,L) {
        xsep = 5;
        ysep = 5;
    }
    // Set the x/y separation between thumbnails
    void separation(int X, int Y) {
        xsep = X;
        ysep = Y;
        autoarrange();
    }
    // Force auto-arrangement of children
    void autoarrange() {
        RearrangeChildren(x(),y(),w(),h());
        // init_sizes();    // XXX: doesn't help
        Fl_Scroll::redraw();
    }
    void draw() {
        // XXX: this trick shouldn't be needed, but without it
        //      children initially start in really strange positions
        static int first = 1;
        if ( first ) { autoarrange(); first = 0; }
        Fl_Scroll::draw();
    }
};

Fl_Double_Window  *G_win       = NULL;
MyThumbnailViewer *G_thumbview = NULL;
static Fl_Pixmap   G_cat(cat_xpm);

// A THUMBNAIL WITH AN IMAGE IN IT
class Thumbnail : public Fl_Box {
    int handle(int event) {
        int ret = 0;
        switch ( event ) {
            case FL_PUSH:
                if ( Fl::event_button() == 1 ) {
                    ret = 1;
                }
                break;
            case FL_RELEASE:
                if ( Fl::event_button() == 1 &&
                     Fl::event_inside(x(),y(),w(),h()) ) {
                    ret = 1;
                    fl_alert("thumbnail '%s' clicked", label());
                }
                break;
        }
        return Fl_Box::handle(event) | ret;
    }
public:
    Thumbnail(int X, int Y, int W, int H, const char *L=0) : Fl_Box(X,Y,W,H,L) {
        image(G_cat);
        box(FL_UP_BOX);
        color(FL_GRAY);
    }
};

/// MAIN
int main() {
    G_win       = new Fl_Double_Window(600,400);
    G_thumbview = new MyThumbnailViewer(10,10,G_win->w()-20,G_win->h()-20);
    G_thumbview->separation(30,30);     // set separation between thumbnails
    G_thumbview->box(FL_FLAT_BOX);
    G_thumbview->color(Fl_Color(46));
    G_thumbview->begin();
    {
        // CREATE A BUNCH OF NEW BOXES
        //     We'll let the thumb viewer position them on x/y,
        //     so we can create them all in the same x/y position..
        //
        char s[20];
        for ( int t=0; t<40; t++ ) {    // create 30 instances
            Thumbnail *tn = new Thumbnail(0,0,80,50);
            sprintf(s, "%04d", t);      // label each instance e.g. "0000"
            tn->copy_label(s);
        }
    }
    G_thumbview->end();
    G_thumbview->autoarrange();         // XXX: should set initial positions, but doesn't?
    G_win->end();
    G_win->resizable(G_win);
    G_win->show();
    return(Fl::run());
}

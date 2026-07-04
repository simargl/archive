#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_File_Browser.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Box.H>


extern Fl_Double_Window *win;
extern Fl_File_Browser *b1;
extern Fl_File_Browser *b2;
extern Fl_Input *i1;
extern Fl_Input *i2;
extern Fl_Menu_Item menu_[];



void open_folder(const char *f, Fl_File_Browser *b, Fl_Input *i);
void b1_clicked();
void b2_clicked();
void i1_changed();
void i2_changed();
void action_view();
void action_edit();
void action_exit();
void action_copy();
void action_move();
extern Fl_Double_Window *win_nf;

extern Fl_Input *win_nf_input;
Fl_Double_Window* action_new_folder();
void action_delete();
#endif

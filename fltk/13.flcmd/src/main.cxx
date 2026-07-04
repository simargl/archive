#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_File_Browser.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/fl_ask.H>
#include <unistd.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include <cstdlib>

bool active_is_first;
Fl_Double_Window *mainwin;
Fl_File_Browser *browser1;
Fl_File_Browser *browser2;
Fl_Input *input1;
Fl_Input *input2;
Fl_Button *button_view;
Fl_Button *button_edit;
Fl_Button *button_copy;
Fl_Button *button_move;
Fl_Button *button_new_folder;
Fl_Button *button_delete;
Fl_Button *button_exit;
Fl_Group *group1;

void button_copy_clicked(Fl_Widget*, void*) {
    if (active_is_first == true) {
        printf("%s\n", "copy from 1 to 2");
    } else {
        printf("%s\n", "copy from 2 to 1");
    }    
}

void browser1_item_clicked(Fl_Widget*, void*) {
    // get selected line
    int index = browser1->value();
    if (index == 0)
        return;
    active_is_first = true;
    browser2->deselect();
    // ignore right click
    if( Fl::event_button() == FL_RIGHT_MOUSE || Fl::event_ctrl() != 0 )
        return;
    // get directory and file name
    const char *file = browser1->text(index);
    const char *dir = input1->value();
    char result[100];
    strcpy(result,dir);
    strcat(result,"/");
    strcat(result,file);
    // change directory or open file
    int ret_val = chdir(result);
    if (ret_val == 0) {
        char *buf;
        buf=(char*)malloc(100*sizeof(char));
        if (getcwd(buf, 100) != NULL) {
            browser1->load(buf);
            input1->value(buf);
        }
    }
}

void browser2_item_clicked(Fl_Widget*, void*) {
    // get selected line
    int index = browser2->value();
    if (index == 0)
        return;
    active_is_first = false;
    browser1->deselect();
    // ignore right click
    if( Fl::event_button() == FL_RIGHT_MOUSE || Fl::event_ctrl() != 0 )
        return;
    // get directory and file name
    const char *file = browser2->text(index);
    const char *dir = input2->value();
    char result[100];
    strcpy(result,dir);
    strcat(result,"/");
    strcat(result,file);
    // change directory or open file
    int ret_val = chdir(result);
    if (ret_val == 0) {
        char *buf;
        buf=(char*)malloc(100*sizeof(char));
        if (getcwd(buf, 100) != NULL) {
            browser2->load(buf);
            input2->value(buf);
        }
    }
}
int main(int argc, char **argv) {
    mainwin = new Fl_Double_Window(775, 610, "FLTK Commander");
    mainwin->color(FL_LIGHT1);
    
    browser1 = new Fl_File_Browser(5, 70, 380, 505);
    browser1->selection_color(FL_DARK_RED);
    browser1->labelfont(5);
    browser1->labelsize(20);
    browser1->labelcolor(FL_GRAY0);
    browser1->textfont(5);
    browser1->textsize(15);
    browser1->type(FL_MULTI_BROWSER);
    browser1->callback(browser1_item_clicked);
    
    browser2 = new Fl_File_Browser(390, 70, 380, 505);
    browser2->selection_color(FL_DARK_RED);
    browser2->labelfont(5);
    browser2->labelsize(20);
    browser2->labelcolor(FL_GRAY0);
    browser2->textfont(5);
    browser2->textsize(15);
    browser2->type(FL_MULTI_BROWSER);
    browser2->callback(browser2_item_clicked);
    
    input1 = new Fl_Input(5, 39, 380, 25);
    input1->selection_color((Fl_Color)219);
    input1->labelcolor(FL_GRAY0);
    input1->textfont(5);
    input1->textsize(15);
    input1->when(FL_WHEN_ENTER_KEY);
    
    input2 = new Fl_Input(390, 40, 380, 25);
    input2->selection_color((Fl_Color)219);
    input2->labelcolor(FL_GRAY0);
    input2->textfont(5);
    input2->textsize(15);
    input2->when(FL_WHEN_ENTER_KEY);
    
    group1 = new Fl_Group(5, 575, 790, 35);
    button_view = new Fl_Button(5, 580, 105, 25, "F3 View");
    button_view->shortcut(0xffc0);
    
    button_edit = new Fl_Button(115, 580, 105, 25, "F4 Edit");
    button_edit->shortcut(0xffc1);
    
    button_copy = new Fl_Button(225, 580, 105, 25, "F5 Copy");
    button_copy->shortcut(0xffc2);
    button_copy->callback(button_copy_clicked);
    
    button_move = new Fl_Button(335, 580, 105, 25, "F6 Move");
    button_move->shortcut(0xffc3);
    
    button_new_folder = new Fl_Button(445, 580, 105, 25, "F7 New Folder");
    button_new_folder->shortcut(0xffc4);
    
    button_delete = new Fl_Button(555, 580, 105, 25, "F8 Delete");
    button_delete->shortcut(0xffc5);
    
    button_exit = new Fl_Button(665, 580, 105, 25, "Alt+F4 Exit");
    button_exit->shortcut(0xffe9);
    group1->end();

    Fl_Menu_Item menu_[] = {
        {"File", 0,  0, 0, 64, FL_NORMAL_LABEL, 0, 14, 0},
        {"Exit", 0,  0, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
        {0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0}
    };
    
    Fl_Menu_Bar* o = new Fl_Menu_Bar(5, 7, 765, 25);
    o->color(FL_LIGHT1);
    o->box(FL_FLAT_BOX);
    o->menu(menu_);

    const char *name = "folder.png"; 
    Fl_PNG_Image *img = new Fl_PNG_Image(name);
    
    Fl::scheme("gtk+");
    mainwin->icon(img);
    mainwin->size_range(700, 410);
    mainwin->end();
    mainwin->resizable(mainwin);
    
    input2->value("/mnt");
    browser2->load("/mnt");
    input1->value("/");
    browser1->load("/");
    
    if (chdir("/") != 0)
        return 1;
    
    mainwin->show();
    return(Fl::run());
}

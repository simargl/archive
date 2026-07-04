/*  Author: simargl <https://github.org/simargl>
 *  License: GPL v3
 */
#include "main.h"

Fl_Double_Window    *window;
Fl_Input            *input;
Fl_File_Browser     *browser;
Fl_Box              *box;

void browser_item_clicked() {
    // ignore right click
    if( Fl::event_button() == FL_RIGHT_MOUSE )
        return;
    // get selected line
    int index = browser->value();
    if (index == 0)
        return;
    // get directory and file name
    const char *file = browser->text(index);
    const char *dir = input->value();
    char result[100];
    strcpy(result,dir);
    strcat(result,"/");
    strcat(result,file);
    // change directory or open file
    int ret_val = chdir(result);
    if (0 == ret_val) {
        char *buf;
        buf=(char *)malloc(100*sizeof(char));
        getcwd(buf, 100);
        browser->load(buf);
        input->value(buf);
    }	else {
        // find extension
        std::string fn = result;
        std::string ext = fn.substr(fn.find_last_of(".") + 1);
        // JPG
        if (ext == "jpeg" || ext == "jpg") {
            Fl_JPEG_Image* jpeg;
            jpeg = new Fl_JPEG_Image(result);
            if (jpeg->w() > box->w()) {
                Fl_Image* r;
                r = jpeg->copy( box->w(), (int)box->w()/((float)jpeg->w()/jpeg->h()) );
                box->image(r);
                // freeing memory
                delete jpeg;                
            }   else {
                box->image(jpeg);
            }
            box->redraw();
            input->redraw();
        }
        // PNG
        if (ext == "png") {
            Fl_PNG_Image* png;
            png = new Fl_PNG_Image(result);
            if (png->w() > box->w()) {
                Fl_Image* r;
                r = png->copy( box->w(), (int)box->w()/((float)png->w()/png->h()) );
                box->image(r);
                // freeing memory
                delete png;
            }   else {
                box->image(png);
            }
            box->redraw();
            input->redraw();
        }
        // BMP
        if (ext == "bmp") {
            Fl_BMP_Image* bmp;
            bmp = new Fl_BMP_Image(result);
            if (bmp->w() > box->w()) {
                Fl_Image* r;
                r = bmp->copy( box->w(), (int)box->w()/((float)bmp->w()/bmp->h()) );
                box->image(r);
                // freeing memory
                delete bmp;
            }   else {
                box->image(bmp);
            }
            box->redraw();
            input->redraw();
        }              
        
    }
}   

void input_changed() {
    const char *result = input->value();
    int ret_val = chdir(result);
    if(0 == ret_val) {
        char *buf;
        buf=(char *)malloc(100*sizeof(char));
        getcwd(buf, 100);
        browser->load(buf);
        input->value(buf);
    }
}

void load_current_directory() {
    browser->load(".");
    char *buf;
    buf=(char *)malloc(100*sizeof(char));
    getcwd(buf, 100);
    input->value(buf);
}

int main(int argc, char **argv) {
    window  = new Fl_Double_Window  (875, 465);
    input   = new Fl_Input          (0, 0, 255, 30);
    browser = new Fl_File_Browser   (0, 30, 255, 435);
    box     = new Fl_Box            (255, 0, 620, 466, "");
    browser->type(FL_HOLD_BROWSER);
    browser->callback((Fl_Callback*)browser_item_clicked);
    input->callback((Fl_Callback*)input_changed);
    // style
    input->box(FL_FLAT_BOX);                browser->box(FL_FLAT_BOX);              box->box(FL_FLAT_BOX);
    input->textfont(5);                     browser->textfont(5);
    input->selection_color(FL_DARK_RED);    browser->selection_color(FL_DARK_RED);
    input->textsize(15);                    browser->textsize(15);
    load_current_directory();
    // icon
    const char *name = "icon.png"; 
    Fl_PNG_Image *img = new Fl_PNG_Image(name);
    window->label("FView <https://github.org/simargl>");
    window->icon(img);
    window->resizable(box);
    window->end();
    window->show(argc, argv);
    return Fl::run();
}

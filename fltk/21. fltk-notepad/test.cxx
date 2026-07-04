//g++ test.cxx -o test `fltk-config --cxxflags --ldflags`

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Native_File_Chooser.H>

#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <cctype>

Fl_Window *win;
Fl_Text_Editor *editor;
Fl_Text_Buffer *textbuf;
Fl_Text_Buffer *stylebuf;

std::string current_file;
bool modified = false;
bool loading_file = false;
int FONT_SIZE = 16;

enum
{
    STYLE_A = 'A',
    STYLE_B = 'B',
    STYLE_C = 'C',
    STYLE_D = 'D',
    STYLE_E = 'E'
};

Fl_Text_Display::Style_Table_Entry styletable[] =
{
    { FL_WHITE, FL_COURIER, 14 },
    { fl_rgb_color(86,156,214), FL_COURIER_BOLD, FONT_SIZE },
    { fl_rgb_color(106,153,85), FL_COURIER, FONT_SIZE },
    { fl_rgb_color(206,145,120), FL_COURIER, FONT_SIZE },
    { fl_rgb_color(181,206,168), FL_COURIER, FONT_SIZE }
};

static const char *keywords[] =
{
    "int","char","float","double","void",
    "if","else","while","for","return",
    "class","struct","public","private",
    "protected","switch","case","break",
    "continue","static","const","bool",
    "true","false","new","delete",
    "include","using","namespace",
    "unsigned","signed","long","short",
    "virtual","override","template",
    "typename","auto","nullptr",
    nullptr
};

bool is_keyword(const std::string &word)
{
    for(int i=0; keywords[i]; i++)
    {
        if(word == keywords[i])
            return true;
    }
    return false;
}

void update_title()
{
    std::string title;

    if(current_file.empty())
        title = "Untitled";
    else
        title = current_file;

    if(modified)
        title = "*" + title;

    title += " - FLTK Notepad++";

    win->copy_label(title.c_str());
}

void style_parse(const char *text, char *style, int len)
{
    int i = 0;

    while(i < len)
    {
        if(i < len - 1 &&
           text[i] == '/' &&
           text[i + 1] == '/')
        {
            while(i < len && text[i] != '\n')
                style[i++] = STYLE_C;
        }
        else if(text[i] == '"')
        {
            style[i++] = STYLE_D;

            while(i < len)
            {
                style[i] = STYLE_D;

                if(text[i] == '"')
                {
                    i++;
                    break;
                }
                i++;
            }
        }
        else if(isdigit((unsigned char)text[i]))
        {
            while(i < len &&
                  (isdigit((unsigned char)text[i]) ||
                   text[i] == '.'))
            {
                style[i++] = STYLE_E;
            }
        }
        else if(isalpha((unsigned char)text[i]) ||
                text[i] == '_')
        {
            int start = i;

            while(i < len &&
                 (isalnum((unsigned char)text[i]) ||
                  text[i] == '_'))
            {
                i++;
            }

            std::string word(text + start, i - start);

            char s = is_keyword(word)
                     ? STYLE_B
                     : STYLE_A;

            for(int j=start;j<i;j++)
                style[j] = s;
        }
        else
        {
            style[i++] = STYLE_A;
        }
    }
}

void update_style()
{
    const char *text = textbuf->text();

    int len = strlen(text);

    char *style = new char[len + 1];

    memset(style, STYLE_A, len);

    style_parse(text, style, len);

    style[len] = '\0';

    stylebuf->text(style);

    delete[] style;

    free((void*)text);
}

void text_modified_cb(int,
                      int,
                      int,
                      int,
                      const char *,
                      void *)
{
    update_style();

    if(!loading_file)
    {
        modified = true;
        update_title();
    }
}

bool save_current_file();

bool check_save_changes()
{
    if(!modified)
        return true;

    std::string msg =
        "Save changes to ";

    if(current_file.empty())
        msg += "Untitled";
    else
        msg += current_file;

    msg += "?";

    int r = fl_choice(
        msg.c_str(),
        "Cancel",
        "No",
        "Yes");

    if(r == 0)
        return false;

    if(r == 2)
        return save_current_file();

    return true;
}

void load_file(const char *filename)
{
    std::ifstream file(filename);

    if(!file)
    {
        fl_alert("Unable to open file.");
        return;
    }

    std::stringstream ss;
    ss << file.rdbuf();

    loading_file = true;

    textbuf->text(ss.str().c_str());

    loading_file = false;

    current_file = filename;
    modified = false;

    update_style();
    update_title();
}

bool save_current_file()
{
    if(current_file.empty())
    {
        Fl_Native_File_Chooser chooser;

        chooser.title("Save File");
        chooser.type(
            Fl_Native_File_Chooser::BROWSE_SAVE_FILE);

        if(chooser.show())
            return false;

        current_file = chooser.filename();
    }

    std::ofstream file(current_file);

    if(!file)
    {
        fl_alert("Unable to save file.");
        return false;
    }

    const char *text = textbuf->text();

    file << text;

    free((void*)text);

    modified = false;

    update_title();

    return true;
}

void new_file()
{
    if(!check_save_changes())
        return;

    loading_file = true;

    textbuf->text("");

    loading_file = false;

    current_file.clear();
    modified = false;

    update_style();
    update_title();
}

void open_file()
{
    if(!check_save_changes())
        return;

    Fl_Native_File_Chooser chooser;

    chooser.title("Open File");
    chooser.type(
        Fl_Native_File_Chooser::BROWSE_FILE);

    if(chooser.show())
        return;

    load_file(chooser.filename());
}

void menu_cb(Fl_Widget *w, void *)
{
    Fl_Menu_Bar *m =
        (Fl_Menu_Bar*)w;

    const Fl_Menu_Item *item =
        m->mvalue();

    if(!item)
        return;

    std::string cmd(item->label());

    if(cmd == "&New")
        new_file();

    else if(cmd == "&Open")
        open_file();

    else if(cmd == "&Save")
        save_current_file();

    else if(cmd == "&Quit")
    {
        if(check_save_changes())
            win->hide();
    }
}

void close_cb(Fl_Widget *, void *)
{
    if(check_save_changes())
        win->hide();
}

int main(int argc, char **argv)
{
    Fl::background(30,30,30);
    Fl::foreground(220,220,220);

    win = new Fl_Window(
        1000,
        700,
        "Untitled - FLTK Notepad++");

    win->callback(close_cb);

    Fl_Menu_Bar *menu =
        new Fl_Menu_Bar(
            0,
            0,
            1000,
            25);

    menu->box(FL_FLAT_BOX);
    menu->color(fl_rgb_color(45,45,45));
    menu->selection_color(fl_rgb_color(70,70,70));
    menu->labelcolor(FL_WHITE);
    menu->textcolor(FL_WHITE);
    menu->textsize(14);

    menu->add(
        "&File/&New",
        FL_CTRL + 'n',
        menu_cb);

    menu->add(
        "&File/&Open",
        FL_CTRL + 'o',
        menu_cb);

    menu->add(
        "&File/&Save",
        FL_CTRL + 's',
        menu_cb);

    menu->add(
        "&File/&Quit",
        FL_CTRL + 'q',
        menu_cb);

    textbuf = new Fl_Text_Buffer();
    stylebuf = new Fl_Text_Buffer();

    editor = new Fl_Text_Editor(
        0,
        25,
        1000,
        675);

    editor->buffer(textbuf);

    editor->textfont(FL_COURIER);
    editor->textsize(FONT_SIZE);

    editor->color(
        fl_rgb_color(53,53,53));

    editor->textcolor(FL_WHITE);

    editor->highlight_data(
        stylebuf,
        styletable,
        sizeof(styletable) /
        sizeof(styletable[0]),
        'A',
        nullptr,
        nullptr);

    textbuf->add_modify_callback(
        text_modified_cb,
        nullptr);

    update_style();
    update_title();

    if(argc > 1)
        load_file(argv[1]);

    Fl::background(45,45,45);
    Fl::background2(53,53,53);
    Fl::foreground(220,220,220);

    Fl::set_color(FL_BACKGROUND_COLOR, 45,45,45);
    Fl::set_color(FL_BACKGROUND2_COLOR, 53,53,53);
    Fl::set_color(FL_FOREGROUND_COLOR, 220,220,220);
    Fl::set_color(FL_SELECTION_COLOR, 70,70,70);

    Fl::scheme("gtk+");

    win->resizable(editor);
    win->end();
    win->show(argc, argv);

    return Fl::run();
}

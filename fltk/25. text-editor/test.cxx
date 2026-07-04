#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/fl_ask.H>

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <cctype>

// --------------------------------------------------
// Globals
// --------------------------------------------------

static Fl_Window* win = nullptr;
static Fl_Text_Editor* editor = nullptr;

static Fl_Text_Buffer* textbuf = nullptr;
static Fl_Text_Buffer* stylebuf = nullptr;

static std::string currentFile;

// --------------------------------------------------
// Dark Theme (Adwaita-like)
// --------------------------------------------------

static void apply_dark_theme()
{
    Fl::scheme("gtk+");

    Fl::background(40, 44, 52);        // window bg
    Fl::background2(30, 34, 42);       // widget bg
    Fl::foreground(220, 223, 228);     // text

    Fl::set_color(FL_SELECTION_COLOR, 64, 128, 255);
    Fl::set_color(FL_GRAY, 40, 44, 52);
    Fl::set_color(FL_DARK3, 30, 34, 42);
}

// --------------------------------------------------
// Syntax Highlighting
// --------------------------------------------------

static Fl_Text_Display::Style_Table_Entry styletable[] = {
    { fl_rgb_color(220,223,228), FL_COURIER,      14 }, // A normal
    { fl_rgb_color(86,156,214),  FL_COURIER_BOLD, 14 }, // B keyword
    { fl_rgb_color(106,153,85),  FL_COURIER,      14 }, // C comment
    { fl_rgb_color(206,145,120), FL_COURIER,      14 }, // D string
    { fl_rgb_color(181,206,168), FL_COURIER,      14 }  // E number
};

static const char* keywords[] = {
    "auto","break","case","char","class","const",
    "continue","default","delete","do","double",
    "else","enum","extern","false","float","for",
    "goto","if","inline","int","long","namespace",
    "new","operator","private","protected","public",
    "register","return","short","signed","sizeof",
    "static","struct","switch","template","this",
    "true","typedef","union","unsigned","virtual",
    "void","volatile","while","bool","using",
    nullptr
};

static bool is_keyword(const std::string& word)
{
    for (int i = 0; keywords[i]; i++)
        if (word == keywords[i]) return true;
    return false;
}

// --------------------------------------------------
// Syntax Highlight Engine
// --------------------------------------------------

static void update_style()
{
    char* text = textbuf->text();

    int len = (int)strlen(text);
    std::string style(len, 'A');

bool in_string = false;
bool in_comment = false;

for (int i = 0; i < len; i++)
{
    // -------------------------
    // Comment start //
    // -------------------------
    if (!in_string &&
        i + 1 < len &&
        text[i] == '/' &&
        text[i + 1] == '/')
    {
        in_comment = true;
    }

    if (in_comment)
    {
        style[i] = 'C';
        if (text[i] == '\n')
            in_comment = false;
        continue;
    }

    // -------------------------
    // String handling
    // -------------------------
    if (!in_comment)
    {
        if (text[i] == '"' )
        {
            // only toggle if NOT escaped
            if (i == 0 || text[i - 1] != '\\')
                in_string = !in_string;

            style[i] = 'D';
            continue;
        }

        if (in_string)
        {
            style[i] = 'D';
            continue;
        }
    }

    // -------------------------
    // Default number highlight
    // -------------------------
    if (!in_string && isdigit((unsigned char)text[i]))
    {
        style[i] = 'E';
    }
}

    // keywords
    int i = 0;
    while (i < len)
    {
        if (isalpha((unsigned char)text[i]) || text[i] == '_')
        {
            int start = i;

            while (i < len &&
                  (isalnum((unsigned char)text[i]) ||
                   text[i] == '_'))
                i++;

            std::string word(text + start, i - start);

            if (is_keyword(word))
            {
                for (int j = start; j < i; j++)
                    style[j] = 'B';
            }
        }
        else i++;
    }

    stylebuf->text(style.c_str());

    // FIX: correct deallocation
    delete[] text;
}

static void style_cb(int, int, int, int, const char*, void*)
{
    update_style();
}

// --------------------------------------------------
// File Operations
// --------------------------------------------------

static bool load_file(const char* filename)
{
    FILE* fp = fopen(filename, "rb");
    if (!fp) return false;

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    std::vector<char> buf(size + 1);

    fread(buf.data(), 1, size, fp);
    fclose(fp);

    buf[size] = 0;

    textbuf->text(buf.data());

    currentFile = filename;
    update_style();

    return true;
}

static bool save_file(const char* filename)
{
    FILE* fp = fopen(filename, "wb");
    if (!fp) return false;

    char* txt = textbuf->text();

    fwrite(txt, 1, strlen(txt), fp);
    fclose(fp);

    delete[] txt; // FIX

    currentFile = filename;
    return true;
}

// --------------------------------------------------
// Menu Callbacks
// --------------------------------------------------

static void new_cb(Fl_Widget*, void*)
{
    textbuf->text("");
    currentFile.clear();
    update_style();
}

static void open_cb(Fl_Widget*, void*)
{
    const char* fn = fl_file_chooser("Open File", "*", "");
    if (fn) load_file(fn);
}

static void saveas_cb(Fl_Widget*, void*)
{
    const char* fn = fl_file_chooser("Save File", "*", "");
    if (fn)
    {
        if (!save_file(fn))
            fl_alert("Failed to save file.");
    }
}

static void save_cb(Fl_Widget*, void*)
{
    if (currentFile.empty())
    {
        saveas_cb(nullptr, nullptr);
        return;
    }

    if (!save_file(currentFile.c_str()))
        fl_alert("Failed to save file.");
}

static void quit_cb(Fl_Widget*, void*)
{
    win->hide();
}

// --------------------------------------------------
// Search / Replace
// --------------------------------------------------

static void find_cb(Fl_Widget*, void*)
{
    const char* search = fl_input("Find:");
    if (!search || !*search) return;

    int pos = editor->insert_position();

    int found = textbuf->search_forward(pos, search, &pos);

    if (found)
    {
        editor->insert_position(pos);
        editor->show_insert_position();

        editor->buffer()->select(
            pos,
            pos + (int)strlen(search));
    }
    else fl_message("Text not found.");
}

static void replace_cb(Fl_Widget*, void*)
{
    const char* search = fl_input("Find:");
    if (!search || !*search) return;

    const char* repl = fl_input("Replace with:");
    if (!repl) return;

    int pos = 0;

    while (textbuf->search_forward(pos, search, &pos))
    {
        textbuf->replace(pos,
                         pos + (int)strlen(search),
                         repl);

        pos += (int)strlen(repl);
    }

    update_style();
}

// --------------------------------------------------
// MAIN
// --------------------------------------------------

int main(int argc, char** argv)
{
    apply_dark_theme();

    textbuf  = new Fl_Text_Buffer();
    stylebuf = new Fl_Text_Buffer();

    win = new Fl_Window(1000, 700, "FLTK Dark Editor (Adwaita-like)");

    Fl_Menu_Bar* menu = new Fl_Menu_Bar(0, 0, 1000, 25);

    menu->color(fl_rgb_color(40,44,52));
    menu->labelcolor(fl_rgb_color(220,223,228));

    menu->add("&File/&New",     FL_CTRL + 'n', new_cb);
    menu->add("&File/&Open",    FL_CTRL + 'o', open_cb);
    menu->add("&File/&Save",    FL_CTRL + 's', save_cb);
    menu->add("&File/Save &As", 0,            saveas_cb);
    menu->add("&File/&Quit",    FL_CTRL + 'q', quit_cb);

    menu->add("&Edit/&Find",    FL_CTRL + 'f', find_cb);
    menu->add("&Edit/&Replace", FL_CTRL + 'h', replace_cb);

    editor = new Fl_Text_Editor(0, 25, 1000, 675);

    editor->buffer(textbuf);

    editor->color(fl_rgb_color(30,34,42));
    editor->textcolor(fl_rgb_color(220,223,228));
    editor->cursor_color(fl_rgb_color(220,223,228));
    editor->selection_color(fl_rgb_color(64,128,255));

    editor->highlight_data(
        stylebuf,
        styletable,
        sizeof(styletable) / sizeof(styletable[0]),
        'A',
        nullptr,
        nullptr
    );

    textbuf->add_modify_callback(style_cb, nullptr);

    update_style();

    win->resizable(editor);
    win->end();
    win->show(argc, argv);

    return Fl::run();
}

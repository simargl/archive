// g++ test.cxx -o test $(fltk-config --cxxflags) $(fltk-config --ldflags)

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_RGB_Image.H>
#include <cstdlib>
#include <cstdio>
#include <string>

// Global widgets
Fl_Input *search_input = NULL;
Fl_Browser *browse_browser = NULL;
Fl_Browser *installed_browser = NULL;
Fl_Browser *updates_browser = NULL;

Fl_Text_Buffer *browse_details_buf = NULL;
Fl_Text_Buffer *installed_details_buf = NULL;
Fl_Text_Buffer *updates_details_buf = NULL;

Fl_Text_Buffer *output_buffer = NULL;
Fl_Text_Display *output_display = NULL;

// Winamp Classic Color Palette
Fl_Color winamp_dark_bg = fl_rgb_color(45, 45, 48);     // Charcoal window background
Fl_Color winamp_panel_bg = fl_rgb_color(15, 15, 15);    // Deep black display screens
Fl_Color winamp_green_text = fl_rgb_color(0, 230, 0);   // Retro neon green text
Fl_Color winamp_amber_text = fl_rgb_color(255, 176, 0); // Winamp amber highlight text
Fl_Color winamp_btn_bg = fl_rgb_color(70, 70, 75);      // Metallic grey buttons
Fl_Color winamp_btn_fg = fl_rgb_color(240, 240, 240);   // Bright button text

// Forward declaration
void refresh_installed_internal(bool log_to_console);

// Helper function to run shell commands and capture output
std::string exec_command(const char* cmd) {
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
        return "Error: popen() failed!";
    }
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

// Helper to get clean package name from a browser entry string
std::string extract_pkg_name(const std::string& entry) {
    if (entry.empty()) return "";
    size_t space_pos = entry.find(' ');
    return (space_pos != std::string::npos) ? entry.substr(0, space_pos) : entry;
}

// Load ALL available packages on startup
void load_default_browse() {
    output_buffer->text("Loading all available system packages...\n");
    Fl::check();
    browse_browser->clear();
    
    FILE* pipe = popen("apt-cache search .", "r");
    if (!pipe) {
        output_buffer->text("Failed to load available packages.\n");
        return;
    }

    char buffer[256];
    int count = 0;
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        std::string line(buffer);
        if (!line.empty() && line[line.length()-1] == '\n') line.erase(line.length()-1);
        browse_browser->add(line.c_str());
        count++;
    }
    pclose(pipe);

    char summary[128];
    snprintf(summary, sizeof(summary), "Loaded all %d available packages successfully.\n", count);
    output_buffer->text(summary);
}

// Tab 1: Browse & Search
void search_cb(Fl_Widget*, void*) {
    const char *query = search_input->value();
    if (!query || query[0] == '\0') {
        load_default_browse();
        return;
    }

    output_buffer->text(("Searching APT cache for: " + std::string(query) + "\n").c_str());
    browse_browser->clear();

    std::string cmd = "apt-cache search " + std::string(query);
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        output_buffer->text("Failed to execute search.\n");
        return;
    }

    char buffer[256];
    int count = 0;
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        std::string line(buffer);
        if (!line.empty() && line[line.length()-1] == '\n') line.erase(line.length()-1);
        browse_browser->add(line.c_str());
        count++;
    }
    pclose(pipe);

    char summary[128];
    snprintf(summary, sizeof(summary), "Found %d packages matching query.\n", count);
    output_buffer->text(summary);
}

// Selection callback for Browse tab
void browse_select_cb(Fl_Widget*, void*) {
    int line = browse_browser->value();
    if (line <= 0) return;
    std::string pkg = extract_pkg_name(browse_browser->text(line));
    if (pkg.empty()) return;

    std::string details = exec_command(("apt-cache show " + pkg).c_str());
    if (details.empty()) details = "No package details available.";
    browse_details_buf->text(details.c_str());
}

void browse_install_cb(Fl_Widget*, void*) {
    int line = browse_browser->value();
    if (line <= 0) {
        output_buffer->text("Please select a package from the Browse list.\n");
        return;
    }
    std::string pkg = extract_pkg_name(browse_browser->text(line));
    std::string cmd = "apt-get install -y " + pkg;
    std::string result = "Executing: " + cmd + "\n" + exec_command(cmd.c_str());
    output_buffer->text(result.c_str());
    
    refresh_installed_internal(false);
}

// Tab 2: Installed Packages (Strictly filtered to truly installed packages)
void refresh_installed_internal(bool log_to_console) {
    if (log_to_console) {
        output_buffer->text("Fetching installed packages list...\n");
        Fl::check();
    }
    
    installed_browser->clear();

    // Query packages and filter only those whose status is 'install ok installed'
    FILE* pipe = popen("dpkg-query -W -f='${Status}|${Package} - ${Version}\\n'", "r");
    if (!pipe) {
        if (log_to_console) output_buffer->text("Failed to query installed packages.\n");
        return;
    }

    char buffer[512];
    int count = 0;
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        std::string line(buffer);
        if (!line.empty() && line[line.length()-1] == '\n') line.erase(line.length()-1);
        
        size_t pipe_pos = line.find('|');
        if (pipe_pos != std::string::npos) {
            std::string status = line.substr(0, pipe_pos);
            std::string pkg_info = line.substr(pipe_pos + 1);
            
            if (status == "install ok installed") {
                installed_browser->add(pkg_info.c_str());
                count++;
            }
        }
    }
    pclose(pipe);

    if (log_to_console) {
        char summary[128];
        snprintf(summary, sizeof(summary), "Loaded %d installed packages.\n", count);
        output_buffer->text(summary);
    }
}

void refresh_installed_btn_cb(Fl_Widget*, void*) {
    refresh_installed_internal(true);
}

// Selection callback for Installed tab
void installed_select_cb(Fl_Widget*, void*) {
    int line = installed_browser->value();
    if (line <= 0) return;
    std::string pkg = extract_pkg_name(installed_browser->text(line));
    if (pkg.empty()) return;

    std::string details = exec_command(("apt-cache show " + pkg).c_str());
    if (details.empty()) {
        details = exec_command(("dpkg -s " + pkg).c_str());
    }
    if (details.empty()) details = "No package details available.";
    installed_details_buf->text(details.c_str());
}

// Remove with autoremove enabled
void remove_installed_cb(Fl_Widget*, void*) {
    int line = installed_browser->value();
    if (line <= 0) {
        output_buffer->text("Please select a package from the Installed list.\n");
        return;
    }
    std::string pkg = extract_pkg_name(installed_browser->text(line));
    std::string cmd = "apt-get remove -y " + pkg + " && apt-get autoremove -y";
    std::string result = "Executing: " + cmd + "\n" + exec_command(cmd.c_str());
    output_buffer->text(result.c_str());
    
    refresh_installed_internal(false);
}

// Tab 3: Updates
void check_updates_cb(Fl_Widget*, void*) {
    output_buffer->text("Checking for system updates...\n");
    Fl::check();
    updates_browser->clear();

    exec_command("apt-get update");

    FILE* pipe = popen("apt list --upgradable 2>/dev/null | tail -n +2", "r");
    if (!pipe) {
        output_buffer->text("Failed to check for updates.\n");
        return;
    }

    char buffer[256];
    int count = 0;
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        std::string line(buffer);
        if (!line.empty() && line[line.length()-1] == '\n') line.erase(line.length()-1);
        updates_browser->add(line.c_str());
        count++;
    }
    pclose(pipe);

    char summary[128];
    snprintf(summary, sizeof(summary), "Found %d available updates.\n", count);
    output_buffer->text(summary);
}

// Selection callback for Updates tab
void updates_select_cb(Fl_Widget*, void*) {
    int line = updates_browser->value();
    if (line <= 0) return;
    std::string pkg = extract_pkg_name(updates_browser->text(line));
    if (pkg.empty()) return;

    std::string details = exec_command(("apt-cache show " + pkg).c_str());
    if (details.empty()) details = "No package details available.";
    updates_details_buf->text(details.c_str());
}

void apply_updates_cb(Fl_Widget*, void*) {
    output_buffer->text("Applying system upgrades...\n");
    Fl::check();
    std::string res = "Executing: apt-get upgrade -y\n" + exec_command("apt-get upgrade -y");
    output_buffer->text(res.c_str());
    check_updates_cb(NULL, NULL);
    refresh_installed_internal(false);
}

int main(int argc, char **argv) {
    Fl::scheme("gtk+");

    Fl_Window *window = new Fl_Window(980, 740, "Winamp Classic APT Frontend");
    window->color(winamp_dark_bg);

    // Create a built-in 16x16 amber package icon programmatically
    uchar icon_data[16 * 16 * 3];
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            int idx = (y * 16 + x) * 3;
            if (x == 0 || x == 15 || y == 0 || y == 15 || y == 5) {
                icon_data[idx] = 255;   // R
                icon_data[idx+1] = 176; // G
                icon_data[idx+2] = 0;   // B
            } else {
                icon_data[idx] = 15;    // Dark background
                icon_data[idx+1] = 15;
                icon_data[idx+2] = 15;
            }
        }
    }
    Fl_RGB_Image app_icon(icon_data, 16, 16);
    window->icon(&app_icon);

    Fl_Tabs *tabs = new Fl_Tabs(10, 10, 960, 490);
    tabs->color(winamp_dark_bg);
    tabs->labelsize(16);

    // --- TAB 1: BROWSE ---
    Fl_Group *tab_browse = new Fl_Group(15, 40, 950, 455, "Browse");
    tab_browse->color(winamp_dark_bg);
    tab_browse->labelsize(16);
    {
        search_input = new Fl_Input(15, 60, 735, 36, "Search Packages (Press Enter):");
        search_input->align(FL_ALIGN_TOP_LEFT);
        search_input->color(winamp_panel_bg);
        search_input->textcolor(winamp_amber_text);
        search_input->labelcolor(winamp_amber_text);
        search_input->textfont(FL_COURIER);
        search_input->textsize(16);
        search_input->labelsize(16);
        search_input->callback(search_cb);
        search_input->when(FL_WHEN_ENTER_KEY);

        Fl_Button *search_btn = new Fl_Button(760, 60, 200, 36, "Search");
        search_btn->color(winamp_btn_bg);
        search_btn->labelcolor(winamp_btn_fg);
        search_btn->labelsize(16);
        search_btn->callback(search_cb);

        browse_browser = new Fl_Browser(15, 115, 520, 310, "Available Packages");
        browse_browser->align(FL_ALIGN_TOP_LEFT);
        browse_browser->color(winamp_panel_bg);
        browse_browser->textcolor(winamp_green_text);
        browse_browser->labelcolor(winamp_amber_text);
        browse_browser->textfont(FL_COURIER);
        browse_browser->textsize(16);
        browse_browser->labelsize(16);
        browse_browser->type(FL_HOLD_BROWSER);
        browse_browser->callback(browse_select_cb);

        browse_details_buf = new Fl_Text_Buffer();
        Fl_Text_Display *browse_details_disp = new Fl_Text_Display(545, 115, 415, 310, "Package Details");
        browse_details_disp->align(FL_ALIGN_TOP_LEFT);
        browse_details_disp->color(winamp_panel_bg);
        browse_details_disp->textcolor(winamp_amber_text);
        browse_details_disp->labelcolor(winamp_amber_text);
        browse_details_disp->textfont(FL_COURIER);
        browse_details_disp->textsize(15);
        browse_details_disp->labelsize(16);
        browse_details_disp->buffer(browse_details_buf);

        Fl_Button *install_btn = new Fl_Button(15, 438, 180, 36, "Install Selected");
        install_btn->color(winamp_btn_bg);
        install_btn->labelcolor(winamp_btn_fg);
        install_btn->labelsize(16);
        install_btn->callback(browse_install_cb);
    }
    tab_browse->end();

    // --- TAB 2: INSTALLED ---
    Fl_Group *tab_installed = new Fl_Group(15, 40, 950, 455, "Installed");
    tab_installed->color(winamp_dark_bg);
    tab_installed->labelsize(16);
    {
        installed_browser = new Fl_Browser(15, 60, 520, 365, "Installed Packages (Local Database)");
        installed_browser->align(FL_ALIGN_TOP_LEFT);
        installed_browser->color(winamp_panel_bg);
        installed_browser->textcolor(winamp_green_text);
        installed_browser->labelcolor(winamp_amber_text);
        installed_browser->textfont(FL_COURIER);
        installed_browser->textsize(16);
        installed_browser->labelsize(16);
        installed_browser->type(FL_HOLD_BROWSER);
        installed_browser->callback(installed_select_cb);

        installed_details_buf = new Fl_Text_Buffer();
        Fl_Text_Display *installed_details_disp = new Fl_Text_Display(545, 60, 415, 365, "Package Details");
        installed_details_disp->align(FL_ALIGN_TOP_LEFT);
        installed_details_disp->color(winamp_panel_bg);
        installed_details_disp->textcolor(winamp_amber_text);
        installed_details_disp->labelcolor(winamp_amber_text);
        installed_details_disp->textfont(FL_COURIER);
        installed_details_disp->textsize(15);
        installed_details_disp->labelsize(16);
        installed_details_disp->buffer(installed_details_buf);

        Fl_Button *refresh_btn = new Fl_Button(15, 438, 180, 36, "Refresh List");
        refresh_btn->color(winamp_btn_bg);
        refresh_btn->labelcolor(winamp_btn_fg);
        refresh_btn->labelsize(16);
        refresh_btn->callback(refresh_installed_btn_cb);

        Fl_Button *remove_btn = new Fl_Button(205, 438, 230, 36, "Remove + Autoremove");
        remove_btn->color(winamp_btn_bg);
        remove_btn->labelcolor(winamp_btn_fg);
        remove_btn->labelsize(16);
        remove_btn->callback(remove_installed_cb);
    }
    tab_installed->end();

    // --- TAB 3: UPDATES ---
    Fl_Group *tab_updates = new Fl_Group(15, 40, 950, 455, "Updates");
    tab_updates->color(winamp_dark_bg);
    tab_updates->labelsize(16);
    {
        updates_browser = new Fl_Browser(15, 60, 520, 365, "Pending Package Updates");
        updates_browser->align(FL_ALIGN_TOP_LEFT);
        updates_browser->color(winamp_panel_bg);
        updates_browser->textcolor(winamp_green_text);
        updates_browser->labelcolor(winamp_amber_text);
        updates_browser->textfont(FL_COURIER);
        updates_browser->textsize(16);
        updates_browser->labelsize(16);
        updates_browser->type(FL_HOLD_BROWSER);
        updates_browser->callback(updates_select_cb);

        updates_details_buf = new Fl_Text_Buffer();
        Fl_Text_Display *updates_details_disp = new Fl_Text_Display(545, 60, 415, 365, "Package Details");
        updates_details_disp->align(FL_ALIGN_TOP_LEFT);
        updates_details_disp->color(winamp_panel_bg);
        updates_details_disp->textcolor(winamp_amber_text);
        updates_details_disp->labelcolor(winamp_amber_text);
        updates_details_disp->textfont(FL_COURIER);
        updates_details_disp->textsize(15);
        updates_details_disp->labelsize(16);
        updates_details_disp->buffer(updates_details_buf);

        Fl_Button *check_btn = new Fl_Button(15, 438, 180, 36, "Check Updates");
        check_btn->color(winamp_btn_bg);
        check_btn->labelcolor(winamp_btn_fg);
        check_btn->labelsize(16);
        check_btn->callback(check_updates_cb);

        Fl_Button *upgrade_all_btn = new Fl_Button(205, 438, 210, 36, "Apply All Updates");
        upgrade_all_btn->color(winamp_btn_bg);
        upgrade_all_btn->labelcolor(winamp_btn_fg);
        upgrade_all_btn->labelsize(16);
        upgrade_all_btn->callback(apply_updates_cb);
    }
    tab_updates->end();

    tabs->end();

    // --- TRANSACTION / LOG CONSOLE ---
    output_buffer = new Fl_Text_Buffer();
    output_display = new Fl_Text_Display(10, 515, 960, 215, "Transaction Details & Output Log");
    output_display->align(FL_ALIGN_TOP_LEFT);
    output_display->color(winamp_panel_bg);
    output_display->textcolor(winamp_amber_text);
    output_display->labelcolor(winamp_amber_text);
    output_display->textfont(FL_COURIER);
    output_display->textsize(16);
    output_display->labelsize(16);
    output_display->buffer(output_buffer);
    output_buffer->text("Winamp Classic APT Manager initialized. It really whips the llama's ass!\n");

    // Populate all entries on startup
    load_default_browse();
    refresh_installed_internal(false);

    window->end();
    window->show(argc, argv);
    return Fl::run();
}

// g++ test.cxx -o test `fltk-config --cxxflags --ldflags`

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/fl_draw.H>
#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>

// PI definition for rotation math
const double PI = 3.14159265358979323846;

// Structure to store individual mandala particles/lines
struct MandalaElement {
    int shape_type; // 0: line, 1: circle/ellipse
    double r, theta;
    double x2, y2;  // Offset positions
    Fl_Color color;
};

class MandalaCanvas : public Fl_Widget {
    std::vector<MandalaElement> elements;
    int symmetry;
    double dissolve_factor; // 0.0 = intact, 1.0 = fully dissolved
    bool is_dissolving;

public:
    MandalaCanvas(int X, int Y, int W, int H) 
        : Fl_Widget(X, Y, W, H), symmetry(12), dissolve_factor(0.0), is_dissolving(false) {
        srand(time(NULL));
        generate_new_mandala();
    }

    void generate_new_mandala() {
        elements.clear();
        dissolve_factor = 0.0;
        is_dissolving = false;

        // Palette inspired by traditional Tibetan sand colors
        Fl_Color palette[] = { FL_RED, FL_BLUE, FL_YELLOW, FL_GREEN, FL_WHITE, FL_CYAN };
        int num_colors = sizeof(palette) / sizeof(palette[0]);

        // Generate layers of geometry from center outwards
        int layers = 6 + rand() % 5;
        for (int l = 0; l < layers; ++l) {
            double base_radius = (l + 1) * 35.0;
            Fl_Color layer_color = palette[rand() % num_colors];
            int element_type = rand() % 2;

            // Generate features for a single slice of symmetry
            for (int i = 0; i < 5; ++i) {
                MandalaElement elem;
                elem.shape_type = element_type;
                elem.r = base_radius + (rand() % 20 - 10);
                elem.theta = (rand() % 100) / 100.0 * (2 * PI / symmetry);
                elem.x2 = 10 + rand() % 40;
                elem.y2 = 10 + rand() % 40;
                elem.color = layer_color;
                elements.push_back(elem);
            }
        }
        redraw();
    }

    void start_dissolve() {
        if (!is_dissolving) {
            is_dissolving = true;
            Fl::add_timeout(0.03, dissolve_callback, this);
        }
    }

    static void dissolve_callback(void* data) {
        MandalaCanvas* canvas = (MandalaCanvas*)data;
        if (canvas->dissolve_factor < 1.0) {
            canvas->dissolve_factor += 0.02; // Control how fast it wipes away
            canvas->redraw();
            Fl::repeat_timeout(0.03, dissolve_callback, canvas);
        } else {
            canvas->is_dissolving = false;
            canvas->generate_new_mandala(); // Regenerate anew (the cycle of rebirth)
        }
    }

protected:
    void draw() override {
        // Draw deep black canvas background
        fl_rectf(x(), y(), w(), h(), FL_BLACK);

        int cx = x() + w() / 2;
        int cy = y() + h() / 2;

        // Set clipping to protect borders
        fl_push_clip(x(), y(), w(), h());

        // Render shapes using radial symmetry
        for (const auto& elem : elements) {
            fl_color(elem.color);

            for (int i = 0; i < symmetry; ++i) {
                double angle = (2 * PI / symmetry) * i + elem.theta;

                // Physics distortion simulation mirroring sweeping sand away
                double sand_drift_x = 0.0;
                double sand_drift_y = 0.0;
                if (dissolve_factor > 0.0) {
                    // Random scatter vectors pushing outward and down
                    double seed = sin(elem.r) * i;
                    sand_drift_x = sin(seed) * dissolve_factor * 250.0;
                    sand_drift_y = abs(cos(seed)) * dissolve_factor * 350.0; 
                }

                // Initial polar coordinates to Cartesian transformation
                int x1 = cx + (elem.r * cos(angle)) + sand_drift_x;
                int y1 = cy + (elem.r * sin(angle)) + sand_drift_y;

                if (elem.shape_type == 0) {
                    // Draw symmetrical geometry line layers
                    int target_x = cx + ((elem.r + elem.x2) * cos(angle + 0.1)) + sand_drift_x;
                    int target_y = cy + ((elem.r + elem.x2) * sin(angle + 0.1)) + sand_drift_y;
                    fl_line(x1, y1, target_x, target_y);
                } else {
                    // Draw concentric bounding circle components
                    fl_circle(x1, y1, elem.x2 * (1.0 - dissolve_factor * 0.5));
                }
            }
        }
        fl_pop_clip();
    }
};

// Main Window Setup
int main(int argc, char** argv) {
    Fl_Double_Window* window = new Fl_Double_Window(600, 680, "Tibetan Sand Mandala Generator");
    window->color(fl_rgb_color(30, 30, 30));

    // Instantiating custom drawing canvas
    MandalaCanvas* canvas = new MandalaCanvas(20, 20, 560, 560);

    // Build control layout
    Fl_Button* btn_gen = new Fl_Button(80, 600, 180, 45, "Build Mandala");
    btn_gen->box(FL_ROUNDED_BOX);
    btn_gen->color(fl_rgb_color(160, 120, 40));
    btn_gen->labelcolor(FL_WHITE);
    btn_gen->callback([](Fl_Widget*, void* c) { 
        ((MandalaCanvas*)c)->generate_new_mandala(); 
    }, canvas);

    Fl_Button* btn_wipe = new Fl_Button(340, 600, 180, 45, "Wipe Away (Impermanence)");
    btn_wipe->box(FL_ROUNDED_BOX);
    btn_wipe->color(fl_rgb_color(140, 40, 40));
    btn_wipe->labelcolor(FL_WHITE);
    btn_wipe->callback([](Fl_Widget*, void* c) { 
        ((MandalaCanvas*)c)->start_dissolve(); 
    }, canvas);

    window->end();
    window->show(argc, argv);
    return Fl::run();
}

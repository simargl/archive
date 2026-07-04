import sys
import os
from fltk import *

# --- Authentic Adwaita Dark Color Palette ---
COLOR_BG = 0x24242400          # Base window surface (Adwaita Window Base)
COLOR_BUTTON = 0x30303000      # Normal button background (Adwaita Card Idle)
COLOR_ACTIVE_GRAY = 0x3d3d3d00 # Refined hover/active selection (Adwaita Hover)
COLOR_TEXT = 0xffffff00        # Sharp white text

class HoverButton(Fl_Button):
    def __init__(self, x, y, w, h, label):
        super().__init__(x, y, w, h, label)
        self.box(FL_FLAT_BOX)
        self.color(COLOR_BUTTON)
        self.labelcolor(COLOR_TEXT)
        self.labelsize(14)
        self.labelfont(FL_HELVETICA) # Kept standard and flat across all states
        self.visible_focus(0)        # Absolutely no ugly legacy dotted focus boxes
        self.is_active = False

    def set_active_state(self, active):
        """Swaps only the background color layout without shifting font geometry."""
        self.is_active = active
        if active:
            self.color(COLOR_ACTIVE_GRAY)
        else:
            self.color(COLOR_BUTTON)
        self.redraw()

    def draw(self):
        """Flat rendered modern drawing routine."""
        fl_color(self.color())
        fl_rectf(self.x(), self.y(), self.w(), self.h())
        
        fl_color(self.labelcolor())
        fl_font(self.labelfont(), self.labelsize())
        fl_draw(self.label(), self.x(), self.y(), self.w(), self.h(), FL_ALIGN_CENTER)


class FullscreenLogoutMenu(Fl_Window):
    def __init__(self):
        screen_w = Fl.w()
        screen_h = Fl.h()
        
        # CHANGED: We offset the starting coordinate and expand the bounds past the screen limit.
        # This breaks the X11 strut boundary and forces a full overlay canvas over tint2.
        super().__init__(-1, -1, screen_w + 2, screen_h + 2)
        
        self.border(0)          
        self.set_override()     
        self.color(COLOR_BG)
        
        self.labels = ["Cancel", "Logout", "Reboot", "Poweroff", "Suspend"]
        self.commands = {
            "Logout": "killall X & killall Xorg & killall Xorg.bin & killall labwc & killall sway & killall weston &",
            "Reboot": "wmreboot &",
            "Poweroff": "wmpoweroff &",
            "Suspend": "pm-suspend &"
        }
        
        button_size = 100
        spacing = 10
        total_buttons = len(self.labels)
        
        total_width = (total_buttons * button_size) + ((total_buttons - 1) * spacing)
        start_x = (screen_w - total_width) // 2
        start_y = (screen_h - button_size) // 2
        
        self.active_index = 0 
        self.buttons = []
        
        for i, label in enumerate(self.labels):
            bx = start_x + (i * (button_size + spacing))
            btn = HoverButton(bx, start_y, button_size, button_size, label)
            btn.callback(self.on_button_clicked)
            self.buttons.append(btn)
            
        if self.buttons:
            self.buttons[self.active_index].set_active_state(True)
            
        self.end()

    def update_selection(self, new_index):
        if self.active_index == new_index:
            return
        self.buttons[self.active_index].set_active_state(False)
        self.active_index = new_index
        self.buttons[self.active_index].set_active_state(True)

    def on_button_clicked(self, widget):
        name = widget.label()
        Fl.grab(None)
        if name in self.commands:
            os.system(self.commands[name])
            sys.exit(0)
        elif name == "Cancel":
            sys.exit(0)

    def handle(self, event):
        # 1. Mouse movements & Intersections
        if event in (FL_MOVE, FL_PUSH):
            mx = Fl.event_x()
            my = Fl.event_y()
            
            for i, btn in enumerate(self.buttons):
                if (btn.x() <= mx <= btn.x() + btn.w()) and (btn.y() <= my <= btn.y() + btn.h()):
                    self.update_selection(i)
                    if event == FL_PUSH:
                        self.on_button_clicked(btn)
                    break
            return 1

        # 2. Keyboard shortcuts & Browsing
        elif event == FL_SHORTCUT:
            key = Fl.event_key()
            
            if key == FL_Escape:
                Fl.grab(None)
                sys.exit(0)
                return 1
                
            elif key in (FL_Enter, FL_KP_Enter):
                if self.buttons:
                    active_btn = self.buttons[self.active_index]
                    self.on_button_clicked(active_btn)
                    return 1
            
            elif key == FL_Right:
                next_idx = (self.active_index + 1) % len(self.buttons)
                self.update_selection(next_idx)
                return 1
                    
            elif key == FL_Left:
                next_idx = (self.active_index - 1) % len(self.buttons)
                self.update_selection(next_idx)
                return 1
                    
        return super().handle(event)


if __name__ == "__main__":
    Fl.scheme(None) 
    Fl.visual(FL_DOUBLE|FL_INDEX)
    
    app = FullscreenLogoutMenu()
    
    # We use explicit show, relying on the -1 layout offset instead of app.fullscreen()
    app.show()
    
    # Grab device inputs cleanly across raw displays
    app.take_focus()
    Fl.grab(app)
    
    Fl.run()

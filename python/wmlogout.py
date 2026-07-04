#!/usr/bin/env python3

import gi
import subprocess

gi.require_version('Gtk', '3.0')
from gi.repository import Gtk, GdkPixbuf

class LogoutDialog(Gtk.Dialog):
    def __init__(self, parent):
        Gtk.Dialog.__init__(self, None, parent, 0)
        
        self.set_modal(True)
        self.set_default_response(Gtk.ResponseType.CANCEL)
        self.set_position(Gtk.WindowPosition.CENTER_ALWAYS)
        self.set_decorated(False)
        self.set_skip_taskbar_hint(False)  # Show in the taskbar
        self.set_skip_pager_hint(True)
        self.set_resizable(False)
        self.set_border_width(5)
        self.set_title("WM Logout")
        
        # Load the XPM image data
        xpm_data = [
"64 64 2 1 ",
"  c None",
". c white",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                               ...                              ",
"                              .....                             ",
"                              .....                             ",
"                              .....                             ",
"                        ..    .....    .                        ",
"                      .....   .....   ....                      ",
"                     ......   .....  .......                    ",
"                    .......   .....  ........                   ",
"                   ........   .....   ........                  ",
"                  .......     .....    .......                  ",
"                  ......      .....      ......                 ",
"                 ......       .....       ......                ",
"                 .....        .....       ......                ",
"                ......        .....        ......               ",
"                .....         .....         .....               ",
"                .....         .....         .....               ",
"               .....           ...          .....               ",
"               .....                         .....              ",
"               .....                         .....              ",
"               .....                         .....              ",
"               .....                         .....              ",
"               .....                         .....              ",
"               .....                        ......              ",
"               ......                       .....               ",
"                .....                       .....               ",
"                .....                      ......               ",
"                ......                     .....                ",
"                 ......                   ......                ",
"                 ......                  ......                 ",
"                  ......                .......                 ",
"                   .......             .......                  ",
"                   .........         ........                   ",
"                    ........................                    ",
"                      .....................                     ",
"                       ...................                      ",
"                        ................                        ",
"                           ...........                          ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                ",
"                                                                "
        ]
        
        # Create the GdkPixbuf from the XPM data
        pixbuf = GdkPixbuf.Pixbuf.new_from_xpm_data(xpm_data)
        
        # Set the window icon
        self.set_icon(pixbuf)

        self.add_button("Cancel", Gtk.ResponseType.CANCEL)
        self.add_button("Logout", Gtk.ResponseType.OK)
        self.add_button("Reboot", Gtk.ResponseType.YES)
        self.add_button("Poweroff", Gtk.ResponseType.APPLY)
        self.add_button("Suspend", Gtk.ResponseType.ACCEPT)
        self.connect("response", self.on_dialog_response)

    def on_dialog_response(self, dialog, response):
        if response == Gtk.ResponseType.OK:
            subprocess.run(["busybox", "killall", "X"])
            subprocess.run(["busybox", "killall", "Xorg"])
            subprocess.run(["busybox", "killall", "Xorg.bin"])
        elif response == Gtk.ResponseType.YES:
            subprocess.run(["wmreboot"])
        elif response == Gtk.ResponseType.APPLY:
            subprocess.run(["wmpoweroff"])
        elif response == Gtk.ResponseType.ACCEPT:
            subprocess.run(["pm-suspend"])

dialog = LogoutDialog(None)
dialog.run()
dialog.destroy()

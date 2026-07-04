from fltk import *
import sys

# Create the main window (width, height, title)
window = Fl_Window(300, 180, "Hello World")

# Create the box to hold the text
box = Fl_Box(20, 20, 260, 140, "Hello, World!")

# Style the box (optional)
box.box(FL_UP_BOX)
box.labelsize(24)

# Show the window and start the FLTK event loop
window.end()
window.show(sys.argv)
Fl.run()

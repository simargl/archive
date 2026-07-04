#!/usr/bin/env python3

import gi
import sys

gi.require_version('Gtk', '3.0')
gi.require_version('GtkSource', '3.0')
from gi.repository import Gtk as gtk
from gi.repository import GtkSource as gtksource
from gi.repository import Pango, Gdk

# Globals
search_term = ""
search_iter = None
REPLACE_RESPONSE_ID = 1

# File chooser if no argument is passed
def select_file_dialog():
    dialog = gtk.FileChooserDialog(
        title="Select a file",
        action=gtk.FileChooserAction.OPEN,
    )
    dialog.add_buttons(
        gtk.STOCK_CANCEL, gtk.ResponseType.CANCEL,
        gtk.STOCK_OPEN, gtk.ResponseType.OK
    )
    response = dialog.run()
    filename = dialog.get_filename() if response == gtk.ResponseType.OK else None
    dialog.destroy()
    return filename

# Determine filename
if len(sys.argv) >= 2:
    filename = sys.argv[1]
else:
    filename = select_file_dialog()
    if filename is None:
        print("No file selected. Exiting.")
        sys.exit(0)

# Create window and view
window = gtk.Window()
window.set_title(filename)
window.set_default_size(800, 600)
window.maximize()

view = gtksource.View()
buffer = view.get_buffer()

# Highlight tag
tag_table = buffer.get_tag_table()
highlight_tag = gtk.TextTag(name="highlight")
highlight_tag.set_property("background", "red")
highlight_tag.set_property("foreground", "white")
tag_table.add(highlight_tag)

# Read file
try:
    with open(filename, 'r') as f:
        txt = f.read()
        buffer.set_text(txt)
except Exception as e:
    print(text="Error opening file: {}".format(e))
    sys.exit(1)

# Setup font and style
css = "textview { font-family: Consolas; font-size: 14pt; }"
css_provider = gtk.CssProvider()
css_provider.load_from_data(css.encode('utf-8'))
style_context = view.get_style_context()
style_context.add_provider(css_provider, gtk.STYLE_PROVIDER_PRIORITY_USER)

view.set_show_line_numbers(True)
view.set_show_line_marks(True)
view.set_editable(True)

# Scrollable container
scrolled = gtk.ScrolledWindow()
scrolled.set_policy(gtk.PolicyType.AUTOMATIC, gtk.PolicyType.AUTOMATIC)
scrolled.add(view)
window.add(scrolled)

# Syntax and theme
lm = gtksource.LanguageManager()
lang = lm.guess_language(filename)
if lang:
    buffer.set_language(lang)
    buffer.set_highlight_syntax(True)

scheme_mgr = gtksource.StyleSchemeManager()
scheme = scheme_mgr.get_scheme("solarized-dark")
if scheme:
    buffer.set_style_scheme(scheme)

# Search dialog
search_dialog = None
def search_text():
    dialog = gtk.Dialog(title="Search", parent=window, flags=0)
    dialog.set_transient_for(window)
    dialog.add_button(gtk.STOCK_CLOSE, gtk.ResponseType.CLOSE)
    dialog.set_default_size(300, 50)

    entry = gtk.Entry()
    entry.set_placeholder_text("Enter search term")

    box = dialog.get_content_area()
    box.pack_start(entry, True, True, 0)
    entry.show()

    def on_entry_activate(widget):
        perform_search(widget.get_text())
        # dialog.destroy()

    def on_response(dialog_, response):
        dialog_.destroy()

    entry.connect("activate", on_entry_activate)
    dialog.connect("response", on_response)
    dialog.show_all()



# Perform search and highlight

def perform_search(term):
    global search_term, search_iter
    search_term = term
    start, end = buffer.get_bounds()
    buffer.remove_tag_by_name("highlight", start, end)

    if not term:
        return

    # Search from last position or beginning
    current_iter = search_iter if search_iter else buffer.get_iter_at_mark(buffer.get_insert())
    text = buffer.get_text(buffer.get_start_iter(), buffer.get_end_iter(), True)
    offset = current_iter.get_offset()
    match_pos = text.find(term, offset)

    if match_pos == -1:
        match_pos = text.find(term, 0)  # wrap around

    if match_pos != -1:
        match_start = buffer.get_iter_at_offset(match_pos)
        match_end = buffer.get_iter_at_offset(match_pos + len(term))
        buffer.select_range(match_start, match_end)
        buffer.place_cursor(match_start)
        view.scroll_to_iter(match_start, 0.2, True, 0.5, 0.5)
        buffer.apply_tag_by_name("highlight", match_start, match_end)
        search_iter = buffer.get_iter_at_offset(match_pos + len(term))
    else:
        search_iter = None

# Replace dialog

def replace_text():
    dialog = gtk.Dialog(title="Replace", parent=window, flags=0)
    dialog.set_default_size(300, 100)
    box = dialog.get_content_area()

    entry_find = gtk.Entry()
    entry_replace = gtk.Entry()
    entry_find.set_placeholder_text("Find")
    entry_replace.set_placeholder_text("Replace with")

    box.pack_start(entry_find, True, True, 0)
    box.pack_start(entry_replace, True, True, 0)

    dialog.add_button(gtk.STOCK_CANCEL, gtk.ResponseType.CANCEL)
    dialog.add_button("Replace", REPLACE_RESPONSE_ID)
    dialog.show_all()

    response = dialog.run()
    if response == REPLACE_RESPONSE_ID:
        perform_replace(entry_find.get_text(), entry_replace.get_text())
    dialog.destroy()

def perform_replace(find_text, replace_with):
    start, end = buffer.get_bounds()
    text = buffer.get_text(start, end, True)
    new_text = text.replace(find_text, replace_with)
    buffer.set_text(new_text)

# Save

def save_buffer():
    start_iter, end_iter = buffer.get_bounds()
    text = buffer.get_text(start_iter, end_iter, True)
    try:
        with open(filename, 'w') as f:
            f.write(text)
        buffer.set_modified(False)
        update_title()
    except Exception as e:
        print(text="Error opening file: {}".format(e))

# Title
window_title = filename
def update_title():
    if buffer.get_modified():
        window.set_title("* " + window_title)
    else:
        window.set_title(window_title)

buffer.connect("changed", lambda buf: update_title())

def open_file():
    global filename, window_title
    new_file = select_file_dialog()
    if new_file:
        try:
            with open(new_file, 'r') as f:
                text = f.read()
                buffer.set_text(text)
                buffer.set_modified(False)
                filename = new_file
                window_title = filename
                update_title()
                lang = lm.guess_language(filename)
                if lang:
                    buffer.set_language(lang)
        except Exception as e:
            print(text="Error opening file: {}".format(e))

# Keybindings
def on_key_press(widget, event):
    ctrl = event.state & Gdk.ModifierType.CONTROL_MASK
    if ctrl and event.keyval == Gdk.KEY_s:
        save_buffer(); return True
    if ctrl and event.keyval == Gdk.KEY_f:
        search_text(); return True
    if ctrl and event.keyval == Gdk.KEY_h:
        replace_text(); return True
    if ctrl and event.keyval == Gdk.KEY_z:
        buffer.undo(); return True
    if ctrl and event.keyval == Gdk.KEY_o:
        open_file(); return True
    return False

window.connect("key-press-event", on_key_press)

# Quit handler

def on_window_delete(widget, event):
    if buffer.get_modified():
        dialog = gtk.MessageDialog(
            parent=window,
            message_type=gtk.MessageType.QUESTION,
            buttons=gtk.ButtonsType.YES_NO,
            text="You have unsaved changes. Save before exiting?"
        )
        response = dialog.run()
        dialog.destroy()
        if response == gtk.ResponseType.YES:
            save_buffer()
    gtk.main_quit()
    return False

window.connect("delete-event", on_window_delete)

# Show GUI
update_title()
window.set_icon_name("accessories-text-editor")
window.show_all()
gtk.main()

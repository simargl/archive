import tkinter as tk
from tkinter import ttk

# Create the main window
root = tk.Tk()
root.title("Hello World App")

# Define the main frame
main_frame = ttk.Frame(root, padding="10")
main_frame.pack(fill='both', expand=True)

# Label asking for user's name
name_label = ttk.Label(main_frame, text="What is your name?")
name_label.grid(row=0, column=0, padx=5, pady=5, sticky='w')

# Entry widget to input the name
name_entry = ttk.Entry(main_frame, width=30)
name_entry.grid(row=0, column=1, padx=5, pady=5)

# Set focus to the Entry widget
name_entry.focus()

# Status bar at the bottom
status_var = tk.StringVar()
status_bar = ttk.Label(root, textvariable=status_var, relief='sunken', anchor='w')
status_bar.pack(side='bottom', fill='x')

# Function to greet the user and update the status bar
def say_hello(event=None):
    user_name = name_entry.get()
    greeting = "Hello, {}!".format(user_name)
    status_var.set(greeting)

# Bind Enter key to the Entry widget
name_entry.bind('<Return>', say_hello)
name_entry.bind('<KP_Enter>', say_hello)

# Button to trigger greeting
greet_button = ttk.Button(main_frame, text="Say Hello", command=say_hello)
greet_button.grid(row=1, column=0, columnspan=2, pady=10)

style = ttk.Style()
style.theme_use('clam')

# Run the application
root.mainloop()

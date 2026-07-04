import random
import string
import tkinter as tk
from tkinter import messagebox, filedialog

def generate_letters():
    vowels = ['a', 'e', 'i', 'o', 'u']
    consonants = [ch for ch in string.ascii_lowercase if ch not in vowels + ['q', 'w', 'x', 'y']]
    
    # Select exactly 5 vowels
    selected_vowels = random.sample(vowels, 5)
    # Select 7 consonants
    selected_consonants = [random.choice(consonants) for _ in range(7)]
    
    letters = []
    vowel_idx = 0
    consonant_idx = 0
    
    for i in range(12):
        if i % 2 == 0:
            # Even position: consonant
            if consonant_idx < len(selected_consonants):
                letters.append(selected_consonants[consonant_idx])
                consonant_idx += 1
            else:
                # Fallback if something's off
                letters.append(random.choice(consonants))
        else:
            # Odd position: vowel
            if vowel_idx < len(selected_vowels):
                letters.append(selected_vowels[vowel_idx])
                vowel_idx += 1
            else:
                # Fallback if something's off
                letters.append(random.choice(vowels))
    
    return letters

def load_dictionary(file_path):
    with open(file_path, 'r', encoding='utf-8') as file:
        words = set(line.strip().lower() for line in file)
    return words

def can_form_word(word, letters):
    temp_letters = list(letters)  # Use list() for copying in Python 3.5
    for letter in word:
        if letter in temp_letters:
            temp_letters.remove(letter)
        else:
            return False
    return True

def find_longest_word(dictionary, letters):
    longest_word = ''
    for word in dictionary:
        if len(word) > len(longest_word) and can_form_word(word, letters):
            longest_word = word
    return longest_word

class WordGameGUI:
    def __init__(self, master):
        self.master = master
        self.master.title("Word Game")
        
        # Load dictionary
        self.dictionary = load_dictionary('rjecnik')
        
        # Generate letters
        self.letters = generate_letters()
        
        # GUI Elements
        self.letters_label = tk.Label(master, text="Letters: " + ' '.join(self.letters), font=("Arial", 14))
        self.letters_label.pack(pady=10)
        
        self.entry_label = tk.Label(master, text="Enter your word:", font=("Arial", 12))
        self.entry_label.pack()
        
        self.word_entry = tk.Entry(master, font=("Arial", 12))
        self.word_entry.pack(pady=5)
        
        self.check_button = tk.Button(master, text="Check Word", command=self.check_word)
        self.check_button.pack(pady=5)
        
        self.result_label = tk.Label(master, text="", font=("Arial", 12))
        self.result_label.pack(pady=10)
        
        self.longest_word_button = tk.Button(master, text="Find Longest Word", command=self.show_longest_word)
        self.longest_word_button.pack(pady=5)
        
        self.longest_word_label = tk.Label(master, text="", font=("Arial", 12))
        self.longest_word_label.pack(pady=10)
        
        self.new_game_button = tk.Button(master, text="New Game", command=self.new_game)
        self.new_game_button.pack(pady=5)
        
    def check_word(self):
        user_word = self.word_entry.get().lower()
        if user_word not in self.dictionary:
            self.result_label.config(text="Your word is not in the dictionary.", fg='red')
        elif not can_form_word(user_word, self.letters):
            self.result_label.config(text="Your word cannot be formed from the given letters.", fg='red')
        else:
            self.result_label.config(text="Your word is valid!", fg='green')
    
    def show_longest_word(self):
        longest_word = find_longest_word(self.dictionary, self.letters)
        if longest_word:
            self.longest_word_label.config(text="The longest word is: {}".format(longest_word))
        else:
            self.longest_word_label.config(text="No valid words found.")
    
    def new_game(self):
        self.letters = generate_letters()
        self.letters_label.config(text="Letters: " + ' '.join(self.letters))
        self.word_entry.delete(0, tk.END)
        self.result_label.config(text="")
        self.longest_word_label.config(text="")

if __name__ == "__main__":
    root = tk.Tk()
    app = WordGameGUI(root)
    root.mainloop()

from gtts import gTTS
import os

# Read text from load.txt
with open("load.txt", "r", encoding="utf-8") as file:
    text = file.read()

# Language code for Serbian
language = 'hr'

# Create TTS object
tts = gTTS(text=text, lang=language, slow=False)

# Save the audio file
tts.save("serbian_tts.mp3")

# Play the audio (optional, depends on your OS)
os.system("mpv serbian_tts.mp3")  # Windows
# os.system("xdg-open serbian_tts.mp3")  # Linux
# os.system("open serbian_tts.mp3")  # macOS

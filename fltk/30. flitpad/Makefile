CXX = g++
CXXFLAGS = -std=c++11 $(shell fltk-config --cxxflags)
LDFLAGS = $(shell fltk-config --ldflags)

TARGET = flitpad
SRC = main.cxx

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
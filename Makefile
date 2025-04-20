CC = g++
CFLAGS = -std=c++17 -Wall -O2
LDFLAGS = -lSDL2 -lGL

# Platform-specific settings
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    LDFLAGS += -lSDL2main
endif
ifeq ($(OS),Windows_NT)
    LDFLAGS += -lmingw32 -lSDL2main
    CFLAGS += -I/usr/include/SDL2
    LDFLAGS += -L/usr/lib
endif

TARGET = snake_game
SOURCES = src/main.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean

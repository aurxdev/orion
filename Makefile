CC = gcc
CFLAGS = -Wall
SDLFLAGS = -lSDL2 -lm
TARGET = raycaster

all: $(TARGET)

$(TARGET): 
	$(CC) $(CFLAGS) $(TARGET).c -o $(TARGET) $(SDLFLAGS)

clean:
	rm -f *.o $(TARGET)
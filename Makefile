CC=gcc
CFLAGS=-Os -s `sdl2-config --cflags`
LDFLAGS=`sdl2-config --libs` -lm -lz
TARGET=viewer
SOURCES=src/mif.c src/viewer.c
OBJECTS=$(SOURCES:.c=.o)

all: $(TARGET) minimize

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

minimize:
	strip $(TARGET)

clean:
	rm -rf $(OBJECTS) $(TARGET)


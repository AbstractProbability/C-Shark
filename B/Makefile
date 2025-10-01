CC = gcc
CFLAGS = -Wall -Iinclude -std=c99 -D_DEFAULT_SOURCE -Wextra -Werror -Wno-unused-parameter -fno-asm
DEBUGFLAGS = -g -O0
TARGET = cshark

OBJECTS = main.o control.o dialog.o

all: $(TARGET)
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) -lpcap

# files
main.o: include/dialog.h src/main.c
	$(CC) $(CFLAGS) -c src/main.c

dialog.o: include/dialog.h src/dialog.c
	$(CC) $(CFLAGS) -c src/dialog.c

control.o: include/control.h src/control.c
	$(CC) $(CFLAGS) -c src/control.c

.PHONY: clean
clean:
	rm -f $(OBJECTS) $(TARGET)
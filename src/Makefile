CC = gcc
CFLAGS = -Wall -Iinclude -std=c99 -D_DEFAULT_SOURCE -Wextra -Werror -Wno-unused-parameter -fno-asm
DEBUGFLAGS = -g -O0
TARGET = cshark

OBJECTS = main.o control.o dialog.o

all: $(TARGET)
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) -lpcap -pthread

# files
main.o: ./dialog.h ./main.c
	$(CC) $(CFLAGS) -c ./main.c

dialog.o: ./dialog.h ./dialog.c
	$(CC) $(CFLAGS) -c ./dialog.c

control.o: ./control.h ./control.c
	$(CC) $(CFLAGS) -c ./control.c

.PHONY: clean
clean:
	rm -f $(OBJECTS) $(TARGET)
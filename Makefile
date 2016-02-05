OV_TARGET = ov
LIBS = -lcurl
CC = gcc
CFLAGS = -g -Wall -Iovlib

.PHONY: default all clean

default: $(OV_TARGET)

all: default

OV_LIB_OBJECTS = $(patsubst %.c, %.o, $(wildcard ovlib/*.c))
OV_OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c)) $(OV_LIB_OBJECTS)
HEADERS = $(wildcard ovlib/*.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(OV_TARGET) $(OV_OBJECTS)

$(OV_TARGET): $(OV_OBJECTS)
	$(CC) $(OV_OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o ovlib/*.o
	-rm -f $(OV_TARGET)


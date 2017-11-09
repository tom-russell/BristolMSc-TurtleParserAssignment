CFLAGS = -O2 -Wall -Wextra -Wfloat-equal -pedantic -ansi -lm
INCS = parser.h
TARGET = turtle_parse
SOURCES = parsing_funcs.c $(TARGET).c
CC = gcc

all: $(TARGET)

$(TARGET): $(SOURCES) $(INCS)
	$(CC) $(SOURCES) -o $(TARGET) $(CFLAGS)

clean:
	rm -f $(TARGET)

run: all
	./$(TARGET) 

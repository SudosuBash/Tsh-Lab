CC := gcc
CFLAGS := -Wall -Wextra -std=gnu11 -g -Iinclude
LDFLAGS :=

SRCDIR := src
SRCS := tsh.c $(wildcard $(SRCDIR)/*.c)
OBJS := $(SRCS:.c=.o)
TARGET := tsh

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

# vim: set ft=make:

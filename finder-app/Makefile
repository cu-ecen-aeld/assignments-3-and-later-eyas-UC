# Sample Makefile

CC = gcc
CFLAGS = -Wall -g
TARGET = writer
SRCS = writer.c
OBJS = $(SRCS:.c=.o)


all: $(TARGET)

$(TARGET): $(OBJS)
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
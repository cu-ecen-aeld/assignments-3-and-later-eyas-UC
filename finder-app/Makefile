# Sample Makefile



# define variables 
CC = gcc
CFLAGS = -Wall -g
TARGET = writer
SRCS = writer.c
OBJS = $(SRCS:.c=.o)
BUILD_COMMAND=$(CROSS_COMPILE)$(CC) $(CFLAGS) 

# the default target 
all: $(TARGET)

# link objects
$(TARGET): $(OBJS)
	$(BUILD_COMMAND) -o $@ $^

# wild card for all objects and .c files
%.o: %.c
	$(BUILD_COMMAND) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
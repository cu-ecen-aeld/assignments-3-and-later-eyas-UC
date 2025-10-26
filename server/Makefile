# socket server

# define variables 
CC = gcc
CFLAGS = -Wall -g
TARGET = aesdsocket
SRCS = aesdsocket.c
SRCS += signal_handler.c
SRCS += socket_core.c

HEADERS = signal_handler.h 
HEADERS += socket_core.h 
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
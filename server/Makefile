CROSS_COMPILE ?= 
CC = $(CROSS_COMPILE)gcc

TARGET = aesdsocket
SRCS = aesdsocket.c
OBJS = $(SRCS:.c=.o)

CFLAGS = -Wall -Wextra -O2

.PHONY: all clean

all: $(TARGET)

default: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@

# Transform .c in .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
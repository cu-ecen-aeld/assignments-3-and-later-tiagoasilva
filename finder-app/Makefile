TARGET_EXEC := writer
SRC_FILES := writer.c

CC := $(CROSS_COMPILE)gcc

OBJS := $(SRC_FILES:.c=.o)

all: $(TARGET_EXEC)

# Rule to link object files into the final executable
$(TARGET_EXEC): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Rule to compile .c files into .o object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET_EXEC) $(OBJS)

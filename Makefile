CC      := gcc
CFLAGS  := -std=c11 -O2 -Wall -Wextra -Wshadow -Wconversion
LDFLAGS :=

TARGET := arcdir_tool

SRCS := \
    main.c \
    arcdir.c \
    parse.c \
    misc.c

OBJS := $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -fsanitize=address,undefined
LDFLAGS = -fsanitize=address,undefined

SRC = src/main.c src/bitio.c
OBJ = $(SRC:.c=.o)
TARGET = compressor

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean

TARGET := tests.out
CC     := cc
CFLAGS := -Wall -Werror -Wpedantic -O0 -g

all: $(TARGET)

$(TARGET): avltest.c avl.c
		$(CC) -o $(TARGET) $(FLAGS) $^

clean:
		rm -rf $(TARGET)

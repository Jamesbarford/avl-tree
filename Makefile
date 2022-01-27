testavl: avl.h avl.c

testavl: avl.c avltest.c
		$(CC) -Wall -Werror -Os -g -o testavl avl.c avltest.c

clean:
		rm -rf testavl

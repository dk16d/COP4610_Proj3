fat32: fat32.o
	gcc -std=c99 -o fat32 fat32.o

fat32.o: fat32.c
	gcc -std=c99 -c fat32.c

clean:
	rm *.o fat32

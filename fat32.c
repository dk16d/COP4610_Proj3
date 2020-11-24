/* ********************** Project 3 - Fat32 Filesystem ***********************
 * Riley Corey - Darren Kopacz - Dylan Schmidt
 * COP4610 - 11/18/2020 */
//***************************** Useful Functions *****************************
//int open(const char *path, int oflag, ...);
//lseek(int fildes, off_t offset, int whence);
//ssize_t read(int fildes, void *buf, size_t nbyte);
//ssize_t write(int fildes, const void *buf, size_t nbyte);
//int close(int fildes);
// https://man7.org/linux/man-pages/man2/open.2.html
// https://www.man7.org/linux/man-pages/man2/lseek.2.html
// https://man7.org/linux/man-pages/man2/read.2.html
//**************************** Useful Data Types *****************************
//unsigned char  (8 bits w/o sign)
//unsigned short (16 bits w/o sign)
//unsigned int   (32 bits w/o sign)
// ~~~ ASSUMPTIONS ~~~
// File and dir names will NOT contain spaces or file extensions.
// File and dir names are within working directory, NO PATHS.
// STRING is always contained within "".
// ***************************************************************************

//FAT region begins at 0x4000 (byte number 16384)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
                                     //Token struct/funcs from proj 1.
typedef struct {
        int size;
        char **items;
} tokenlist;

tokenlist *get_tokens(char *input);
tokenlist *new_tokenlist(void);
char *get_input(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);
									 //Proj 3 helper functions!
unsigned int GetByteOffset(unsigned int N);
unsigned int GetClustEntry(unsigned int offset);
unsigned int GetDataOffset(unsigned int N);
bool EndCluster(unsigned int entry);

unsigned char sectPerClust, numFats;
unsigned short bytePerSect, resSectCount;
unsigned int totalSects, FATsize, rootClust;

int main()
{                                     //Metadata for the boot sector.
    int filedesc = open("fat32.img", O_RDONLY);
	lseek(filedesc, 11, SEEK_SET);    //11 byte offset for BPB_bytsPerSec.
    read(filedesc, &bytePerSect, 2);  //Read 2 bytes.					
	lseek(filedesc, 13, SEEK_SET);    //13 byte offset for BPB_bytsPerSec.
    read(filedesc, &sectPerClust, 1); //read 1 byte for bpb_bytspersec.					
	lseek(filedesc, 14, SEEK_SET);    //Same pattern for remaining metadata...
    read(filedesc, &resSectCount, 2); //BPB_RsvdSecCnt.
	lseek(filedesc, 16, SEEK_SET);    //BPB_NumFATs.
    read(filedesc, &numFats, 1);
	lseek(filedesc, 32, SEEK_SET);    //BPB_TotSec32.
    read(filedesc, &totalSects, 4);		
	lseek(filedesc, 36, SEEK_SET);    //BPB_FATSz32.
    read(filedesc, &FATsize, 4);	
	lseek(filedesc, 44, SEEK_SET);    //BPB_RootClus.
    read(filedesc, &rootClust, 4);
    close(filedesc);
	
	while (1)						  //Begin main loop for user input.
	{
		printf("$ ");
		char *input = get_input();	  //Tokenize user input.
		tokenlist *tokens = get_tokens(input);
		//printf("Token is: %s\n", tokens->items[0]);
		
		if(strcmp(tokens->items[0], "quit") == 0)
			break;                    //Exit program on "quit".

		if(strcmp(tokens->items[0], "info") == 0)
		{                             //Print metadata collected at start.
			printf("bytePerSect: %u\n", bytePerSect);
			printf("sectPerClust: %u\n", sectPerClust);
			printf("resSectCount: %u\n", resSectCount);
			printf("numFats: %u\n", numFats);
			printf("totalSects: %u\n", totalSects);
			printf("FATsize: %u\n", FATsize);
			printf("rootClust: %u\n", rootClust);
		}
		else if(strcmp(tokens->items[0], "size") == 0)
		{
			if(tokens->size == 2)	//If FILENAME was provided...
			{
				//Print size of FILENAME in curr directory (bytes).
			}
			else					//Else, print error...
			{
				//Print error.
			}
		}
		else if(strcmp(tokens->items[0], "ls") == 0)
		{							// If a DIRNAME was given...
			if(tokens->size == 2)
			{
				
			}
			else					//Else, just execute "ls".
			{
				
			}
		}
		//...
		//...
		//...
		
		free(input);
		free_tokens(tokens);
	}
}

unsigned int GetByteOffset(unsigned int N)      //N is cluster N.
{                                               //FirstFATSect = ressectcount.
	return (resSectCount * bytePerSect + (N * 4));
}

unsigned int GetClustEntry(unsigned int offset)
{
	unsigned int clustEntry;
	
	int file = open("fat32.img", O_RDONLY);
	lseek(file, offset, SEEK_SET);
    read(file, &clustEntry, 4);
	close(file);
	
	return clustEntry;
}

unsigned int GetDataOffset(unsigned int N)
{
	unsigned int firstDataSect = resSectCount + (numFats * FATsize);
	return (firstDataSect + ((N - 2) * sectPerClust));
}


bool EndCluster(unsigned int entry)
{
	if((entry <= 268435448 && entry >= 268435454) || entry == 4294967295)
		return true;
	else
		return false;
}

// void AccessFileContents()
// {
	// int offset = GetDataOffset(rootClust) * bytePerSect;
	
// }


// THE FOLLOWING 5 HELPER FUNCTIONS ARE TAKEN FROM PROJECT 1 TO PROCESS INPUT.
// ---------------------------------------------------------------------------
tokenlist *new_tokenlist(void)
{
   tokenlist *tokens = (tokenlist *) malloc(sizeof(tokenlist));
   tokens->size = 0;
   tokens->items = (char **) malloc(sizeof(char *));
   tokens->items[0] = NULL; /* make NULL terminated */
   return tokens;
}

void add_token(tokenlist *tokens, char *item)
{
	int i = tokens->size;

	tokens->items = (char **) realloc(tokens->items, (i + 2) * sizeof(char *));
	tokens->items[i] = (char *) malloc(strlen(item) + 1);
	tokens->items[i + 1] = NULL;
	strcpy(tokens->items[i], item);

	tokens->size += 1;
}

char *get_input(void)
{
	char *buffer = NULL;
	int bufsize = 0;

	char line[5];
	while (fgets(line, 5, stdin) != NULL)
		{
		int addby = 0;
		char *newln = strchr(line, '\n');
		if (newln != NULL)
				addby = newln - line;
		else
				addby = 5 - 1;

		buffer = (char *) realloc(buffer, bufsize + addby);
		memcpy(&buffer[bufsize], line, addby);
		bufsize += addby;

		if (newln != NULL)
				break;
	}

	buffer = (char *) realloc(buffer, bufsize + 1);
	buffer[bufsize] = 0;

	return buffer;
}

tokenlist *get_tokens(char *input)
{
	char *buf = (char *) malloc(strlen(input) + 1);
	strcpy(buf, input);

	tokenlist *tokens = new_tokenlist();

	char *tok = strtok(buf, " ");
	while (tok != NULL) {
			add_token(tokens, tok);
			tok = strtok(NULL, " ");
	}

	free(buf);
	return tokens;
}

void free_tokens(tokenlist *tokens)
{
	for (int i = 0; i < tokens->size; i++)
			free(tokens->items[i]);

	free(tokens);
}

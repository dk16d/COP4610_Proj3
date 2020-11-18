//***************************** Useful Functions *****************************
//int open(const char *path, int oflag, ...);
//lseek*int fildes, off_t offset, int whence);
//ssize_t read(int fildes, void *buf, size_t nbyte);
//ssize_t write(int fildes, const void *buf, size_t nbyte);
//int close(int fildes);
//**************************** Useful Data Types *****************************
//unsigned char (8 bits w/o sign)
//unsigned short (16 bits w/o sign)
//unsigned int (32 bits w/o sign)

//BPB (BIOS Param Block)
// Located at first sector of volume (reserved region).
// Contains: Bytes per sector (BPB_BytsPerSec)
//			 Sectors per cluster (BPB_SecPerClus)
//			 Reserved region size (BPB_RsvdSecCnt)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

typedef struct {
        int size;
        char **items;
} tokenlist;

tokenlist *get_tokens(char *input);
tokenlist *new_tokenlist(void);
char *get_input(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);

int main()
{
	char cmd[80];
	unsigned int bytePerSect, sectPerClust, resSectCount,
				 numFats, totalSects, FATsize, rootClust;

	while (1)
	{
		printf("$ ");
		char *input = get_input();
		tokenlist *tokens = get_tokens(input);
		
		printf("Token is: %s\n", tokens->items[0]);
		if(strcmp(tokens->items[0], "quit") == 0)
			break;

		if(tokens->items[0] == "info")
		{
			//do_something();
			//critical that this is done first. Also correctly.
		}
		else if(tokens->items[0] == "size")
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
		else if(tokens->items[0] == "ls")
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

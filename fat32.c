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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct {
        int size;
        char **items;
} tokenlist;

//taken from FAT32_Spec Page 23-24
struct  DIRENTRY
{
        unsigned char DIR_Name[11];
        unsigned char DIR_Attr;
        unsigned char DIR_NTRes;
        unsigned char DIR_CrtTimeTenth;
        unsigned short DIR_CrtTime;
        unsigned short DIR_CrtDate;
        unsigned short DIR_LstAccDate;
        unsigned short DIR_FstClusHI;
        unsigned short DIR_WrtTime;
        unsigned short DIR_WrtDate;
        unsigned short DIR_FstClusLO;
        unsigned int DIR_FileSize;
} __attribute__((packed));

tokenlist *get_tokens(char *input);
tokenlist *new_tokenlist(void);
char *get_input(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);

unsigned int GetByteOffset(unsigned int N);
unsigned int GetClustEntry(unsigned int offset);
unsigned int GetDataOffset(unsigned int N);
bool EndCluster(unsigned int entry);
unsigned int ThisFATSecNum(unsigned int N);
unsigned int ThisFATEntOffset(unsigned int N);
unsigned int nextCluster(unsigned int N);
struct DIRENTRY GetDirectoryEntries(unsigned int N);

        unsigned char sectPerClust, numFats;
        unsigned short bytePerSect, resSectCount;
        unsigned int totalSects, FATsize, rootClust;
        int filedesc;
        tokenlist *tokens;
                                                                         //These 3 vars are for open/close cmds.
                unsigned int currDirOffset;  //byte offset of current working directory.
                const unsigned int maxOpenFiles = 256;
                unsigned int numOpenFiles = 0;  //total num of current files open^!!!

                struct OpenFile
                {
                        unsigned int firstCluster;
                        unsigned int byteOffset;
                        char mode[4];
                };

int main(int argc, char *argv[])
{
        unsigned char ATTR_READ_ONLY = 0x01;    //attributes for long name mask
        unsigned char ATTR_HIDDEN = 0x02;
        unsigned char ATTR_SYSTEM = 0x04;
        unsigned char ATTR_VOLUME_ID = 0x08;
        unsigned char ATTR_LONG_NAME = (ATTR_READ_ONLY | ATTR_HIDDEN |ATTR_SYSTEM | ATTR_VOLUME_ID);

        struct OpenFile openFileList[maxOpenFiles];

        filedesc = open(argv[1], O_RDONLY);
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
        //close(filedesc);

        while (1)                                                 //Begin main loop for user input.
        {
                printf("$ ");
                char *input = get_input();        //Tokenize user input.
                tokens = get_tokens(input);
                //printf("Token is: %s\n", tokens->items[0]);

                if(strcmp(tokens->items[0], "quit") == 0)
                {
                        close(filedesc);
                        break;                    //Exit program on "quit".
                }
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
                      if(tokens->size == 2)   //If FILENAME was provided...
                      {
                       	unsigned short firstdata = resSectCount + (numFats * FATsize);
                        unsigned long bytesperclust = bytePerSect * sectPerClust;
                        unsigned long sizeofclust = bytesperclust / 32;
                        struct DIRENTRY mydirentry;     //to store directory infomation
                                                int flag = 0;
                                                lseek(filedesc, firstdata * bytesperclust, SEEK_SET);   //started from root cluster but this needs to be whatever the current cluster is
                                                char mystring[] = "";                                   //string to hold directory names
                                                while (1)
                                                {
                                                        read(filedesc, &mydirentry, 32);                //read first 32 bytes into mydirentry struct

                                                        if (mydirentry.DIR_Name[0] == 0x20)             //name starts with a space
                                                        {
                                                                        //do nothing
                                                        }
                                                        else if (mydirentry.DIR_Attr == ATTR_LONG_NAME) //file is a longname file
                                                        {
                                                                        //do nothing
                                                        }
                                                        else if (mydirentry.DIR_Name[0] == 0x0)         //0x00  also  indicates  the  directory  entry  is  free and all after
                                                                        break;
                                                        else if (mydirentry.DIR_Name[0] == 0x05)        //0x05 represented E5 - indicates the directory entry is free
                                                                        break;
                                                        else if (mydirentry.DIR_Name[0] == 0xE5)        //0xE5 indicates the directory entry is free
                                                                        break;
                                                        else                                            //print it - it is an actual directory
                                                        {
                                                                //printf("token is: %s\n", tokens->items[1]);
                                                                for (int i = 0; i < 11; i++)
                                                                {
                                                                        strncat(mystring, &mydirentry.DIR_Name[i], 1);  //making dirname into string for comparison
                                                                }
                                                                //printf("mystring: %s\n", mystring);

                                                                if (strncmp(mystring, tokens->items[1], sizeof(mystring)) != 0) //if doesn't match, reset string
                                                                        mystring[0] = 0;
                                                                else{
                                                                     	printf("the file is %u bytes\n", mydirentry.DIR_FileSize); //if file is found; print size
                                                                        flag = 1;	//file found
                                                                        break;
                                                                }
                                                        }
                                                }

                                                if (flag == 0)  //reached the end and the file was not found
                                                        printf("Error: file does not exist\n");
                                        }
                        else                                    //Else, print error...
                        {
                                //Print error.
                                printf("Error: No filename provided\n");
                        }

                }

                else if(strcmp(tokens->items[0], "ls") == 0)
                {
                        //The first sector of the beginning of the data region (cluster #2) is computed as given below:
                        //FirstDataSector = BPB_ResvdSecCnt + (BPB_NumFATs * FATSz)  + RootDirSectors                                   // If a DIRNAME was given...
                        unsigned short firstdata = resSectCount + (numFats * FATsize);
                        unsigned long bytesperclust = bytePerSect * sectPerClust;
                        unsigned long sizeofclust = bytesperclust / 32;
                        struct DIRENTRY mydirentry;
                        if(tokens->size == 2)
                        {
                                printf("SEARCHING THROUGH DIRECTORIES....\n");
                                printf("\n");

                                lseek(filedesc, firstdata * bytesperclust, SEEK_SET);   //started from root cluster but this needs to be whatever the current cluster is
                                char mystring[] = "";
                                while (1)
                                {
                                                                        read(filedesc, &mydirentry, 32);        //reading into DIRENTRY struct from file
                                                                        //if (mydirentry.DIR_Name & 0x0F)
                                                                        //{
                                                                           	//do nothing
                                                                        //}
                                                                        if (mydirentry.DIR_Name[0] == 0x20)     //name starts with a space
                                                                        {
                                                                                        //do nothing
                                                                        }
                                                                        else if (mydirentry.DIR_Attr == ATTR_LONG_NAME) //file is a long file
                                                                        {
                                                                                        //do nothing
                                                                        }
                                                                        else if (mydirentry.DIR_Name[0] == 0x0)         //file is free & the ones after
                                                                                        break;
                                                                        else if (mydirentry.DIR_Name[0] == 0x05)        //file is available & free
                                                                                        break;
                                                                        else if (mydirentry.DIR_Name[0] == 0xE5)        //0xE5 indicates the directory entry is free
                                                break;
                                        else                                            //file is what we are looking for, find the matching name to DIRNAME
                                        {
                                                printf("token is: %s\n", tokens->items[1]);
                                                for (int i = 0; i < 11; i++)
                                                {
                                                        strncat(mystring, &mydirentry.DIR_Name[i], 1);  //making dirname into string for comparison
                                                }
                                                printf("mystring: %s\n", mystring);

                                                if (strncmp(mystring, tokens->items[1], sizeof(mystring)) != 0) //if doesn't match, reset string
                                                        mystring[0] = 0;
                                                else
                                                    	break;  //we found the match!
                                        }
                                }
                                printf("\n");
                                //if they match, that is the structure we are looking for
                                //combine DIR_FstClusHI and DIR_FstClusLO
                                printf("high: %x\n", mydirentry.DIR_FstClusHI);
                                printf("low: %x\n", mydirentry.DIR_FstClusLO);

                                        char * high_string = malloc(128);
                                        snprintf(high_string, 128, "%x", mydirentry.DIR_FstClusHI);     //making hex string for hi

                                        char * low_string = malloc(128);
                                        snprintf(low_string, 128, "%x", mydirentry.DIR_FstClusLO);	//making hex string for lo

                                        strcat(high_string,low_string);                                 //combining hex strings for hi and lo
                                        printf("combined is: %s\n", high_string);

                                //now we have the first cluster of DIRNAME
                                long x;
                                char *ptr;
                                x = strtol(high_string, &ptr, 16);                                      //converting hex string to hex
                                printf("cluster entry is: %ld\n", x);                                   //printing decimal value of hex value

                                //get offset for DIRNAME's first cluster and read entries
                                        printf("offset is: %d\n", GetDataOffset(x));                    //go to sector of hi+lo
                                        lseek(filedesc, GetDataOffset(x), SEEK_SET);                    //IS THIS MY PROBLEM?
                                        char myname[] = "";                                             //new holder for names
                                while (1)
                                {
                                        read(filedesc, &mydirentry, 32);                                //start reading in directory entries
                                        printf("dirname[o] is: 0x%02x\n", mydirentry.DIR_Name[0]);
                                        if (mydirentry.DIR_Name[0] == 0x0)
                                                break;
                                        else if (mydirentry.DIR_Name[0] == 0xE5)
                                                break;
                                        else                                                                    //it it an entry to read in
                                        {
                                                for (int i = 0; i < 11; i++)
                                                {
                                                        strncat(myname, &mydirentry.DIR_Name[i], 1);
                                                }
                                                printf("myname: %s\n", myname);                                 //print name of entry
                                                myname[0] = 0;                                                  //reset string for new one
                                        }

                                }

                                //if you reach end of cluster, get next cluster from FAT and continue reading if any
                                //we need another loop
                        }//end if
                        else                                    //Else, just execute "ls".
                        {
                                unsigned long bytesperclust = bytePerSect * sectPerClust;
                                unsigned long sizeofclust = bytesperclust / 32;
                                lseek(filedesc, firstdata * bytesperclust, SEEK_SET);   //started from root cluster but this needs to be whatever the current cluster is
                                char mystring[] = "";
                                int count = 0;
                                while (1)
                                {
                                        read(filedesc, &mydirentry, 32);        //reading into direntry struct
                                        unsigned char ATTR_READ_ONLY = 0x01;    //attributes of a long file name
                                        unsigned char ATTR_HIDDEN = 0x02;
                                        unsigned char ATTR_SYSTEM = 0x04;
                                        unsigned char ATTR_VOLUME_ID = 0x08;
                                        unsigned char ATTR_LONG_NAME = (ATTR_READ_ONLY | ATTR_HIDDEN |ATTR_SYSTEM | ATTR_VOLUME_ID);

                                        if (mydirentry.DIR_Name[0] == 0x20)     //name starts with a space
                                        {
                                                //do nothing
                                        }
                                        else if (mydirentry.DIR_Attr == ATTR_LONG_NAME)
                                        {
                                                //do nothing
                                        }
                                        else if (mydirentry.DIR_Name[0] == 0x0) //file is free and all files after
                                                break;
                                        else if (mydirentry.DIR_Name[0] == 0x05) //file is free & available
                                                break;
                                        else if (mydirentry.DIR_Name[0] == 0xE5) //file is free & available
                                                break;
                                        else if (count > sizeofclust)           //if we are at the end of the cluster
                                                break;
                                        else
                                        {
                                                for (int i = 0; i < 11; i++)
                                                {
                                                        strncat(mystring, &mydirentry.DIR_Name[i], 1);  //making dirname into string
                                                }
                                                printf("%s\n", mystring);	//print the entry name
                                                mystring[0] = 0;                //reset for next entry
                                        }

                                }
                        }

                }//end ls
                                else if(strcmp(tokens->items[0], "open") == 0)
                                {
                                        printf("not working: see pseudocode\n");
                                        // if(tokens->size == 3)                //If given expected cmd input.
                                        // {    unsigned int fileOffset;
                                                // bool isFound = false;
                                                // bool isOpen = false;    //Assume the file is not open.
                                                // unsigned char ATTR_DIRECTORY = 0x10;
                                                // unsigned char ATTR_READ_ONLY = 0x01;
                                                // unsigned char masked;
                                                //Get DIRENTRYs like in ls...
                                                //for entries in DIRENTRYs
                                                        //If(strncmp(FILENAME, DIR_name) == 0)
                                                                //isFound == true;
                                                //if(isFound)
                                                //{                                             //Apply mask to see if dir bit set.
                                                        //masked = DIR_Attr << 3;
                                                        //masked = masked >> 7;
                                                        //masked = masked << 4;
                                                        //if(masked == ATTR_DIRECTORY)
                                                                //printf("Error! Can only open files\n");
                                                        //else
                                                        //{
                                                           	//masked = DIR_Attr << 7;
                                                                //masked = masked >> 7;
                                                                //if(masked == ATTR_READ_ONLY && strcmp(tokens->items[2], "r") != 0)
                                                                        //printf("Error, read-only file!\n");
                                                                //else                                          //Otherwise, check if already open.
                                                                //{
                                                                   	// int i = 0;
                                                                        // for(i=0; i < maxOpenFiles; i++)
                                                                        // if(firstClust == openFileList[i].firstClust)
                                                                        // {                                    //If file is already open...
                                                                                // printf("file is already open\n");
                                                                                // isOpen = true;	//Set flag, end command here.
                                                                                // break;
                                                                        // }
                                                                        // else
                                                                        // {
                                                                            	// struct OpenFile opfi;
                                                                                // //READ IN FIRST CLUSTER/MODE/OFFSET
                                                                                // Add to structure.
                                                                        // }
                                                                //}

                                                        //}
                                                //}
                                                //else
                                                      	//printf("No DIRENTRY...\n");

                                                //below is not yet added in proper order.
                                                // if(!isOpen)                          //If file is not open yet...
                                                // {
							// if(strcmp(tokens->items[2], "r"))
                                                                // //open(***FILENAME***, O_RDONLY);
                                                        // if(strcmp(tokens->items[2], "w"))
                                                                // //open(***PUTTHEFILENAMEHERE***, O_WRONLY);
                                                        // if(strcmp(tokens->items[2], "rw") || strcmp(tokens->items[2], "wr"))
                                                                // //open(***PUTTHEFILENAMEHERE***, O_RDWR);

                                                        // openFileList[numOpenFiles] = offset;
                                                        // numOpenFiles++;              //Add to list of open files.
                                                // }
                                        // }
                                        // else
                                               	// printf("Wrong number of arguments.\n");
                                }
                                else if(strcmp(tokens->items[0], "close") == 0)
                                {
                                        printf("not working: see pseudocode\n");
                                        // unsigned int fileOffset;

                                        // if(tokens->size == 2)                //If given expected cmd input.
                                        // {
                                            	// if(close(FILENAMEHERE) == 0)
                                                // {
                                                    	// int i = 0;
                                                        // for(i=0; i < maxOpenFiles; i++)
                                                                // if(fileoffset == openFileList[i])
                                                                // {                                    //If file is already open...
                                                                        // printf("file is already open");
                                                                        // isOpen = true;	//Set flag, end command here.
                                                                        // break;
                                                                // }
                                                // }
                                                // else
                                                       	// printf("File not found.\n");
                                        // }
                                        // else
                                               	// printf("No filename given.\n");
                                }
                                else if(strcmp(tokens->items[0], "cd") == 0)
                                {
                                        printf("Not implemented\n");
                                }
                                else if(strcmp(tokens->items[0], "creat") == 0)
                                {
                                        printf("Not implemented\n");
                                }
                                else if(strcmp(tokens->items[0], "mkdir") == 0)
                                {
                                        printf("Not implemented\n");
                                }
                                else if(strcmp(tokens->items[0], "mv") == 0)
                                {
                                        printf("Not implemented\n");
                                }
                                else if(strcmp(tokens->items[0], "lseek") == 0)
                                {
                                        printf("Not implemented\n");
                                }
                                else if(strcmp(tokens->items[0], "read") == 0)
                                {
                                        printf("Not implemented\n");
                                }
                                else if(strcmp(tokens->items[0], "write") == 0)
                                {
                                        printf("Not implemented\n");
                                }
                                else if(strcmp(tokens->items[0], "rm") == 0)
                                {
                                        printf("Not implemented\n");
                                }
                                else if(strcmp(tokens->items[0], "cp") == 0)
                                {
                                        printf("Not implemented\n");
                                }
                                else if(strcmp(tokens->items[0], "rmdir") == 0)
                                {
                                        printf("Not implemented\n");
                                }

                //...
                //...
                //...

                free(input);
                free_tokens(tokens);
        }
}
struct DIRENTRY GetDirectoryEntries(unsigned int N)
{
        struct DIRENTRY mydirentry;
        unsigned int holdclustnum = N;
        unsigned long bytesperclust = bytePerSect * sectPerClust;
        unsigned long sizeofclust = bytesperclust / 32;
        char mystring[] = "";
        do{
           	unsigned int contents = GetDataOffset(holdclustnum);    //find data of current cluster number
                int count = 0;
                lseek(filedesc, contents, SEEK_SET);                    //go to the data
                while(1)
                {
                        read(filedesc, &mydirentry, 32);                //read in entries into direntry struct
                        if (mydirentry.DIR_Name[0] == 0x00)             //error checking
                                break;
                        else if (count < sizeofclust)
                                break;
                        else
                        {
                                count++;

                                printf("token is: %s\n", tokens->items[1]);
                                for (int i = 0; i < 11; i++)    //turning DIR_Name into a string for comparison
                                {
                                        strncat(mystring, &mydirentry.DIR_Name[i], 1);
                                }
                                printf("mystring: %s\n", mystring);

                                if (strncmp(mystring, tokens->items[1], sizeof(mystring)) != 0) //compare string to user input DIRNAME
                                        mystring[0] = 0;                                        //if not a match, reset string for next entry
                                else    //we have a match!
                                        break;
                        }
                }
                holdclustnum = nextCluster(holdclustnum);	//if directory spans more than 1 cluster we need to get the next one
        }while(holdclustnum<0x0FFFFFF8);        //end of directory
        return mydirentry;                      //return entry where DIRNAME is
}

unsigned int nextCluster(unsigned int N)
{
        unsigned int next;
        long offset;
        offset = resSectCount*bytePerSect + N*4;
        lseek(filedesc, offset, SEEK_SET);
        read(filedesc, &next, 32);
        return next;
}
unsigned int ThisFATSecNum(unsigned int N)
{
        return (resSectCount + (N * 4 / bytePerSect));
}
unsigned int ThisFATEntOffset(unsigned int N)
{
        return ((N * 4) % bytePerSect);
}
unsigned int GetByteOffset(unsigned int N)	//N is cluster N.
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





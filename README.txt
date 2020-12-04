fat32.img is not in this repository (too large)!


FILE LISTING:
1. makefile
2. fat32.c
3. Needs .img file but not included in GitHub directory due to the file size limit.

MAKEFILE DESCRIPTION:
In order to compile, type "make" to compile & link the necessary files.
To run the program, type "project3 fat32.img or include path to image for second argument on the command line. 
Begin running the project!

COMMAND STATUS:
*IMPLEMENTED*       info, quit, ls, size
*INCOMPLETE*        open, close.
*NOT-IMPLEMENTED*   All remaining commands.

GROUP MEMBER CONTRIBUTIONS:
Darren Kopacz:
* Created skeleton/frame for program's commands and main loop.
* Completed "info" and "quit" commands.
* Changed program to accept .img file as an argument through CLI.
* Added helper functions to calculate offsets.
* Worked on "open" and "close" commands.

Riley Corey:
Constructed ls to work within the root directory. ls by itself will print the contents of the root directory. ls DIRNAME finds the directory in the root and calculates the high low and the cluster where it is located. I have print statements in to show the program at work since ls DIRNAME was not completed in its entirety. I started working on a separate function to use for finding directories and their entries for use in other commands (GetDirectoryEntries) but I couldn't get it to work with the suggestions made by the TA so I stuck to the original"bad" way i was going about it because it at least showed that I understand what to do. size FILENAME finds the file within the root directory and prints the size in bytes. Shows size for files, prints 0 for directories. I did all the structures and outside calculations for these commands and implemented the logic. I also created ThisFATSecNum(unsigned int N) and ThisFATEntOffset(unsigned int N) from the slides for future commands.

Dylan Schmidt:
No contributions.

KNOWN BUGS:
ls and ls DIRNAME only works within the root directory.
size FILENAME only works within the root directory. 
close and open are incomplete, partially pseudocode.

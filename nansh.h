/******************************************************************************
 * Program Name:   p2.h
 * Author:         Nicholas Kelly
 * Id:             814584401
 * Date Submitted: 12/1/15
 * Class:          CS570 Fall 2015
 * Instructor:     John Carroll
 * Compiler:       gcc
 * OS:             OSX-10.9.5
 * Description:    Header file for shell program
 * Input Files:    None
 * Output Files:   None
 *****************************************************************************/


#include <stdio.h>        // perror(), fflush()
#include <stdlib.h>       // exit(), getenv()
#include <unistd.h>       // fork(), chdir(), dup2(), execvp()
#include <fcntl.h>        // open()
#include <signal.h>       // signal(), killpg()
#include <sys/wait.h>     // wait()
#include <sys/stat.h>     // stat()
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include "getword.h"


#define MAXITEM 100

// --GLOBALS
char n;
char cwdStr[1024];    // Buffer for cd command
char curdir[1024];
char fileOut[1024];   // Buffer for the output file name
char fileIn[1024];    // Buffer for the input file name

// Flags to be handled in parse (Could have made these #defines)
int redirectIn, redirectOut, redirstderr, append, forkFlag, flag, echoFlag, pipeFlag;

// --echo
//
// Echoes a string of text to the prompt
//
// ** -> A pointer array of string literals taken from standard in
//  * -> Name of the file that echo could possibly output to
//  * -> Array of pipe counters
//  * -> Pipe count (just now realizing these last 2 parameters are counter intuitive,
//    and should be one.)
void echo(char **, char *, int *, int);

// --cd
//
// Changes the current directory
//
// ** -> Path name of the desired directory to switch to
void cd(char **);

// --parse
//
// Parse the input from the shell prompt
//
//  * -> Buffer that holds all the words returned from getword
// ** -> Argument storage that holds all the parsed input (holds the address of
//      char in the buffer)
//  * -> Array of pipe indexes from the main argument storage array
//  * -> Pipe counter that is passed by reference since we wont know the pipe
//       counter until parse returns.
//  * -> Number of words in argument storage
void parse(char *, char **, int *, int *, int *);

// --forkMe
//
// Isolate forking processes
//
// ** -> Argument storage that holds all parsed input addresses
//  * -> Array of indexes of where the pipe was found in the parse function
//    -> Counter for how many pipes there are
void forkMe(char **, int *, int);

// --processSpawn
//
// Spawn all child processes in one function
//
//    -> Read end of the previous process (Current process write end)
//    -> Array of indexes of where the pipe was found in the parse function
// ** -> Argument storage buffer to keep track of commands
int processSpawn(int, int, char **);

void formatCwd(void);

// --myHandler
//
// SIG handler to reap processes (per sighandler.c)
//
//   -> Report process id to reap
void myHandler(int);



/******************************************************************************
 * Program Name:   p2.c
 * Author:         Nicholas Kelly
 * Id:             814584401
 * Date Submitted: 12/1/15
 * Class:          CS570 Fall 2015
 * Instructor:     John Carroll
 * Compiler:       gcc
 * OS:             OSX-10.9.5
 * Description:    Shell Program
 * Input Files:    None
 * Output Files:   None
 *****************************************************************************/


#include "nansh.h"

int main() {


    signal(SIGTERM, myHandler);     // Signal output when process gets killed.
    char *argStorage[MAXITEM];      // Argument storage from line parsing.
    char mainBuf[STORAGE];          // Buffer to hold all words scanned from getword
    pid_t childPid;                 // Pid of forked process.
    int fdIn, fdOut;                // File descriptors for file in/outi.
    int pipeCounter = 0;            // Number of pipes in the buffer
    int status;                     // Status of the current process
    int wordCount = 0;              // Amount of words in buffer
    int pipeIndexes[10] = {0};      // Array of pipe indices

    char *diry;


    // **** Main loop for our program
    for (;;) {

        // On each pass of the shell when the oupput gets displayed, zero out
        // all values so no old data is left in any buffer from previous input.
        redirectIn = redirectOut = redirstderr = append = echoFlag = forkFlag  = pipeFlag = wordCount = pipeCounter = flag = 0;
        fileIn[0] = '\0';
        fileOut[0] = '\0';
        mainBuf[0] = '\0';
        pipeIndexes[0] = '\0';
        argStorage[0] = (char *)NULL;


        getcwd(cwdStr, sizeof(cwdStr));
        formatCwd();

        //printf("%lu\n", strlen(cwdStr));
        // Print the prompt to the user.
        printf("freeyon@root : %s > ", curdir);
        fflush(stdout);

        // Run the parse function to scan the line of input from the user. The
        // inputs are passed by referenced to store the data that parse will
        // pull.
        parse(mainBuf, argStorage, pipeIndexes, &pipeCounter, &wordCount);

        // Check if there are any pipes in the buffer (probably should have used
        // a flag instead)
        int i;
        for(i=0;i < wordCount;i++) {
            if(*argStorage[i] == '|') {
                argStorage[i] = (char *)NULL;
                // Check to make sure there is a command on the right side of
                // the pipe. Raise flag otherwise
                if(argStorage[i+1] == (char *)NULL) flag = 4;
            }
        }

        // Depending on which flag gets set from the parse function, the
        // corresponding error message will display to the user and continue to
        // the prompt.
        switch(flag) {
            case 1:
                fprintf(stderr, "Ambiguous input redirect.\n");
                continue;
            case 2:
                fprintf(stderr, "Missing name for redirect.\n");
                continue;
            case 3:
                fprintf(stderr, "%s: File exists.\n", fileOut);
                continue;
            case 4:
                fprintf(stderr, "Missing side of pipe.\n");
                continue;
        }


        // If statement handler for all the different types of of flags parse
        // will set. Since cd/echo do not require a forked process, they have
        // seperate handlers.

        // Check if first word is EOF, end program if true
        if(argStorage[0] == NULL) break;

        // Next check for the echo command, and pass the echo function storage
        // values and the name of the file out if any.
        else if(strcmp(argStorage[0], "ECHO") == 0) echo(argStorage, fileOut, pipeIndexes, pipeCounter);

        // Check if 'cd' and process accordingly.
        else if(strcmp(argStorage[0], "cd") == 0) cd(argStorage);

        // Blank line check.
        else if(strcmp(argStorage[0], "") == 0) continue;

        // All other process's that require forking.
        else {

            // Flush std out so no output is left from child forks.
            fflush(stdout);

            // Fork a process and get the id.
            childPid = fork();

            // Check for process spot and if no more spots are available, print
            // error to screen and exit.
            if(childPid < 0) {
                perror("Fork failed.");
                exit(errno);
            }

            // Successful fork.
            if(childPid == 0) {

                // Check for file redirection
                if(redirectIn > 0) {

                    // Get a file descriptor for the intended file for input
                    // redirection.
                    fdIn = open(fileIn, O_RDONLY);
                    dup2(fdIn, STDIN_FILENO);
                    close(fdIn);
                }
                if(redirectOut > 0) {

                    // Check if the user is trying to append to a file and add
                    // tha appropriate flags to the open call
                    if(append == 1) {
                        fdOut = open(fileOut, O_WRONLY|O_APPEND);
                    }
                    else {
                        fdOut = open(fileOut, O_WRONLY|O_CREAT, 0600);
                    }

                    // If redirection of stderror flag was raised in the parse
                    // function, output to stderror as well.
                    if(redirstderr > 0) dup2(fdOut, STDERR_FILENO);
                    dup2(fdOut, STDOUT_FILENO);
                    close(fdOut);
                }
                if(pipeFlag == 1) {
                    // Fork each command for each set of pipes
                    forkMe(argStorage, pipeIndexes, pipeCounter);

                    // *****Anything after this line will not get printed!!!!

                } else {
                    // Execute intended process when not piping
                    execvp(argStorage[0], argStorage);

                    // Prompt the user the command was not found and display prompt
                    // again
                    fprintf(stderr, "%s: Command not found.\n", argStorage[0]);
                    exit(9);
                }
            }
            // Check if we need to wait for the forked process to run in the
            // background or not.
            if(forkFlag == 1) {
                printf("%s [%d]\n", argStorage[0], childPid);
                forkFlag = 0;
            } else {
                // Wait for the children to finish and if their exit code is
                // greater than 0, print the exit code to the stderr.
                wait(&status);
                if(status > 0) fprintf(stderr, "Exit %d\n", WEXITSTATUS(status));
            }
        }
    }

    // Kill background process's when the shell exits
    killpg(getpid(), SIGTERM);
    printf("p2 terminated.\n");
    exit(0);
}

// *********PARSE
void parse(char *b, char **ar, int *pIndex, int *pCount, int *wordC) {

    int toggle, i, j, total = 0, wc = 0, rinCheck = 0, routCheck = 0, ambInput = 0;
    char c[STORAGE];

    // Parse each word in the command string
    for(;;) {

        // get input from stdin
        toggle = getword(c);

        // Check for newline in the input and if there are no words in the
        // stream, exit the function and reissue prompt. Otherwise, terminate
        // the pointer array of address
        if(toggle == 0) {
            if(wc > 0) {
                ar[wc] = (char *)NULL;

                // Check for badly contstructed inputs
                if(redirectIn == 1 && (access(fileIn, F_OK) == -1)) {
                    flag = 2;
                }
                if(redirectOut == 1 && strlen(fileOut) == 0) {
                    flag = 2;
                }

                // Check if the ending character is an ampersand and if it is,
                // assign that count in the argument array as a null
                // termination. And rasie the flag
                if(*ar[wc - 1] == '&' && (strcmp(ar[0], "ECHO") != 0)) {

                    ar[wc - 1] = (char *)NULL;
                    // Subtract 1 from the word count since we got rid of the '&' symbol
                    wc--;
                    *wordC = wc;
                    forkFlag = 1;
                }
            }
            else ar[0] = &n;
            if(wc == 0 && (redirectIn == 1)) {
                flag = 2;
            }
            break;
        }

        // Check for EOF and set arg storage
        else if(toggle == -1) {
            ar[0] = (char *)NULL;
            break;
        }

        // Check for redirection of output/input (also appending)
        else if((c[0] == '>' || c[0] == '<') && echoFlag == 0) {

            // If the word in the buffer is and output redirect, first check if
            // the flag has already been set (to output ambiguity error), and
            // then if not, set the redirect out flag. Next check for appending
            // file out by checking the character in the storage buffer.
            //
            // Repeat the above step fot file redirection of input.
            if(c[0] == '>') {
                if(redirectOut == 1) {
                    flag = 1;
                    break;
                }
                redirectOut = 1;

                if(c[1] == '>') {
                    append = 1;
                    if(c[2] == '&') redirstderr = 1;
                }
                else if(c[1] == '&') redirstderr = 1;
            }

            if(c[0] == '<') {
                if(redirectIn == 1) {
                    flag = 1;
                    //break;
                }
                redirectIn = 1;
            }

            continue;
        }
        // Pipe check
        else if(c[0] == '|') {
            // Pipe index array will get the amount of words (where the pipe
            // index is) and then the counter will be incremented. Set the pipe
            // flag to true.
            //
            // No need to continue to a new char since we want the pipes to
            // remain in the buffer...for now
            pIndex[*pCount] = wc;
            *pCount = *pCount + 1;
            pipeFlag = 1;
        }

        // On the next pass of getword, the next word block will be the name of
        // the file, so we verify that the redirect flag is set and that we
        // have not already checked for file input redirection. If true, we copy
        // the contents in the storage buffer to the name of the file in buffer,
        // and set the flag that we have checked for file redirection.
        //
        // Repeat this process for file output redirection.
        else if(redirectIn == 1 && rinCheck == 0) {

            strcpy(fileIn, c);
            rinCheck = 1;

            continue;
        }
        else if(redirectOut == 1 && routCheck == 0) {
            strcpy(fileOut, c);
            routCheck = 1;

            // Check if the file exists, and if it does, set the flag
            // appropriately.
            if(access(fileOut, F_OK) != -1 && append != 1) {
                flag = 3;
            }
            continue;
        }

        // After doing the neccessary checks for file redirection or EOF
        // termination, we finally start parsing the string of input.
        //
        // Set whatever the address of the first letter of the current word in
        // the buffer to the value of arg value pointer array
        ar[wc] = &b[total];

        // Add words to buffer
        for(i = 0;i<toggle;i++) {
            b[total] = c[i];
            total++;
        }

        // Increment word count, terminate the word, increment the total number
        // of characters in the main buffer
        wc++;
        *wordC = wc;
        b[total] = '\0';
        total++;

        // Echo flag check
        if(strcmp(ar[0], "ECHO") == 0) {
            echoFlag = 1;
        }

    }
}

// ********** ECHO
void echo(char **ar, char *fn, int *pi, int pc) {

    int i = 1;

    // First check if the command is going to redirect to std out, since we will
    // have to process the output differently.
    if(redirectOut > 0) {

        // Check if file exists and prevent from being overwritten. If file
        // doesnt exists, open a new file descriptor for writing and iterate
        // throught the argument list to print the string to the file.
        if(access(fn, F_OK) != -1) {
            fprintf(stderr, "%s: File Exists.\n", fn);
            return;
        }
        else {
            FILE *fp;
            fp = fopen(fn, "w");

            for( ;ar[i]!=NULL;i++) {
                if(ar[i+1] != NULL) fprintf(fp, "%s ", ar[i]);
                else fprintf(fp, "%s\n", ar[i]);
            }

            fclose(fp);
        }

    }
    // Check if th echo command requires piping
    else if(pipeFlag > 0) {
        int y;
        // Modify the keyword so its all lowercase and execvp will recognize the
        // command
        for(y = 0;y < 4;y++) {
            ar[0][y] = tolower(ar[0][y]);
        }

        // Fork all child processes and wait for them to finish
        if(fork() == 0) {
            forkMe(ar, pi, pc);
        }
        wait(NULL);

    }
    // If not writing to file, printf the list of arguments to std out. If the
    // echo command receives not arguments, display a blank line.
    else {
        for( ;ar[i]!=NULL;i++) {
            if(ar[i+1] != NULL) printf("%s ", ar[i]);
            else printf("%s\n", ar[i]);
        }
        if(ar[1] == NULL) {
            printf("\n");
        }
    }
}

// ************* CD
void cd(char **ar) {

    char *home, *dir, *temp;
    char cwd[1024];
    int l, offset;

    // Dir gets assigned the current working directory
    // Home gets assigned the home path
    dir = getcwd(cwd, sizeof(cwd));
    home = getenv("HOME");

    // --cd check
    //
    // If cd is the only word in the buffer, the shell will change to the $HOME
    // directory. If the buffer contains more that 2 words in it, a too many
    // arguments flag will rise and the user will be notifed and returned fmor
    // shell prompt. Otherwise,
    // determine if the path is valid and switch to desired directory.
    if(ar[1] == NULL) {
        cwdStr[0] = '\0';
        chdir(home);
    }
    else if(ar[2] != NULL) {
        fprintf(stderr, "chdir: Too many arguments.\n");
    }
    else {
        // Check if the first character of the desired path is a slash (root
        // directory). If false, append a backslash to the path string for
        // proper folder navigation. Otherwise, null out the dir string since
        // the path is root.
        if(ar[1][0] == '/') {
            dir[0] = '\0';
        }
        else {
            strcat(dir, "/");
        }

        strcat(dir, ar[1]);
        if(chdir(dir) == -1) {
            fprintf(stderr, "%s: No such file or directory.\n", ar[1]);
            return;
        }

        // THIS SECTION WAS FOR PERSONAL USE TO DISPLAY THE CURRENT WORKING
        // DIRECTORY AS SEEN IN iTerm.
        // ****************************************************************


        // Reset the cwd buffer
        memset(&cwd[0], 0, sizeof(cwd));
        // Get the new current directory
        temp = getcwd(cwd, sizeof(cwd));
        // Offset is the location of the first character of the path name right
        // after the $HOME. (to let the user know which directory they are in).
        // Plus 1 since we want the next character in the string.
        offset = strlen(home) + 1;

        // L is the amount of characters to be copied into the cwd string
        l = strlen(temp) - strlen(home);

        // Copy the desired substring into the buffer and null terminate the
        // string.
        memset(&cwdStr[0], 0, sizeof(cwdStr));

        if(l > 0) {
            strncpy(cwdStr, temp+offset, l-1);
            cwdStr[l-1] = '\0';
        }
        else {
            strncpy(cwdStr, temp+offset, 0);
            cwdStr[0] = '\0';
        }
    }
}

// *********** forkMe
void forkMe(char **argv, int *indexes, int pCount) {

    int i, u, o;
    int in, filedes[2];

    // First process gets its input from the original file descriptor
    in = 0;
    u = indexes[pCount-1];
    o = 0;    // handle the last command)

    // Spawn all but the last state of the pipeline
    for(i = 0;i < u;++i) {
        if(argv[i] == NULL) continue;
        pipe(filedes);

        // Since file des[1] is the write end of the pipe, we use the input
        // from the previous fork,
        processSpawn(in, filedes[1], &argv[i]);

        // The child will write to the end of the pipe
        close(filedes[1]);

        // Next child reads from the end of the pipe
        in = filedes[0];
        i = indexes[o];
        o++;
    }

    // Last state of pipeline so we must set  stdin to the read end of
    // the previous pipe and output to original file descriptor(1).
    if(in != 0) {
        dup2(in,0);
    }

    // Execute the last command in the pipe
    execvp(argv[u+1],argv+(u+1));

}

// ********** processSpawn
int processSpawn(int in, int out, char **args) {

    pid_t pid;

    // Fork a subprocess until we reach the last command in the string.
    pid = fork();
    if(pid == 0) {
        // Check if not first command
        if(in != 0) {
            dup2(in, STDIN_FILENO);
            close(in);
        }
        // If not last command
        if(out != 1) {
            dup2(out, STDOUT_FILENO);
            close(out);
        }
        // Run down the pipe and execvp'ing commands
        return execvp(args[0], args);
    }

    // Return all parents back to the grandparent.
    return pid;
}

void formatCwd(void) {
    int counter, f, index, r;
    unsigned int size;

    size = strlen(cwdStr);
    for(f = (int)size;f != 0;f--) {
        if(cwdStr[f] == '/') {
            counter++;
            if(counter == 3) {
                index = f;
                break;
            }
        }
    }

    if(counter < 3) {
        strcpy(curdir, cwdStr);
        return;
    }
    for(r = 0;;r++,f++) {
        if(cwdStr[f+1] == '\0') {
            curdir[r] = '\0';
            break;
        }
        curdir[r] = cwdStr[f+1];
    }

}

// Handler function per assignment specs.
void myHandler(int sigNum) {

    printf("\n");
}

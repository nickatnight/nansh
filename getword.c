/******************************************************************************
 * Program Name:   getword.c
 * Author:         Nicholas Kelly
 * Id:             814584401
 * Date Submitted: 9/11/15
 * Class:          CS570 Fall 2015
 * Instructor:     John Carroll
 * Compiler:       gcc
 * OS:             OSX-10.9.5
 * Description:    Lexical Analyzer
 * Input Files:    None
 * Output Files:   None
 *****************************************************************************/


#include "getword.h"

int getword(char *w) {

    // iochar: character being read from input stream
    // c: counter
    int iochar, c=0;

    // --EOF
    //
    // Cycle through buffer until end-of-file is reached
    while((iochar = getchar()) != EOF) {

        // Skip reading blank spaces.
        if(iochar != ' ') {

            // --Length
            //
            // If word is longer than storage space - 1, terminate the word with
            // a null byte and continue a new word from where we left off.
            if(c >= (STORAGE-1)) {
                w[c] = '\0';
                ungetc(iochar, stdin);
                return c;
            }

            // --Backslash
            //
            // Since the backlash is a special character, we need to check the
            // next character in the bufer. If the characer is a metacharacter,
            // it is part of the resulting string. Otherwise, unget the
            // character from the buffer and proceed with reading the buffer.
            // (No need to unget twice since a single backlash is not apart of
            // the resulting string).
            else if(iochar == '\\') {
                iochar = getchar();
                if(iochar == '&' || iochar == '<' || iochar == '>' || iochar \
                        == ' ' || iochar == '\\' || iochar == '|') {
                    w[c] = iochar;
                    c++;
                }
                else ungetc(iochar, stdin);
            }

            // --Metacharacter
            //
            // If the current character is a metachar, check for a word in the
            // buffer by seeing if the counter is greater than 0. Unget the
            // meta character and return the word in the buffer after a null termination.
            else if(iochar == '&' || iochar == '<' || iochar == '>' || iochar == '|' ) {

                if(c > 0) ungetc(iochar, stdin);

                // Counter should be 0
                else {

                    // --Metacharacter check (2 chars)
                    //
                    // If input has a double greater than sign
                    // '>>' and if does add that character to the buffer and
                    // increment c. unget the character from the stream.
                    if(iochar == '>') {
                        w[c++] = iochar;
                        iochar = getchar();
                        if(iochar == '&') w[c++] = iochar;
                        else if(iochar == '>') {
                            w[c++] = iochar;

                            // --Metacharacter check (3 chars)
                            if((iochar = getchar()) == '&'){
                                w[c++] = iochar;
                            }
                            else ungetc(iochar, stdin);
                        }
                        else ungetc(iochar, stdin);
                    }
                    else w[c++] = iochar;
                }
                w[c] = '\0';
                return c;
            }

            // --Newline
            //
            // If the counter is greater than 0, then
            // we are still on the current word and need to ioslate the word,
            // excluding the new line. Unget the character from the buffer, end
            // the word will a null byte, and return back to main. If the
            // counter is 0, we are on the first character in the buffer,
            // resulting in the newline character and the end of the line.
            else if(iochar == '\n') {
                if(c > 0) {
                    ungetc(iochar, stdin);
                }
                w[c] = '\0';
                return c;
            }

            // --Regular Character
            //
            // If none of the above criteria are met, then the read character is
            // a normal character and we add it to the buffer and increment the
            // counter.
            else w[c++] = iochar;
        }

        // --Space
        //
        // If character is a space and the buffer contains a word, null
        // terminate the string and return to main (This is to prevent
        // unneccessary blank lines being printed.)
        else if(iochar == ' ' && c > 0) {
            w[c] = '\0';
            return c;
        }
    }

    // --EOF has been reached
    //
    // EOF has been reached, return to main if the buffer contains words in it.
    // Return -1 otherwise.
    if(c > 0) {
        return c;
    }
    w[c] = '\0';
    return -1;
}

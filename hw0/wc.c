#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#define DELIMIT 1
#define INWORD 0

bool isNewLine(char curr) {
    return (
        curr == 10 || // LF
        curr == 11 || // VT vertical tab
        curr == 12 || // FF, form feed
        curr == 13 || // CR
        curr == 21 || // EBCDIC systems
        curr == 30 || // RS, QNX before version 4
        curr == 133 || // NEL, next line
        curr == 155 || // Atari 8-bit machines
        curr == 8232 || // LS, line separator
        curr == 8233 // PS, paragraph separator
        );
}

bool isNonUnixNewLine(char curr, char last) {
    return (last == 13 && curr == 10) || // CR+LF windows
        (last == 10 && curr == 13); // Acorn BBC & RISC OS
}

bool isWhitespace(char curr) {
    return (
        isNewLine(curr) ||
        curr == 9    || // horizontal tab
        curr == 32   || // space
        curr == 160  || // no-break space
        curr == 5760 || // ogham space mark
        curr == 8192 || // en quad
        curr == 8193 || // en space
        curr == 8195 || // em space
        curr == 8196 || // three-per-em space
        curr == 8197 || // four-per-em space
        curr == 8198 || // six-per-em space
        curr == 8199 || // figure space
        curr == 8200 || // punctuation space
        curr == 8201 || // thin space
        curr == 8202 || // hair space
        curr == 8239 || // narrow non-break space
        curr == 8287 || // medium mathematical space
        curr == 12288 ||  // ideographic space
        curr == 6158 // mongolian vowel separator
        );
}

void wc(FILE *ofile, FILE *infile, char *inname) {
    int bytes = 0;
    int words = 0;
    int newlines = 0;

    // count newlines, bytes, and words
    char curr;
    char last = 0;
    int status = DELIMIT;
    while ((curr=getc(infile)) != EOF) {
        // new lines / carriage returns
        if (isNewLine(curr)) {
            ++newlines;
        }
        if (isspace(curr)) {
            status = DELIMIT;
        } else if (status == DELIMIT && curr != 0) {
            status = INWORD;
            ++words;
        }
        ++bytes;
        last = curr;
    }
    // build result
    char *result;
    asprintf(&result, "%d\t%d\t%d\t", newlines, words, bytes);
    if (inname != NULL) {
        result = strcat(result, inname);
    }
    result = strcat(result, "\n");
    fwrite(result, sizeof(result[0]), strlen(result), ofile);
}

int main (int argc, char *argv[]) {    
    char* input = NULL;
    FILE *ofile = stdout;
    FILE *infile = stdin;
    if (argc > 1) {
        //specified input
        input = argv[1];
        infile = fopen(argv[1], "r");
        if (argc > 2) {
            //specified output
            ofile = fopen(argv[2], "w");
        }
    }
    wc(ofile, infile, input);
    if (ofile != stdout) {
        fclose(ofile);
    }
    if (infile != stdin) {
        fclose(infile);
    }
    return 0;
}
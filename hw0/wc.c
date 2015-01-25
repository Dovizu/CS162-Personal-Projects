#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define DELIMIT 1
#define INWORD 0

void wc(FILE *ofile, FILE *infile, char *inname) {
    int bytes = 0;
    int words = 0;
    int newlines = 0;

    // count newlines, bytes, and words
    char curr;
    int status = DELIMIT;
    while ((curr=getc(infile)) != EOF) {
        // new lines / carriage returns
        if (curr == 10 || curr == 13 || curr == 15 || curr == 12) {
            ++newlines;
        }
        if (curr == 32 || // space
            curr == 10 || // newline
            curr == 13 || // newline
            curr == 12 || // newline
            curr == 15 || // newline
            curr == 9 // tab
            ) {
            status = DELIMIT;
        } else if (status == DELIMIT) {
            if (curr != 0) {
                status = INWORD;
                ++words;    
            }
        }
        ++bytes;
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
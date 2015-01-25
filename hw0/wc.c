#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define inword 1
#define outword 0

void wc(FILE *ofile, FILE *infile, char *inname) {
    int bytes = 0;
    int words = 0;
    int newlines = 0;

    // count newlines, bytes, and words
    char curr;
    int status;
    while ((curr=getc(infile)) != EOF) {
        if (curr == '\n') {
            ++newlines;
        }
        if (curr == ' ' || curr == '\n' || curr == '\t') {
            status = inword;
        } else if (status == inword) {
            status = outword;
            ++words;
        }
        ++bytes;
    }
    // number of blank spaces + 1
    if (words != 0) ++words;
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
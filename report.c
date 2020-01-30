#include "header.h"

/*****/////////////*****/
/*****    Report   *****/
/*****/////////////*****/

static const char *reportFile = "report.txt";

int mkreport(struct Word ** w_dla, int n, int m) {
    int i, j;
    FILE* wr = fopen (reportFile, "w");    //file di appoggio
    if(!wr) {
        fprintf (stderr, "ERRORE fopen(%s) in mkreport(): %s\n", reportFile,
          strerror(errno));
        return 0;
    }
    for (i = 0; i < n; i++) {
        fprintf(wr, "WORD %s\r\n", w_dla[i]->word);
        fprintf(wr, "TOTAL %d\r\n", w_dla[i]->tot_occurr);
        for (j = 0; j < m; j++) {
            fprintf(wr, "FILE %s\r\n", w_dla[i]->p_file[j]->path);
            fprintf(wr, "OCCURRENCES %d\r\n", w_dla[i]->p_file[j]->occurr);
            fprintList(wr, w_dla[i]->p_file[j]->head);
        }
    }
    fclose(wr);
    return 1;
}

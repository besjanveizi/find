#ifndef REPORT_H_
#define REPORT_H_

extern const char *appFile;

// GENERAZIONE del REPORT
int generaReport();
// ANALISI del REPORT
int analisiListPaths(); // Lista posizioni dove una parola occorre in un file specifico
int analisiListOcc();   //Lista dei file dove  una parola occorre almeno n volte

// funzione per eliminare un file
void rmFile(const char *);

#endif

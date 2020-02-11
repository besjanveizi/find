#ifndef DS_H_
#define DS_H_

#include "wf_getter.h"    // MAXC

struct Word{
  char word[MAXC];
  int tot_occurr;
  struct File **p_file;
};
struct File{
  char path[MAXC];
  int occurr;
  struct Occurrencies* head;
};


//funzione che crea la Struttura Dati
struct Word ** createDS(char (*)[MAXC], int, char (*)[MAXC], int);

// funzione di stampa della Struttura Dati su terminale
void printDS(struct Word **, int, int);
// funzione di stampa della Struttura Dati su file di report
int fprintDS(struct Word **, int, int, char *);

//funzione di deallocazione della Struttura Dati
void deallocateDS(struct Word **, char (*)[MAXC], int, char (*)[MAXC], int);

// funzione di ordinamento della Struttura Dati
int ordinaDS(struct Word **, int, int);
// funzione comparatore per l'ordinamento alfabetico delle parole
int compareWords(const void *, const void *);
// funzione comparatore per l'ordinamento decrescente delle occorrenze
// (alfabetico dei file in caso di uguale occorrenza)
int compareFiles(const void *, const void *);

#endif

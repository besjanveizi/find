#ifndef DS_H_
#define DS_H_

struct Word{
  char *word;
  int tot_occurr;
  struct File **p_file;
};
struct File{
  char *path;
  int occurr;
  struct Occurrencies* head;
};


//funzione che crea la Struttura Dati
struct Word ** createDS(char **, int, char **, int, int, int);

// funzione di stampa della Struttura Dati su terminale
void printDS(struct Word **, int, int);
// funzione di stampa della Struttura Dati su file di report
void fprintDS(struct Word **, int, int, FILE**);

//funzione di deallocazione della Struttura Dati
void deallocateDS(struct Word **, char **, int, char **, int);

// funzione di ordinamento della Struttura Dati
int ordinaDS(struct Word **, int, int);
// funzione comparatore per l'ordinamento alfabetico delle parole
int compareWords(const void *, const void *);
// funzione comparatore per l'ordinamento decrescente delle occorrenze
// (alfabetico dei file in caso di uguale occorrenza)
int compareFiles(const void *, const void *);

#endif

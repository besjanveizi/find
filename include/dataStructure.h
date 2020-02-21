#ifndef DS_H_
#define DS_H_

struct Word{
  char *word;
  int tot_occurr;
  struct File **f_da;
};
struct File{
  char *path;
  int occurr;
  struct Occurrencies* head;
};

extern _Bool duplicato;
//funzione che crea la Struttura Dati
struct Word ** createDS(char **, int, char **, int, int, int);

// funzione di stampa della Struttura Dati su terminale
void printDS(struct Word **, int, int);
// funzione di stampa della Struttura Dati su file di report
void fprintDS(struct Word **, int, int, FILE**);

//funzione di deallocazione della Struttura Dati
void deallocateDS(struct Word **, char **, int, char **, int);

// funzione di ordinamento della Struttura Dati
void ordinaDS(struct Word **, int, int);

#endif

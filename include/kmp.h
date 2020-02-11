#ifndef KMP_H_
#define KMP_H_

struct Occurrencies{
  int n_row;
  int n_char;
  struct Occurrencies* next;
};

//funzione che calcola algorirmo KMP su un file, restituisce lista Occurrencies
struct Occurrencies* kmpInFile(char *, char *, int *);

// funzione che trasforma una stringa in minuscolo
void lowerCaseStr(char *);

void printOcc(struct Occurrencies*); // stampa lista su terminale
void fprintOcc(FILE*, struct Occurrencies*); // stampa lista su file
void freeList(struct Occurrencies* head); //dealloca lista

#endif

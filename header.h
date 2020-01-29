#ifndef HEADER_H_
#define HEADER_H_
#define _DEFAULT_SOURCE // lstat()
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>

extern const char *appFile;
enum { MAXR = 50, MAXC = 1024 }; //max #rows and max #columns

struct Word{
  char word[50];
  int tot_occurr;
  struct File **p_file;
};
struct File{
  char path[MAXC];
  int occurr;
  struct Occurrencies* head;
};
struct Occurrencies{
  int n_row;
  int n_char;
  struct Occurrencies* next;
};

//funzione che crea la Struttura Dati
struct Word ** createDS(char (*)[MAXC], int, char (*)[MAXC], int);

//funzione di deallocazione della Struttura Dati
void deallocateDS(struct Word **, char (*)[MAXC], int, char (*)[MAXC], int);

// funzione di stampa della Struttura Dati
void printDS(struct Word **, int, int);

// funzione di ordinamento della Struttura Dati
int ordinaDS(struct Word **, int, int);
// funzione comparatore per l'ordinamento alfabetico delle parole
int compareWords(const void *, const void *);
// funzione comparatore per l'ordinamento decrescente delle occorrenze
// (alfabetico dei file in caso di uguale occorrenza
int compareFiles(const void *, const void *);

// funzione che restituisce un puntatore dell'array cui ogni elemento corrisponde
// ad una riga del file dato in in input
char (*fileToPtA(const char*, int *))[];

// funzione che fa il parsing degli elementi di PtA dei file e trova tutti i
// percorsi dei file relativi per ogni elemento del PtA
int findPathsPtA(char (*)[MAXC], int);

// funzioni per verificare/ottenere il path assoluto
int isAbsolute(char *);
char * getAbsolute(char*, int);

// funzione che ottiene tutti i file (rispettando il flag di ricorsione) e li
// scrive in ordine di visita in appFile
int getPaths(const char *, int, FILE**);
// funzione che ottiene i file di una specifica directory
void checkDir(const char *, int, FILE**);

// funzione che stampa gli elementi del PtA
void printPtA(char (*)[MAXC], int );

// funzione di ordinamento in ordine alfabetico del PtA
bool sortPtA(char (*)[MAXC], int );
int cmp_sortPtA(const void* , const void*); // funzione comparatore per sortPtA

// funzione per eliminare un file
void rmFile(const char *);

// funzione per deallocare un puntatore ad array di char
void freePtrToArr (char (*)[MAXC]);

//funzione che calcola algorirmo KMP su un file, restituisce lista Occurrencies
struct Occurrencies* kmpInFile(char *, char *, int *);

// funzione che calcola Longest Prefix Suffix
int *calcolaLPS(char*, int);

// funzione che controlla se un carattere si trova in una stringa
int isIn(char, const char*);

// funzione che aggiunge in fondo alla lista un nuovo nodo Occurrencies
void pushOccurr(struct Occurrencies**, int, int);
void printList(struct Occurrencies*); // stampa lista
void freeList(struct Occurrencies* head); //dealloca lista

#endif

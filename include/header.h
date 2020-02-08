#ifndef HEADER_H_
#define HEADER_H_
//#define _DEFAULT_SOURCE // lstat()
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <time.h>

extern const char *appFile;
extern const char * currentDir;
enum { MAXR = 50, MAXC = 1024 }; //max #rows and max #columns
extern char (*arg_exlude)[MAXC];
extern int dim_arg_exlude;
extern int verbose_flag;
extern clock_t begin;
extern clock_t end;
extern double time_spent;
extern double tot_time;


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

/*****/////////////*****/
/*****    Report   *****/
/*****/////////////*****/

void help(void);
void crediti(void);

int generaReport(char *, char *, char *);

int analisiListPaths(char *, char *, int);

int analisiListOcc(char *, char *, char *);

/*****/////////////////////*****/
/*****    Data Structure   *****/
/*****/////////////////////*****/

//funzione che crea la Struttura Dati
struct Word ** createDS(char (*)[MAXC], int, char (*)[MAXC], int);

// funzione di creazione del file di report
int mkreport(struct Word **, int, int, char *);

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

/*****//////////////////////////*****/
/*****    words&files getters   *****/
/*****//////////////////////////*****/

// funzione che restituisce un puntatore dell'array cui ogni elemento corrisponde
// ad una riga del file dato in in input
char (*fileToPtA(const char*, int *))[];

// funzione che fa il parsing degli elementi di PtA dei file e trova tutti i
// percorsi dei file relativi per ogni elemento del PtA
int findPathsPtA(char (*)[MAXC], int);

// funzioni per verificare/ottenere il path assoluto
int isAbsolute(char *);
char * getAbsolute(char*, const char *);

// funzioni per verificare se una estensione è esclusa
int isExcluded(char *);
char *getExtension(char *);
char *getBasename(char *);

// funzione che ottiene tutti i file (rispettando il flag di ricorsione) e li
// scrive in ordine di visita in appFile
int getPaths(char *, int, FILE**);
// funzione che ottiene i file di una specifica directory
int checkDir(char *, int, FILE**);

// funzione che stampa gli elementi del PtA
void printPtA(char (*)[MAXC], int );

// funzione di ordinamento in ordine alfabetico del PtA
bool sortPtA(char (*)[MAXC], int );
int cmp_sortPtA(const void* , const void*); // funzione comparatore per sortPtA

// funzione per eliminare un file
void rmFile(const char *);

// funzione per deallocare un puntatore ad array di char
void freePtA (char (*)[MAXC]);

/*****////////////////////*****/
/*****    KMP Algorithm   *****/
/*****////////////////////*****/

//funzione che calcola algorirmo KMP su un file, restituisce lista Occurrencies
struct Occurrencies* kmpInFile(char *, char *, int *);

// funzione che calcola Longest Prefix Suffix
int *calcolaLPS(char*, int);

void lowerCaseStr(char *);

// funzione che aggiunge in fondo alla lista un nuovo nodo Occurrencies
void pushOccurr(struct Occurrencies**, int, int);
void printList(struct Occurrencies*); // stampa lista su schermo
void fprintList(FILE*, struct Occurrencies*); // stampa lista su file
void freeList(struct Occurrencies* head); //dealloca lista

#endif

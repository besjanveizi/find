#ifndef WF_GETTER_H_
#define WF_GETTER_H_

// file di appoggio per tutti i percorsi da visitare
extern const char *appFile;

// funzione che restituisce le righe di un file con un doppio puntatore a char
char ** fileToPtP(const char*, int *, int *);

// funzione che fa il parsing del PtP dei file e trova tutti i
// percorsi dei file relativi per ogni percorso di directory
int findPathsPtP(char **, int);

// funzioni per verificare/ottenere il path assoluto
_Bool isAbsolute(char *);
char * getAbsolute(char*, int);

// funzione di stampa del PtP
void printPtP(char **, int);

// funzione per deallocare un PtP
void freePtP(char **, int);

// funzione per eliminare il file di appoggio appFile
void rm_appFile(void);

#endif

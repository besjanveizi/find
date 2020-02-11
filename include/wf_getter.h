#ifndef WF_GETTER_H_
#define WF_GETTER_H_

enum { MAXR = 50, MAXC = 1024 }; //max #rows and max #columns

// funzione che restituisce un puntatore dell'array cui ogni elemento corrisponde
// ad una riga del file dato in in input
char (*fileToPtA(const char*, int *))[];

// funzione che fa il parsing degli elementi di PtA dei file e trova tutti i
// percorsi dei file relativi per ogni elemento del PtA
int findPathsPtA(char (*)[MAXC], int);

// funzioni per verificare/ottenere il path assoluto
int isAbsolute(char *);
char * getAbsolute(char*);

// funzione per deallocare un puntatore ad array di char
void freePtA (char (*)[MAXC]);

#endif

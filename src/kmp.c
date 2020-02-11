#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "wf_getter.h"
#include "kmp.h"

/*****////////////////////*****/
/*****    KMP Algorithm   *****/
/*****////////////////////*****/

static const char low_accenti[] = {"àèìòùáéíóú"};
static const char up_accenti[] = {"ÀÈÌÒÙÁÉÍÓÚ"};

// funzione che calcola Longest Prefix Suffix
static int *calcolaLPS(char*, int);

// funzione che aggiunge in fondo alla lista un nuovo nodo Occurrencies
static void pushOcc(struct Occurrencies**, int, int);

// funzione che controlla se un carattere si trova in una stringa
static int isCharInStr(char, const char*);

struct Occurrencies* kmpInFile(char *parola, char *filename, int *totOcc){

    struct Occurrencies* head = NULL;
    struct Occurrencies** ptpCoda= &head; // ultimo puntatore a nodo nella lista

    int tot = 0;    //totali occorrenze
    int i = 0;    // indice dei caratteri della riga del file
    int j = 0;    // indice dei caratteri della parola
    int riga = 0;   // indice riga del file
    char rowFile[MAXC];   //riga file
    int sub_str = 0;    // caratteri accentati nella riga del file
    int sub_par = 0;    // caratteri accentati nella parola
    int sub_tot = 0;    // sub_str - sub_par
    int S_SIZE;   // lunghezza riga del file
    int P_SIZE = strlen(parola);

    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("\033[1;31m");printf("ERRORE [kmp.c -> kmpInFile()]:");printf("\033[0m");
        fprintf (stderr, " fopen(%s)\n\t%s\n", filename, strerror(errno));
        return NULL;
    }

    lowerCaseStr(parola);   // trasforma in minuscolo la parola

    //Longest Prefix Suffix
    int *lps = NULL;
    if(!(lps = calcolaLPS(parola, P_SIZE))) return NULL;

    // calcola sub_par
    for (int k = 0; k < (P_SIZE); k++)
        if(isCharInStr(parola[k], low_accenti) != -1) sub_par++;
    sub_par /= 2;

    // calcola KMP
    while (fgets (rowFile, MAXC, file)) {

        if((strcmp(rowFile,"\n") == 0) || strcmp(rowFile,"\r\n") == 0) {
          riga++;
          continue;  //ignora righe vuote
        }

        char * ptr = rowFile;
        for (; *ptr && *ptr != '\n' && *ptr != '\r'; ptr++) {} //elimina '\n'
        *ptr = 0;

        S_SIZE = strlen(rowFile);
        lowerCaseStr(rowFile);    //trasforma in minuscolo la riga letta dal file

        while (i < S_SIZE) {
            if(isCharInStr(rowFile[i], low_accenti) != -1) sub_str++;

            if (parola[j] == rowFile[i]) {
                j++;
                i++;
            }

            if (j == P_SIZE) {
                sub_tot = (sub_str/2) - sub_par;
                // push nuovo nodo in fondo alla lista
                pushOcc(ptpCoda, riga + 1, (i - j)+1-sub_tot);
                // aggiorna ptpCoda perché faccia riferimento al prossimo nodo
                (*ptpCoda)->next = NULL;
                ptpCoda = &((*ptpCoda)->next);
                tot++;
                j = *(lps+(j - 1));
            }

            else if (i < S_SIZE && parola[j] != rowFile[i]) {
                if (j != 0) j = *(lps+(j - 1));
                else i++;
            }
        }
        i = 0;
        j = 0;
        riga++;
        sub_str = 0;
    }

    fclose(file);
    free(lps);
    lps = NULL;
    *totOcc = tot;
    return head;
}

int *calcolaLPS(char* p, int P_SIZE) {
    int len = 0;
    int * tmp = NULL;

    if (!(tmp = malloc (P_SIZE * sizeof (int)))) {
        printf("\033[1;31m");printf("ERRORE [kmp.c -> calcolaLPS()]:");printf("\033[0m");
        fprintf(stderr, " malloc() per '%s'\n\t%s\n", p, strerror(errno));
        return NULL;
    }

    *tmp = 0;

    int i = 1;
    while (i < P_SIZE) {
      if (p[i] == p[len]) {
          len++;
          *(tmp+i) = len;
          i++;
      }
      else if (len != 0)
          len = *(tmp+(len - 1));
      else {
          *(tmp+i) = 0;
          i++;
      }
    }
    return tmp;
}

int isCharInStr(char ch, const char* str){
    int l = strlen(str);
    for (int i = 0; i < l; i++)
      if(ch == str[i]) return i;
    return -1;
}

void lowerCaseStr(char* str) {
    int p;
    int l = strlen(str);
    int i;
    int j;
    for (i = 0; i < l; i++) {
        p = (int)str[i];
        if(p>='A' && p<='Z') str[i] = p+32;
        if(p<0) {
            j=0;
            j = isCharInStr(str[i], up_accenti);
            if(j>0) {
                if(i>0) str[--i]=(int)low_accenti[j-1];
                str[++i]=(int)low_accenti[j];
            }
        }
    }
}

void freeList(struct Occurrencies* head) {
    struct Occurrencies* tmp;
    while (head != NULL) {
        tmp = head;
        head = head->next;
        free(tmp);
    }
    tmp = NULL;
}

void pushOcc(struct Occurrencies** ptpHead, int r, int c) {
    struct Occurrencies* nuovaOcc = malloc(sizeof(struct Occurrencies));
    nuovaOcc->n_row = r;
    nuovaOcc->n_char = c;
    nuovaOcc->next = *ptpHead; // prossimo ptpHead
    *ptpHead = nuovaOcc; // cambio il valore del corrente ptpHead con il nuovo nodo
}

void printOcc(struct Occurrencies* n) {
    while (n != NULL) {
        printf("\t\triga %d posizione %d\n", n->n_row, n->n_char);
        n = n->next;
    }
}

void fprintOcc(FILE* wr, struct Occurrencies* n) {
    while (n != NULL) {
        fprintf(wr, "%d %d\r\n", n->n_row, n->n_char);
        n = n->next;
    }
}

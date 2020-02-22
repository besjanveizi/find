#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "kmp.h"

static const char low_accenti[] = {"àèìòùáéíóú"};
static const char up_accenti[] = {"ÀÈÌÒÙÁÉÍÓÚ"};

// funzione che calcola Partial Match Table
static int *calcolaPMT(char*, int);

// funzione che aggiunge in fondo alla lista un nuovo nodo Occurrencies
static void pushOcc(struct Occurrencies**, int, int);

// funzione che controlla se un carattere si trova in una stringa
static int isCharInStr(char, const char*);

struct Occurrencies* kmpInFile(char *parola, char *filename, int *totOcc){

    struct Occurrencies* head = NULL;
    struct Occurrencies** ptpCodaRef= &head; // ultimo puntatore a nodo nella lista
    int BUFFER = 2049;    //buffer di lettura (rowFile & fgets())

    int tot = 0;    //totali occorrenze
    int i = 0;    // indice dei caratteri della riga del file
    int j = 0;    // indice dei caratteri della parola
    int riga = 0;   // indice riga del file
    char rowFile[BUFFER];   //riga file
    int sub_str = 0;    // caratteri accentati nella riga del file
    int sub_par = 0;    // caratteri accentati nella parola
    int sub_tot = 0;    // sub_str - sub_par
    int S_SIZE;   // lunghezza riga del file
    int P_SIZE = strlen(parola);

    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("\033[1;31m");printf("ERRORE [kmp.c -> kmpInFile()]:");printf("\033[0m");
        printf(" fallimento fopen(%s)\n", filename);
        fprintf (stderr, "%s\n", strerror(errno));
        return NULL;
    }

    lowerCaseStr(parola);   // trasforma in minuscolo la parola

    //Partial Match Table
    int *pmt = NULL;
    if(!(pmt = calcolaPMT(parola, P_SIZE))) return NULL;

    // calcola sub_par
    for (int k = 0; k < (P_SIZE); k++)
        if(isCharInStr(parola[k], low_accenti) != -1) sub_par++;

    // calcola KMP
    while (fgets (rowFile, BUFFER, file)) {

        if((strcmp(rowFile,"\n") == 0) || strcmp(rowFile,"\r\n") == 0) {
          riga++;
          continue;  //ignora righe vuote
        }

        char * ptr = rowFile;
        for (; *ptr && *ptr != '\n' && *ptr != '\r'; ptr++) {}
        *ptr = 0;   //termina la stringa se trovi '\n' o '\r' nella riga letta

        S_SIZE = strlen(rowFile);
        lowerCaseStr(rowFile);    //trasforma in minuscolo la riga letta dal file

        while (i < S_SIZE) {
            if(isCharInStr(rowFile[i], low_accenti) != -1) sub_str++;

            if (parola[j] == rowFile[i]) {
                j++;
                i++;
            }

            if (j == P_SIZE) {
                sub_tot = sub_str - sub_par;
                // push nuovo nodo in fondo alla lista
                pushOcc(ptpCodaRef, riga + 1, (i - j)+1-sub_tot);
                // aggiorna ptpCodaRef perché faccia riferimento al prossimo nodo
                (*ptpCodaRef)->next = NULL;
                ptpCodaRef = &((*ptpCodaRef)->next);
                tot++;
                j = *(pmt+(j - 1));
            }

            else if (i < S_SIZE && parola[j] != rowFile[i]) {
                if (j != 0) j = *(pmt+(j - 1));
                else i++;
            }
        }
        i = 0;
        j = 0;
        riga++;
        sub_str = 0;
    }
    fclose(file);
    free(pmt);
    pmt = NULL;
    *totOcc = tot;
    return head;
}

int *calcolaPMT(char* p, int P_SIZE) {
    int l_pref = 0;   //lunghezza prefisso precedente
    int * tmp = NULL;

    if (!(tmp = malloc (P_SIZE * sizeof (int)))) {
        printf("\033[1;31m");printf("ERRORE [kmp.c -> calcolaPMT()]:");printf("\033[0m");
        printf(" fallimento allocazione per '%s'\n", p);
        fprintf(stderr, "%s\n", strerror(errno));
        return NULL;
    }

    *tmp = 0; // primo carattere è sempre 0

    int i = 1;  //range indice di riferimento (1 : P_SIZE)
    while (i < P_SIZE) {
      if (p[i] == p[l_pref]) {
          l_pref++;
          *(tmp+i) = l_pref;
          i++;
      }
      else if (l_pref != 0)
          l_pref = *(tmp+(l_pref - 1));
      else {
          *(tmp+i) = 0;
          i++;
      }
    }
    return tmp;
}

int isCharInStr(char ch, const char* str){
    int l = strlen(str);
    int i;
    for (i = 0; i < l; i++)
      if((ch == str[i]) && (i % 2)) return i;
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

void pushOcc(struct Occurrencies** ptpHeadRef, int r, int c) {
    struct Occurrencies* nuovaOcc = malloc(sizeof(struct Occurrencies));
    nuovaOcc->n_row = r;
    nuovaOcc->n_char = c;
    nuovaOcc->next = *ptpHeadRef; // prossimo ptpHeadRef
    *ptpHeadRef = nuovaOcc; // cambio il valore del corrente ptpHeadRef con il nuovo nodo
}

void printOcc(struct Occurrencies* n) {
    while (n != NULL) {
        printf("\t\triga %d posizione %d\n", n->n_row, n->n_char);
        n = n->next;
    }
}

void fprintOcc(FILE** wr, struct Occurrencies* n) {
    while (n != NULL) {
        fprintf(*wr, "%d %d\r\n", n->n_row, n->n_char);
        n = n->next;
    }
}

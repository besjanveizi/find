#include "header.h"

/*****////////////////////*****/
/*****    KMP Algorithm   *****/
/*****////////////////////*****/

const char accenti[] = {"éèòàùì"};

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
        fprintf (stderr, "ERRORE fopen(%s) in kmpInFile(): %s\n", filename,
          strerror(errno));
        exit(EXIT_FAILURE);
    }

    //Longest Prefix Suffix
    int *lps = NULL;
    if(!(lps = calcolaLPS(parola, P_SIZE))) exit(EXIT_FAILURE);

    // calcola sub_par
    for (int k = 0; k < (P_SIZE-1); k++)
        if(isIn(parola[k], accenti)) sub_par++;
    if(sub_par>0){
        if((sub_par%2)==0) sub_par /= 2;
        else {
            sub_par++;
            sub_par /= 2;
        }
    }

    // calcola KMP
    while (fgets (rowFile, MAXC, file)) {

        char * ptr = rowFile;
        for (; *ptr && *ptr != '\n'; ptr++) {} //elimina '\n'
        *ptr = 0;
        S_SIZE = strlen(rowFile);

        if(strcmp(rowFile,"\n") == 0) { //riga vuota
            riga++;
            continue;
        }

        while (i < S_SIZE) {
            if(isIn(rowFile[i], accenti))
                sub_str++;

            if (parola[j] == rowFile[i]) {
                j++;
                i++;
            }

            if (j == P_SIZE) {
                sub_tot = 0;
                sub_tot += (sub_str/2);
                sub_tot -= sub_par;
                // push nuovo nodo in fondo alla lista
                pushOccurr(ptpCoda, riga + 1, (i - j)+1-sub_tot);
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
        fprintf(stderr, "ERRORE malloc() per '%s' in calcolaLPS():\n\t%s\n",
          p, strerror(errno));
        free(tmp);
        tmp = NULL;
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

int isIn(char ch, const char* str){
  int l = strlen(str);
  for (int i = 0; i < l; i++) {
    if(ch == str[i])
      return 1;
  }
  return 0;
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

void pushOccurr(struct Occurrencies** ptpHead, int r, int c) {
    struct Occurrencies* nuovaOcc = malloc(sizeof(struct Occurrencies));
    nuovaOcc->n_row = r;
    nuovaOcc->n_char = c;
    nuovaOcc->next = *ptpHead; // prossimo ptpHead
    *ptpHead = nuovaOcc; // cambio il valore del corrente ptpHead con il nuovo nodo
}

void printList(struct Occurrencies* n) {
    while (n != NULL) {
        printf("\t\t\t\triga %d posizione %d\n", n->n_row, n->n_char);

        n = n->next;
    }
}

void fprintList(FILE* wr, struct Occurrencies* n) {
    while (n != NULL) {
        fprintf(wr, "%d %d\r\n", n->n_row, n->n_char);
        n = n->next;
    }
}

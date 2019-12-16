#include "header.h"

void printInt(int * array, int l)
{
  int i;
  for(i = 0 ; i < l ; i++)
    printf("%d ", array[i]);
  printf("\n");
}

void printString(char * string)
{
  int i;
  for (i = 0; i < strlen(string); i++)
    printf("%c ", string[i]);
  printf("\n");
}

void calcolaLPS(char* p, int N, int **lps)
{
    int len = 0;
    int * tmp = calloc(N,sizeof(int));
    *tmp = 0;

    int i = 1;
    while (i < N) {
      if (p[i] == p[len]) {
          len++;
          *(tmp+i) = len;
          i++;
      }
      else
      {
        if (len != 0)
          len = *(tmp+(len - 1));
        else {
          *(tmp+i) = 0;
          i++;
        }
      }
    }
    free(*lps);
    *lps = tmp;
}

void ricercaKMP(char* parola, char* stringa)
{
    int M = strlen(parola);
    int N = strlen(stringa);

    //Longest Prefix Suffix
    int * lps = calloc(M,sizeof(int));
    calcolaLPS(parola, M, &lps);

    int i = 0; // indice di stringa[]
    int j = 0; // indice di parola[]
    while (i < N) {
        if (parola[j] == stringa[i]) {
            j++;
            i++;
        }

        if (j == M) {
            printf("\nParola trovata in posizione %d\n", i - j);
            j = *(lps+(j - 1));
        }

        else if (i < N && parola[j] != stringa[i]) {
            if (j != 0)
                j = *(lps+(j - 1));
            else
                i = i + 1;
        }
    }
}

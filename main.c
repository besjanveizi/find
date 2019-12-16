#include "header.h"

int main(void)
{
    char parola[] = "aabcadaabe";
    int i, N = strlen(parola);

    //Longest Prefix Suffix
    int * lps = calloc(N,sizeof(int));

    //inizializza lps
     for(i=0; i<N; i++)
         *(lps+i) = 0;

    calcolaLPS(parola, N, &lps);

    printString(parola);
    printInt(lps, N);
    return 0;
}

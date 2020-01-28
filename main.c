#include "header.h"

int main(int argc, char **argv){
    int n = 0;    // dimensione pta_w
    int m = 0;    // dimensione pta_f
    char (*pta_w)[MAXC] = NULL; // puntatore dell'array words di [MAXC] char
    char (*pta_f)[MAXC] = NULL; // puntatore dell'array files di [MAXC] char

    if(!(pta_w = fileToPtA(argv[1], &n))) return EXIT_FAILURE;

    if(!(pta_f = fileToPtA(argv[2], &m))) return EXIT_FAILURE;

    if(!(findPathsPtA(pta_f, m))) {   //parsing dei files in pta_f
      freePtrToArr(pta_f);
      return EXIT_FAILURE;
    }

    //dealloca e reinizializza files
    freePtrToArr(pta_f);
    m = 0;

    //parsed files in appFile
    if(!(pta_f = fileToPtA(appFile, &m))) return EXIT_FAILURE;

    //rimuovi file di appoggio dove c'erano parsed files
    rmFile(appFile);


    //sort PtA
    if(!sortPtA(pta_w, n)){
      printf("ERRORE DUBLICATO in %s!!\n\n", argv[1]);
      freePtrToArr(pta_w);
      return EXIT_FAILURE;
    }
    if(!sortPtA(pta_f, m)){
      printf("ERRORE DUBLICATO in %s!!\n\n", argv[2]);
      freePtrToArr(pta_f);
      return EXIT_FAILURE;
    }

    //stampa PtA ordinati
    printf("\n***Array Ordinati***\n");
    printf("Words:\n");
    printPtA(pta_w, n);
    printf("Files:\n");
    printPtA(pta_f, m);

    //DLA(dynamic linear array) of pointers of n structs Word
    struct Word **w_dla = NULL;
    //crea DLA
    if(!(w_dla = createDS(pta_w, n, pta_f, m))) {
      deallocateDS(w_dla, pta_w, n, pta_f, m);
      return EXIT_FAILURE;
    }

    // stampa DLA
    printDS(w_dla, n, m);

    //dealloca DLA
    deallocateDS(w_dla, pta_w, n, pta_f, m);
    return EXIT_SUCCESS;
}

#include "header.h"

int main(int argc, char **argv){
    int n = 0;    // dimensione pta_w
    int m = 0;    // dimensione pta_f
    char (*pta_w)[MAXC] = NULL; // puntatore dell'array words di [MAXC] char
    char (*pta_f)[MAXC] = NULL; // puntatore dell'array files di [MAXC] char

    if(!(pta_w = fileToPtA(argv[1], &n))) return EXIT_FAILURE;

    if(!(pta_f = fileToPtA(argv[2], &m))) {
      freePtrToArr(pta_w);
      return EXIT_FAILURE;
    }

    if(!(findPathsPtA(pta_f, m))) {   //parsing dei files in pta_f
      freePtrToArr(pta_w);
      freePtrToArr(pta_f);
      return EXIT_FAILURE;
    }

    //dealloca e reinizializza files
    freePtrToArr(pta_f);
    m = 0;

    //parsed files in appFile
    if(!(pta_f = fileToPtA(appFile, &m))) {
      freePtrToArr(pta_w);
      return EXIT_FAILURE;
    }

    //rimuovi file di appoggio dove c'erano parsed files
    rmFile(appFile);

    //DLA(dynamic linear array) of pointers of n structs Word
    struct Word **w_dla = NULL;
    //crea DLA
    if(!(w_dla = createDS(pta_w, n, pta_f, m))) {
      deallocateDS(w_dla, pta_w, n, pta_f, m);
      return EXIT_FAILURE;
    }

    // ordina Data Structure
    if(!ordinaDS(w_dla, n, m)) {
      deallocateDS(w_dla, pta_w, n, pta_f, m);
      return EXIT_FAILURE;
    }

    if(!mkreport(w_dla, n, m)) {
      deallocateDS(w_dla, pta_w, n, pta_f, m);
      return EXIT_FAILURE;
    }

    //dealloca Data Structure
    deallocateDS(w_dla, pta_w, n, pta_f, m);
    return EXIT_SUCCESS;
}

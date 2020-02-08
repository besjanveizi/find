#include "header.h"

/*****/////////////*****/
/*****    Report   *****/
/*****/////////////*****/

static char (*rowFile)[MAXC] = NULL;

static char* searchWord;
static char* searchFile;
static char* searchOcc;
static char* searchTot;

static int l_Wbuf = strlen("WORD");
static int l_Tbuf = strlen("TOTAL");
static int l_Fbuf = strlen("FILE");
static int l_Obuf = strlen("OCCURRENCES");
clock_t begin;
clock_t end;
double time_spent;

const char *appFile = "append.txt";
static char * getSubstr(char *, int);

int generaReport(char *arg_words, char *arg_input, char *arg_output) {
  if(arg_exlude){
    printf("Excluded files with extension:\t");
    printPtA(arg_exlude, dim_arg_exlude);
  }
  int n = 0;    // dimensione pta_w
  int m = 0;    // dimensione pta_f
  char (*pta_w)[MAXC] = NULL; // puntatore dell'array words di [MAXC] char
  char (*pta_f)[MAXC] = NULL; // puntatore dell'array files di [MAXC] char

  if(!(pta_w = fileToPtA(arg_words, &n))) return 0;
  printf("Words filepath: '%s'\n", arg_words);

  for (int i = 0; i < n; i++)
    lowerCaseStr(pta_w[i]);   // trasforma ogni parola in minuscolo

  if(!(pta_f = fileToPtA(arg_input, &m))) {
    freePtA(pta_w);
    return 0;
  }
  printf("Input filepath: '%s'\n", arg_input);
  begin = clock();
  //printPtA(pta_f, m);
  if(!(findPathsPtA(pta_f, m))) {   //parsing dei files in pta_f
    freePtA(pta_w);
    freePtA(pta_f);
    return 0;
  }

  //dealloca e reinizializza files
  freePtA(pta_f);
  m = 0;

  //parsed files in appFile
  if(!(pta_f = fileToPtA(appFile, &m))) {
    freePtA(pta_w);
    return 0;
  }

  //rimuovi file di appoggio dove c'erano parsed files
  rmFile(appFile);
  end = clock();
  time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  if(verbose_flag) printf(" -- Visitati tutti i file specificati in '%s' (%lf)\n", arg_input, time_spent);

  //DLA(dynamic linear array) of pointers of n structs Word
  struct Word **w_dla = NULL;
  //crea DLA
  if(!(w_dla = createDS(pta_w, n, pta_f, m))) {
    deallocateDS(w_dla, pta_w, n, pta_f, m);
    return 0;
  }
  // ordina Data Structure
  begin = clock();
  if(!ordinaDS(w_dla, n, m)) {
    deallocateDS(w_dla, pta_w, n, pta_f, m);
    return EXIT_FAILURE;
  }
  end = clock();
  time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  if(verbose_flag) printf(" -- Data Structure Ordinata (%lf)\n", time_spent);

  if(arg_output) {
    char outputFile[1024];
    if(!isAbsolute(arg_output)) strcpy(outputFile, getAbsolute(arg_output, currentDir));
    if(outputFile[0] == '\0') return EXIT_FAILURE;
    printf("Output filepath: '%s'\n", outputFile);
    if(!mkreport(w_dla, n, m, outputFile)) {
      deallocateDS(w_dla, pta_w, n, pta_f, m);
      return 0;
    }
    printf("[ REPORT GENERATO ]\n");
  }

  //printDS(w_dla, n, m);

  //dealloca Data Structure
  deallocateDS(w_dla, pta_w, n, pta_f, m);
  return 1;
}

int analisiListPaths(char *reportFile, char *parola, int oInput) {
    char *end;
    int i = 0; // indice per righe "WORD" e "FILE"
    int j = 0; // indice per righe "OCCURRENCES"
    int n_rows = 0;
    int countOcc = 0;
    int totOcc;
    bool occPresente = false;

    if(!(rowFile = fileToPtA(reportFile, &n_rows))) return 0;

    lowerCaseStr(parola);   // trasforma parola in minuscolo

    for(; i < n_rows; i++) { // ciclo per trovare 'parola'
        if(strncmp(rowFile[i], "WORD", l_Wbuf) != 0) continue; // skip riga se on comincia con "WORD"
        else {
            searchWord = getSubstr(rowFile[i] ,l_Wbuf); // get 'parola' from rowFile[i]
            if (strcmp(searchWord, parola)==0) {
                //break; trovato parola
                j = i + 1; // check "TOTAL"
                searchTot = getSubstr(rowFile[j], l_Tbuf);
                // converti searchTot in int base 10
                totOcc = strtol(searchTot, &end, 10);
                if(totOcc == 0) {
                    printf("Non c'è alcuna occorrenza della parola '%s' tra i file specificati\n",
                      searchWord);
                    free(rowFile);
                    rowFile = NULL;
                    return 1; // "TOTAL" 0
                }
                j++; // check "FILES" of parola
                printf("La parola '%s' occorre in totale %d volte:\n", searchWord, totOcc);
                for(; j < n_rows && countOcc < totOcc; j++) {
                    searchFile = getSubstr(rowFile[j], l_Fbuf);
                    j++; // la riga dopo "FILE" è *sempre* "OCCURRENCES"
                    searchOcc = getSubstr(rowFile[j], l_Obuf); // get occorrenza from rowFile[j]
                    int oFile = strtol(searchOcc, &end, 10); // converti searchOcc in int base 10
                    if(oFile >= oInput) {
                      printf("\tpath: '%s'\tOCC: %d\n", searchFile, oFile); //controllo occorrenza
                      occPresente = true;
                    }
                    countOcc += oFile;
                    j = j + oFile; //skip riga "OCCURRENCES"

                }
                if(!occPresente) printf("Tuttavia non ci sono occorrenze pari/maggiori di '%d' tra i file specificati\n",
                  oInput);
                free(rowFile);
                rowFile = NULL;
                return 1; // ho stampato tutte le occorrenze per ciascun path per parola
            }
        }
    }
    printf("La parola '%s' non era presente nel file delle parole, perciò non si trova nel report\n", parola);
    free(rowFile);
    rowFile = NULL;
    return 1;
}

int analisiListOcc(char *reportFile, char * parola, char *absPath) {
  char *end;
  int i = 0; // indice per righe "WORD"
  int j = 0; // indice per righe "FILE"
  int k=0; // indice per righe "OCCURRENCES"
  int countOcc; // #occorrenze di parola in absPath
  int skipOcc; // #occorrenze da saltare per controllare prossimo "FILE"
  int n_rows = 0;

  if(!(rowFile = fileToPtA(reportFile, &n_rows))) return 0;

  lowerCaseStr(parola);   // trasforma parola in minuscolo

  for(; i < n_rows; i++) {
      if(strncmp(rowFile[i], "WORD", l_Wbuf) != 0) continue;
      else {
          searchWord = getSubstr(rowFile[i], l_Wbuf);
          if (strcmp(searchWord, parola)==0) { //trovato la parola
              j = i + 1; // check "TOTAL"
              searchTot = getSubstr(rowFile[j], l_Tbuf);
              // converti searchTot in int base 10
              int tot = strtol(searchTot, &end, 10);
              if(tot == 0) {
                  printf("Non c'è alcuna occorrenza della parola '%s' tra i file specificati\n",
                    searchWord);
                  free(rowFile);
                  rowFile = NULL;
                  return 1; // "TOTAL" 0
              }
              j++; // check "FILES" of parola
              for(; j<n_rows; j++) {
                  if((strncmp(rowFile[j], "WORD", l_Wbuf) == 0)) break;

                  searchFile = getSubstr(rowFile[j], l_Fbuf);
                  if(strcmp(searchFile, absPath) == 0) {
                      //trovato absPath
                      k=j+1;
                      searchOcc = getSubstr(rowFile[k], l_Obuf);
                      countOcc = strtol(searchOcc, &end, 10);
                      k++;
                      printf("La parola '%s' si trova %d volte nel file %s nelle seguenti posizioni:\n",
                        searchWord, countOcc, searchFile);
                      while(countOcc) {
                          printf("%s\n", rowFile[k]);
                          k++;
                          countOcc--;
                      }
                      free(rowFile);
                      rowFile = NULL;
                      return 1;
                  }
                  // not the file, skip occurrences
                  j++;
                  searchOcc = getSubstr(rowFile[j], l_Obuf);
                  skipOcc = strtol(searchOcc, &end, 10);
                  j += skipOcc;
              }
              printf("Il path '%s' non era presente nel file dei percorsi, perciò non si trova nel report\n",
                absPath);
              free(rowFile);
              rowFile = NULL;
              return 1; //path non trovato, errore in absPath
          }
      }
  }
  printf("La parola '%s' non era presente nel file delle parole, perciò non si trova nel report\n", parola);
  free(rowFile);
  rowFile = NULL;
  return 1;
}

char * getSubstr(char * str, int buffer) {
    // remove '\r'
    char * ptr = str;
    for (; *ptr && *ptr != '\r'; ptr++) {}
    *ptr = 0;
    //get substring after buffer + ' '
    return str + (buffer+1);
}

void help() {

	printf("\n\t\t\t\t\t      [   H   E   L   P   ]\n\n\n");

	printf("\033[0;34m");
	printf("Elenco delle opzioni [ -brevi ] | [ --complete ]\n\n");
	printf("\033[0m");

	printf("Opzioni che");
	printf("\033[01;33m"); printf(" NON RICHIEDONO "); printf("\033[0m");
	printf("<argomento>:\n\n");

	printf("[ -h ] | [ --help ]                           : stampa questa schermata di aiuto\n\n");

	printf("Opzioni che");
	printf("\033[01;33m"); printf(" RICHIEDONO "); printf("\033[0m");
	printf("<argomento>:\n\n");

	printf("[ -w <filepath>   ] | [ --words   <filepath>   ]  : <filepath> è il file per le parole\n");
	printf("[ -i <filepath>   ] | [ --input   <filepath>   ]  : <filepath> è il file per i path\n");
	printf("[ -o <filepath>   ] | [ --output  <filepath>   ]  : <filepath> è il file di output del report\n");
	printf("[ -e <ext1> <ext2>] | [ --exclude <ext1> <ext2>]  : <ext1> <ext2> sono le estensioni dei file da escludere\n");
	printf("[ -r <filepath>   ] | [ --report  <filepath>   ]  : <filepath> è il file considerato per l'analisi del report\n");
  printf("[ -s <word> <n>   ] | [ --show    <word> <n>   ]  : seleziona <word> che occorre <n> volte (se omesso, n = 1)\n");
  printf("[ -f <filepath>   ] | [ --file    <filepath>   ]  : seleziona <word> che occorre nel file <filepath>\n");
  printf("\nFLAG: ");
  printf("[ -v ] | [ --verbose ]  : flag verbose\n\n");
  printf("\033[0;34m"); printf("Esempi di utilizzo dei comandi:\n\n"); printf("\033[0m");
  printf("1) Genera il report <out.txt> nella cartella ./result dalle parole contenute in <words.txt> sui file specificati\n");
  printf("   con path relativo o assoluto nel file <in.txt> escludendo file .pdf, .epub e .doc.\n");
  printf("   Mostra estensivamente il processo di analisi.\n");
  printf("\033[0;32m");
	printf("\t ./find --words words.txt --input ./in.txt --output result/out.txt --exclude pdf epub doc -verbose\n\n");
  printf("\033[0m");
  printf("2) Analizza il report contenuto nel file <out.txt> della cartella ./result, stampando la lista dei file dove\n");
  printf("   occorre almeno <3> volte la parola <aria>\n");
  printf("\033[0;32m");
	printf("\t ./find --report ./result/out.txt --show aria 3\n\n");
  printf("\033[0m");
  printf("3) Analizza il report contenuto nel file <out.txt> della cartella ./result, stampando tutte le posizioni\n");
  printf("   dove la parola <aria> occorre nel file <5maggio.txt> contenuto nella cartella ./files\n");
  printf("\033[0;32m");
  printf("\t ./find --report ./result/out.txt --show aria --file files/5maggio.txt\n");
  printf("\033[0m");
  printf("\n\nN.B.\n");
  printf("L'argomento <filepath> può essere sia relativo che assoluto.\n\n");
  exit(0);
}

void crediti() {
  printf("\033[01;36m");
	printf("\n\n\t\t\t\t\t  FIND PROGRAM\n\n"); printf("\033[0m");
	printf("- membri del gruppo :  Besjan Veizi, Mariano Sabatino\n");
	printf("\n");
	printf("- breve descrizione:\n");
	printf("\t\t   FIND e' un programma in grado di individuare il numero\n");
  printf("\t\t   di occorrenze di un insieme di stringhe all'interno di\n");
  printf("\t\t   un gruppo di file utilizzando l'Algoritmo di Knuth-Morris e Pratt.\n");
  printf("\t\t   Ad ogni esecuzione l'applicazione produrrà in output la lista\n");
  printf("\t\t   dei file analizzati con le occorrenze della stringa nel testo\n");
  printf("\t\t   insieme alle informazioni riguardante la posizioni della stesse.\n");
  printf("\t\t   Le stesse informazioni prodotte in output potranno essere salvate\n");
  printf("\t\t   su di un file esterno su cui si possono eseguire 2 operazioni di analisi:\n");
  printf("\t\t     - Stampare la lista dei file dove occorre almeno <n> volte la parola <word>\n");
  printf("\t\t     - Stampare tutte le posizioni dove la parola <word> occorre nel file <filepath>\n\n");
  printf("- digita"); printf("\033[01;33m"); printf("  ./find -h  "); printf("\033[0m");
  printf("per maggiori informazioni sui comandi.\n");
	exit(0);
}

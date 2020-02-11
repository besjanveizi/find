#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>       // occPresente
#include <time.h>
#include "find.h"          // menù arguments
#include "report.h"
#include "dataStructure.h" // MAXC, isAbsolute(), getAbsolute(), fileToPtA(), freePtA(),
                           // findPathsPtA(), fprintDS(), ordinaDS(), deallocateDS()
#include "kmp.h"           // lowerCaseStr()

static char wordsFile[MAXC];
static char inputFile[MAXC];
static char reportFile[MAXC];
static char selectedFile[MAXC];

static char (*rowFile)[MAXC];

static char* searchWord;
static char* searchFile;
static char* searchOcc;
static char* searchTot;

static int l_Wbuf = strlen("WORD");
static int l_Tbuf = strlen("TOTAL");
static int l_Fbuf = strlen("FILE");
static int l_Obuf = strlen("OCCURRENCES");

const char *appFile = "append.txt";
static char * getSubstr(char *, int);

int generaReport() {
  clock_t time_begin;
  clock_t time_end;
  double time_spent;

  int n = 0;    // dimensione pta_w
  int m = 0;    // dimensione pta_f
  char (*pta_w)[MAXC] = NULL; // puntatore di array di [MAXC] char per parole
  char (*pta_f)[MAXC] = NULL; // puntatore di array di [MAXC] char per files

  if(!isAbsolute(arg_words))
    strcpy(wordsFile, getAbsolute(arg_words));
  if(wordsFile[0] == '\0') {
    printf("-w | --words argument failure\n");
    return 0;
  }

  if(!isAbsolute(arg_input))
    strcpy(inputFile, getAbsolute(arg_input));
  if(inputFile[0] == '\0') {
    printf("-i | --input argument failure\n");
    return 0;
  }


  if(!(pta_w = fileToPtA(wordsFile, &n))) return 0;
  printf("Words filepath: '%s'\n", wordsFile);

  if(!(pta_f = fileToPtA(inputFile, &m))) {
    freePtA(pta_w);
    return 0;
  }
  printf("Input filepath: '%s'\n", inputFile);
  time_begin = clock();
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
  time_end = clock();
  time_spent = (double)(time_end - time_begin) / CLOCKS_PER_SEC;
  if(verbose_flag) printf(" -- Visitati tutti i file specificati in '%s' (%lf)\n", inputFile, time_spent);

  //DLA(dynamic linear array) of pointers of n structs Word
  struct Word **w_dla = NULL;
  //crea DLA
  if(!(w_dla = createDS(pta_w, n, pta_f, m))) {
    deallocateDS(w_dla, pta_w, n, pta_f, m);
    return 0;
  }
  // ordina Data Structure
  time_begin = clock();
  if(!ordinaDS(w_dla, n, m)) {
    deallocateDS(w_dla, pta_w, n, pta_f, m);
    return 0;
  }
  time_end = clock();
  time_spent = (double)(time_end - time_begin) / CLOCKS_PER_SEC;
  if(verbose_flag) printf(" -- Data Structure Ordinata (%lf)\n", time_spent);

  if(arg_output) {
    char outputFile[MAXC];
    if(!isAbsolute(arg_output)) strcpy(outputFile, getAbsolute(arg_output));
    if(outputFile[0] == '\0') return 0;
    printf("Output filepath: '%s'\n", outputFile);
    if(!fprintDS(w_dla, n, m, outputFile)) {
      deallocateDS(w_dla, pta_w, n, pta_f, m);
      return 0;
    }
    printf("[ REPORT GENERATO ]\n");
  }

  // printDS(w_dla, n, m);   // stampa DS su terminale

  //dealloca Data Structure
  deallocateDS(w_dla, pta_w, n, pta_f, m);
  return 1;
}

int analisiListPaths() {
    char *end;
    int i = 0; // indice per righe "WORD" e "FILE"
    int j = 0; // indice per righe "OCCURRENCES"
    int n_rows = 0;
    int countOcc = 0;
    int totOcc;
    bool occPresente = false;

    if(!isAbsolute(arg_report))
      strcpy(reportFile, getAbsolute(arg_report));
    if(reportFile[0] == '\0') {
      printf("-r | --report argument failure\n");
      return 0;
    }

    if(!(rowFile = fileToPtA(reportFile, &n_rows))) return 0;
    printf("Report filepath: '%s'\n", reportFile);

    lowerCaseStr(arg_show);   // trasforma parola in minuscolo

    for(; i < n_rows; i++) { // ciclo per trovare 'parola'
        if(strncmp(rowFile[i], "WORD", l_Wbuf) != 0) continue; // skip riga se on comincia con "WORD"
        else {
            searchWord = getSubstr(rowFile[i] ,l_Wbuf); // get 'parola' from rowFile[i]
            if (strcmp(searchWord, arg_show)==0) {
                // trovato parola
                j = i + 1; // check "TOTAL"
                searchTot = getSubstr(rowFile[j], l_Tbuf);
                // converti searchTot in int base 10
                totOcc = strtol(searchTot, &end, 10);
                if(totOcc == 0) {
                    printf("Non c'è alcuna occorrenza della parola '%s' tra i file specificati\n",
                      searchWord);
                    freePtA(rowFile);
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
                freePtA(rowFile);
                return 1; // ho stampato tutte le occorrenze per ciascun path per parola
            }
        }
    }
    printf("La parola '%s' non era presente nel file delle parole, perciò non si trova nel report\n", arg_show);
    freePtA(rowFile);
    return 1;
}

int analisiListOcc() {
  char *end;
  int i = 0; // indice per righe "WORD"
  int j = 0; // indice per righe "FILE"
  int k=0; // indice per righe "OCCURRENCES"
  int countOcc; // #occorrenze di parola in selectedFile
  int skipOcc; // #occorrenze da saltare per controllare prossimo "FILE"
  int n_rows = 0;

  if(!isAbsolute(arg_report))
    strcpy(reportFile, getAbsolute(arg_report));
  if(reportFile[0] == '\0') {
    printf("-r | --report argument failure\n");
    return 0;
  }

  if(!isAbsolute(arg_file))
    strcpy(selectedFile, getAbsolute(arg_file));
  if(selectedFile[0] == '\0') {
    printf("-f | --file argument failure\n");
    return 0;
  }

  if(!(rowFile = fileToPtA(reportFile, &n_rows))) return 0;
  printf("Report filepath: '%s'\n", reportFile);

  lowerCaseStr(arg_show);   // trasforma parola in minuscolo

  for(; i < n_rows; i++) {
      if(strncmp(rowFile[i], "WORD", l_Wbuf) != 0) continue;
      else {
          searchWord = getSubstr(rowFile[i], l_Wbuf);
          if (strcmp(searchWord, arg_show)==0) { //trovato la parola
              j = i + 1; // check "TOTAL"
              searchTot = getSubstr(rowFile[j], l_Tbuf);
              // converti searchTot in int base 10
              int tot = strtol(searchTot, &end, 10);
              if(tot == 0) {
                  printf("Non c'è alcuna occorrenza della parola '%s' tra i file specificati\n",
                    searchWord);
                  freePtA(rowFile);
                  return 1; // "TOTAL" 0
              }
              j++; // check "FILES" of parola
              for(; j<n_rows; j++) {
                  if((strncmp(rowFile[j], "WORD", l_Wbuf) == 0)) break;

                  searchFile = getSubstr(rowFile[j], l_Fbuf);
                  if(strcmp(searchFile, selectedFile) == 0) {
                      //trovato selectedFile
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
                      freePtA(rowFile);
                      return 1;
                  }
                  // not the file, skip occurrences
                  j++;
                  searchOcc = getSubstr(rowFile[j], l_Obuf);
                  skipOcc = strtol(searchOcc, &end, 10);
                  j += skipOcc;
              }
              printf("Il path '%s' non era presente nel file dei percorsi, perciò non si trova nel report\n",
                selectedFile);
              freePtA(rowFile);
              return 1; //path non trovato, errore in selectedFile
          }
      }
  }
  printf("La parola '%s' non era presente nel file delle parole, perciò non si trova nel report\n", arg_show);
  freePtA(rowFile);
  return 1;
}

void rmFile(const char * filename) {
    char cmd[120];
    strcpy(cmd, "rm ");
    strcat(cmd, filename);
    system(cmd);
}

char * getSubstr(char * str, int buffer) {
    // remove '\r'
    char * ptr = str;
    for (; *ptr && *ptr != '\r'; ptr++) {}
    *ptr = 0;
    //get substring after buffer + ' '
    return str + (buffer+1);
}

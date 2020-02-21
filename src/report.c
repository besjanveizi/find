#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>          // occPresente
#include <time.h>
#include "find.h"             // menù arguments
#include "report.h"
#include "dataStructure.h"    // createDS(), fprintDS(), ordinaDS(), deallocateDS(), duplicato
#include "wf_getter.h"        // tutte le funzioni esterne ed appFile
#include "kmp.h"              // lowerCaseStr()

// dimensione minima di lettura di una linea da un file (1 carattere)
static int max_char = 1;

// funzione per ottenere percorso assoluto degli argomenti <filepath> nei comandi di input dal modulo find
static char * checkNgetAbs(char *);

// percorsi assoluti degli argomenti <filepath> nei comandi di input dal modulo find
static char *wordsFile = NULL;
static char *inputFile = NULL;
static char *outputFile = NULL;
static char *reportFile = NULL;
static char *selectedFile = NULL;

// report file ptp
static char **rowFile;

//funzione per ottenere i valori nel report file
static char * getSubstr(char *, int);

//valori nel report file
static char* searchWord; // riga "WORD ..."
static char* searchTot;  // riga "TOTAL ..."
static char* searchFile; // riga "FILE ..."
static char* searchOcc;  // riga "OCCURRENCES ..."

// bytes buffer per strncmp() nell ANALISI
static int l_Wbuf = strlen("WORD");
static int l_Tbuf = strlen("TOTAL");
static int l_Fbuf = strlen("FILE");
static int l_Obuf = strlen("OCCURRENCES");

int generaReport() {
  clock_t time_begin;
  clock_t time_end;
  double time_spent;

  int n = 0;    // dimensione ptp_w
  int m = 0;    // dimensione ptp_f
  char **ptp_w = NULL; // puntatore di array di char [MAXC] per parole
  char **ptp_f = NULL; // puntatore di array di char [MAXC] per i percorsi dei files

  wordsFile = checkNgetAbs(arg_words);
  if(!wordsFile) {
    printf("-w | --words argument failure\n");
    return 0;
  }

  inputFile = checkNgetAbs(arg_input);
  if(!inputFile) {
    printf("-i | --input argument failure\n");
    free(wordsFile);
    return 0;
  }

  if(!(ptp_w = fileToPtP(wordsFile, &n, &max_char))) {
    freePtP(ptp_w, n);
    free(wordsFile);
    free(inputFile);
    return 0;
  }
  printf("Words filepath: '%s'\n", wordsFile);
  int maxc_w = max_char;

  if(!(ptp_f = fileToPtP(inputFile, &m, &max_char))) {
    freePtP(ptp_w, n);
    freePtP(ptp_f, m);
    free(wordsFile);
    free(inputFile);
    return 0;
  }
  printf("Input filepath: '%s'\n", inputFile);

  if(verbose_flag) printf("  Lista completa dei file:\n");
  if(arg_exclude){
    printf("* Esclusi i file con estensione:  ");
    printPtP(arg_exclude, dim_arg_exclude);
  }
  time_begin = clock();
  if(!(findPathsPtP(ptp_f, m))) {   //parsing dei files in ptp_f
    freePtP(ptp_w, n);
    freePtP(ptp_f, m);
    free(wordsFile);
    free(inputFile);
    rm_appFile();
    return 0;
  }

  //dealloca e reinizializza files
  freePtP(ptp_f, m);
  m = 0;

  //parsed files in appFile
  if(!(ptp_f = fileToPtP(appFile, &m, &max_char))) {
    freePtP(ptp_w, n);
    freePtP(ptp_f, m);
    free(wordsFile);
    free(inputFile);
    rm_appFile();
    return 0;
  }
  int maxc_f = max_char;

  //rimuovi file di appoggio dove c'erano parsed files
  rm_appFile();
  time_end = clock();
  time_spent = (double)(time_end - time_begin) / CLOCKS_PER_SEC;
  if(verbose_flag)
    printf(" -- Visitati tutti i file specificati in '%s' (%lf)\n", inputFile, time_spent);

  //DLA(dynamic linear array) of pointers of n structs Word
  struct Word **w_da = NULL;
  //crea DLA
  if(!(w_da = createDS(ptp_w, n, ptp_f, m, maxc_w, maxc_f))) {
    free(wordsFile);
    free(inputFile);
    deallocateDS(w_da, ptp_w, n, ptp_f, m);
    return 0;
  }
  // ordina Data Structure
  time_begin = clock();
  ordinaDS(w_da, n, m);
  if(duplicato) {  //controlla se ci sono elementi duplicati tra le parole o i percorsi
    free(wordsFile);
    free(inputFile);
    deallocateDS(w_da, ptp_w, n, ptp_f, m);
    return 0;
  }
  time_end = clock();
  time_spent = (double)(time_end - time_begin) / CLOCKS_PER_SEC;
  if(verbose_flag) printf(" -- Data Structure Ordinata (%lf)\n", time_spent);
  if(arg_output) {
    outputFile = checkNgetAbs(arg_output);
    if(!outputFile) {
      printf("-o | --output argument failure\n");
      free(wordsFile);
      free(inputFile);
      deallocateDS(w_da, ptp_w, n, ptp_f, m);
      return 0;
    }
    FILE* oF = fopen (outputFile, "w+");    //file di output per report
    if(!oF) {
        printf("\033[1;31m");printf("ERRORE [report.c -> createDS()]:");printf("\033[0m");
        printf(" fallimento fopen(%s)\n", outputFile);
        fprintf (stderr, "%s\n", strerror(errno));
        free(wordsFile);
        free(inputFile);
        free(outputFile);
        deallocateDS(w_da, ptp_w, n, ptp_f, m);
        return 0;
    }
    printf("Output filepath: '%s'\n", outputFile);
    fprintDS(w_da, n, m, &oF);
    fclose(oF);
    printf("\033[0;32m");printf("[ REPORT GENERATO ]\n");printf("\033[0m");
  }

  // if(verbose_flag) printDS(w_da, n, m);   // stampa DS su terminale

  free(wordsFile);
  free(inputFile);
  free(outputFile);

  //dealloca Data Structure
  deallocateDS(w_da, ptp_w, n, ptp_f, m);
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

    reportFile = checkNgetAbs(arg_report);
    if(!reportFile) {
      printf("-r | --report argument failure\n");
      return 0;
    }

    if(!(rowFile = fileToPtP(reportFile, &n_rows, &max_char))) {
      free(reportFile);
      return 0;
    }
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
                    freePtP(rowFile, n_rows);
                    free(reportFile);
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
                freePtP(rowFile, n_rows);
                free(reportFile);
                return 1; // ho stampato tutte le occorrenze per ciascun path per parola
            }
        }
    }
    printf("La parola '%s' non era presente nel file delle parole, perciò non si trova nel report\n", arg_show);
    freePtP(rowFile, n_rows);
    free(reportFile);
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

  reportFile = checkNgetAbs(arg_report);
  if(!reportFile) {
    printf("-r | --report argument failure\n");
    return 0;
  }

  selectedFile = checkNgetAbs(arg_file);
  if(!selectedFile) {
    printf("-f | --file argument failure\n");
    free(reportFile);
    return 0;
  }

  if(!(rowFile = fileToPtP(reportFile, &n_rows, &max_char))) {
    free(reportFile);
    free(selectedFile);
    return 0;
  }
  printf("Report filepath: '%s'\n", reportFile);
  printf("Selected file: '%s'\n", selectedFile);

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
                  freePtP(rowFile, n_rows);
                  free(reportFile);
                  free(selectedFile);
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
                      freePtP(rowFile, n_rows);
                      free(reportFile);
                      free(selectedFile);
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
              freePtP(rowFile, n_rows);
              free(reportFile);
              free(selectedFile);
              return 1; //path non trovato, errore in selectedFile
          }
      }
  }
  printf("La parola '%s' non era presente nel file delle parole, perciò non si trova nel report\n", arg_show);
  freePtP(rowFile, n_rows);
  free(reportFile);
  free(selectedFile);
  return 1;
}

char * checkNgetAbs(char * str) {
  char * abs = NULL;
  int optimal_size = 1 + strlen(str);
  if(!isAbsolute(str)) {
      optimal_size += size_cwd;
      abs = getAbsolute(str, optimal_size);
      if(abs[0] == '\0') {
          free(abs);
          return NULL;
      }
  }
  else {
    abs = malloc(optimal_size);
    strcpy(abs, str);   // isAbsolute()
  }
  return abs;
}

char * getSubstr(char * str, int buffer) {
    // remove '\r'
    char * ptr = str;
    for (; *ptr && *ptr != '\r'; ptr++) {}
    *ptr = 0;
    //get substring after buffer + ' '
    return str + (buffer+1);
}

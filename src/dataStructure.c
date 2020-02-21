#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>  // duplicato
#include <time.h>
#include "find.h"   // verbose_flag
#include "dataStructure.h"
#include "wf_getter.h"    //freePtP()
#include "kmp.h"    //kmpInFile(), struct Occurrencies, fprintOcc(), freeList()

// variabile di controllo di un duplicato tra le parole o tra i files
bool duplicato = false;

// funzione comparatore per l'ordinamento alfabetico delle parole
static int compareWords(const void *, const void *);
// funzione comparatore per l'ordinamento decrescente delle occorrenze
// (alfabetico dei file in caso di uguale occorrenza)
static int compareFiles(const void *, const void *);

struct Word ** createDS(char **ptp_w, int n, char **ptp_f, int m, int maxc_w, int maxc_f) {

  int i, j;
  clock_t time_begin;
  clock_t time_end;
  double time_spent;
  double tot_time = 0;

  //array dinamico di strutture Word dinamicamente allocate
  struct Word **w_da = malloc(n * sizeof(struct Word *));
  if(!w_da){
    printf("\033[1;31m");printf("ERRORE [dataStructure.c -> createDS()]:");printf("\033[0m");
    printf(" fallimento allocazione w_da\n");
    fprintf(stderr, "%s\n", strerror(errno));
    return NULL;
  }

  //alloca n struct Word, ciascun w_da[i] punta ad uno di esse
  for(i = 0; i < n; i++){
    w_da[i] = malloc(sizeof(struct Word));
    if(!(w_da[i])){
      printf("\033[1;31m");printf("ERRORE [dataStructure.c -> createDS()]:");printf("\033[0m");
      printf(" fallimento allocazione w_da[%d]\n", i);
      fprintf(stderr, "%s\n", strerror(errno));
      return NULL;
    }
    //array dinamico di strutture File dinamicamente allocate
    w_da[i]->f_da  = malloc(m * sizeof(struct File *));
    if(!(w_da[i]->f_da)){
      printf("\033[1;31m");printf("ERRORE [dataStructure.c -> createDS()]:");printf("\033[0m");
      printf(" fallimento allocazione w_da[%d]->f_da\n", i);
      fprintf(stderr, "%s\n", strerror(errno));
      return NULL;
    }
    w_da[i]->word = malloc(maxc_w);
    strcpy(w_da[i]->word, ptp_w[i]);
    w_da[i]->tot_occurr = 0;
    time_begin = clock();
    if(verbose_flag) printf(" -- Ricerca occorrenze della parola '%s'\n", w_da[i]->word);

    //alloca m struct File, ciascun w_da[i]->f_da[j] punta ad uno di esse
    for (j = 0; j < m; j++) {
      w_da[i]->f_da[j] = malloc(sizeof(struct File));
      if(!(w_da[i]->f_da[j])){
        printf("\033[1;31m");printf("ERRORE [dataStructure.c -> createDS()]:");printf("\033[0m");
        printf(" fallimento allocazione w_da[%d]->f_da[%d]\n", i, j);
        fprintf(stderr, "%s\n", strerror(errno));
        return NULL;
      }

      w_da[i]->f_da[j]->path = malloc(maxc_f);
      strcpy(w_da[i]->f_da[j]->path, ptp_f[j]);
      w_da[i]->f_da[j]->head = kmpInFile(w_da[i]->word, w_da[i]->f_da[j]->path, &w_da[i]->f_da[j]->occurr);
      // if(!(w_da[i]->f_da[j]->head)){
      //   fprintf(stderr, "ERRORE kmpInFile(w_da[%d]->f_da[%d]->head):\n\t%s\n",i ,j , strerror(errno));
      //   return NULL;
      // }

      time_end = clock();
      time_spent = (double)(time_end - time_begin) / CLOCKS_PER_SEC;
      if(verbose_flag) printf("\tsul file '%s' (%lf)\n", w_da[i]->f_da[j]->path, time_spent);
      w_da[i]->tot_occurr += w_da[i]->f_da[j]->occurr;
      tot_time += time_spent;
    }
  }
  if(verbose_flag) printf(" -- Data Structure Creata (%lf)\n", tot_time);
  return w_da;
}

void fprintDS(struct Word ** w_da, int n, int m, FILE** oF) {
    int i, j;
    for (i = 0; i < n; i++) {
        fprintf(*oF, "WORD %s\r\n", w_da[i]->word);
        fprintf(*oF, "TOTAL %d\r\n", w_da[i]->tot_occurr);
        for (j = 0; j < m; j++) {
            fprintf(*oF, "FILE %s\r\n", w_da[i]->f_da[j]->path);
            fprintf(*oF, "OCCURRENCES %d\r\n", w_da[i]->f_da[j]->occurr);
            fprintOcc(oF, w_da[i]->f_da[j]->head);
        }
    }
    fprintf(*oF, "\r\n");
}

void printDS(struct Word ** w_da, int n, int m) {
    int i, j;
    printf("\n\n***OUTPUT***\n");
    for (i = 0; i < n; i++){
        printf("WORD %s\n", w_da[i]->word);
        printf("TOTAL %d\n", w_da[i]->tot_occurr);
        for (j = 0; j < m; j++){
            printf(" ->\tFILE %s\r\n", w_da[i]->f_da[j]->path);
            printf("   \tOCCURRENCES %d\r\n", w_da[i]->f_da[j]->occurr);
            printOcc(w_da[i]->f_da[j]->head);
      }
      printf("-------------------------------------------------\n");
    }
}

void deallocateDS(struct Word ** w_da, char **ptp_w, int n, char **ptp_f, int m) {
    int i ,j;
    //memory management ptp_w e ptp_f
    freePtP(ptp_w, n);
    freePtP(ptp_f, m);
    //memory management data structure
    if(w_da) {
        for (i = 0; i < n; i++) {
            for (j = 0; j < m; j++){
                if((w_da[i]->f_da[j]->head) != NULL) freeList(w_da[i]->f_da[j]->head);
                if((w_da[i]->f_da[j]->path) != NULL) {
                  free(w_da[i]->f_da[j]->path);
                  w_da[i]->f_da[j]->path = NULL;

                }
                if((w_da[i]->f_da[j]) != NULL) {
                    free(w_da[i]->f_da[j]);
                    w_da[i]->f_da[j] = NULL;
                }
            }
            if((w_da[i]->f_da) != NULL){
                free(w_da[i]->f_da);
                w_da[i]->f_da = NULL;
            }
            if((w_da[i]->word) != NULL) {
              free(w_da[i]->word);
              w_da[i]->word = NULL;

            }

            if((w_da[i]) != NULL) {
                free(w_da[i]);
                w_da[i] = NULL;
            }
        }
        free(w_da);
        w_da = NULL;
    }
}

void ordinaDS(struct Word ** w_da, int n, int m) {
  qsort(w_da, n, sizeof(struct Word *), compareWords);
  if(duplicato) return;
  for (int i = 0; i < n; i++){
      qsort(w_da[i]->f_da, m, sizeof(struct File *), compareFiles);
      if(duplicato) return;
  }
}

int compareWords(const void *s1, const void *s2) {
  struct Word **e1 = (struct Word **) s1;
  struct Word **e2 = (struct Word **) s2;
  int result = strcmp((*e1)->word, (*e2)->word);
  if(result==0) {
      printf("\033[1;31m");printf("ERRORE:");printf("\033[0m");
      printf(" duplicato '%s' tra le parole!!\n\n",(*e1)->word);
      duplicato = true;
  }
  return result;
}

int compareFiles(const void *s1, const void *s2) {
  struct File **e1 = (struct File **) s1;
  struct File **e2 = (struct File **) s2;
  int result = strcmp((*e1)->path, (*e2)->path);
  if(result==0) {
      printf("\033[1;31m");printf("ERRORE:");printf("\033[0m");
      printf(" duplicato '%s' tra i files!!\n\n", (*e1)->path);
      duplicato = true;
  }
  if((*e1)->occurr < (*e2)->occurr) return +1;
  else if((*e1)->occurr > (*e2)->occurr) return -1;
  else return result;
}

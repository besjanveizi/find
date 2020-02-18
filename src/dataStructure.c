#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>  // dublicato
#include <time.h>
#include "find.h"   // verbose_flag
#include "dataStructure.h"
#include "wf_getter.h"    //freePtP()
#include "kmp.h"    //kmpInFile(), struct Occurrencies, fprintOcc(), freeList()

static bool dublicato = false;

struct Word ** createDS(char **ptp_w, int n, char **ptp_f, int m, int maxc_w, int maxc_f) {

  int i, j;
  clock_t time_begin;
  clock_t time_end;
  double time_spent;
  double tot_time = 0;
  //DLA(dynamic linear array) of pointers of n structs Word
  struct Word **w_dla = malloc(n * sizeof(struct Word *));
  if(!w_dla){
    printf("\033[1;31m");printf("ERRORE [dataStructure.c -> createDS()]:");printf("\033[0m");
    fprintf(stderr, " malloc(w_dla):\n\t%s\n", strerror(errno));
    return NULL;
  }

  for(i = 0; i < n; i++){
    //allocate struct Word and w_dla[i] points to it
    w_dla[i] = malloc(sizeof(struct Word));
    if(!(w_dla[i])){
      printf("\033[1;31m");printf("ERRORE [dataStructure.c -> createDS()]:");printf("\033[0m");
      fprintf(stderr, " malloc(w_dla[%d]):\n\t%s\n",i , strerror(errno));
      return NULL;
    }
    //DLA of pointers of m structs File
    w_dla[i]->p_file  = malloc(m * sizeof(struct File *));
    if(!(w_dla[i]->p_file)){
      printf("\033[1;31m");printf("ERRORE [dataStructure.c -> createDS()]:");printf("\033[0m");
      fprintf(stderr, " malloc(w_dla[%d]->p_file):\n\t%s\n",i , strerror(errno));
      return NULL;
    }
    w_dla[i]->word = malloc(maxc_w);
    strcpy(w_dla[i]->word, ptp_w[i]);
    w_dla[i]->tot_occurr = 0;
    time_begin = clock();
    if(verbose_flag) printf(" -- Ricerca occorrenze della parola '%s'\n", w_dla[i]->word);
    for (j = 0; j < m; j++) {
      //allocate struct File and w_dla[i]->p_file[j] points to it
      w_dla[i]->p_file[j] = malloc(sizeof(struct File));
      if(!(w_dla[i]->p_file[j])){
        printf("\033[1;31m");printf("ERRORE [dataStructure.c -> createDS()]:");printf("\033[0m");
        fprintf(stderr, " malloc(w_dla[%d]->p_file[%d]):\n\t%s\n",i ,j , strerror(errno));
        return NULL;
      }

      w_dla[i]->p_file[j]->path = malloc(maxc_f);
      strcpy(w_dla[i]->p_file[j]->path, ptp_f[j]);
      w_dla[i]->p_file[j]->head = kmpInFile(w_dla[i]->word, w_dla[i]->p_file[j]->path, &w_dla[i]->p_file[j]->occurr);
      // if(!(w_dla[i]->p_file[j]->head)){
      //   fprintf(stderr, "ERRORE kmpInFile(w_dla[%d]->p_file[%d]->head):\n\t%s\n",i ,j , strerror(errno));
      //   return NULL;
      // }

      time_end = clock();
      time_spent = (double)(time_end - time_begin) / CLOCKS_PER_SEC;
      if(verbose_flag) printf("\tsul file '%s' (%lf)\n", w_dla[i]->p_file[j]->path, time_spent);
      w_dla[i]->tot_occurr += w_dla[i]->p_file[j]->occurr;
      tot_time += time_spent;
    }
  }
  if(verbose_flag) printf(" -- Data Structure Creata (%lf)\n", tot_time);
  return w_dla;
}

void fprintDS(struct Word ** w_dla, int n, int m, FILE** oF) {
    int i, j;
    for (i = 0; i < n; i++) {
        fprintf(*oF, "WORD %s\r\n", w_dla[i]->word);
        fprintf(*oF, "TOTAL %d\r\n", w_dla[i]->tot_occurr);
        for (j = 0; j < m; j++) {
            fprintf(*oF, "FILE %s\r\n", w_dla[i]->p_file[j]->path);
            fprintf(*oF, "OCCURRENCES %d\r\n", w_dla[i]->p_file[j]->occurr);
            fprintOcc(oF, w_dla[i]->p_file[j]->head);
        }
    }
}

void printDS(struct Word ** w_dla, int n, int m) {
    int i, j;
    printf("\n\n***OUTPUT***\n");
    for (i = 0; i < n; i++){
        printf("WORD %s\n", w_dla[i]->word);
        printf("TOTAL %d\n", w_dla[i]->tot_occurr);
        for (j = 0; j < m; j++){
            printf(" ->\tFILE %s\r\n", w_dla[i]->p_file[j]->path);
            printf("   \tOCCURRENCES %d\r\n", w_dla[i]->p_file[j]->occurr);
            printOcc(w_dla[i]->p_file[j]->head);
      }
      printf("-------------------------------------------------\n");
    }
}

void deallocateDS(struct Word ** w_dla, char **ptp_w, int n, char **ptp_f, int m) {
    int i ,j;
    //memory management ptp_w e ptp_f
    freePtP(ptp_w, n);
    freePtP(ptp_f, m);
    //memory management data structure
    if(w_dla) {
        for (i = 0; i < n; i++) {
            for (j = 0; j < m; j++){
                if((w_dla[i]->p_file[j]->head) != NULL) freeList(w_dla[i]->p_file[j]->head);
                if((w_dla[i]->p_file[j]->path) != NULL) {
                  free(w_dla[i]->p_file[j]->path);
                  w_dla[i]->p_file[j]->path = NULL;

                }
                if((w_dla[i]->p_file[j]) != NULL) {
                    free(w_dla[i]->p_file[j]);
                    w_dla[i]->p_file[j] = NULL;
                }
            }
            if((w_dla[i]->p_file) != NULL){
                free(w_dla[i]->p_file);
                w_dla[i]->p_file = NULL;
            }
            if((w_dla[i]->word) != NULL) {
              free(w_dla[i]->word);
              w_dla[i]->word = NULL;

            }

            if((w_dla[i]) != NULL) {
                free(w_dla[i]);
                w_dla[i] = NULL;
            }
        }
        free(w_dla);
        w_dla = NULL;
    }
}

int ordinaDS(struct Word ** w_dla, int n, int m) {
  qsort(w_dla, n, sizeof(struct Word *), compareWords);
  if(dublicato) return 0;
  for (int i = 0; i < n; i++){
      qsort(w_dla[i]->p_file, m, sizeof(struct File *), compareFiles);
      if(dublicato) return 0;
  }
  return 1;
}

int compareWords(const void *s1, const void *s2) {
  struct Word **e1 = (struct Word **) s1;
  struct Word **e2 = (struct Word **) s2;
  if(strcmp((*e1)->word, (*e2)->word)==0) {
      printf("\033[1;31m");printf("ERRORE:");printf("\033[0m");
      printf(" dublicato '%s' tra le parole!!\n\n",(*e1)->word);
      dublicato = true;
  }
  return strcmp((*e1)->word, (*e2)->word);
}

int compareFiles(const void *s1, const void *s2) {
  struct File **e1 = (struct File **) s1;
  struct File **e2 = (struct File **) s2;
  if(strcmp((*e1)->path, (*e2)->path)==0) {
      printf("\033[1;31m");printf("ERRORE:");printf("\033[0m");
      printf(" dublicato '%s' tra i files!!\n\n", (*e1)->path);
      dublicato = true;
  }
  if((*e1)->occurr < (*e2)->occurr) return +1;
  else if((*e1)->occurr > (*e2)->occurr) return -1;
  else return strcmp((*e1)->path, (*e2)->path);
}

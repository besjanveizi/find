#include "header.h"

/*****/////////////////////*****/
/*****    Data Structure   *****/
/*****/////////////////////*****/

static bool dublicato = false;
double tot_time;
//static const char *reportFile = "report.txt";

struct Word ** createDS(char (*pta_w)[MAXC], int n, char (*pta_f)[MAXC], int m) {
  int i, j;
  tot_time = 0;
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
    strcpy(w_dla[i]->word, pta_w[i]);
    w_dla[i]->tot_occurr = 0;
    begin = clock();
    if(verbose_flag) printf(" -- Ricerca occorrenze della parola '%s'\n", w_dla[i]->word);
    for (j = 0; j < m; j++) {
      //allocate struct File and w_dla[i]->p_file[j] points to it
      w_dla[i]->p_file[j] = malloc(sizeof(struct File));
      if(!(w_dla[i]->p_file[j])){
        printf("\033[1;31m");printf("ERRORE [dataStructure.c -> createDS()]:");printf("\033[0m");
        fprintf(stderr, " malloc(w_dla[%d]->p_file[%d]):\n\t%s\n",i ,j , strerror(errno));
        return NULL;
      }
      strcpy(w_dla[i]->p_file[j]->path, pta_f[j]);
      w_dla[i]->p_file[j]->head = kmpInFile(w_dla[i]->word, w_dla[i]->p_file[j]->path, &w_dla[i]->p_file[j]->occurr);
      // if(!(w_dla[i]->p_file[j]->head)){
      //   fprintf(stderr, "ERRORE kmpInFile(w_dla[%d]->p_file[%d]->head):\n\t%s\n",i ,j , strerror(errno));
      //   return NULL;
      // }

      end = clock();
      time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
      if(verbose_flag) printf("\tsul file '%s' (%lf)\n", w_dla[i]->p_file[j]->path, time_spent);
      w_dla[i]->tot_occurr += w_dla[i]->p_file[j]->occurr;
      tot_time += time_spent;
    }
  }
  if(verbose_flag) printf(" -- Data Structure Creata (%lf)\n", tot_time);
  return w_dla;
}

int mkreport(struct Word ** w_dla, int n, int m, char * reportFile) {
    int i, j;
    FILE* wr = fopen (reportFile, "w");    //file di output per report
    if(!wr) {
        printf("\033[1;31m");printf("ERRORE [dataStructure.c -> mkreport()]:");printf("\033[0m");
        fprintf (stderr, " fopen(%s)\n\t%s\n", reportFile,
          strerror(errno));
        return 0;
    }
    for (i = 0; i < n; i++) {
        fprintf(wr, "WORD %s\r\n", w_dla[i]->word);
        fprintf(wr, "TOTAL %d\r\n", w_dla[i]->tot_occurr);
        for (j = 0; j < m; j++) {
            fprintf(wr, "FILE %s\r\n", w_dla[i]->p_file[j]->path);
            fprintf(wr, "OCCURRENCES %d\r\n", w_dla[i]->p_file[j]->occurr);
            fprintList(wr, w_dla[i]->p_file[j]->head);
        }
    }
    fclose(wr);
    return 1;
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
            printList(w_dla[i]->p_file[j]->head);
      }
      printf("-------------------------------------------------\n");
    }
}

void deallocateDS(struct Word ** w_dla, char (*pta_w)[MAXC], int n, char (*pta_f)[MAXC], int m) {
    int i ,j;
    //memory management array 2D pta_w e pta_f
    freePtA(pta_w);
    freePtA(pta_f);
    //memory management data structure
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++){
            if((w_dla[i]->p_file[j]->head) != NULL) freeList(w_dla[i]->p_file[j]->head);
            if((w_dla[i]->p_file[j]) != NULL) {
                free(w_dla[i]->p_file[j]);
                w_dla[i]->p_file[j] = NULL;
            }
        }
        if((w_dla[i]->p_file) != NULL){
            free(w_dla[i]->p_file);
            w_dla[i]->p_file = NULL;
        }

        if((w_dla[i]) != NULL) {
            free(w_dla[i]);
            w_dla[i] = NULL;
        }
    }
    free(w_dla);
    w_dla = NULL;
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

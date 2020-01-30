#include "header.h"

/*****/////////////////////*****/
/*****    Data Structure   *****/
/*****/////////////////////*****/

const char *appFile = "append.txt";
static bool dublicato = false;

struct Word ** createDS(char (*pta_w)[MAXC], int n, char (*pta_f)[MAXC], int m) {
  int i, j;
  //DLA(dynamic linear array) of pointers of n structs Word
  struct Word **w_dla = malloc(n * sizeof(struct Word *));
  if(!w_dla){
    fprintf(stderr, "ERRORE malloc(w_dla):\n\t%s\n", strerror(errno));
    return NULL;
  }

  for(i = 0; i < n; i++){
    //allocate struct Word and w_dla[i] points to it
    w_dla[i] = malloc(sizeof(struct Word));
    if(!(w_dla[i])){
      fprintf(stderr, "ERRORE malloc(w_dla[%d]):\n\t%s\n",i , strerror(errno));
      return NULL;
    }
    //DLA of pointers of m structs File
    w_dla[i]->p_file  = malloc(m * sizeof(struct File *));
    if(!(w_dla[i]->p_file)){
      fprintf(stderr, "ERRORE malloc(w_dla[%d]->p_file):\n\t%s\n",i , strerror(errno));
      return NULL;
    }
    strcpy(w_dla[i]->word, pta_w[i]);
    w_dla[i]->tot_occurr = 0;

    for (j = 0; j < m; j++) {
      //allocate struct File and w_dla[i]->p_file[j] points to it
      w_dla[i]->p_file[j] = malloc(sizeof(struct File));
      if(!(w_dla[i]->p_file[j])){
        fprintf(stderr, "ERRORE malloc(w_dla[%d]->p_file[%d]):\n\t%s\n",i ,j , strerror(errno));
        return NULL;
      }
      strcpy(w_dla[i]->p_file[j]->path, pta_f[j]);
      w_dla[i]->p_file[j]->head = kmpInFile(w_dla[i]->word, w_dla[i]->p_file[j]->path, &w_dla[i]->p_file[j]->occurr);
      if(!(w_dla[i]->p_file[j]->head)){
        fprintf(stderr, "ERRORE kmpInFile(w_dla[%d]->p_file[%d]->head):\n\t%s\n",i ,j , strerror(errno));
        return NULL;
      }
      w_dla[i]->tot_occurr += w_dla[i]->p_file[j]->occurr;
    }
  }
  return w_dla;
}

void printDS(struct Word ** w_dla, int n, int m) {
    int i, j;
    printf("\n\n***DATA STRUCTURE***\t&w_dla: %p\n", &w_dla);
    for (i = 0; i < n; i++){
        printf("&w_dla[%d]: %p\n",i , &w_dla[i]);
        printf("  word: %s \t &w_dla[%d]->word: %p\n", w_dla[i]->word,
            i , &w_dla[i]->word);
        printf("  tot occ: %d\n", w_dla[i]->tot_occurr);
        printf("  --&w_dla[%d]->p_file: %p --\n",i , &w_dla[i]->p_file);
        for (j = 0; j < m; j++){
            printf("  \t\tpath: %s \t occ: %d \t &w_dla[%d]->p_file[%d]->path: %p\n",
            w_dla[i]->p_file[j]->path, w_dla[i]->p_file[j]->occurr, i, j,
                &w_dla[i]->p_file[j]->path);
            printList(w_dla[i]->p_file[j]->head);
      }
    }
}

void deallocateDS(struct Word ** w_dla, char (*pta_w)[MAXC], int n, char (*pta_f)[MAXC], int m) {
    int i ,j;
    //memory management array 2D pta_w e pta_f
    freePtrToArr(pta_w);
    freePtrToArr(pta_f);
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
      printf("EXIT: errore dublicato '%s' tra le parole!!\n\n",(*e1)->word);
      dublicato = true;
  }
  return strcmp((*e1)->word, (*e2)->word);
}

int compareFiles(const void *s1, const void *s2) {
  struct File **e1 = (struct File **) s1;
  struct File **e2 = (struct File **) s2;
  if(strcmp((*e1)->path, (*e2)->path)==0) {
      printf("EXIT: errore dublicato '%s' tra i files!!\n\n", (*e1)->path);
      dublicato = true;
  }
  if((*e1)->occurr < (*e2)->occurr) return +1;
  else if((*e1)->occurr > (*e2)->occurr) return -1;
  else return strcmp((*e1)->path, (*e2)->path);
}

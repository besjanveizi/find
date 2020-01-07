#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

struct Word{
  char word[50];
  int tot_occurr;
  struct File **p_file;
};
struct File{
  char path[100];
  int occurr;
  struct Occurrencies* head;
};
struct Occurrencies{
  int n_row;
  int n_char;
  struct Occurrencies* next;
};

static bool dublicato = false;
// Defining comparator function as per the requirement
static int myCompare(const void* a, const void* b)
{

    if((*(const char**)a) == (*(const char**)b)){
      dublicato = true;
    }
    return strcmp(*(const char**)a, *(const char**)b);
}

// Function to sort the array
void sort(const char* arr[], int n)
{
    // calling qsort function to sort the array
    // with the help of Comparator
    qsort(arr, n, sizeof(const char*), myCompare);
    if(dublicato){
      printf("ERRORE DUBLICATO in %s!!\n\n", arr);
      exit(1);
    }
}


int main(void){
  srand(time(0));

  const char* myWords[3];
  myWords[0] = "zebra";
  myWords[1] = "balena";
  myWords[2] = "antilope";

  const char* myFiles[10];
  myFiles[0] = "/Users/besjan/Desktop/git/file7";
  myFiles[1] = "/Users/besjan/Desktop/git/file1";
  myFiles[2] = "/Users/besjan/Desktop/git/file6";
  myFiles[3] = "/Users/besjan/Desktop/git/file9";
  myFiles[4] = "/Users/besjan/Desktop/git/file5";
  myFiles[5] = "/Users/besjan/Desktop/git/file2";
  myFiles[6] = "/Users/besjan/Desktop/git/file4";
  myFiles[7] = "/Users/besjan/Desktop/git/file0";
  myFiles[8] = "/Users/besjan/Desktop/git/file3";
  myFiles[9] = "/Users/besjan/Desktop/git/file8";

  int i,j;
  int n = sizeof(myWords) / sizeof(myWords[0]); //length of myWords
  int m = sizeof(myFiles) / sizeof(myFiles[0]);//length of myFiles

  //sort myWords and myFiles
  sort(myWords, n);
  sort(myFiles, m);

  // Print the sorted arrays
  printf("\nSorted myWords is\n");
  for (i = 0; i < n; i++)
      printf("%d: %s \n", i, myWords[i]);

  printf("\nSorted myFiles is\n");
  for (j = 0; j < m; j++)
      printf("%d: %s \n", j, myFiles[j]);


  //DLA(dynamic linear array) of pointers of n structs Word
  struct Word **w_dla = malloc(n * sizeof(struct Word *));

  for(i = 0; i < n; i++){
    //allocate struct Word and w_dla[i] points to it
    w_dla[i] = malloc(sizeof(struct Word));
    //DLA of pointers of m structs File
    w_dla[i]->p_file  = malloc(m * sizeof(struct File *));

    strcpy(w_dla[i]->word, myWords[i]);
    w_dla[i]->tot_occurr = 0;
    for (j = 0; j < m; j++) {
      //allocate struct File and w_dla[i]->p_file[j] points to it
      w_dla[i]->p_file[j] = malloc(sizeof(struct File));

      strcpy(w_dla[i]->p_file[j]->path, myFiles[j]);
      w_dla[i]->p_file[j]->occurr = 0;


      //da rivedere da qui fino alla }
      //si tratta di riempire le Occurrencies
      w_dla[i]->p_file[j]->head = (struct Occurrencies*)malloc(sizeof(struct Occurrencies));
      w_dla[i]->p_file[j]->head->n_row = (rand() % (10 - 0 + 1)) + 0;
      w_dla[i]->p_file[j]->head->n_char = (rand() % (10 - 0 + 1)) + 0;
    }
  }


  for (i = 0; i < n; i++){
    printf("word: %s \t &w_dla[i]: %p\n", w_dla[i]->word, &w_dla[i]);
    printf("--&w_dla[i]->p_file: %p --\n", &w_dla[i]->p_file);
    for (j = 0; j < m; j++){
      printf("\t\tpath: %s \t &w_dla[i]->p_file[j]->path: %p\n", w_dla[i]->p_file[j]->path, &w_dla[i]->p_file[j]->path);
    }
  }

  for (i = 0; i < n; i++) {
    for (j = 0; j < m; j++){
      free(w_dla[i]->p_file[j]);
    }
    free(w_dla[i]->p_file);
    free(w_dla[i]);
  }
  free(w_dla);

  return 0;
}

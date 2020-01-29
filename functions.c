#include "header.h"

const char *appFile = "append.txt";
static bool dublicato = false;
const char accenti[] = {"éèòàùì"};
static char *currentDir;
char cwd[1024];

/** FUNCTION DEFINITIONS**/

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

//filesgetter
char (*fileToPtA(const char* filename, int *n))[MAXC]{
    char (*array)[MAXC] = NULL;
    int m = 0, max_righe = MAXR;
    FILE *fp = fopen (filename, "r");

    if (!fp) {
        fprintf(stderr, "ERRORE fopen(%s) in fileToPtA(): %s\n", filename,
          strerror(errno));
        return NULL;
    }

    if (!(array = malloc (MAXR * sizeof *array))) { // alloca MAXR puntatori
        fprintf(stderr, "ERRORE malloc() per '%s' in fileToPtA():\n\t%s\n",
          filename, strerror(errno));
        freePtrToArr(array);
        return NULL;
    }

    while (fgets (array[m], MAXC, fp)) {
        if(strcmp(array[m],"\n") == 0) continue;  //ignora righe vuote
        char *ptr = array[m];
        // trova il primo '\n' nella riga appena letta
        for (; *ptr && *ptr != '\n'; ptr++) {}

        if (*ptr != '\n') {
            int c;
            //controlla che le righe lette non superino MAXC caratteri
            //in caso lo superano, i caratteri successivi sono ignorati
            while ((c = fgetc (fp)) != '\n' && c != EOF) {}
        }
        *ptr = 0; // nul - termine riga
        if (++m == max_righe) { // realloc (*array)
            void *tmptr = realloc (array, 2 * max_righe * sizeof *array);
            if (!tmptr) {
              fprintf(stderr, "ERRORE realloc() per '%s' in fileToPtA():\n\t%s\n",
                filename, strerror(errno));
              printf("Sono state registrate soltanto %d righe dal file %s\n",
              m, filename);
                break;  // non leggere più e mantieni ciò che hai letto
            }
            array = tmptr;  //assegna il puntatore aggiornato a array
            max_righe *= 2; //aggiorna dimensione delle righe
        }
    }
    *n = m;
    fclose (fp);
    return array;
}

int findPathsPtA(char (*files)[MAXC], int n) {
  int i;
  FILE* appendFile = fopen (appFile, "a");    //file di appoggio
  if(!appendFile) {
      fprintf (stderr, "ERRORE fopen(%s) in findPathsPtA(): %s\n", appFile,
        strerror(errno));
      return 0;
  }

  int r_flag = 0;
  currentDir = getcwd(cwd, sizeof(cwd));
  //printf("CWD: %s\n", currentDir);
  //parsing delle righe dei files/cartelle
  for (i = 0; i < n; i++) {
    char *ptr = files[i];
    int l = strlen(ptr);
    const char *ricursive_tag = strrchr(ptr, ' ');
    if(l > 4 && ricursive_tag && (strcmp(ricursive_tag," [r]")==0)){
        for (int j = 0;j < (l-4); ptr++) {j++;}
        *ptr = 0;
        r_flag = 1;
    }
    else if(ricursive_tag && (strcmp(ricursive_tag," [r]")==0))
        continue;   //ignora righe " [r]"

    l = strlen(files[i]);
    if(isAbsolute(files[i])) continue;
    else strcpy(files[i], getAbsolute(files[i], l));
    if(strcmp(files[i], "") == 0) {
        fclose(appendFile);
        rmFile(appFile);
        return 0;
    }

    //printf("%d) stringa: %s rf:%d\n", i, files[i], r_flag);
    if(!getPaths(files[i], r_flag, &appendFile)){
        fclose(appendFile);
        rmFile(appFile);
        return 0;
    }
    r_flag = 0;   //reset r_flag
  }
  fclose(appendFile);
  return 1;
}

int isAbsolute(char * path_str) {
	if(strncmp(path_str, "/", 1) == 0) return 1;
	else return 0;
}

char * getAbsolute(char *str, int l){

  int i, cPunto = 0, cSlash = 0, cUp;
		char absPath[1024];
		char *abs;
		strcpy(absPath, currentDir);

		if(str[0] != '.') {
			strcat(absPath, "/");
			strcat(absPath, str);

			abs = absPath;
			return abs;
		}

		else if(str[1] != '.'){
				for(int j=0; j<l; j++)
					str[j] = str[j+1];

				strcat(absPath, str);

        if(absPath[strlen(absPath)-1] == '/') absPath[strlen(absPath)-1] = '\0';

				abs = absPath;
			 	return abs;
		}

		else {
				char *ptr_str = str;
		    char *ptr_cwd = absPath;

		    for(; *ptr_str && ((*ptr_str == '.') || (*ptr_str == '/')); ptr_str++)
		      if(*ptr_str == '.') cPunto++;

		    for(; *ptr_cwd; ptr_cwd++)
		      if(*ptr_cwd == '/') cSlash++;

		    ptr_cwd = NULL;
		    ptr_cwd = absPath;

		    cUp = cSlash - (cPunto/2);
		    if(cUp < 0){
						printf("ERRORE getAbsolute(): impossibile risalire a %s\n", str);
						return "";
				}

		    for(i = 0; *ptr_cwd && i < (cUp+1); ptr_cwd++)
		        if (*ptr_cwd == '/') i++;
		    *ptr_cwd = 0;

		    strcat(absPath, ptr_str);
        if(absPath[strlen(absPath)-1] == '/') absPath[strlen(absPath)-1] = '\0';

				abs = absPath;
			 	return abs;
		}
}

int getPaths(const char *pathname, int rf, FILE** appendFile) {
    struct stat s;

    if(lstat(pathname,&s) == 0) {
        if(S_ISDIR(s.st_mode))
            checkDir(pathname, rf, appendFile);
        else if(S_ISREG(s.st_mode))
            fprintf(*appendFile, "\n%s", pathname);

        else {    //pathname non è una cartella o un file regolare
            printf("ALERT:\n");
            printf("\tIl programma gestisce solo cartelle e file regolari\n");
            printf("\tIl path '%s' verrà ignorato per generazione del report\n",
            pathname);
        }
        return 1;
    }
    else {
        fprintf (stderr, "ERRORE stat(%s): %s\n", pathname, strerror(errno));
        return 0;
    }
}

void checkDir(const char *dirPath, int rf, FILE** appendFile) {
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(dirPath))){
        fprintf (stderr, "ERRORE opendir(%s) in checkDir(): %s\n", dirPath,
          strerror(errno));
        return;
    }
    while ((entry = readdir(dir)) != NULL) {

        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0 ||
            strcmp(entry->d_name, ".DS_Store") == 0 ||
            strcmp(entry->d_name, appFile) == 0)
                continue;

        if (entry->d_type == DT_DIR && rf == 1) {
            char path[1024] = "";
            strcat(path, dirPath);
            strcat(path, "/");
            strcat(path, entry->d_name);
            checkDir(path, rf, appendFile);
        }
        else if(entry->d_type == DT_REG)
            fprintf(*appendFile, "\n%s/%s", dirPath, entry->d_name);
        //file type diversi da directories (con r_flag = 1) o file regolari sono ignorati
        else {
            if (entry->d_type == DT_DIR && rf == 0) continue;
            printf("ALERT:\n");
            printf("\tIl programma gestisce solo cartelle e file regolari\n");
            printf("\tIl path '%s/%s' verrà ignorato per generazione del report\n",
            dirPath, entry->d_name);
        }
    }
    closedir(dir);
}

void printPtA(char (*arr)[MAXC], int n){
  int i = 0;
  printf(" line[%2d] : '%s'\n", i + 1, *(arr+0));
  for (i = 1; i < n; i++) printf (" line[%2d] : '%s'\n", i + 1, arr[i]);
}

int cmp_sortPtA(const void* a, const void* b) {
    if(strcmp(a, b)==0) dublicato = true;
    return strcmp(a, b);
}

bool sortPtA(char (*arr)[MAXC], int n) {
    qsort(*arr, n, sizeof *arr, cmp_sortPtA);
    if(dublicato) return 0;
    else return 1;
}

void rmFile(const char * filename) {
    char cmd[120];
    strcpy(cmd, "rm ");
    strcat(cmd, filename);
    system(cmd);
}

void freePtrToArr(char (*ptr)[MAXC]) {
  free (ptr);
  ptr = NULL;
}

//KMP
struct Occurrencies* kmpInFile(char *parola, char *filename, int *totOcc){

    struct Occurrencies* head = NULL;
    struct Occurrencies** ptpCoda= &head; // ultimo puntatore a nodo nella lista

    int tot = 0;    //totali occorrenze
    int i = 0;    // indice dei caratteri della riga del file
    int j = 0;    // indice dei caratteri della parola
    int riga = 0;   // indice riga del file
    char rowFile[MAXC];   //riga file
    int sub_str = 0;    // caratteri accentati nella riga del file
    int sub_par = 0;    // caratteri accentati nella parola
    int sub_tot = 0;    // sub_str - sub_par
    int S_SIZE;   // lunghezza riga del file
    int P_SIZE = strlen(parola);

    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf (stderr, "ERRORE fopen(%s) in kmpInFile(): %s\n", filename,
          strerror(errno));
        exit(EXIT_FAILURE);
    }

    //Longest Prefix Suffix
    int *lps = NULL;
    if(!(lps = calcolaLPS(parola, P_SIZE))) exit(EXIT_FAILURE);

    // calcola sub_par
    for (int k = 0; k < (P_SIZE-1); k++)
        if(isIn(parola[k], accenti)) sub_par++;
    if(sub_par>0){
        if((sub_par%2)==0) sub_par /= 2;
        else {
            sub_par++;
            sub_par /= 2;
        }
    }

    // calcola KMP
    while (fgets (rowFile, MAXC, file)) {

        char * ptr = rowFile;
        for (; *ptr && *ptr != '\n'; ptr++) {} //elimina '\n'
        *ptr = 0;
        S_SIZE = strlen(rowFile);

        if(strcmp(rowFile,"\n") == 0) { //riga vuota
            riga++;
            continue;
        }

        while (i < S_SIZE) {
            if(isIn(rowFile[i], accenti))
                sub_str++;

            if (parola[j] == rowFile[i]) {
                j++;
                i++;
            }

            if (j == P_SIZE) {
                sub_tot = 0;
                sub_tot += (sub_str/2);
                sub_tot -= sub_par;
                // push nuovo nodo in fondo alla lista
                pushOccurr(ptpCoda, riga + 1, (i - j)+1-sub_tot);
                // aggiorna ptpCoda perché faccia riferimento al prossimo nodo
                (*ptpCoda)->next = NULL;
                ptpCoda = &((*ptpCoda)->next);
                tot++;
                j = *(lps+(j - 1));
            }

            else if (i < S_SIZE && parola[j] != rowFile[i]) {
                if (j != 0) j = *(lps+(j - 1));
                else i++;
            }
        }
        i = 0;
        j = 0;
        riga++;
        sub_str = 0;
    }

    fclose(file);
    free(lps);
    lps = NULL;
    *totOcc = tot;
    return head;
}

int *calcolaLPS(char* p, int P_SIZE) {
    int len = 0;
    int * tmp = NULL;

    if (!(tmp = malloc (P_SIZE * sizeof (int)))) {
        fprintf(stderr, "ERRORE malloc() per '%s' in calcolaLPS():\n\t%s\n",
          p, strerror(errno));
        free(tmp);
        tmp = NULL;
        return NULL;
    }

    *tmp = 0;

    int i = 1;
    while (i < P_SIZE) {
      if (p[i] == p[len]) {
          len++;
          *(tmp+i) = len;
          i++;
      }
      else if (len != 0)
          len = *(tmp+(len - 1));
      else {
          *(tmp+i) = 0;
          i++;
      }
    }
    return tmp;
}

int isIn(char ch, const char* str){
  int l = strlen(str);
  for (int i = 0; i < l; i++) {
    if(ch == str[i])
      return 1;
  }
  return 0;
}

void freeList(struct Occurrencies* head) {
    struct Occurrencies* tmp;
    while (head != NULL) {
        tmp = head;
        head = head->next;
        free(tmp);
    }
    tmp = NULL;
}

void pushOccurr(struct Occurrencies** ptpHead, int r, int c) {
    struct Occurrencies* nuovaOcc = malloc(sizeof(struct Occurrencies));
    nuovaOcc->n_row = r;
    nuovaOcc->n_char = c;
    nuovaOcc->next = *ptpHead; // prossimo ptpHead
    *ptpHead = nuovaOcc; // cambio il valore del corrente ptpHead con il nuovo nodo
}

void printList(struct Occurrencies* n) {
    while (n != NULL) {
        printf("\t\t\t\triga %d posizione %d\n", n->n_row, n->n_char);

        n = n->next;
    }
}

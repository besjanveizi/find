#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>  // lstat()
#include <sys/stat.h>   // lstat()
#include <unistd.h>     // lstat()
#include <dirent.h>     //opendir() & readdir()
#include "find.h"       //verbose_flag, arg_exclude, dim_arg_exlude
#include "wf_getter.h"

const char *appFile = "append.txt";

// funzione che ottiene tutti i file (rispettando il flag di ricorsione) e li
// scrive in ordine di visita in appFile
static int getPaths(char *, int, FILE**, int);
// funzione che ottiene i file di una specifica directory
static int checkDir(char *, int, FILE**, int);

// funzioni per verificare se una estensione è esclusa
static int isExcluded(char *);
static char *getExtension(char *);
static char *getBasename(char *);

static bool no_ext;   // per files senza estensione

char ** fileToPtP(const char* filename, int *n, int *max_char){
    char **array = NULL;
    int m = 0;
    int l_row_max = *max_char;
    int c;
    int l_row, n_rows = 0;
    FILE *fp = fopen (filename, "r");

    if (!fp) {
        printf("\033[1;31m");printf("ERRORE [wf_getter.c -> fileToPtP()]:");printf("\033[0m");
        printf(" fallimento fopen(%s)\n", filename);
        fprintf(stderr, "%s\n", strerror(errno));
        return NULL;
    }

    // ciclo per calcolo del numero delle righe
    // e la dimensione della riga massima nel file
    while((c = fgetc (fp)) != EOF) {
        l_row=1;
        if(c=='\n' || c=='\r') continue;
        while ((c = fgetc (fp)) != '\n') {
          if(c == '\r') break;
          l_row++;
        }
        //aggiorna dimesione max di lettura per fgets()
        if(l_row > l_row_max) l_row_max = l_row+1;    //+ '\0', termine stringa
        n_rows++;
    }
    *max_char = l_row_max;

    if(n_rows==0) {    //file vuoto, nessun elemento nel PtP
      if(strcmp(filename, appFile) != 0) {
          printf("\033[1;35m");printf("WARNING:");printf("\033[0m");
          printf(" non è stato possibile leggere alcuna riga dal file '%s'\n", filename);
          printf("Controlla che sia un file non vuoto\n");
      }
      else {
        printf("\033[1;35m");printf("WARNING:");printf("\033[0m");
        printf(" nessun file trovato per la ricerca\n");
        printf("Controlla il file per l'argomento --input oppure gli argomenti per --exclude\n");
      }
      fclose (fp);
      return NULL;
    }

    if (!(array = malloc (n_rows * sizeof *array))) { // alloca n_rows puntatori
        printf("\033[1;31m");printf("ERRORE [wf_getter.c -> fileToPtP()]:");printf("\033[0m");
        printf(" fallimento allocazione %d puntatori per '%s'\n", n_rows, filename);
        fprintf(stderr, "%s\n", strerror(errno));
        fclose (fp);
        return NULL;
    }
    for (; m < n_rows; m++) {
      if(!(array[m]= malloc(*max_char))) {
        printf("\033[1;31m");printf("ERRORE [wf_getter.c -> fileToPtP()]:");printf("\033[0m");
        printf(" fallimento allocazione %d-esimo puntatore [%d]\n", m + 1, *max_char);
        fprintf(stderr, "%s\n", strerror(errno));
        fclose (fp);
        return NULL;
      }
    }

    //reset file pointer a inizio file e indice PtP
    fseek(fp, 0, SEEK_SET);
    m = 0;

    while (m < n_rows && fgets (array[m], *max_char, fp)) {
        if((strcmp(array[m],"\n") == 0) || strcmp(array[m],"\r\n") == 0)
          continue;  //ignora righe vuote
        char *ptr = array[m];
        // trova il primo '\n' nella riga appena letta
        for (; *ptr && *ptr != '\n' && *ptr != '\r'; ptr++) {}
        *ptr = 0; // nul - termine riga
        m++;
    }
    fclose (fp);
    *n = m;
    return array;
}

int findPathsPtP(char **files, int n) {
    int r_flag = 0;   //recursive flag
    FILE* appendFile = fopen (appFile, "a");    //file di appoggio
    if(!appendFile) {
        printf("\033[1;31m");printf("ERRORE [wf_getter.c -> findPathsPtP()]:");printf("\033[0m");
        printf(" fallimento fopen(%s)\n", appFile);
        fprintf (stderr, "%s\n", strerror(errno));
        return 0;
    }

    //parsing delle righe dei files/cartelle
    for (int i = 0; i < n; i++) {
        char *ptr = files[i];
        int l = strlen(ptr);
        int optimal_size = 1 + l;
        char * str_app  = NULL;

        //check for recursive tag
        const char *ricursive_tag = strrchr(ptr, ' ');
        if(l > 4 && ricursive_tag && (strcmp(ricursive_tag," [r]")==0)){
            for (int j = 0;j < (l-4); ptr++) {j++;}
            *ptr = 0;
            r_flag = 1;  //set recursive flag
        }
        else if(ricursive_tag && (strcmp(ricursive_tag," [r]")==0))
            continue;   //ignora righe " [r]"

        // !isAbsolute() -> getAbsolute()
        if(!isAbsolute(files[i])) {
            optimal_size += size_cwd;
            str_app=getAbsolute(files[i], optimal_size);
            if(str_app[0] == '\0') {
                fclose(appendFile);
                return 0;
            }
        }
        else {
          str_app  = malloc(optimal_size);
          strcpy(str_app, files[i]);   // isAbsolute()
        }

        if(!getPaths(str_app, r_flag, &appendFile, optimal_size)) {
            fclose(appendFile);
            free(str_app);
            return 0;
        }
        r_flag = 0;   //reset recursive flag
        free(str_app);
        str_app = NULL;
    }
    fclose(appendFile);
    return 1;
}

bool isAbsolute(char * path_str) {
	if(strncmp(path_str, "/", 1) == 0) return true;
	else return false;
}

char * getAbsolute(char *str, int optimal_size){
    int i, cPunto = 0, cSlash = 0, cUp;
		char *absPath = malloc(optimal_size);
		strcpy(absPath, currentDir);

		if(str[0] != '.') {
			strcat(absPath, "/");
			strcat(absPath, str);
		}

		else if(str[1] != '.'){
        str = str + 1;
				strcat(absPath, str);
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
            printf("\033[1;31m");printf("ERRORE [wf_getter.c]:");printf("\033[0m");
						printf(" getAbsolute(%s)\nImpossibile risalire al file o alla cartella\n", str);
            free(absPath);
						return "";
				}

		    for(i = 0; *ptr_cwd && i < (cUp+1); ptr_cwd++)
		        if (*ptr_cwd == '/') i++;
		    *ptr_cwd = 0;

		    strcat(absPath, ptr_str);
		}

    if(absPath[strlen(absPath)-1] == '/') absPath[strlen(absPath)-1] = '\0';
    return absPath;
}

int getPaths(char *pathname, int rf, FILE** appendFile, int optimal_size) {
    struct stat s;
    no_ext = false;
    if(lstat(pathname,&s) == 0) {
        if(S_ISDIR(s.st_mode)) {
            if(!checkDir(pathname, rf, appendFile, optimal_size)) return 0;
        }
        else if(S_ISREG(s.st_mode)) {
            if(isExcluded(getExtension(pathname)) && !no_ext) return 1;
            else fprintf(*appendFile, "\n%s\r\n", pathname);
            if(verbose_flag) printf("\t%s\n", pathname);
        }

        else {    //pathname non è una cartella o un file regolare
            printf("\033[1;35m");printf("ALERT:\n");printf("\033[0m");
            printf("\tIl programma gestisce solo cartelle e file regolari\n");
            printf("\tIl path '%s' verrà ignorato per generazione del report\n",
              pathname);
        }
        return 1;
    }
    else {
        printf("\033[1;31m");printf("ERRORE [wf_getter.c -> getPaths()]:");printf("\033[0m");
        printf(" path '%s' in --input argument file\n", pathname);
        fprintf (stderr, "%s\n", strerror(errno));
        return 0;
    }
}

int checkDir(char *dirPath, int rf, FILE** appendFile, int optimal_size) {
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(dirPath))){
        printf("\033[1;31m");printf("ERRORE [wf_getter.c -> checkDir()]:");printf("\033[0m");
        printf(" fallimento opendir(%s)\n", dirPath);
        fprintf (stderr, "%s\n", strerror(errno));
        return 0;
    }
    while ((entry = readdir(dir)) != NULL) {

        char * path = NULL;

        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0 )
            continue;

        if (entry->d_type == DT_DIR && rf == 1) {
            int opt = optimal_size;
            int dir_l = strlen(dirPath);
            int newDir_l = strlen(entry->d_name);
            if((opt - dir_l - newDir_l - 1) < 0) opt = opt + newDir_l + 1;
            path = malloc(opt);
            strcpy(path, dirPath);
            strcat(path, "/");
            strcat(path, entry->d_name);
            checkDir(path, rf, appendFile, opt);
        }
        else if(entry->d_type == DT_REG) {
            if(isExcluded(getExtension(entry->d_name)) && !no_ext) continue;
            fprintf(*appendFile, "\n%s/%s\r\n", dirPath, entry->d_name);
            no_ext = false;
            if(verbose_flag) printf("\t%s/%s\n", dirPath, entry->d_name);
        }
        //file type diversi da directories (con r_flag = 1) o file regolari sono ignorati
        else {
            if (entry->d_type == DT_DIR && rf == 0) continue;
            printf("\033[1;35m");printf("ALERT:\n");printf("\033[0m");
            printf("\tIl programma gestisce solo cartelle e file regolari\n");
            printf("\tIl path '%s/%s' verrà ignorato per generazione del report\n",
            dirPath, entry->d_name);
        }
        if(path){
          free(path);
          path = NULL;
        }
    }
    closedir(dir);
    return 1;
}

char *getExtension(char *filename) {
  char *filebase = getBasename(filename);
  char *dot = strrchr(filebase, '.');
  if(!dot) {no_ext = true; return NULL;} //no extension
  else if(dot == filebase || (strcmp(dot, ".") == 0)) return NULL;
  else return dot + 1;
}

char *getBasename(char *path){
    char *base = strrchr(path, '/');
    return base ? base+1 : path;
}

int isExcluded(char * ext) {
  if(ext == NULL) return 1;
  for (int i = 0; i < dim_arg_exclude; i++)
    if(strcmp(ext, arg_exclude[i]) == 0) return 1;
  return 0;
}

void printPtP(char **arr, int n){
  for (int i = 0; i < n;) {
    printf ("'%s'", arr[i]);
    i++;
    if(i<n) printf("| ");
  }
  printf("\n");
}

void freePtP(char **ptp, int n) {
  if(ptp){
      for (int i = 0; i < n; i++){
            if(ptp[i]){
                free (ptp[i]);
                ptp[i] = NULL;
            }
      }
      free(ptp);
      ptp = NULL;
  }
}

void rm_appFile(void) {
    char cmd[120];
    strcpy(cmd, "rm ");
    strcat(cmd, appFile);
    system(cmd);
}

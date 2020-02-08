#include "header.h"

/*****//////////////////////////*****/
/*****    words&files getters   *****/
/*****//////////////////////////*****/

char (*fileToPtA(const char* filename, int *n))[MAXC]{
    char (*array)[MAXC] = NULL;
    int m = 0, max_righe = MAXR;
    FILE *fp = fopen (filename, "r");

    if (!fp) {
        printf("\033[1;31m");printf("ERRORE [wf_getter.c -> fileToPtA()]:");printf("\033[0m");
        fprintf(stderr, " fopen(%s)\n\t%s\n", filename, strerror(errno));
        return NULL;
    }

    if (!(array = malloc (MAXR * sizeof *array))) { // alloca MAXR puntatori
        printf("\033[1;31m");printf("ERRORE [wf_getter.c -> fileToPtA()]:");printf("\033[0m");
        fprintf(stderr, " malloc() per '%s':\n\t%s\n", filename, strerror(errno));
        freePtA(array);
        return NULL;
    }

    while (fgets (array[m], MAXC, fp)) {
        if((strcmp(array[m],"\n") == 0) || strcmp(array[m],"\r\n") == 0)
          continue;  //ignora righe vuote
        char *ptr = array[m];
        // trova il primo '\n' nella riga appena letta
        for (; *ptr && *ptr != '\n' && *ptr != '\r'; ptr++) {}
        if (*ptr != '\r' && *ptr != '\n') {
            int c;
            //controlla che le righe lette non superino MAXC caratteri
            //in caso lo superano, i caratteri successivi sono ignorati
            while ((c = fgetc (fp)) != '\n' && c != EOF) {}
        }
        *ptr = 0; // nul - termine riga
        if (++m == max_righe) { // realloc (*array)
            void *tmptr = realloc (array, 2 * max_righe * sizeof *array);
            if (!tmptr) {
              printf("\033[1;31m");printf("ERRORE [wf_getter.c -> fileToPtA()]:");printf("\033[0m");
              fprintf(stderr, " realloc() per '%s':\n\t%s\n", filename, strerror(errno));
              printf("\033[1;35m");
              printf("Sono stati allocati soltanto %d elementi dal file %s\n",
              m, filename);
              printf("\033[0m");
                break;  // non leggere più e mantieni ciò che hai letto
            }
            array = tmptr;  //assegna il puntatore aggiornato a array
            max_righe *= 2; //aggiorna dimensione delle righe
        }
    }
    fclose (fp);
    if(m==0) {    //file vuoto, nessun elemento nel PtA
      if(strcmp(filename, appFile) != 0) {
          printf("\033[1;35m");printf("WARNING:");printf("\033[0m");
          printf(" il file '%s' è vuoto\n", filename);
      }
      else {
        printf("\033[1;35m");printf("WARNING:");printf("\033[0m");
        printf(" nessun file trovato per la ricerca\n");
        printf("Controlla il file per l'argomento -i oppure gli argomenti per -e\n");
      }
      return NULL;
    }
    *n = m;
    return array;
}

int findPathsPtA(char (*files)[MAXC], int n) {
  int i;
  int r_flag = 0;   //recursive flag
  FILE* appendFile = fopen (appFile, "a");    //file di appoggio
  if(!appendFile) {
      printf("\033[1;31m");printf("ERRORE [wf_getter.c -> findPathsPtA()]:");printf("\033[0m");
      fprintf (stderr, " fopen(%s):\n\t%s\n", appFile, strerror(errno));
      return 0;
  }

  //parsing delle righe dei files/cartelle
  for (i = 0; i < n; i++) {
      char *ptr = files[i];
      int l = strlen(ptr);
      int getPathsOut;
      const char *ricursive_tag = strrchr(ptr, ' ');
      if(l > 4 && ricursive_tag && (strcmp(ricursive_tag," [r]")==0)){
          for (int j = 0;j < (l-4); ptr++) {j++;}
          *ptr = 0;
          r_flag = 1;  //set recursive flag
      }
      else if(ricursive_tag && (strcmp(ricursive_tag," [r]")==0))
          continue;   //ignora righe " [r]"

      if(isAbsolute(files[i])) continue;
      else strcpy(files[i], getAbsolute(files[i], currentDir));
      if(files[i][0] == '\0') {
          fclose(appendFile);
          rmFile(appFile);
          return 0;
      }
      getPathsOut = getPaths(files[i], r_flag, &appendFile);
      if(getPathsOut == 0) {
          fclose(appendFile);
          rmFile(appFile);
          return 0;
      }

      r_flag = 0;   //reset recursive flag
  }
  fclose(appendFile);
  return 1;
}

int isAbsolute(char * path_str) {
	if(strncmp(path_str, "/", 1) == 0) return 1;
	else return 0;
}

char * getAbsolute(char *str, const char * currentDir){
    int i, cPunto = 0, cSlash = 0, cUp;
		char absPath[1024];
		char *abs;

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
						printf(" getAbsolute(%s)\n\tImpossibile risalire alla cartella\n", str);
						return "";
				}

		    for(i = 0; *ptr_cwd && i < (cUp+1); ptr_cwd++)
		        if (*ptr_cwd == '/') i++;
		    *ptr_cwd = 0;

		    strcat(absPath, ptr_str);
		}

    if(absPath[strlen(absPath)-1] == '/') absPath[strlen(absPath)-1] = '\0';
    abs = absPath;
    return abs;
}

int getPaths(char *pathname, int rf, FILE** appendFile) {
    struct stat s;
    // int checkDirOut;

    if(lstat(pathname,&s) == 0) {
        if(S_ISDIR(s.st_mode)) {
            //checkDirOut = checkDir(pathname, rf, appendFile);
            if(!checkDir(pathname, rf, appendFile)) return 0;
        }
        else if(S_ISREG(s.st_mode)) {
            if(isExcluded(getExtension(pathname))) return 1;
            else fprintf(*appendFile, "\n%s\r\n", pathname);
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
        fprintf (stderr, " lstat(%s):\n\t%s\n", pathname, strerror(errno));
        return 0;
    }
}

int checkDir(char *dirPath, int rf, FILE** appendFile) {
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(dirPath))){
        printf("\033[1;31m");printf("ERRORE [wf_getter.c -> checkDir()]:");printf("\033[0m");
        fprintf (stderr, " opendir(%s)\n\t%s\n", dirPath,
          strerror(errno));
        return 0;
    }
    while ((entry = readdir(dir)) != NULL) {

        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0 )
            continue;

        if (entry->d_type == DT_DIR && rf == 1) {
            char path[1024] = "";
            strcat(path, dirPath);
            strcat(path, "/");
            strcat(path, entry->d_name);
            checkDir(path, rf, appendFile);
        }
        else if(entry->d_type == DT_REG) {
            if(isExcluded(getExtension(entry->d_name))) continue;
            fprintf(*appendFile, "\n%s/%s\r\n", dirPath, entry->d_name);
        }
        //file type diversi da directories (con r_flag = 1) o file regolari sono ignorati
        else {
            if (entry->d_type == DT_DIR && rf == 0) continue;
            printf("\033[1;35m");printf("ALERT:\n");printf("\033[0m");
            printf("\tIl programma gestisce solo cartelle e file regolari\n");
            printf("\tIl path '%s/%s' verrà ignorato per generazione del report\n",
            dirPath, entry->d_name);
        }
    }
    closedir(dir);
    return 1;
}

char *getExtension(char *filename) {
  char *filebase = getBasename(filename);
  char *dot = strrchr(filebase, '.');
  if(!dot || dot == filebase || (strcmp(dot, ".") == 0)) return NULL;
  return dot + 1;
}

char *getBasename(char *path){
    char *base = strrchr(path, '/');
    return base ? base+1 : path;
}

int isExcluded(char * ext) {
  if(ext == NULL) return 1;
  for (int i = 0; i < dim_arg_exlude; i++)
    if(strcmp(ext, arg_exlude[i]) == 0) return 1;
  return 0;
}

void printPtA(char (*arr)[MAXC], int n){
  for (int i = 0; i < n;) {
    printf ("'%s' ", arr[i]);
    i++;
    if(i<n) printf("| ");
  }
  printf("\n");
}

void rmFile(const char * filename) {
    char cmd[120];
    strcpy(cmd, "rm ");
    strcat(cmd, filename);
    system(cmd);
}

void freePtA(char (*ptr)[MAXC]) {
  free (ptr);
  ptr = NULL;
}

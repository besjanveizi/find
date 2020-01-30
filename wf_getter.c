#include "header.h"

/*****//////////////////////////*****/
/*****    words&files getters   *****/
/*****//////////////////////////*****/

static char *currentDir;

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
  char cwd[1024];
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

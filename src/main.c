#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include "header.h"

const char * currentDir;
char (*arg_exlude)[MAXC];
int dim_arg_exlude;
int verbose_flag;

int main (int argc, char **argv) {
  //current directory
  char cwd[1024];
  currentDir = getcwd(cwd, sizeof(cwd));

  char *arg_words = NULL;   //-w argument
  char *arg_input = NULL;   //-i argument
  char *arg_output = NULL;    //-o argument
  verbose_flag = 0;   //-v argument

  arg_exlude = NULL;    //-e argument (pointer to chars)
  dim_arg_exlude = 0;   //finale massimo #elementi di arg_exlude
  char *arg_report = NULL;    //-r argument
  char *arg_show = NULL;    //-s argument
  int oInput = 1;   //optional -s sub-argument
  char *arg_file = NULL;    //-f argument

  if (argc == 1) crediti();
  int c;
  int intit_arg_exlude = 5; //iniziale massimo #elementi di arg_exlude
  int i = 0;

  while (1)
    {
      static struct option long_options[] =
        {
          {"verbose",   no_argument,       &verbose_flag, 1},

          {"help",      no_argument,             0, 'h'},
          //-- Generazione Report--//
          {"words",     required_argument,       0, 'w'},
          {"input",     required_argument,       0, 'i'},
          {"output",    required_argument,       0, 'o'},
          {"exclude",   required_argument,       0, 'e'},
          //-- Analisi Report--//
          {"report",    required_argument,       0, 'r'},
          {"show",      required_argument,       0, 's'},
          {"file",      required_argument,       0, 'f'},
          {0, 0, 0, 0}
        };
      int option_index = 0;

      c = getopt_long_only (argc, argv, "hw:i:o:e:r:",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
        break;

      switch (c)
        {
        case 0:
            printf("Flag %s in set\n", long_options[option_index].name);
          break;

        case 'h':   //-help, --help, -h, --h
          printf("\033[H\033[J"); // pulisce la console
          help();
          break;

        case 'w':
          arg_words = optarg;   //parametro per file delle parole
          break;

        case 'i':
          arg_input = optarg;   // parametro per file dei percorsi
          break;

        case 'o':
          arg_output = optarg;    //parametro per file del report
          break;

        case 'e':
          if (!(arg_exlude = malloc (intit_arg_exlude * sizeof *arg_exlude))) {
              printf("\033[1;31m");printf("ERRORE:");printf("\033[0m");
              fprintf(stderr, " malloc() per arg_exlude in main.c:\n\t%s\n",
                strerror(errno));
              return EXIT_FAILURE;
          }
          for(optind-- ;optind < argc && *argv[optind] != '-'; optind++){
                //ogni argv[optind] rappresenta un estenzione da escludere
                if(argv[optind][0] == '.') strcpy(arg_exlude[i], argv[optind]+1);
                else strcpy(arg_exlude[i], argv[optind]);
                if (++i == intit_arg_exlude) { // realloc (*arg_exlude)
                    void *tmptr = realloc (arg_exlude, 2 * intit_arg_exlude * sizeof *arg_exlude);
                    if (!tmptr) {
                      printf("\033[1;31m");printf("ERRORE:");printf("\033[0m");
                      fprintf(stderr, " realloc() per arg_exlude in main.c:\n\t%s\n",
                        strerror(errno));
                      printf("\033[1;35m");
                      printf("Sono state registrate soltanto %d righe per arg_exlude\n", i);
                      printf("\033[0m");
                        break;  // non leggere più e mantieni ciò che hai letto
                    }
                    arg_exlude = tmptr;  //assegna il puntatore aggiornato ad arg_exlude
                    intit_arg_exlude *= 2; //aggiorna dimensione delle righe
                }
          }
          dim_arg_exlude = i;
          break;

        case 'r':
          arg_report = optarg;
          break;
        case 's':
          arg_show = optarg;
          break;
        case 'f':
          arg_file = optarg;
          break;

        case '?':
          /* getopt_long_only already printed an error message. */
          break;

        default:
          abort();
        }
    }

  // check non-option ARGV-elements for oInput for analisiListPaths()
  while (optind < argc)
    {
      char *end;
      int tmp = strtol(argv[optind], &end, 10);
      if (tmp>1) oInput=tmp;
      if(++optind < argc) continue;
      else break;
    }

    if(arg_words && arg_input) {    //generazione report
      char wordsFile[1024];
      if(!isAbsolute(arg_words))
        strcpy(wordsFile, getAbsolute(arg_words, currentDir));
      if(wordsFile[0] == '\0') return EXIT_FAILURE;

      char inputFile[1024];
      if(!isAbsolute(arg_input))
        strcpy(inputFile, getAbsolute(arg_input, currentDir));
      if(inputFile[0] == '\0') return EXIT_FAILURE;

      if(!generaReport(wordsFile, inputFile, arg_output))
        return EXIT_FAILURE;

      if (arg_exlude) {
        free(arg_exlude);
        arg_exlude = NULL;
      }
    }

    else if(arg_report && arg_show) {
      printf("\n-- ANALISI REPORT\n");
      char reportFile[1024];
      if(!isAbsolute(arg_report))
        strcpy(reportFile, getAbsolute(arg_report, currentDir));
      if(reportFile[0] == '\0') return EXIT_FAILURE;
      printf("Report filepath: '%s'\n", reportFile);

      FILE *fd = fopen (reportFile, "r"); // fopen report file
      if(!fd) {
          printf("\033[1;31m");printf("ERRORE:");printf("\033[0m");
          fprintf (stderr, " fopen(%s) in main.c:\n\t%s\n", reportFile,
            strerror(errno));
          return EXIT_FAILURE;
      }

      if(arg_file) {
        char selectedFile[1024];
        if(!isAbsolute(arg_file))
          strcpy(selectedFile, getAbsolute(arg_file, currentDir));
        if(selectedFile[0] == '\0') return EXIT_FAILURE;
        printf("\nLista posizioni dove la parola <%s> occorre nel file <%s>\n",
          arg_show, selectedFile);
        if(!analisiListOcc(reportFile, arg_show, selectedFile)) {
          fclose(fd);
          return EXIT_FAILURE;
        }
      }
      else {
        printf("\nLista dei file dove occorre almeno <%d> volte la parola <%s>\n",
          oInput, arg_show);
        if (!analisiListPaths(reportFile, arg_show, oInput)) {
            fclose(fd);
            return EXIT_FAILURE;
        }
      }
      fclose(fd);
    }

    else {
      printf("\033[1;31m");printf("ERRORE:");printf("\033[0m");
      printf(" sulla scelta delle opzioni, digitare -help per info sui comandi\n");
    }

    return EXIT_SUCCESS;
}

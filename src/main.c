#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>     // getopt_long_only()
#include <unistd.h>     // getcwd()
#include "find.h"
#include "report.h"     // generaReport(), analisiListPaths(), analisiListOcc()
#include "wf_getter.h"  //freePtP()

char * currentDir;
int size_cwd;
char **arg_exclude;
int dim_arg_exclude = 0;
int verbose_flag;
char *arg_words;
char *arg_input;
char *arg_output;
char *arg_report;
char *arg_show;
char *arg_file;
int oInput = 1;

static void help(void);
static void info(void);

int main (int argc, char **argv) {

    if (argc == 1) info();
    int c;

    while (1) {
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

        c = getopt_long_only (argc, argv, "hw:i:o:e:r:", long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
          break;

        switch (c) {
            case 0:
                printf("Flag %s is set\n", long_options[option_index].name);
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
                optind--;
                int start_optind = optind;
                int dim_ext;
                int i = 0;
                for (; optind < argc && *argv[optind] != '-'; optind++) {
                  dim_arg_exclude++;
                }
                if (!(arg_exclude = malloc (dim_arg_exclude * sizeof *arg_exclude))) {
                    printf("\033[1;31m");printf("ERRORE [main.c]:");printf("\033[0m");
                    printf(" fallimento allocazione per arg_exclude\n");
                    fprintf(stderr, "%s\n", strerror(errno));
                    return EXIT_FAILURE;
                }
                for(optind = start_optind ;i<dim_arg_exclude; optind++, i++){
                      //ogni argv[optind] rappresenta un estenzione da escludere
                      dim_ext = 1+strlen(argv[optind]);
                      arg_exclude[i] = malloc(dim_ext);
                      // elimina il . iniziale se viene inserito
                      if(argv[optind][0] == '.') strcpy(arg_exclude[i], argv[optind]+1);
                      else strcpy(arg_exclude[i], argv[optind]);
                }
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
                if (arg_exclude)
                  freePtP(arg_exclude, dim_arg_exclude);
              abort();
        }
    }

    // check non-option ARGV-elements for oInput for analisiListPaths()
    while (optind < argc) {
        char *end;
        int tmp = strtol(argv[optind], &end, 10);
        if (tmp>1) oInput=tmp;
        if(++optind < argc) continue;
        else break;
    }

    //current working directory
    size_cwd = 20;
    char *buf_cwd = malloc(size_cwd);
    currentDir = getcwd(buf_cwd, size_cwd);

    while (currentDir == NULL && errno == ERANGE) {
        size_cwd *= 2;
        buf_cwd = realloc(buf_cwd, size_cwd);
        currentDir = getcwd(buf_cwd, size_cwd);
    }

    if(arg_words && arg_input)   //generazione report
        if(!generaReport()) {
          if(arg_exclude) freePtP(arg_exclude, dim_arg_exclude);
          free(buf_cwd);
          buf_cwd = NULL;
          return EXIT_FAILURE;
        }
    if(arg_exclude) freePtP(arg_exclude, dim_arg_exclude);

    if(arg_report && arg_show) {
        printf("\n-- ANALISI REPORT\n");

        if(arg_file) {
          printf("\nLista posizioni dove la parola <%s> occorre nel file <%s>\n",
            arg_show, arg_file);
          if(!analisiListOcc()) {
            free(buf_cwd);
            buf_cwd = NULL;
            return EXIT_FAILURE;
          }
        }
        else {
          printf("\nLista dei file dove occorre almeno <%d> volte la parola <%s>\n",
            oInput, arg_show);
          if (!analisiListPaths()) {
            free(buf_cwd);
            buf_cwd = NULL;
            return EXIT_FAILURE;
          }
        }
    }

    else if (!(arg_words && arg_input) && !(arg_report && arg_show)){
      printf("\033[1;31m");printf("ERRORE:");printf("\033[0m");
      printf(" sulla scelta delle opzioni, digitare -help per info sui comandi\n");
    }
    free(buf_cwd);
    buf_cwd = NULL;
    return EXIT_SUCCESS;
}

void help() {

	printf("\n\t\t\t\t\t      [   H   E   L   P   ]\n\n\n");

	printf("\033[0;34m");
	printf("Elenco delle opzioni [ -brevi ] | [ --complete ]\n\n");
	printf("\033[0m");

	printf("Opzioni che");
	printf("\033[01;33m"); printf(" NON RICHIEDONO "); printf("\033[0m");
	printf("<argomento>:\n\n");

	printf("[ -h ] | [ --help ]                           : stampa questa schermata di aiuto\n\n");

	printf("Opzioni che");
	printf("\033[01;33m"); printf(" RICHIEDONO "); printf("\033[0m");
	printf("<argomento>:\n\n");

	printf("[ -w <filepath>   ] | [ --words   <filepath>   ]  : <filepath> è il file per le parole\n");
	printf("[ -i <filepath>   ] | [ --input   <filepath>   ]  : <filepath> è il file per i path\n");
	printf("[ -o <filepath>   ] | [ --output  <filepath>   ]  : <filepath> è il file di output del report\n");
	printf("[ -e <ext1> <ext2>] | [ --exclude <ext1> <ext2>]  : <ext1> <ext2> sono le estensioni dei file da escludere\n");
	printf("[ -r <filepath>   ] | [ --report  <filepath>   ]  : <filepath> è il file considerato per l'analisi del report\n");
  printf("[ -s <word> <n>   ] | [ --show    <word> <n>   ]  : seleziona <word> che occorre <n> volte (se omesso, n = 1)\n");
  printf("[ -f <filepath>   ] | [ --file    <filepath>   ]  : seleziona <word> che occorre nel file <filepath>\n");
  printf("\nFLAG: ");
  printf("[ -v ] | [ --verbose ]  : flag verbose\n\n");
  printf("\033[0;34m"); printf("Esempi di utilizzo dei comandi:\n\n"); printf("\033[0m");
  printf("1) Genera il report <out.txt> nella cartella ./result dalle parole contenute in <words.txt> sui file specificati\n");
  printf("   con path relativo o assoluto nel file <in.txt> escludendo file .pdf, .epub e .doc.\n");
  printf("   Mostra estensivamente il processo di analisi.\n");
  printf("\033[0;32m");
	printf("\t ./find --words words.txt --input ./in.txt --output result/out.txt --exclude pdf epub doc -verbose\n\n");
  printf("\033[0m");
  printf("2) Analizza il report contenuto nel file <out.txt> della cartella ./result, stampando la lista dei file dove\n");
  printf("   occorre almeno <3> volte la parola <aria>\n");
  printf("\033[0;32m");
	printf("\t ./find --report ./result/out.txt --show aria 3\n\n");
  printf("\033[0m");
  printf("3) Analizza il report contenuto nel file <out.txt> della cartella ./result, stampando tutte le posizioni\n");
  printf("   dove la parola <aria> occorre nel file <5maggio.txt> contenuto nella cartella ./files\n");
  printf("\033[0;32m");
  printf("\t ./find --report ./result/out.txt --show aria --file files/5maggio.txt\n");
  printf("\033[0m");
  printf("\n\nN.B.\n");
  printf("L'argomento <filepath> può essere sia relativo che assoluto.\n\n");
  exit(0);
}

void info() {
  printf("\033[01;36m");
	printf("\n\n\t\t\t\t\t  FIND PROGRAM\n\n"); printf("\033[0m");
	printf("- membri del gruppo :  Besjan Veizi, Mariano Sabatino\n");
	printf("\n");
	printf("- breve descrizione:\n");
	printf("\t\t   FIND e' un programma in grado di individuare il numero\n");
  printf("\t\t   di occorrenze di un insieme di stringhe all'interno di\n");
  printf("\t\t   un gruppo di file utilizzando l'Algoritmo di Knuth-Morris e Pratt.\n");
  printf("\t\t   Ad ogni esecuzione l'applicazione produrrà in output la lista\n");
  printf("\t\t   dei file analizzati con le occorrenze della stringa nel testo\n");
  printf("\t\t   insieme alle informazioni riguardante la posizioni della stesse.\n");
  printf("\t\t   Le stesse informazioni prodotte in output potranno essere salvate\n");
  printf("\t\t   su di un file esterno su cui si possono eseguire 2 operazioni di analisi:\n");
  printf("\t\t     - Stampare la lista dei file dove occorre almeno <n> volte la parola <word>\n");
  printf("\t\t     - Stampare tutte le posizioni dove la parola <word> occorre nel file <filepath>\n\n");
  printf("- digita"); printf("\033[01;33m"); printf("  ./find -h  "); printf("\033[0m");
  printf("per maggiori informazioni sui comandi.\n");
	exit(0);
}

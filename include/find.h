#ifndef FIND_H_
#define FIND_H_

extern char * currentDir;   // current working directory
extern int size_cwd;        // dimesione necessaria per allocare currentDir

extern char **arg_exclude;  // --exclude argument
extern int dim_arg_exclude; // #arguments of --exclude

extern int verbose_flag;    // --verbose argument

extern char *arg_words;     // --words argument
extern char *arg_input;     // --input argument
extern char *arg_output;    // --output argument

extern char *arg_report;    // --report argument
extern char *arg_show;      // --show argument
extern char *arg_file;      // --file argument
extern int oInput;          // optional --show sub-argument <n>

#endif

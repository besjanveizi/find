#ifndef FIND_H_
#define FIND_H_

extern char * currentDir;
extern int size_cwd;
extern char **arg_exclude;  //-e argument
extern int dim_arg_exclude;
extern int verbose_flag;  //-v argument

extern char *arg_words;   //-w argument
extern char *arg_input;   //-i argument
extern char *arg_output;  //-o argument

extern char *arg_report;  //-r argument
extern char *arg_show;    //-s argument
extern char *arg_file;    //-f argument
extern int oInput;        //optional -s sub-argument

#endif

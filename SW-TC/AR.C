/*
**   AR -- File Archiver
**
**   usage: ar -{dptux} arcfile [file...]
**
**   Ar collects text files into a single archive file.
**   Files can be extracted, new ones added,
**   old ones replaced or deleted, and
**   a list of the archive contents produced.
**
**   The first argument is a switch from the following set.
**   The second argument is the name of the archive file.
**   The third and subsequent arguments are file names.
**
**   -d  delete named files from the library.
**   -p  print named, or all, files on stdout.
**   -t  table of contents of named, or all, files to stdout.
**   -u  update the archive by adding/replacing files
**       (used to create a new library).
**       If no file names are specified in the command line,
**       they are obtained from stdin.
**   -x  extract named, or all, files.
**
**   Control-S to pause execution and control-C to abort.
**
**   This program was given as a class assignment in the
**   Computer Science Department at the University of Arizona.
**   It was contributed by Ernext Payne.  Orignially it was
**   written to work with tape archives, but this version has
**   been modified for higher speed operation with diskette
**   archives under CP/M.
*/

#include <stdio.h>
#include <ctype.h>

#define NAMESIZE 30
#define MAXLINE  500
#define MAXFILES 20
#define HDR      ">>>"
#define YES      1
#define NO       0

char tname[]="  ar.$$$";
char *fnptr[MAXFILES];
int fstatus[MAXFILES];
int nfiles;
int errchk;

void     acopy(FILE *, FILE *, int);
void     addfile(char *, FILE *);
void     amove(char *, char *);
void     cant(char *);
void     drop(char *);
void     error(char *);
void     extract(char *, char);
void     fcopy(FILE *, FILE *);
int      fmatch(char *, int);
unsigned fsize(FILE *);
void     fskip(FILE *, int);
void     getfns(int, char *[]);
int      gethdr(FILE *, char *, char *);
char *   getwrd(char *, char *);
FILE *   mustopen(char *, char *);
void     notfound(void);
void     replace(FILE *, FILE *, char);
void     table(char *);
void     update(char *);
int      same(char *, char *);
void     usage(void);
void     left(char *);
int      getarg(int, char *, int, int, char *[]);

#pragma warn -pia
#pragma warn -rvl

main(argc, argv) int argc; char *argv[]; {
  char cmd[3], aname[NAMESIZE];
  if(getarg(1,   cmd,        3, argc, argv) == EOF) usage();
  if(getarg(2, aname, NAMESIZE, argc, argv) == EOF) usage();
  if(aname[1] == ':') {
    tname[0] = aname[0];
    tname[1] = aname[1];
    }
  else left(tname);
  getfns(argc, argv);
  switch(toupper(cmd[1])) {
    case 'D': drop(aname);
              break;
    case 'T': table(aname);
              break;
    case 'U': update(aname);
              break;
    case 'X':
    case 'P': extract(aname, toupper(cmd[1]));
              break;
     default: usage();
    }
  }

/* acopy - copy size characters from fpi to fpo */
void
acopy(fpi, fpo, size) FILE *fpi, *fpo; int size; {
  int c;
  while((size--) > 0) {
    if((c = getc(fpi)) == EOF)
      break;
    putc(c, fpo);
    }
  }

/* addfile - add file "name" to archive */
void
addfile(name, fp) char *name; FILE *fp; {
  FILE *nfp;
  if((nfp = fopen(name, "r")) == NULL) {
    fprintf(stderr, "%s: can't open\n", name);
    errchk = 1;
    }
  if (errchk == 0) {
    if(name[1] == ':') name += 2;
    fprintf(fp, "%s %s %d\n", HDR, name, fsize(nfp));
    fcopy(nfp, fp);
    fclose(nfp);
    fprintf(stderr, " copied new %s\n", name);
    }
  }

/* amove - move file1 to file2 */
void
amove(file1, file2) char *file1, *file2; {
  if(errchk) {
    printf("fatal errors - archive not altered\n");
    unlink(file1);
    exit(7);
    }
  unlink(file2);
  if(file2[1] == ':') file2 += 2;
  if(rename(file1, file2)) {
    printf("can't rename %s to %s\n", file1, file2);
    exit(7);
    }
  }

/* cant - print file name and die */
void
cant(name) char *name; {
  fprintf(stderr, "%s: can't open\n", name);
  exit(7);
  }

/* drop - delete files from archive */
void
drop(aname) char *aname; {
  FILE *afp, *tfp;
  if(nfiles <= 0) /* protect innocents  */
    error("delete by name only");
  afp = mustopen(aname, "r");
  tfp = mustopen(tname, "w");
  replace(afp, tfp, 'd');
  notfound();
  fclose(afp);
  fclose(tfp);
  amove(tname, aname);
  }

/* error - print message and die */
void
error(msg) char *msg; {
  fprintf(stderr, "%s\n", msg);
  exit(7);
  }

/* extract - extract files from archive */
void
extract(aname, cmd) char *aname, cmd; {
  FILE *afp, *efp;
  char ename[NAMESIZE], in[MAXLINE];
  int size;
  afp = mustopen(aname, "r");
  if(cmd == 'P') efp = stdout;
  else           efp = NULL;
  while((size = gethdr(afp, in, ename)) >= 0)
    if(!fmatch(ename, YES)) fskip(afp, size);
    else {
      if(efp != stdout) efp = fopen(ename, "w");
      if(efp == NULL) {
        fprintf(stderr, "%s: can't create\n", ename);
        errchk = 1;
        fskip(afp, size);
        }
      else {
        acopy(afp, efp, size);
        if(cmd == 'P') fprintf(stderr, "printed %s\n", ename);
        else           fprintf(stderr, "created %s\n", ename);
        if(efp != stdout) fclose(efp);
        }
      }
  notfound();
  fclose(afp);
  }

/* fcopy - copy file in to file out */
void
fcopy(in, out) FILE *in, *out; {
  int c;
  while((c = getc(in)) != EOF) {
    putc(c, out);
    }
  }

/* fmatch - check if name matches argument list */
int
fmatch(name, quit) char *name; int quit; {
  int i, done;
  char *fnp;
  if(nfiles <= 0) return(1);
  done = YES;
  for(i=0;i<nfiles;++i) {
    fnp = fnptr[i];
    if(fnp[1] == ':') fnp += 2;
    if(same(name, fnp) == 0) {
      fstatus[i] = 1;
      return(1);
      }
    if(fstatus[i] == 0) done = NO;
    }
  if(quit && done) exit(0);
  return(0);
  }

/* fsize - size of file in characters */
unsigned
fsize(fp) FILE *fp; {
  unsigned i;
  for(i=0; getc(fp) != EOF; ++i) ;
  rewind(fp);
  return(i);
  }

/* fskip - skip n characters on file fp */
void
fskip(fp, n) FILE *fp; int n; {
  while((n--) > 0) {
    if(fgetc(fp) == EOF) break;
    }
  }

/* getfns - get file names into fname, check for duplicates */
void
getfns(argc, argv) int argc; char *argv[]; {
  int i, j;
  nfiles = argc - 3;
  if(nfiles > MAXFILES)
    error("too many file names");
  for(i=0, j=3; i < nfiles; i++, j++)
    fnptr[i] = argv[j];
  for(i = 0; i < nfiles-1; ++i)
    for(j = i + 1; j < nfiles; ++j) {
      if(same(fnptr[i], fnptr[j]) == 0) {
        fprintf(stderr,"%s:duplicate file names\n", fnptr[i]);
        exit(7);
        }
    }
  }

/* gethdr - get header info from fp */
int
gethdr(fp, buf, name) FILE *fp; char *buf, *name; {
  if(fgets(buf, MAXLINE, fp) == NULL)
    return(-1);
  buf = getwrd(buf, name);
  if(strcmp(name, HDR) != 0)
    error("archive not in proper format");
  buf = getwrd(buf, name);
  return(atoi(buf));
  }

/* getwrd - copy first word of s to t */
char *
getwrd(s, t) char *s, *t; {
  while(isspace(*s)) ++s;
  while(*s != '\0' && !isspace(*s))  *t++ = *s++;
  *t = '\0';
  return(s);
  }

/* mustopen - open file or die */
FILE *
mustopen(name, mode) char *name, *mode; {
  FILE *fp;
  if(fp = fopen(name, mode)) return (fp);
  cant(name);
  }

/* notfound - print "not found" message */
void
notfound() {
  int i;
  for(i=0;i<nfiles;++i)
    if(fstatus[i] == 0) {
      fprintf(stderr, "%s not in archive\n", fnptr[i]);
      errchk = 1;
    }
  }

/* replace - replace or delete files */
void
replace(afp, tfp, cmd) FILE *afp, *tfp; char cmd; {
  char in[MAXLINE], uname[NAMESIZE];
  int size;
  while((size = gethdr(afp, in, uname)) >= 0) {
    if(fmatch(uname, NO)) {
      if(cmd == 'u')    /* add new one */
        addfile(uname, tfp);
      fskip(afp, size);  /* discard old one */
      fprintf(stderr, "dropped old %s\n", uname);
      }
    else {
      fputs(in, tfp);
      acopy(afp, tfp, size);
      }
    }
  }

/* table - print table of archive contents  */
void
table(aname) char *aname; {
  char in[MAXLINE], lname[NAMESIZE];
  FILE *afp;
  int size;
  afp = mustopen(aname, "r");
  while((size = gethdr(afp, in, lname)) >= 0) {
    if(fmatch(lname, YES)) printf("%s\n", lname);
    fskip(afp, size);
    }
  notfound();
  fclose(afp);
  }

/* update - update existing files, add new ones at end */
void
update(aname) char *aname; {
  FILE *afp, *tfp;
  int i;
  char fn[NAMESIZE];
  if((afp = fopen(aname, "r+")) == NULL)
    /* maybe archive does not exist yet */
    afp = mustopen(aname, "w+");
  tfp = mustopen(tname, "w+");
  replace(afp, tfp, 'u');   /* update existing */
  if(nfiles > 0) {
    for(i=0;i<nfiles;++i) /* add new ones */
      if(fstatus[i] == 0) {
        addfile(fnptr[i], tfp);
        if(errchk) break;
        fstatus[i] = 1;
        }
    }
  else
    while(1) {
      if(isatty(fileno(stdin))) fprintf(stdout, "file - ");
      if(!fgets(fn, NAMESIZE, stdin) || *fn == '\n') break;
      for(i=0; fn[i]; i++)
        if((fn[i] = toupper(fn[i])) == '\n') fn[i] = NULL;
      addfile(&fn[0], tfp);
      if(errchk) break;
      }
  fclose(afp);
  fclose(tfp);
  amove(tname, aname);
  }

/* return <0, 0, >0 according to s<t, s=t, s>t*/
int
same(s, t) char *s, *t; {
  while(toupper(*s) == toupper(*t)) {
    if(toupper(*s) == 0) return (0);
    ++s; ++t;
    }
  return (toupper(*s) - toupper(*t));
  }

/* usage - abort with a usage message */
void
usage() {
  error("usage: ar -{dptux} arcfile [file...]");
  }

/*
** left -- left adjust and null terminate a string
*/
void
left(str) char *str; {
  char *str2;
  str2=str;
  while(*str2==' ') ++str2;
  while(*str++ = *str2++);
  }

/*
** Get command line argument. 
** Entry: n    = Number of the argument.
**        s    = Destination string pointer.
**        size = Size of destination string.
**        argc = Argument count from main().
**        argv = Argument vector(s) from main().
** Returns number of characters moved on success,
** else EOF.
*/
int
getarg(n, s, size, argc, argv)
  int n; char *s; int size, argc; char *argv[]; {
  char *str;
  int i;
  if(n < 0 || n >= argc) {
    *s = NULL;
    return EOF;
    }
  i = 0;
  str = argv[n];
  while(i < size) {
    if((s[i] = str[i]) == NULL) break;
    ++i;
    }
  s[i] = NULL;
  return i;
  }


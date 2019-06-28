/*
** Small-Tools definitions.
**
** The metacharacter definitions do not completely
** follow UNIX or the Software-Tools definitions.
**
** You may change anything to suit your preferences.
*/
#define MAXFN      30  /* max file name space */
#define EXTMARK    '.' /* file extension mark */
#define MAXLINE   192  /* max text line space */

#define CLEAR "\33[2J" /* screen erase, requires ANSI.SYS */
#define CRTWIDE    80  /* screen width */
#define CRTHIGH    24  /* screen height */

#define PTRWIDE    80  /* page width */
#define PTRHIGH    66  /* page height */
#define PTRSKIP     8  /* page perforation skips */
#define PTRHDR      2  /* page header lines */

#define MAXPAT    257  /* max pattern in internal format */
#define CHAR       'c' /* identifies a character */
#define BOL       '\"' /* beginning of line */
#define EOL       '\'' /* end of line */
#define ANY        '?' /* any character */
#define CCL        '[' /* begin character class */
#define NCCL       '~' /* negation of character class */
#define CCLEND     ']' /* end of character class */
#define CLOSURE    '*' /* zero or more occurrences */
#define DITTO      '^' /* whatever string matches pattern */
#define ESCAPE     ':' /* escape character */
#define NOT        '~' /* negation character */

#define DITCODE    -3  
#define COUNT       1
#define PREVCL      2
#define START       3
#define CLOSIZE     4


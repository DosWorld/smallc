/*
** sw.h -- Header File for Small-Windows Functions
**
**         Include in all Small-Windows programs.
*/
extern char     *files(char *);
extern void      fileset(int, int);

extern int       mfill(int, char *[], int, int);
extern void      mhide(int);
extern int       mopen(int, int, int, int, int, int, int, int);
extern int       mpick(int, char *[], int);
extern void      mreset(int);
extern void      mshow(int);
extern void      msort(char *[], int, int, int);

extern void      vatt(int, int, int, int, int, int);
extern void      vchr(char, int, int, int, int, int);
extern void      vchra(char, int, int, int, int, int, int);
extern void      vcursor(int, int);
extern void      vdrop(int, int, int, int, int, int);
extern void      vfast();
extern void      vgoto(int, int, int);
extern int       visat(int);
extern void      vlift(int, int, int, int, int, int);
extern int       vmode(int);
extern int       vpage(int);
extern int       vpoint(int, int, int, int, int, int, int);
extern void      vshow(char far *, int, int, int, int, int);
extern void      vslow();
extern void      vstow(char far *, int, int, int, int, int);
extern void      vstr(char *, int, int, int, int, int);
extern void      vstra(char *, int, int, int, int, int, int);

extern int       wactive();
extern void      wallow(int);
extern void      watt(int, int);
extern void      wauto(int, int);
extern void      wblank(int);
extern int       wchop(int);
extern void      wchr(char, int);
extern void      wchra(char, int, int);
extern void      wclean(int);
extern int       wcol();
extern int       wcsize();
extern void      wcursor(int);
extern void      wdrop(int);
extern void      wframe(int, int);
extern char      wgetc();
extern int       wgetf(char *, char *, int);
extern char     *wgets(char *, int);
extern int       wgo(int, int);
extern int       wgoto(int, int);
extern void      whide();
extern void      wlift(int);
extern int       wmode(int);
extern void      wmove(int, int);
extern void      wnl(int);
extern int       wok(int);
extern int       wopen(int, int, int, int, int);
extern int       wpage(int);
extern char far *wpoint();
extern void      wpop(int);
extern int       wprintf();
extern void      wpush(int, int);
extern char      wputc(char);
extern char      wputca(char, int);
extern void      wputs(char *);
extern void      wputsa(char *, int);
extern void      wsbuf(int, int);
extern int       wrow();
extern int       wscanf();
extern void      wsdis(int, void (*)());
extern int       wseen(int);
extern void      wselect(int);
extern void      wshow();
extern void      wstr(char *, int);
extern void      wstra(char *, int, int);
extern void      wsxow(void (*)(), char far *);

extern char     *itou(unsigned, char *, int);
extern void      itoab(unsigned, char *, int);
extern char     *ltou(unsigned long, char *, int);
extern int       poll(int);
extern int       utoi(char *, int *);


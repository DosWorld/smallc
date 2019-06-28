/*                                 LINKERIO.C                              */


/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               DOS_error                                 |
  |                                                                         |
  |                             O/S dependent                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void DOS_error(char_ptr format, ... )
BeginDeclarations
va_list                                argptr;
bit_16                                 error_number;
EndDeclarations
BeginCode
 If outregs.x.cflag
  Then
   error_number = outregs.x.ax;
   If error_number Exceeds 58
    Then
     error_number = 59;
    EndIf;
   fprintf(stdout,"\nDOS Error (Code %u): \"%s\"\n",
                  outregs.x.ax, DOS_error_text[error_number]);
   va_start(argptr,format);
   vfprintf(stdout,format,argptr);
   end_linker(16);
  EndIf;
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              DOS_int21                                  |
  |                                                                         |
  |                             O/S dependent                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void DOS_int21(char_ptr format, ... )
BeginDeclarations
va_list                                argptr;
bit_16                                 error_number;
EndDeclarations
BeginCode
 intdosx(Addr(inregs), Addr(outregs), Addr(segregs));
 If (outregs.x.cflag) AndIf
    (Not ((inregs.h.al  Is 0x41) AndIf (outregs.x.ax Is 2)))
  Then
   error_number = outregs.x.ax;
   If error_number Exceeds 58
    Then
     error_number = 59;
    EndIf;
   fprintf(stdout,"\nDOS Error (Code %u): \"%s\"\n",
                  outregs.x.ax, DOS_error_text[error_number]);
   va_start(argptr,format);
   vfprintf(stdout,format,argptr);
   end_linker(16);
  EndIf;
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           linker_error                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void linker_error(unsigned severity, char_ptr format, ...)
BeginDeclarations
va_list                                argptr;
EndDeclarations
BeginCode
 fprintf(stdout,"\nLinker Error (Severity %d)\n",severity);
 va_start(argptr,format);
 vfprintf(stdout,format,argptr);
 If severity Exceeds 7
  Then
   end_linker(severity);
  Else
   return;
  EndIf;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            linker_message                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void linker_message(char_ptr format, ...)
BeginDeclarations
va_list                                argptr;
static char                            flag = 0;
EndDeclarations
BeginCode
 if(!flag) { /* Issue Signon message */
  flag = -1;
  fprintf(stdout,"VAL 8086 linker - %s\n",__DATE__); }
 va_start(argptr,format);
 vfprintf(stdout,format,argptr);
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                 print                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void print(char_ptr format, ...)
BeginDeclarations
va_list                                argptr;
bit_16                                 len;
EndDeclarations
BeginCode
 va_start(argptr,format);
 vsprintf(CharPtr(object_file_element), format, argptr);
 len = strlen(CharPtr(object_file_element));
 If len IsZero
  Then
   return;
  EndIf;
 If object_file_element[len-1] Is '\n'
  Then
   object_file_element[len-1] = '\000';
   strcat(CharPtr(object_file_element), "\r\n");
   len++;
  EndIf;
 file_write(BytePtr(object_file_element), Bit_32(len));
 return;
EndCode

/*                                 FILES.C                                 */

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         add_extension_to_file                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr add_extension_to_file(string_ptr fn, string_ptr ext)
BeginDeclarations
EndDeclarations
BeginCode
 If index_string(fn,0,colon_string) IsNot 1
  Then  /* AUX:, CON:, or PRN: */
   return(fn);
  EndIf;
 If index_string(fn,0,dot_string) Is 0xFFFF
  Then
   concat_string(fn,ext);
  EndIf;
return(fn);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         add_files_to_list                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void add_files_to_list(file_info_list *file_list, string_ptr fn)
BeginDeclarations
bit_16                                 already_in_list;
byte_ptr                               matched_file;
bit_16                                 compare_len;
file_info_ptr                          file_entry;
#define File_entry                     (*file_entry)
#define File_list                      (*file_list)
bit_16                                 rc;
EndDeclarations
BeginCode
 matched_file = String(current_filename);
 rc = start_file_search(fn, 0);
 If rc IsNotZero
  Then
   linker_error(4,"No matching files for file specification:\n"
                  "\t\"%Fs\"\n",
                  String(fn));
  EndIf;
 While rc IsZero
  BeginWhile
   compare_len = Length(current_filename) + 1;
   already_in_list = 0;
   TraverseList(File_list, file_entry)
    BeginTraverse
     already_in_list = far_compare(matched_file,
                                   BytePtr(File_entry.filename),compare_len)
                       IsZero;
     ExitIf(already_in_list);
    EndTraverse;
   If Not already_in_list
    Then
     file_entry = (file_info_ptr)
                   allocate_memory(Addr(static_pool),
                                   Bit_32(sizeof(file_info_type)) +
                                   Bit_32(compare_len) - 1L);
     File_entry.attribute       = (*DTA).attribute;
     File_entry.time_stamp      = (*DTA).time_stamp;
     File_entry.date_stamp      = (*DTA).date_stamp;
     File_entry.file_size       = (*DTA).file_size;
     File_entry.pass_count      =
     File_entry.module_count    = 0;
     far_move(File_entry.filename, matched_file, compare_len);
     Insert file_entry AtEnd InList File_list EndInsert;
    EndIf;
   rc = continue_file_search();
  EndWhile;
 return;
EndCode
#undef File_entry
#undef File_list

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              change_extension                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr change_extension(string_ptr  fn, string_ptr ext)
BeginDeclarations
EndDeclarations
BeginCode
 trunc_string(fn, reverse_index_string(fn,0xFFFF,dot_string));
 concat_string(fn, ext);
 return(fn);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                       continue_file_search                              |
  |                                                                         |
  |                         O/S dependent                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 continue_file_search()
BeginDeclarations
bit_16                                 rc;
EndDeclarations
BeginCode
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         Set up a new DTA                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 old_DTA = get_DTA_address();
 set_DTA_address(DTA);

 inregs.h.ah = 0x4F;                   /* Continue file search */
 rc = (bit_16) intdos(Addr(inregs), Addr(outregs));
 set_DTA_address(old_DTA);
 If rc IsNotZero
  Then
   copy_string(current_filename, null_string);
  Else
   far_to_lower((*DTA).filename, 12);
   copy_string(current_filename, current_path);
   concat_string(current_filename, string((*DTA).filename));
  EndIf;
 return(rc);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                       default_directory                                 |
  |                                                                         |
  |                         O/S dependent                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr default_directory(string_ptr drive, string_ptr directory)
BeginDeclarations
EndDeclarations
BeginCode
 inregs.h.ah = 0x47;                   /* Get current directory*/
 inregs.h.dl = *String(drive) - 'a' + 1;
 inregs.x.si = Offset(String(directory)) + 1;
 segregs.ds  = Segment(String(directory));
 intdosx(Addr(inregs), Addr(outregs), Addr(segregs));
 *String(directory) = '\\';
 Length(directory) = far_index(String(directory), 0);
 far_to_lower(String(directory), Length(directory));
 If LastCharIn(directory) IsNot '\\'
  Then
   concat_string(directory,backslash_string);
  EndIf;
 return(directory);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         default_drive                                   |
  |                                                                         |
  |                         O/S dependent                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr default_drive()
BeginDeclarations
string_ptr                             drive;
EndDeclarations
BeginCode
 drive = make_constant_string(Addr(static_pool), (byte *) " :");
 inregs.h.ah = 0x19;                   /* Report current drive */
 intdosx(Addr(inregs), Addr(outregs), Addr(segregs));
/* DOS_int21("Failed to get current drive.\n");*/
 *String(drive) = (char) (outregs.h.al + 'a');
 return(drive);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          file_close_for_read                            |
  |                                                                         |
  |                           O/S dependent                                 |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void file_close_for_read()
BeginDeclarations
#define File                           infile
EndDeclarations
BeginCode
 If File.file_handle Exceeds 4
  Then  /* Only issue close if not one of the standard handles. */
   inregs.h.ah = 0x3E;                   /* Close for read */
   inregs.x.bx = File.file_handle;
   DOS_int21("Trouble closing \"%Fs\".\n",
             (*File.file_info).filename);
  EndIf;
 return;
EndCode
#undef File

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         file_close_for_write                            |
  |                                                                         |
  |                           O/S dependent                                 |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void file_close_for_write()
BeginDeclarations
#define File                           outfile
EndDeclarations
BeginCode
 If File.bytes_in_buffer Exceeds 0
  Then
   inregs.h.ah = 0x40;               /* Write */
   inregs.x.bx = File.file_handle;
   inregs.x.cx = File.bytes_in_buffer;
   inregs.x.dx = Offset(File.buffer);
   segregs.ds  = Segment(File.buffer);
   DOS_int21("Trouble writing file \"%Fs\" at byte %lu.\n",
             (*File.file_info).filename, File.next_buffer_position);
  EndIf;
 If File.file_handle Exceeds 4
  Then  /* Only issue close if not one of the standard handles. */
   inregs.h.ah = 0x3E;                   /* Close for read */
   inregs.x.bx = File.file_handle;
   DOS_int21("Trouble closing \"%Fs\".\n",
             (*File.file_info).filename);
  EndIf;
 return;
EndCode
#undef File

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          file_exists                                    |
  |                                                                         |
  |                         O/S dependent                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 file_exists(string_ptr fn, bit_16 attr)
BeginDeclarations
bit_16                                 rc;
EndDeclarations
BeginCode
 old_DTA = get_DTA_address();
 set_DTA_address(DTA);
 inregs.h.ah = 0x4E;                   /* Start file search */
 segregs.ds  = Segment(String(fn));
 inregs.x.dx = Offset(String(fn));
 inregs.x.cx = attr;
 rc = (bit_16) intdosx(Addr(inregs), Addr(outregs), Addr(segregs));
 set_DTA_address(old_DTA);
 return(rc IsZero);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              file_delete                                |
  |                                                                         |
  |                             O/S dependent                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void file_delete(file_info_ptr file_info)
BeginDeclarations
#define File_info                      (*file_info)
EndDeclarations
BeginCode
 inregs.h.ah = 0x41;                   /* Delete file */
 inregs.x.dx = Offset(File_info.filename);
 segregs.ds  = Segment(File_info.filename);
 DOS_int21("Trouble deleting file \"%Fs\".\n",
           File_info.filename);
 return;
EndCode
#undef File_info

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            file_IO_limit                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void file_IO_limit(bit_16 limit)
BeginDeclarations
#define File                           infile
EndDeclarations
BeginCode
 If (limit IsZero) OrIf (limit Exceeds File.buffer_size)
  Then
   File.IO_limit = File.buffer_size;
  Else
   File.IO_limit = limit;
  EndIf;
 return;
EndCode
#undef File

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         file_open_for_read                              |
  |                                                                         |
  |                           O/S dependent                                 |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void file_open_for_read(file_info_ptr file_info)
BeginDeclarations
#define File                           infile
#define File_info                      (*file_info)
EndDeclarations
BeginCode
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                    Initialize the data structure                        |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 File.current_byte               = File.buffer;
 File.IO_limit                   = File.buffer_size;
 File.start_of_buffer_position   =
 File.next_buffer_position       = 0L;
 File.bytes_in_buffer            =
 File.bytes_left_in_buffer       =
 File.byte_position              = 0;
 File.file_info                  = file_info;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                    Open the file and save the handle                    |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 If compare_string(string(File_info.filename), device_AUX) IsZero
  Then
   File.file_handle = 3;
   return;
  EndIf;
 If compare_string(string(File_info.filename), device_CON) IsZero
  Then
   File.file_handle = 0;
   return;
  EndIf;
 If compare_string(string(File_info.filename), device_PRN) IsZero
  Then
   File.file_handle = 4;
   return;
  EndIf;
 inregs.h.ah = 0x3D;                   /* Open for read */
 inregs.h.al = 0x00;                   /* Access code */
 inregs.x.dx = Offset(File_info.filename);
 segregs.ds  = Segment(File_info.filename);
 DOS_int21("Trouble opening \"%Fs\" for input.\n",
           File_info.filename);
 File.file_handle = outregs.x.ax;
 return;
EndCode
#undef File
#undef File_info

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         file_open_for_write                             |
  |                                                                         |
  |                           O/S dependent                                 |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void file_open_for_write(file_info_ptr file_info)
BeginDeclarations
#define File                           outfile
#define File_info                      (*file_info)
EndDeclarations
BeginCode
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                    Initialize the data structure                        |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 File.current_byte               = File.buffer;
 File.bytes_left_in_buffer       =
 File.IO_limit                   = File.buffer_size;
 File.start_of_buffer_position   =
 File.next_buffer_position       = 0L;
 File.bytes_in_buffer            =
 File.byte_position              = 0;
 File.file_info                  = file_info;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                    Open the file and save the handle                    |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 If compare_string(string(File_info.filename), device_AUX) IsZero
  Then
   File.file_handle = 3;
   return;
  EndIf;
 If compare_string(string(File_info.filename), device_CON) IsZero
  Then
   File.file_handle = 1;
   return;
  EndIf;
 If compare_string(string(File_info.filename), device_PRN) IsZero
  Then
   File.file_handle = 4;
   return;
  EndIf;
 inregs.h.ah = 0x3C;                   /* Open for write */
 inregs.x.cx = 0x00;                   /* File attribute */
 inregs.x.dx = Offset(File_info.filename);
 segregs.ds  = Segment(File_info.filename);
 DOS_int21("Trouble opening \"%Fs\" for output.\n",
           File_info.filename);
 File.file_handle = outregs.x.ax;
 return;
EndCode
#undef File
#undef File_info

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           file_position                                 |
  |                                                                         |
  |                           O/S dependent                                 |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void file_position(bit_32 position)
BeginDeclarations
#define File                           infile
EndDeclarations
BeginCode
 If (position NotLessThan File.start_of_buffer_position) AndIf
    (position LessThan    File.next_buffer_position)
  Then
   File.byte_position        = Bit_16(position-File.start_of_buffer_position);
   File.current_byte         = Addr(File.buffer[File.byte_position]);
   File.bytes_left_in_buffer = File.bytes_in_buffer - File.byte_position;
  Else
   inregs.h.ah = 0x42;                   /* Move file pointer */
   inregs.h.al = 0x00;                   /* Relative to start of file */
   inregs.x.bx = File.file_handle;
   inregs.x.cx = High(position);
   inregs.x.dx = Low(position);
   DOS_int21("Trouble positioning file \"%Fs\" to byte %lu.\n",
             (*File.file_info).filename, position);
   File.start_of_buffer_position   =
   File.next_buffer_position       = position;
   File.byte_position              =
   File.bytes_in_buffer            =
   File.bytes_left_in_buffer       = 0;
   File.current_byte               = File.buffer;
  EndIf;
 return;
EndCode
#undef File

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               file_read                                 |
  |                                                                         |
  |                             O/S dependent                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void file_read(byte_ptr into, bit_16 length)
BeginDeclarations
#define File                           infile
EndDeclarations
BeginCode
 While length Exceeds 0
  BeginWhile
   If length Exceeds File.bytes_left_in_buffer
    Then
     If File.bytes_left_in_buffer Exceeds 0
      Then
       far_move(into, File.current_byte, File.bytes_left_in_buffer);
       length -= File.bytes_left_in_buffer;
       into   += File.bytes_left_in_buffer;
      EndIf;
     inregs.h.ah = 0x3F;               /* Read */
     inregs.x.bx = File.file_handle;
     inregs.x.cx = File.IO_limit;
     inregs.x.dx = Offset(File.buffer);
     segregs.ds  = Segment(File.buffer);
     DOS_int21("Trouble reading file \"%Fs\" at byte %lu.\n",
               (*File.file_info).filename, File.next_buffer_position);
     File.bytes_in_buffer            =
     File.bytes_left_in_buffer       = outregs.x.ax;
     File.current_byte               = File.buffer;
     File.byte_position              = 0;
     File.start_of_buffer_position   = File.next_buffer_position;
     File.next_buffer_position       = File.start_of_buffer_position +
                                       File.bytes_in_buffer;
    Else
     far_move(into, File.current_byte, length);
     into                      += length;
     File.current_byte         += length;
     File.bytes_left_in_buffer -= length;
     File.byte_position        += length;
     length                     = 0;
    EndIf;
  EndWhile;
 return;
EndCode
#undef File

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              file_write                                 |
  |                                                                         |
  |                             O/S dependent                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void file_write(byte_ptr from, bit_32 length)
BeginDeclarations
#define File                           outfile
EndDeclarations
BeginCode
 While length Exceeds 0L
  BeginWhile
   If length Exceeds Bit_32(File.bytes_left_in_buffer)
    Then
     far_move(File.current_byte, from, File.bytes_left_in_buffer);
     length               -= Bit_32(File.bytes_left_in_buffer);
     from                 += File.bytes_left_in_buffer;
     File.bytes_in_buffer += File.bytes_left_in_buffer;
     inregs.h.ah = 0x40;               /* Write */
     inregs.x.bx = File.file_handle;
     inregs.x.cx = File.bytes_in_buffer;
     inregs.x.dx = Offset(File.buffer);
     segregs.ds  = Segment(File.buffer);
     DOS_int21("Trouble writing file \"%Fs\" at byte %lu.\n",
               (*File.file_info).filename, File.next_buffer_position);
     File.current_byte               = File.buffer;
     File.bytes_left_in_buffer       = File.buffer_size;
     File.bytes_in_buffer            =
     File.byte_position              = 0;
     File.start_of_buffer_position   = File.next_buffer_position;
     File.next_buffer_position       = File.start_of_buffer_position +
                                       File.bytes_left_in_buffer;
    Else
     far_move(File.current_byte, from, Bit_16(length));
     from                      += Bit_16(length);
     File.current_byte         += Bit_16(length);
     File.bytes_left_in_buffer -= Bit_16(length);
     File.byte_position        += Bit_16(length);
     File.bytes_in_buffer      += Bit_16(length);
     length                     = 0;
    EndIf;
  EndWhile;
 return;
EndCode
#undef File

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                        get_DTA_address                                  |
  |                                                                         |
  |                         O/S dependent                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
DTA_ptr get_DTA_address()
BeginDeclarations
DTA_ptr                                DTA_address;
EndDeclarations
BeginCode
 inregs.h.ah = 0x2F;                   /* Get DTA address */
 intdosx(Addr(inregs), Addr(outregs), Addr(segregs));
 DTA_address = (DTA_ptr) MakeFarPtr(segregs.es, outregs.x.bx);
 return(DTA_address);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                       process_filename                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
string_ptr process_filename(string_ptr fn)
BeginDeclarations
bit_16                                 left;
bit_16                                 right;
EndDeclarations
BeginCode
 lowercase_string(fn);
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                       Check for AUX:, CON: & PRN:                       |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 If compare_string(substr(fn,0,4), device_AUX) IsZero
  Then
   copy_string(fn, device_AUX);
   return(fn);
  EndIf;
 If compare_string(substr(fn,0,4), device_CON) IsZero
  Then
   copy_string(fn, device_CON);
   return(fn);
  EndIf;
 If compare_string(substr(fn,0,4), device_PRN) IsZero
  Then
   copy_string(fn, device_PRN);
   return(fn);               
  EndIf;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                      Add drive designator if missing.                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 If compare_string(substr(fn,1,1), colon_string) IsNotZero
  Then
   paste_string(fn, 0, default_drive_string);
  EndIf;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |          Substitute current directory if not based from root.           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 If compare_string(substr(fn,2,1), backslash_string) IsNotZero
  Then
   default_directory(fn, default_directory_string);
   paste_string(fn, 2, default_directory_string);
  EndIf;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |            Scan out all \. and \.. from filename.                       |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 left  = index_string(fn, -1, backslash_string);
 right = index_string(fn, left+1, backslash_string);
 While right IsNot 0xffff
  BeginWhile
   If compare_string(substr(fn,left,4), backslash_dot_dot_string) IsZero
    Then
     cut_string(fn, left, 3);
     right = left;
     left  = reverse_index_string(fn, right-1, backslash_string);
     If left Is 0xffff
      Then
       return(null_string);
      EndIf;
     cut_string(fn, left, right-left);
     right = index_string(fn, left+1, backslash_string);
     ContinueLoop;
    Else
     If compare_string(substr(fn,left,3), backslash_dot_string) IsZero
      Then
       cut_string(fn, left, 2);
       right = index_string(fn, left+1, backslash_string);
       ContinueLoop;
      EndIf;
    EndIf;
   left  = right;
   right = index_string(fn, left+1, backslash_string);
  EndWhile;
 return(fn);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                        set_DTA_address                                  |
  |                                                                         |
  |                         O/S dependent                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void set_DTA_address(DTA_ptr DTA_address)
BeginDeclarations
EndDeclarations
BeginCode
 inregs.h.ah = 0x1A;                   /* Set DTA address */
 segregs.ds  = Segment(DTA_address);
 inregs.x.dx = Offset(DTA_address);
 intdosx(Addr(inregs), Addr(outregs), Addr(segregs));
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                       start_file_search                                 |
  |                                                                         |
  |                         O/S dependent                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 start_file_search(string_ptr fn, bit_16 attr)
BeginDeclarations
bit_16                                 rc;
EndDeclarations
BeginCode
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         Set up a new DTA                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 old_DTA = get_DTA_address();
 set_DTA_address(DTA);

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                  Remember the current file path                         |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 copy_string(current_path, fn);
 trunc_string(current_path,
              reverse_index_string(current_path, 0xFFFF, backslash_string)+1);
 inregs.h.ah = 0x4E;                   /* Start file search */
 segregs.ds  = Segment(String(fn));
 inregs.x.dx = Offset(String(fn));
 inregs.x.cx = attr;
 rc = (bit_16) intdosx(Addr(inregs), Addr(outregs), Addr(segregs));
 set_DTA_address(old_DTA);
 If rc IsNotZero
  Then
   copy_string(current_filename, null_string);
  Else
   far_to_lower((*DTA).filename, 12);
   copy_string(current_filename, current_path);
   concat_string(current_filename, string((*DTA).filename));
  EndIf;
 return(rc);
EndCode

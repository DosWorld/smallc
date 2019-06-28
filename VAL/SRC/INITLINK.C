/*                                 INITLINK.C                              */

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                       primary_linker_initialization                     |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void primary_linker_initialization(byte *program_directory)
BeginDeclarations
EndDeclarations
BeginCode

 /* Note start time */
 linker_start_time = Now;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                     Memory Pool Initialization                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 initialize_memory();

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                     Constant String Initialization                      |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/* Uninitialized strings */

 current_filename         = allocate_string(Addr(static_pool), TEMP_STRING_LENGTH);
 current_path             = allocate_string(Addr(static_pool), TEMP_STRING_LENGTH);
 default_directory_string = allocate_string(Addr(static_pool), TEMP_STRING_LENGTH);
 default_filename         = allocate_string(Addr(static_pool), TEMP_STRING_LENGTH);
 help_filename            = allocate_string(Addr(static_pool), TEMP_STRING_LENGTH);
 next_token               = allocate_string(Addr(static_pool), TEMP_STRING_LENGTH);
 parm_string              = allocate_string(Addr(static_pool), TEMP_STRING_LENGTH);
 temp_string              = allocate_string(Addr(static_pool), TEMP_STRING_LENGTH);
 token                    = allocate_string(Addr(static_pool), TEMP_STRING_LENGTH);

/* Initialized strings */

 ampersand_string         = make_constant_string(Addr(static_pool),
                                                 (byte *) "&");
 and_string               = make_constant_string(Addr(static_pool),
                                                 (byte *) "and");
 at_string                = make_constant_string(Addr(static_pool),
                                                 (byte *) "@");
 backslash_string         = make_constant_string(Addr(static_pool),
                                                 (byte *) "\\");
 backslash_dot_string     = make_constant_string(Addr(static_pool),
                                                 (byte *) "\\.\\");
 backslash_dot_dot_string = make_constant_string(Addr(static_pool),
                                                 (byte *) "\\..\\");
 bar_string               = make_constant_string(Addr(static_pool),
                                                 (byte *) "|");
 class_string             = make_constant_string(Addr(static_pool),
                                                 (byte *) "class");
 close_angle_string       = make_constant_string(Addr(static_pool),
                                                 (byte *) ">");
 close_paren_string       = make_constant_string(Addr(static_pool),
                                                 (byte *) ")");
 colon_string             = make_constant_string(Addr(static_pool),
                                                 (byte *) ":");
 com_extension_string     = make_constant_string(Addr(static_pool),
                                                 (byte *) ".com");
 comma_string             = make_constant_string(Addr(static_pool),
                                                 (byte *) ",");
 default_drive_string     = default_drive();
 device_AUX               = make_constant_string(Addr(static_pool),
                                                 (byte *) "aux:");
 device_CON               = make_constant_string(Addr(static_pool),
                                                 (byte *) "con:");
 device_PRN               = make_constant_string(Addr(static_pool),
                                                 (byte *) "prn:");
 dot_string               = make_constant_string(Addr(static_pool),
                                                 (byte *) ".");
 env_extension_string     = make_constant_string(Addr(static_pool),
                                                 (byte *) ".env");
 exclamation_string       = make_constant_string(Addr(static_pool),
                                                 (byte *) "!");
 exe_extension_string     = make_constant_string(Addr(static_pool),
                                                 (byte *) ".exe");
 group_string             = make_constant_string(Addr(static_pool),
                                                 (byte *) "group");
 help_extension_string    = make_constant_string(Addr(static_pool),
                                                 (byte *) ".hlp");
 lib_extension_string     = make_constant_string(Addr(static_pool),
                                                 (byte *) ".lib");
 lst_extension_string     = make_constant_string(Addr(static_pool),
                                                 (byte *) ".map");
 minus_string             = make_constant_string(Addr(static_pool),
                                                 (byte *) "-");
 not_string               = make_constant_string(Addr(static_pool),
                                                 (byte *) "not");
 null_string              = make_constant_string(Addr(static_pool),
                                                 (byte *) "");
 obj_extension_string     = make_constant_string(Addr(static_pool),
                                                 (byte *) ".obj");
 open_paren_string        = make_constant_string(Addr(static_pool),
                                                 (byte *) "<");
 open_paren_string        = make_constant_string(Addr(static_pool),
                                                 (byte *) "(");
 or_string                = make_constant_string(Addr(static_pool),
                                                 (byte *) "or");
 plus_string              = make_constant_string(Addr(static_pool),
                                                 (byte *) "+");
 program_directory_string = make_constant_string(Addr(static_pool),
                                                 program_directory);
 segment_string           = make_constant_string(Addr(static_pool),
                                                 (byte *) "segment");
 semicolon_string         = make_constant_string(Addr(static_pool),
                                                 (byte *) ";");
 space_string             = make_constant_string(Addr(static_pool),
                                                 (byte *) " ");
 star_string              = make_constant_string(Addr(static_pool),
                                                 (byte *) "*");
 sys_extension_string     = make_constant_string(Addr(static_pool),
                                                 (byte *) ".sys");
 tilde_string             = make_constant_string(Addr(static_pool),
                                                 (byte *) "~");
 true_string              = make_constant_string(Addr(static_pool),
                                                 (byte *) "true");

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          DOS Initialization                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
  DTA = (DTA_ptr) allocate_memory(Addr(static_pool), 
                                  Bit_32(sizeof(DTA_type)));

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          Other Initialization                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 current_record_header     = (obj_record_header_ptr) object_file_element;
 last_LIDATA_record_header = (obj_record_header_ptr) last_LIDATA_record;
 copy_string(help_filename, program_directory_string);
 process_filename(help_filename);
 change_extension(help_filename, help_extension_string);
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                     secondary_linker_initialization                     |
  |                                                                         |
  +-------------------------------------------------------------------------+

There are several steps in the initialization process which had to be
delayed because switches specified by the user affect the initialization
process.  After user input is processed, this procedure is called and that
initialization occurs. */

void secondary_linker_initialization()
BeginDeclarations
EndDeclarations
BeginCode

 secondary_init_start_time = Now;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         Allocate the I/O buffer.                        |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 infile_buffer              = allocate_memory(Addr(static_pool),
                                              Bit_32(buffer_size.val));
 infile.buffer              = infile_buffer;
 infile.buffer_size         = buffer_size.val;

 outfile_buffer             = allocate_memory(Addr(static_pool),
                                              Bit_32(buffer_size.val));
 outfile.buffer             = outfile_buffer;
 outfile.buffer_size        = buffer_size.val;

 If temp_filename.val IsNull
  Then
   temp_file = (file_info_ptr)
                allocate_memory(Addr(static_pool),
                                Bit_32(sizeof(file_info_type)) + 5L);
   far_move((*temp_file).filename, BytePtr("l.tmp"), 6);
  Else
   copy_string(temp_filename.val, 
               substr(temp_filename.val, 1, Length(temp_filename.val)-2));
   temp_file = (file_info_ptr)
                allocate_memory(Addr(static_pool),
                                Bit_32(sizeof(file_info_type)) + 
                                Length(temp_filename.val));
   far_move((*temp_file).filename, 
            String(temp_filename.val),
            Length(temp_filename.val));
  EndIf;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           Allocate hash tables.                         |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 externals          = (public_entry_ptr_array)
                       allocate_memory(Addr(static_pool),
                                       (Bit_32(max_externals.val)+1L) *
                                        Bit_32(sizeof(group_entry_ptr)));

 gnames             = (group_entry_ptr_array)
                       allocate_memory(Addr(static_pool),
                                       (Bit_32(max_groups.val)+1L) *
                                        Bit_32(sizeof(group_entry_ptr)));

 group_hash_table   = (group_entry_ptr_array)
                       allocate_memory(Addr(static_pool),
                                       Bit_32(group_table_hash_size.val) *
                                        Bit_32(sizeof(group_entry_ptr)));

 lname_hash_table   = (lname_entry_ptr_array)
                       allocate_memory(Addr(static_pool),
                                       Bit_32(lname_table_hash_size.val) *
                                        Bit_32(sizeof(lname_entry_ptr)));

 lnames             = (lname_entry_ptr_array)
                       allocate_memory(Addr(static_pool),
                                       (Bit_32(max_lnames.val)+1L) *
                                        Bit_32(sizeof(lname_entry_ptr)));

 public_hash_table  = (public_entry_ptr_array)
                       allocate_memory(Addr(static_pool),
                                       Bit_32(public_table_hash_size.val) *
                                       Bit_32(sizeof(public_entry_ptr)));

 segment_hash_table = (segment_entry_ptr_array)
                       allocate_memory(Addr(static_pool),
                                       Bit_32(segment_table_hash_size.val) *
                                        Bit_32(sizeof(segment_entry_ptr)));

 snames             = (lseg_ptr_array)
                       allocate_memory(Addr(static_pool),
                                       (Bit_32(max_segments.val)+1L) *
                                        Bit_32(sizeof(lseg_ptr)));

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                  Miscellaneous Secondary Initialization                 |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 c_common_lname  = lookup_lname(8, (byte *) "c_common");
 generated_lname = lookup_lname(11 ,(byte *) "(generated)");
 none_lname      = lookup_lname(6,  (byte *) "(none)");
 If case_ignore.val IsTrue
  Then
   BSS_lname                = lookup_lname(3, (byte *) "bss");
   DGROUP_lname             = lookup_lname(6, (byte *) "dgroup");
   FAR_BSS_lname            = lookup_lname(7, (byte *) "far_bss");
   HUGE_BSS_lname           = lookup_lname(8, (byte *) "huge_bss");
   STACK_lname              = lookup_lname(5, (byte *) "stack");
   codeview_class_DEBSYM    = lookup_lname(6, (byte *) "debsym");
   codeview_class_DEBTYP    = lookup_lname(6, (byte *) "debtyp");
   codeview_segment_SYMBOLS = lookup_lname(9, (byte *) "$$symbols");
   codeview_segment_TYPES   = lookup_lname(7, (byte *) "$$types");
  Else
   BSS_lname                = lookup_lname(3, (byte *) "BSS");
   DGROUP_lname             = lookup_lname(6, (byte *) "DGROUP");
   FAR_BSS_lname            = lookup_lname(7, (byte *) "FAR_BSS");
   HUGE_BSS_lname           = lookup_lname(8, (byte *) "HUGE_BSS");
   STACK_lname              = lookup_lname(5, (byte *) "STACK");
   codeview_class_DEBSYM    = lookup_lname(6, (byte *) "DEBSYM");
   codeview_class_DEBTYP    = lookup_lname(6, (byte *) "DEBTYP");
   codeview_segment_SYMBOLS = lookup_lname(9, (byte *) "$$SYMBOLS");
   codeview_segment_TYPES   = lookup_lname(7, (byte *) "$$TYPES");
  EndIf;
 exefile = (comfile.val IsFalse) AndIf (sysfile.val IsFalse);
 return;
EndCode

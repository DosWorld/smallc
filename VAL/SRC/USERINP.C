/*                                 USERINP.C                               */

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                      get_filenames_from_user                            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void get_filenames_from_user(bit_16 argc, byte *argv[])
BeginDeclarations
byte                                  *env_var;
bit_16                                 i;
file_info_ptr                          file_entry;
#define File_entry                     (*file_entry)
token_stack_ptr                        source_element;
#define Source_element                 (*source_element)
EndDeclarations
BeginCode

 user_input_start_time = Now;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |           concatenate the environment files into parm_string            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 copy_string(parm_string, null_string);
 copy_string(default_filename, program_directory_string);
 process_filename(default_filename);
 change_extension(default_filename, env_extension_string);
 If file_exists(default_filename, 0)
  Then
   concat_string(parm_string, at_string);
   concat_string(parm_string, default_filename);
   concat_string(parm_string, space_string);
  EndIf;
 copy_string(token, default_filename);
 cut_string(token,
            0,
            reverse_index_string(token, 
                                 0xFFFF, 
                                 backslash_string) + 1);
 process_filename(token);
 If (file_exists(token, 0)) AndIf 
    (compare_string(token, default_filename) IsNotZero)
  Then
   concat_string(parm_string, at_string);
   concat_string(parm_string, token);
   concat_string(parm_string, space_string);
  EndIf;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |       concatenate the LINK environment variable into parm_string        |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 env_var = (byte *) getenv("LINK");
 If env_var IsNull
  Then
   env_var = (byte *) "";
  EndIf;
 concat_string(parm_string, string(BytePtr(env_var)));
 concat_string(parm_string, space_string);
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |               concatenate the parm line into parm_string                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 For i=1; i<argc; i++
  BeginFor
   If i Exceeds 1
    Then
     concat_string(parm_string, space_string);
    EndIf;
   concat_string(parm_string, string(BytePtr(argv[i])));
  EndFor;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |             Start input processing with the parm line.                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 source_element                    = get_free_token_source_element();
 Source_element.source_file        = Null;
 Source_element.token_string       = parm_string;
 Source_element.token_string_index = 0;
 Push source_element OnTo token_stack EndPush;
 token_break_char                  = ' ';

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                       Process OBJ file list                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 
 default_extension = obj_extension_string;
 copy_string(default_filename, null_string);
 default_prompt    = "OBJ file(s)%Fs:  ";
 prompting_for     = 1;
 process_user_input_files(Addr(obj_file_list),
                          True);

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         Process EXE file                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 If comfile.val IsTrue
  Then
   default_extension = com_extension_string;
   default_prompt = "COM file[%Fs]:  ";
  Else
   If sysfile.val IsTrue
    Then
     default_extension = sys_extension_string;
     default_prompt = "SYS file[%Fs]:  ";
    Else
     default_extension = exe_extension_string;
     default_prompt = "EXE file[%Fs]:  ";
    EndIf;
  EndIf;

 copy_string(default_filename, string((*(obj_file_list.first)).filename));
 change_extension(default_filename, default_extension);
 prompting_for = 2;
 process_user_output_file(Addr(exe_file_list),
                          False);

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         Process LST file                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 default_extension = lst_extension_string;
 copy_string(default_filename, null_string);
 default_prompt    = "LST file:%Fs  ";
 prompting_for     = 3;
 process_user_output_file(Addr(lst_file_list),
                          False);
 If (First(lst_file_list) IsNull)                 AndIf
    ((map.set IsTrue) OrIf (detail_level.val Exceeds 0))
  Then
   copy_string(token, string((*(exe_file_list.first)).filename));
   change_extension(token, lst_extension_string);
   file_entry = (file_info_ptr)
                 allocate_memory(Addr(static_pool),
                                 Bit_32(sizeof(file_info_type)) +
                                 Bit_32(Length(token)));
   far_move(File_entry.filename, String(token), Length(token)+1);
   Insert file_entry AtEnd InList lst_file_list EndInsert;
  EndIf;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                       Process LIB file list                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 default_extension = lib_extension_string;
 copy_string(default_filename, null_string);
 default_prompt    = "LIB file(s):%Fs  ";
 prompting_for     = 4;
 process_user_input_files(Addr(lib_file_list),
                          False);
 return;
EndCode
#undef File_entry
#undef Source_element

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                      process_user_input_files                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void process_user_input_files(file_info_list *list,
                              bit_16          required)
BeginDeclarations
#define List                           (*list)
EndDeclarations
BeginCode
 scan_out_token();
 Repeat
  BeginRepeat
   get_filename_token(required, list);
   If Length(token) Exceeds 0
    Then
     process_filename(token);
      If index_string(token, 0, colon_string) IsNot 1
       Then
        linker_error(4, "\"%Fs\" is an illegal input file name.\n",
                        String(token));
       Else
         add_extension_to_file(token,default_extension);
         add_files_to_list(list, token);
       EndIf;
    EndIf;
   RepeatIf more_tokens
  EndRepeat;
 return;
EndCode
#undef List

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                      process_user_output_file                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void process_user_output_file(file_info_list *list,
                              bit_16          required)
BeginDeclarations
file_info_ptr                          file_entry;
#define File_entry                     (*file_entry)
#define List                           (*list)
EndDeclarations
BeginCode
 scan_out_token();
 Repeat
  BeginRepeat
   get_filename_token(required, list);
   If Length(token) Exceeds 0
    Then
     process_filename(token);
     add_extension_to_file(token,default_extension);
     If List.first IsNull
      Then
       file_entry = (file_info_ptr)
                     allocate_memory(Addr(static_pool),
                                     Bit_32(sizeof(file_info_type)) +
                                     Bit_32(Length(token)));
       far_move(File_entry.filename, String(token), Length(token)+1);
       Insert file_entry AtEnd InList List EndInsert;
      EndIf;
    EndIf;
   RepeatIf more_tokens
  EndRepeat;
 return;
EndCode
#undef File_entry
#undef List

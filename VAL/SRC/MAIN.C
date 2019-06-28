/*                                  MAIN.C                                 */

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                 main                                    |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void main(bit_16 argc, byte *argv[])
BeginDeclarations
EndDeclarations
BeginCode
 primary_linker_initialization(argv[0]);   link_step++;
 get_filenames_from_user(argc, argv);      link_step++;
 secondary_linker_initialization();        link_step++;
 process_library_directories();            link_step++;
 process_object_modules();                 link_step++;
 process_libraries();                      link_step++;
 order_and_align_segments();               link_step++;
 pass_two();                               link_step++;
 write_executable_image();                 link_step++;
 link_map();
 file_delete(temp_file);
 end_linker(0);
EndCode

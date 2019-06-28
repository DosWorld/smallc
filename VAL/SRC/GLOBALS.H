/*                                 GLOBALS.H                               */

/* Global variables used by the linker in approximate alphabetical order.  */

bit_32                                 address_base;
bit_32                                 align_mask[6]=
                                        {0L, 0L, 1L, 15L, 255L, 3L};
boolean_switch_type                    align_exe_header = {True};
char                                  *align_text[6]=
                                        {"Absolute",
                                         "Byte",
                                         "Word",
                                         "Paragraph",
                                         "Page",
                                         "Dword"};
segment_entry_ptr                      active_segment;
#define Active_segment                 (*active_segment)
string_ptr                             ampersand_string;
string_ptr                             and_string;
string_ptr                             at_string;
string_ptr                             backslash_dot_string;
string_ptr                             backslash_dot_dot_string;
string_ptr                             backslash_string;
string_ptr                             bar_string;
lname_entry_ptr                        BSS_lname;
bit_16_switch_type                     buffer_size = 
                                        {0x8000, 512, 0xFE00, 0x8000, False};
lname_entry_ptr                        c_common_lname;
boolean_switch_type                    case_ignore = {True};
string_ptr                             class_string;
string_ptr                             close_paren_string;
string_ptr                             close_angle_string;
lname_entry_ptr                        codeview_class_DEBSYM;
lname_entry_ptr                        codeview_class_DEBTYP;
bit_16                                 codeview_information_present = False;
lname_entry_ptr                        codeview_segment_SYMBOLS;
lname_entry_ptr                        codeview_segment_TYPES;
string_ptr                             colon_string;
string_ptr                             com_extension_string;
char                                  *combine_text[10]=
                                        {"Private",
                                         "Undefined",
                                         "Public",
                                         "Undefined",
                                         "Public",
                                         "Stack",
                                         "Common",
                                         "Public"
                                         "Undefined",
                                         "Blank Common"};
boolean_switch_type                    comfile = {False};
string_ptr                             comma_string;
bit_16_switch_type                     CPARMAXALLOC =
                                        {0xFFFF, 0x0, 0xFFFF, 0xFFFF, False};
string_ptr                             current_filename;
string_ptr                             current_path;
obj_record_header_ptr                  current_record_header;
#define Current_record_header          (*current_record_header)
bit_32                                 current_record_offset;
string_ptr                             default_directory_string;
string_ptr                             default_drive_string;
string_ptr                             default_extension;
string_ptr                             default_filename;
char_ptr                               default_prompt;
bit_16_switch_type                     detail_level = 
                                        {0, 0, 5, 0, False};
string_ptr                             device_AUX;
string_ptr                             device_CON;
string_ptr                             device_PRN;
lname_entry_ptr                        DGROUP_lname;
boolean_switch_type                    DOSSEG = {False};
string_ptr                             dot_string;
DTA_ptr                                DTA;
pool_descriptor_type                   dynamic_pool;
segment_entry_ptr                      edata_segment = Null;
obj_ptr_type                           end_of_record;
obj_ptr_type                           end_of_last_LIDATA_record;
segment_entry_ptr                      end_segment = Null;
string_ptr                             env_extension_string;
string_ptr                             exclamation_string;
string_ptr                             exe_extension_string;
file_info_list                         exe_file_list = {Null, Null};
EXE_header_ptr                         exe_header;
#define Exe_header                     (*exe_header)
bit_32                                 exe_header_size;
bit_32                                 exec_image_start_time;
boolean_switch_type                    exechecksum = {False};
bit_16                                 exefile;
public_entry_list                      external_list = {Null, Null};
public_entry_ptr_array                 externals;
string_ptr                             false_string;
lname_entry_ptr                        FAR_BSS_lname;
public_entry_ptr                       far_communals = Null;
fixup_type                             fixup;
bit_16                                 FIXUPP_contains_only_threads;
bit_16                                 fixup_index;
bit_32                                 fixup_start_time;
bit_16                                 frame_absolute;
thread_type                            frame_thread[4];
pool_descriptor_type                   free_pool;
lname_entry_ptr                        generated_lname;
group_entry_ptr_array                  gnames;
group_entry_ptr_array                  group_hash_table;
group_entry_list                       group_list = {Null, Null};
bit_16_switch_type                     group_table_hash_size = 
                                          {25, 1, 1023, 25, False};
string_ptr                             group_string;
boolean_switch_type                    help;
string_ptr                             help_extension_string;
string_ptr                             help_filename;
bit_32                                 highest_uninitialized_byte;
lname_entry_ptr                        HUGE_BSS_lname;
public_entry_ptr                       huge_communals = Null;
file_type                              infile;
byte_ptr                               infile_buffer;
bit_16                                 initial_IP = 0;
bit_16                                 initial_CS = 0;
union REGS                             inregs;
segment_entry_ptr                      largest_stack_seg;
#define Largest_stack_seg              (*largest_stack_seg)
bit_32                                 largest_stack_seg_length;
lseg_ptr                               last_LxDATA_lseg;
#define last_LxDATA_Lseg               (*last_LxDATA_lseg)
bit_16                                 last_LxDATA_offset;
bit_8                                  last_LxDATA_record_type;
obj_record_header_ptr                  last_LIDATA_record_header;
#define Last_LIDATA_record_header      (*last_LIDATA_record_header)
bit_8                                  last_LIDATA_record[MAX_ELEMENT_SIZE];
string_ptr                             lib_extension_string;
file_info_list                         lib_file_list = {Null, Null};
bit_32                                 library_directory_start_time;
bit_32                                 library_processing_start_time;
bit_16                                 library_request_count = 0;
bit_16                                 LIDATA_index;
bit_16                                 LIDATA_offset;
bit_16                                 link_step = 0;
bit_32                                 linker_end_time;
bit_32                                 linker_start_time;
lname_entry_ptr_array                  lname_hash_table;
bit_16_switch_type                     lname_table_hash_size = 
                                        {25, 1, 1023, 25, False};
lname_entry_ptr_array                  lnames;
byte_ptr                               lseg_data_ptr;
string_ptr                             lst_extension_string;
file_info_list                         lst_file_list = {Null, Null};
bit_16_switch_type                     map =
                                        {2048, 1, 16384, 2048, False};
bit_32                                 map_start_time;
bit_16_switch_type                     max_externals = 
                                        {512, 1, 1024, 512, False};
bit_16_switch_type                     max_groups = 
                                        {32, 1, 1024, 32, False};
bit_16_switch_type                     max_lnames = 
                                        {512, 1, 1024, 512, False};
bit_16_switch_type                     max_segments = 
                                        {128, 1, 1024, 128, False};
string_ptr                             minus_string;
char                                  *mode_text[2]=
                                        {"Self-relative",
                                         "Segment-relative"};
bit_16                                 more_tokens;
bit_16                                 n_externals;
bit_16                                 n_groups;
bit_16                                 n_lnames;
bit_16                                 n_publics = 0;
bit_16                                 n_publics_to_sort;
bit_16                                 n_relocation_items = 0;
bit_16                                 n_segments;
public_entry_ptr                       near_communals = Null;
bit_32                                 next_available_address;
string_ptr                             next_token;
lname_entry_ptr                        none_lname;
string_ptr                             not_string;
string_ptr                             null_string;
string_ptr                             obj_extension_string;
file_info_list                         obj_file_list = {Null, Null};
obj_ptr_type                           obj_ptr;
boolean_switch_type                    objchecksum = {False};
byte                                   object_file_element[MAX_ELEMENT_SIZE];
bit_32                                 object_module_start_time;
DTA_ptr                                old_DTA;
string_ptr                             open_angle_string;
string_ptr                             open_paren_string;
string_ptr                             or_string;
byte_ptr                               order_expression_char_ptr;
bit_32                                 order_start_time;
text_switch_type                       ordering = {Null};
file_type                              outfile;
byte_ptr                               outfile_buffer;
union REGS                             outregs;
bit_16_switch_type                     pack_code =
                                        {0xFFFA, 1, 0xFFFF, 0xFFFA, False};
string_ptr                             parm_string;
boolean_switch_type                    pause = {False};
string_ptr                             plus_string;
string_ptr                             program_directory_string;
bit_16                                 prompting_for;
bit_16                                 prompt_next_stdin = False;
public_entry_ptr_array                 public_hash_table;
bit_16_switch_type                     public_table_hash_size =
                                        {525, 1, 1023, 525, False};
public_entry_ptr_array                 public_sort_array;
bit_32                                 secondary_init_start_time;
segment_entry_ptr_array                segment_hash_table;
segment_entry_list                     segment_list = {Null, Null};
string_ptr                             segment_string;
bit_16_switch_type                     segment_table_hash_size =
                                        {25, 1, 1023, 25, False};
segment_entry_list                     segments_ordered_list;
segment_entry_list                     segments_unordered_list;
struct SREGS                           segregs;
string_ptr                             semicolon_string;
lseg_ptr_array                         snames;
string_ptr                             space_string;
bit_16_switch_type                     stack =
                                        {0, 0, 0xFFFF, 0, False};
lname_entry_ptr                        STACK_lname;
bit_16                                 stack_segment_found = False;
string_ptr                             star_string;
fixup_type                             start_address;
bit_16                                 start_address_found = False;
pool_descriptor_type                   static_pool;
boolean_switch_type                    statistics = {False};
bit_32                                 statistics_start_time;
bit_32                                 step_time;
string_ptr                             sys_extension_string;
boolean_switch_type                    sysfile = {False};
thread_type                            target_thread[4];
file_info_ptr                          temp_file;
text_switch_type                       temp_filename = {Null};
byte                                   temp_near_string[TEMP_STRING_SIZE];
string_ptr                             temp_string;
temp_file_header_type                  temp_file_header;
string_ptr                             tilde_string;
char                                   time_array[16];
lname_entry_ptr                        tmodule_name;
bit_16                                 tmodule_number = 0;
string_ptr                             token;
byte                                   token_break_char;
bit_16                                 token_is_number;
bit_16                                 token_is_hex_number;
bit_16                                 token_numeric_value;
token_stack_list                       token_stack = {Null, Null};
token_stack_list                       token_stack_free_list = {Null, Null};
token_stack_ptr                        token_stack_member;
token_class_type                       token_type;
bit_32                                 total_time;
string_ptr                             true_string;
char                                  *type_text[5]=
                                        {"Low Byte",
                                         "Offset",
                                         "Base",
                                         "Pointer",
                                         "High Byte"};
bit_32                                 user_input_start_time;

char                                  *DOS_error_text[] =
{
                              "End of file",                         /* 00 */
                              "Invalid function",                    /* 01 */
                              "File not found",                      /* 02 */
                              "Path not found",                      /* 03 */
                              "No handles available",                /* 04 */
                              "Access denied",                       /* 05 */
                              "Invalid handle",                      /* 06 */
                              "Memory control blocks destroyed",     /* 07 */
                              "Insufficient memory",                 /* 08 */
                              "Invalid memory block address",        /* 09 */
                              "Invalid environment",                 /* 0A */
                              "Invalid format",                      /* 0B */
                              "Invalid access code",                 /* 0C */
                              "Invalid data",                        /* 0D */
                              "Reserved",                            /* 0E */
                              "Invalid drive",                       /* 0F */
                              "Attempt to remove current directory", /* 10 */
                              "Not the same device",                 /* 11 */
                              "No more file",                        /* 12 */
                              "Disk write-protected",                /* 13 */
                              "Unknown unit",                        /* 14 */
                              "Drive not ready",                     /* 15 */
                              "Unknown command",                     /* 16 */
                              "CRC error",                           /* 17 */
                              "Bad request structure length",        /* 18 */
                              "Seek error",                          /* 19 */
                              "Unknown media type",                  /* 1A */
                              "Sector not found",                    /* 1B */
                              "Out of paper",                        /* 1C */
                              "Write fault",                         /* 1D */
                              "Read fault",                          /* 1E */
                              "General failure",                     /* 1F */
                              "Sharing violation",                   /* 20 */
                              "Lock violation",                      /* 21 */
                              "Invalid disk change"                  /* 22 */
                              "FCB unavailable",                     /* 23 */
                              "Sharing buffer overflow",             /* 24 */
                              "Reserved",                            /* 25 */
                              "Reserved",                            /* 26 */
                              "Reserved",                            /* 27 */
                              "Reserved",                            /* 28 */
                              "Reserved",                            /* 29 */
                              "Reserved",                            /* 2A */
                              "Reserved",                            /* 2B */
                              "Reserved",                            /* 2C */
                              "Reserved",                            /* 2D */
                              "Reserved",                            /* 2E */
                              "Reserved",                            /* 2F */
                              "Reserved",                            /* 30 */
                              "Reserved",                            /* 31 */
                              "Network request not supported",       /* 32 */
                              "Remote computer not listening",       /* 33 */
                              "Duplicate name on network",           /* 34 */
                              "Network name not found",              /* 35 */
                              "Network busy",                        /* 36 */
                              "Network device no longer exists",     /* 37 */
                              "Net BIOS command limit exceeded",     /* 38 */
                              "Network adapter error",               /* 39 */
                              "Incorrect network response",          /* 3A */
                              "Unexpected network error",            /* 3B */
                              "Incompatible remote adapter",         /* 3C */
                              "Print queue full",                    /* 3D */
                              "Not enough space for print file",     /* 3E */
                              "Print file deleted",                  /* 3F */
                              "Network name deleted",                /* 40 */
                              "Access denied",                       /* 41 */
                              "Network device type incorrect",       /* 42 */
                              "Network name not found",              /* 43 */
                              "Network name limit exceeded",         /* 44 */
                              "Net BIOS session limit exceeded",     /* 45 */
                              "Temporarily paused",                  /* 46 */
                              "Network request not accepted",        /* 47 */
                              "Print or disk redirection is paused", /* 48 */
                              "Reserved",                            /* 49 */
                              "Reserved",                            /* 4A */
                              "Reserved",                            /* 4B */
                              "Reserved",                            /* 4C */
                              "Reserved",                            /* 4D */
                              "Reserved",                            /* 4E */
                              "Reserved",                            /* 4F */
                              "File already exists",                 /* 50 */
                              "Reserved",                            /* 51 */
                              "Cannot make directory",               /* 52 */
                              "Fail on Int 24",                      /* 53 */
                              "Too many redirections",               /* 54 */
                              "Duplicate redirection",               /* 55 */
                              "Invalid password",                    /* 56 */
                              "Invalid parameter",                   /* 57 */
                              "Network data fault",                  /* 58 */
                              "Invalid error code returned from DOS" /* 59 */
                             };

switch_table_type                      switch_table[] =
{
 {2, "ah",  "alignexeheader",  &align_exe_header,        scan_set_switch},
 {2, "bsz", "buffersize",      &buffer_size,             scan_bit_16_switch},
 {2, "ci",  "caseignore",      &case_ignore,             scan_set_switch},
 {2, "co",  "comfile",         &comfile,                 scan_set_switch},
 {2, "cp",  "cparmaxalloc",    &CPARMAXALLOC,            scan_bit_16_switch},
 {2, "det", "detaillevel",     &detail_level,            scan_bit_16_switch},
 {2, "do",  "dosseg",          &DOSSEG,                  scan_set_switch},
 {2, "ex",  "exechecksum",     &exechecksum,             scan_set_switch},
 {2, "ghs", "grouphashsize",   &group_table_hash_size,   scan_bit_16_switch},
 {2, "hlp", "help",            &help,                    scan_help_switch},
 {2, "lhs", "lnamehashsize",   &lname_table_hash_size,   scan_bit_16_switch},
 {3, "mp",  "map",             &map,                     scan_opt_bit_16},
 {4, "mex", "maxexternals",    &max_externals,           scan_bit_16_switch},
 {4, "mgp", "maxgroups",       &max_groups,              scan_bit_16_switch},
 {4, "mln", "maxlnames",       &max_lnames,              scan_bit_16_switch},
 {4, "msg", "maxsegments",     &max_segments,            scan_bit_16_switch},
 {4, "nah", "noalignexeheader",&align_exe_header,        scan_reset_switch},
 {4, "nci", "nocaseignore",    &case_ignore,             scan_reset_switch},
 {4, "nco", "nocomfile",       &comfile,                 scan_reset_switch},
 {4, "ndo", "nodosseg",        &DOSSEG,                  scan_reset_switch},
 {4, "nex", "noexechecksum",   &exechecksum,             scan_reset_switch},
 {4, "nmp", "nomap",           &map,                     scan_reset_bit_16},
 {4, "nob", "noobjchecksum",   &objchecksum,             scan_reset_switch},
 {5, "npc", "nopackcode",      &pack_code,               scan_reset_bit_16},
 {5, "npa", "nopause",         &pause,                   scan_reset_switch},
 {4, "nsy", "nosysfile",       &sysfile,                 scan_reset_switch},
 {4, "nst", "nostatistics",    &statistics,              scan_reset_switch},
 {2, "ob",  "objchecksum",     &objchecksum,             scan_set_switch},
 {2, "ord", "order",           &ordering,                scan_text_switch},
 {3, "pac", "packcode",        &pack_code,               scan_opt_bit_16},
 {3, "pa",  "pause",           &pause,                   scan_set_switch},
 {2, "phs", "publichashsize",  &public_table_hash_size,  scan_bit_16_switch},
 {2, "shs", "segmenthashsize", &segment_table_hash_size, scan_bit_16_switch},
 {4, "stk", "stack",           &stack,                   scan_bit_16_switch},
 {2, "sts", "statistics",      &statistics,              scan_set_switch},
 {2, "sy",  "sysfile",         &sysfile,                 scan_set_switch},
 {2, "tmp", "tempfile",        &temp_filename,           scan_text_switch},
 {0, Null,  Null,              Null,                     Null}
};

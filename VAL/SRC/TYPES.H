/*                                 TYPES.H                                 */

Type
 Structure acbp_struct                       acbp_type;
Type acbp_type                              *acbp_ptr;
Type
 Structure bit_16_switch_struct              bit_16_switch_type;
Type bit_16_switch_type                     *bit_16_switch_ptr;
Type
 Structure boolean_switch_struct             boolean_switch_type;
Type boolean_switch_type                    *boolean_switch_ptr;
Type
 Enumeration combine_enum                    combine_type;
Type
 Structure communal_entry_struct             communal_entry_type;
Type
 Structure DTA_struct                        DTA_type;
Type DTA_type far                           *DTA_ptr;
Type
 Union entry_information_union               entry_information_type;
Type
 Structure EXE_header_struct                 EXE_header_type;
Type EXE_header_type far                    *EXE_header_ptr;
Type
 Structure external_entry_struct             external_entry_type;
Type
 Structure file_info_struct                  file_info_type;
Type file_info_type far                     *file_info_ptr;
Type
 Structure file_struct                       file_type;
Type file_type                              *file_ptr;
Type
 Structure FIX_DAT_struct                    FIX_DAT_type;
Type FIX_DAT_type                           *FIX_DAT_ptr;
Type
 Structure fixup_struct                      fixup_type;
Type
 Structure group_entry_struct                group_entry_type;
Type group_entry_type far                   *group_entry_ptr;
Type group_entry_ptr far                    *group_entry_ptr_array;
Type
 Structure internal_entry_struct             internal_entry_type;
Type
 Structure library_directory_struct          library_directory_type;
Type library_directory_type                 *library_directory_ptr;
Type
 Structure library_entry_struct              library_entry_type;
Type
 Structure library_file_header_struct        library_file_header_type;
Type library_file_header_type               *library_file_header_ptr;
Type
 Structure library_symbol_entry_struct       library_symbol_entry_type;
Type library_symbol_entry_type              *library_symbol_entry_ptr;
Type
 Structure lname_entry_struct                lname_entry_type;
Type lname_entry_type far                   *lname_entry_ptr;
Type lname_entry_ptr far                    *lname_entry_ptr_array;
Type
 Enumeration loc_enum                        loc_type;
Type
 Structure LOCAT_struct                      LOCAT_type;
Type LOCAT_type                             *LOCAT_ptr;
Type
 Structure lseg_struct                       lseg_type;
Type lseg_type far                          *lseg_ptr;
Type lseg_ptr far                           *lseg_ptr_array;
Type
 Structure memory_descriptor_struct          memory_descriptor_type;
Type memory_descriptor_type far             *memory_descriptor_ptr;
Type
 Structure MOD_TYP_struct                    MOD_TYP_type;
Type MOD_TYP_type                           *MOD_TYP_ptr;
Type
 Union obj_ptr_union                         obj_ptr_type;
Type
 Structure obj_record_header_struct          obj_record_header_type;
Type obj_record_header_type                 *obj_record_header_ptr;
Type
 Enumeration obj_mod_rec_type_enum           object_module_record_types;
Type
 Enumeration public_entry_class_enum         public_entry_class_type;
Type
 Structure public_entry_struct               public_entry_type;
Type public_entry_type far                  *public_entry_ptr;
Type public_entry_ptr far                   *public_entry_ptr_array;
Type
 Structure pool_descriptor_struct            pool_descriptor_type;
Type pool_descriptor_type                   *pool_descriptor_ptr;
Type
 Structure segment_entry_struct              segment_entry_type;
Type segment_entry_type far                 *segment_entry_ptr;
Type segment_entry_ptr far                  *segment_entry_ptr_array;
Type
 Structure switch_table_struct               switch_table_type;
Type switch_table_type                      *switch_table_ptr;
Type
Structure temp_file_header_struct            temp_file_header_type;
Type
 Structure text_switch_struct                text_switch_type;
Type text_switch_type                       *text_switch_ptr;
Type
 Structure thread_struct                     thread_type;
Type
 Structure TRD_DAT_struct                    TRD_DAT_type;
Type TRD_DAT_type                           *TRD_DAT_ptr;
Type
 Enumeration token_class_enum                token_class_type;
Type
 Structure token_stack_struct                token_stack_type;
Type token_stack_type far                   *token_stack_ptr;

Structure acbp_struct
 BeginStructure
  unsigned                             p:1;
  unsigned                             b:1;
  unsigned                             c:3;
  unsigned                             a:3;
 EndStructure;

Enumeration align_enum
 BeginEnumeration
  absolute_segment,
  byte_aligned,
  word_aligned,
  paragraph_aligned,
  page_aligned,
  dword_aligned
 EndEnumeration;

Structure bit_16_switch_struct
 BeginStructure
  bit_16                               val;
  bit_16                               min;
  bit_16                               max;
  bit_16                               def;
  bit_16                               set;
 EndStructure;

Structure boolean_switch_struct
 BeginStructure
  bit_16                               val;
 EndStructure;

Enumeration combine_enum
 BeginEnumeration
  private_combine                      = 0,
  public_combine                       = 2,
  stack_combine                        = 5,
  common_combine                       = 6,
  blank_common_combine                 = 9
 EndEnumeration;

Structure DTA_struct
 BeginStructure
  byte                                 DOS_reserved[21];
  bit_8                                attribute;
  bit_16                               time_stamp;
  bit_16                               date_stamp;
  bit_32                               file_size;
  byte                                 filename[13];
  byte                                 filler[85];
 EndStructure;

Structure EXE_header_struct
 BeginStructure
  bit_16                               signature;
  bit_16                               image_length_MOD_512;
  bit_16                               image_length_DIV_512;
  bit_16                               n_relocation_items;
  bit_16                               n_header_paragraphs;
  bit_16                               min_paragraphs_above;
  bit_16                               max_paragraphs_above;
  bit_16                               initial_SS;
  bit_16                               initial_SP;
  bit_16                               checksum;
  bit_16                               initial_IP;
  bit_16                               initial_CS;
  bit_16                               offset_to_relocation_table;
  bit_16                               overlay;
  bit_16                               always_one;
  bit_32                               relocation_table[1];
 EndStructure;

Structure external_entry_struct
 BeginStructure
  bit_16                               reserved;
 EndStructure;

Structure internal_entry_struct
 BeginStructure
  group_entry_ptr                      group;
  lseg_ptr                             lseg;
  bit_16                               frame;
  bit_16                               offset;
 EndStructure;

Structure library_entry_struct
 BeginStructure
  file_info_ptr                        lib_file;
  bit_16                               page;
  bit_8                                requested;
 EndStructure;

Structure communal_entry_struct
 BeginStructure
  public_entry_ptr                     next_communal;
  bit_32                               element_size;
  bit_32                               element_count;
 EndStructure;

Union entry_information_union
 BeginUnion
  library_entry_type                   library_type;
  external_entry_type                  external_type;
  internal_entry_type                  internal_type;
  communal_entry_type                  communal_type;
 EndUnion;

Enumeration public_entry_class_enum
 BeginEnumeration
  unused,
  public_in_library,
  external,
  internal,
  far_communal,
  near_communal
 EndEnumeration;

Structure public_entry_struct
 BeginStructure
  public_entry_ptr                     next;
  public_entry_ptr                     next_congruent;
  bit_8                                type_entry;
  entry_information_type               entry;
  bit_16                               module;
  bit_16                               length;
  byte                                 symbol[1];
 EndStructure;
#define Library                        entry.library_type
#define External                       entry.external_type
#define Internal                       entry.internal_type
#define Communal                       entry.communal_type

ListTypeOf(public_entry);

Structure file_info_struct
 BeginStructure
  file_info_ptr                        next;
  bit_8                                attribute;
  bit_16                               time_stamp;
  bit_16                               date_stamp;
  bit_32                               file_size;
  bit_16                               page_size;
  bit_16                               request_count;
  bit_16                               pass_count;
  bit_16                               module_count;
  public_entry_list                    external_list;
  byte                                 filename[1];
 EndStructure;

ListTypeOf(file_info);

Structure file_struct
 BeginStructure
  file_info_ptr                        file_info;
  bit_16                               file_handle;
  bit_32                               start_of_buffer_position;
  bit_32                               next_buffer_position;
  byte_ptr                             buffer;
  byte_ptr                             current_byte;
  bit_16                               buffer_size;
  bit_16                               IO_limit;
  bit_16                               bytes_in_buffer;
  bit_16                               bytes_left_in_buffer;
  bit_16                               byte_position;
 EndStructure;

Structure FIX_DAT_struct
 BeginStructure
  unsigned                             targt:2;
  unsigned                             p:1;
  unsigned                             t:1;
  unsigned                             frame:3;
  unsigned                             f:1;
 EndStructure;

Structure fixup_struct
 BeginStructure
  void far                            *frame_referent;
  void far                            *target_referent;
  bit_16                               target_offset;
  unsigned                             location_type:3;
  unsigned                             mode:1;
  unsigned                             frame_method:3;
  unsigned                             target_method:2;
  unsigned                             external_error_detected:1;
  unsigned                             reserved:6;
 EndStructure;

Structure group_entry_struct
 BeginStructure
  group_entry_ptr                      next;
  group_entry_ptr                      next_congruent;
  lname_entry_ptr                      group_name;
  segment_entry_ptr                    first_segment;
 EndStructure;

ListTypeOf(group_entry);

Structure library_directory_struct
 BeginStructure
  bit_8                                offset_to_symbol[38];
  byte                                 symbol_area[474];
 EndStructure;

Structure library_file_header_struct
 BeginStructure
  bit_8                                flag;
  bit_16                               page_size;
  bit_32                               directory_position;
  bit_16                               n_directory_blocks;
 EndStructure;

 Structure library_symbol_entry_struct
  BeginStructure
   bit_8                               length_of_symbol;
   byte                                symbol[1];
  EndStructure;

Structure lname_entry_struct
 BeginStructure
  lname_entry_ptr                      next_congruent;
  bit_16                               lname_checksum;
  bit_16                               length;
  byte                                 symbol[1];
 EndStructure;

Structure LOCAT_struct
 BeginStructure
  unsigned                             data_record_offset:10;
  unsigned                             loc:3;
  unsigned                             s:1;
  unsigned                             m:1;
  unsigned                             type_fixupp_record:1;
 EndStructure;

Enumeration loc_enum
 BeginEnumeration
  lobyte_location,
  offset_location,
  base_location,
  pointer_location,
  hibyte_location
 EndEnumeration;

Structure lseg_struct
 BeginStructure
  lseg_ptr                             next;
  segment_entry_ptr                    segment;
  lname_entry_ptr                      tmodule;
  file_info_ptr                        file;
  bit_32                               address;
  bit_32                               length;
  bit_32                               highest_uninitialized_byte;
  bit_8                                align;
  byte_ptr                             data;
 EndStructure;

ListTypeOf(lseg);

Structure memory_descriptor_struct
 BeginStructure
  memory_descriptor_ptr                next;
  byte_ptr                             chunk;
  bit_32                               available;
  bit_32                               size;
  byte_ptr                             unused_base;
 EndStructure;

Structure MOD_TYP_struct
 BeginStructure
  unsigned                             l:1;
  unsigned                             zeros:5;
  unsigned                             mattr:2;
 EndStructure;

Union obj_ptr_union
 BeginUnion
  bit_8                               *b8;
  bit_16                              *b16;
  bit_32                              *b32;
  acbp_ptr                             acbp;
  FIX_DAT_ptr                          FIX_DAT;
  LOCAT_ptr                            LOCAT;
  MOD_TYP_ptr                          MOD_TYP;
  TRD_DAT_ptr                          TRD_DAT;
 EndUnion;

Enumeration obj_mod_rec_type_enum
 BeginEnumeration
  THEADR_record                        = 0x80,
  COMENT_record                        = 0x88,
  MODEND_record                        = 0x8a,
  EXTDEF_record                        = 0x8c,
  TYPDEF_record                        = 0x8e,
  PUBDEF_record                        = 0x90,
  LINNUM_record                        = 0x94,
  LNAMES_record                        = 0x96,
  SEGDEF_record                        = 0x98,
  GRPDEF_record                        = 0x9a,
  FIXUPP_record                        = 0x9c,
  LEDATA_record                        = 0xa0,
  LIDATA_record                        = 0xa2,
  COMDEF_record                        = 0xb0,
  FORREF_record                        = 0xb2,
  MODEXT_record                        = 0xb4,
  MODPUB_record                        = 0xb6
 EndEnumeration;

Structure obj_record_header_struct
 BeginStructure
  bit_8                                rec_typ;
  bit_16                               rec_len;
  byte                                 variant_part[1];
 EndStructure;

Structure pool_descriptor_struct
 BeginStructure
  memory_descriptor_ptr                memory_descriptor_list;
  char_ptr                             pool_id;
  bit_32                               pool_size;
  bit_32                               used_bytes;
 EndStructure;

Structure segment_entry_struct
 BeginStructure
  segment_entry_ptr                    next;
  segment_entry_ptr                    next_congruent;
  lname_entry_ptr                      segment_name;
  lname_entry_ptr                      class_name;
  bit_8                                combine;
  bit_32                               address;
  bit_32                               length;
  bit_32                               highest_uninitialized_byte;
  lseg_list                            lsegs;
  group_entry_ptr                      owning_group;
 EndStructure;

ListTypeOf(segment_entry);

Structure switch_table_struct
 BeginStructure
  bit_8                                min_length;
  char                                *abbr_name;
  char                                *full_name;
  void                                *affected_thing;
  void                  (*switch_processor)(switch_table_ptr current_switch);
 EndStructure;

Structure temp_file_header_struct
 BeginStructure
  bit_8                                rec_typ;
  bit_16                               rec_len;
  lseg_ptr                             lseg;
  bit_16                               offset;
 EndStructure;

Structure text_switch_struct
 BeginStructure
  string_ptr                           val;
 EndStructure;

Structure thread_struct
 BeginStructure
  bit_8                                method;
  void far                            *referent;
  bit_8                                thread_defined;
 EndStructure;

Enumeration token_class_enum
 BeginEnumeration
  continuation_token_type,
  end_of_command_line_token_type,
  filename_token_type,
  indirect_file_token_type,
  line_end_token_type,
  number_token_type,
  separator_token_type,
  switch_token_type,
  switch_end_token_type,
  text_token_type,
  terminator_token_type
 EndEnumeration;

Structure token_stack_struct
 BeginStructure
  token_stack_ptr                      next;
  FILE                                *source_file;
  byte                                 break_char;
  string_ptr                           token_string;
  bit_16                               token_string_index;
 EndStructure;

ListTypeOf(token_stack);

Structure TRD_DAT_struct
 BeginStructure
  unsigned                             thred:2;
  unsigned                             method:3;
  unsigned                             z:1;
  unsigned                             d:1;
  unsigned                             type_fixupp_record:1;
 EndStructure;

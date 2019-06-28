/*                                 MEMORY.C                                */

/* Memory management presents a challenge if one desires to be compatible
   with OS/2.  The problem centers on the fact that memory references
   (e.g., a memory address) differ between the real (8086) and protected
   (80286) modes of operation.  In the real mode, a memory reference is
   is via a "segment/offset" while in the protected mode, a memory reference
   is via a "descriptor/offset".  Both are 32-bit items with the 16-bit
   offset working the same in both.  However, segments and descriptors are
   not necessarily the same.  If compatibility with OS/2 and DOS is an
   objective, then one must not use a descriptor as if it were a segment.
   It is however safe to use a segment as if it were a descriptor.  In
   effect, this means you cannot treat the segment as if it is related to
   a memory address by a factor of 16.  A segment must be treated as if
   were a descriptor.  That is, a segment is treated as a reference (not
   necessarily an address) to the beginning of an area of memory which can
   be up to 64K.  Each byte within the area of memory is accessed via the
   offset mechanism. */

/* The data structure for memory is handled as follows:  At the beginning
   of execution, all available far memory is allocated in 64K chunks, with
   a "memory descriptor" for each chunk.  The memory desciptors for these
   chunks are linked together into memory pools.  Initially, all descriptors
   are linked to the free pool.  As a chunk is needed, it is allocated to
   a pool.  Some pools are needed only for a brief time.  In these cases,
   when a chunk is no longer needed it is returned to the free pool. */


/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         add_chunk_to_pool                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

memory_descriptor_ptr add_chunk_to_pool(pool_descriptor_ptr pool,
                                        bit_32 size)
BeginDeclarations
#define Pool                           (*pool)
memory_descriptor_ptr                  desc;
#define Desc                           (*desc)
memory_descriptor_ptr                  prior;
#define Prior                          (*prior)
EndDeclarations
BeginCode
 prior = Null;
 desc  = free_pool.memory_descriptor_list;
 While desc IsNotNull
  BeginWhile
   If Desc.available NotLessThan size
    Then
     If prior IsNull
      Then
       free_pool.memory_descriptor_list  = Desc.next;
      Else
       Prior.next                        = Desc.next;
      EndIf;
     free_pool.pool_size              -= Desc.available;
     Pool.pool_size                   += Desc.available;
     Desc.next                         = Pool.memory_descriptor_list;
     Pool.memory_descriptor_list       = desc;
     far_set(Desc.chunk, 0, Bit_16(Desc.available));
     return(desc);
    EndIf;
   prior = desc;
   Next(desc);
  EndWhile;
 linker_error (8,"No more free far memory for pool \"%s\"\n."
                 "\tTry running again with smaller buffersize and/or\n"
                 "\tvirtualized fixup processing.\n",
                 Pool.pool_id);
 return(desc); /* To kill warning */
EndCode
#undef Pool
#undef Desc
#undef Prior

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            allocate_memory                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

byte_ptr allocate_memory (pool_descriptor_ptr pool, bit_32 size)
BeginDeclarations
#define Pool                           (*pool)
#define Desc                           (*desc)
#define Prev                           (*prev)
memory_descriptor_ptr                  desc;
memory_descriptor_ptr                  prev;
byte_ptr                               element;
EndDeclarations
BeginCode

/* Traverse the memory descriptor list and allocate the space from the
   first memory descriptor which has enough available space for the entire
   element to be allocated. */

 prev = NULL;
 desc = Pool.memory_descriptor_list;
 While desc IsNotNull
  BeginWhile
   If size NotGreaterThan Desc.available
    Then /* We can take the element from this chunk */
     Pool.used_bytes         += size;
     element                  = Desc.unused_base;
     Desc.unused_base        += Bit_16(size);
     Desc.available          -= size;
     If prev IsNotNull
      Then
       Prev.next                   = Desc.next;
       Desc.next                   = Pool.memory_descriptor_list;
       Pool.memory_descriptor_list = desc;
      EndIf;
     return(element);
    EndIf;
   prev = desc;
   Next(desc);
  EndWhile;

/* Well, none of the memory descriptors had enough space to allocate this
   element.  So, we will have to get another chunk, add it to this pool and
   allocate the space from the new chunk. */

 desc                         = add_chunk_to_pool(pool, size);
 Pool.used_bytes             += size;
 element                      = Desc.unused_base;
 Desc.unused_base            += Bit_16(size);
 Desc.available              -= size;
 return(element);
EndCode
#undef Pool
#undef Desc
#undef Prev

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           initialize_memory                             |
  |                                                                         |
  |                             O/S dependent                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

void initialize_memory()
BeginDeclarations
byte huge                             *far_memory;
bit_32                                 far_memory_size;
bit_32                                 size_this_chunk;
bit_16                                 rc;
memory_descriptor_ptr                  desc;
#define Desc                           (*desc)
EndDeclarations
BeginCode
/* First step for PC-DOS is to gobble all free paragraphs above the heap.
   The variable "far_memory" will point to that area. */

 inregs.h.ah = 0x48;                     /* DOS allocate memory */
 inregs.x.bx = 0xFFFF;                   /* Ask for too much memory */
 rc = intdos(Addr(inregs),Addr(outregs));/* Do the allocate expecting to fail*/
 If (outregs.x.cflag IsFalse) OrIf (rc IsNot 8)
  Then
   DOS_error("Problem allocating memory above heap.\n");
  EndIf;
 inregs.x.bx = outregs.x.bx;             /* Ask for the right amount this time*/
 DOS_int21("Problem allocating memory above heap.\n");

 far_memory      = (byte huge *) MakeFarPtr(outregs.x.ax,0);
 far_memory_size = Bit_32(inregs.x.bx) ShiftedLeft 4;
 If far_memory_size LessThan 65536L
  Then
   linker_error(8,"Too little memory above heap.\n"
                 "\tTry running again with smaller buffersize and/or\n"
                 "\tvirtualized fixup processing.\n");
  EndIf;

/* Initialize the static pool with it's first chunk. This is a little
   tricky because the initial value of several data structures are
   initialized simutaneously. */

 far_set(BytePtr(far_memory), 0, 0);  /* Clear initial chunk */
 desc                                 = (memory_descriptor_ptr) far_memory;
 static_pool.memory_descriptor_list   = desc;
 static_pool.used_bytes               = sizeof(memory_descriptor_type);
 static_pool.pool_size                =
 Desc.size                            =
 Desc.available                       = 65536L;
 Desc.chunk                           =
 Desc.unused_base                     = BytePtr(far_memory);
 Desc.next                            = NULL;
 far_memory                          += Desc.available;
 far_memory_size                     -= Desc.available;
 Desc.available                      -= sizeof(memory_descriptor_type);
 Desc.unused_base                    += sizeof(memory_descriptor_type);

/* Now set up the free pool */

 free_pool.pool_size                  =
 free_pool.used_bytes                 = 0L;
 free_pool.memory_descriptor_list     = NULL;
 While far_memory_size IsNotZero
  BeginWhile
   desc = (memory_descriptor_ptr) 
           allocate_memory(Addr(static_pool),
                           Bit_32(sizeof(memory_descriptor_type)));
   If far_memory_size Exceeds 65536L
    Then
     size_this_chunk                  = 65536L;
    Else
     size_this_chunk                  = far_memory_size;
    EndIf;
   Desc.chunk                         =
   Desc.unused_base                   = BytePtr(far_memory);
   Desc.size                          =
   Desc.available                     = size_this_chunk;
   free_pool.pool_size               += size_this_chunk;
   Desc.next                          = free_pool.memory_descriptor_list;
   free_pool.memory_descriptor_list   = desc;
   far_memory                        += size_this_chunk;
   far_memory_size                   -= size_this_chunk;
  EndWhile;
 free_pool.pool_id                   = "Free pool";
 static_pool.pool_id                 = "Static pool";
 dynamic_pool.pool_id                = "Dynamic pool";
 dynamic_pool.memory_descriptor_list = Null;
 dynamic_pool.pool_size              =
 dynamic_pool.used_bytes             = 0L;
 return;
EndCode
#undef Desc

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              release_pool                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

void release_pool(pool_descriptor_ptr pool)
BeginDeclarations
#define Pool                           (*pool)
memory_descriptor_ptr                  desc;
#define Desc                           (*desc)
memory_descriptor_ptr                  next;
EndDeclarations
BeginCode
 desc  = Pool.memory_descriptor_list;
 While desc IsNotNull
  BeginWhile
   Desc.available                    = Desc.size;
   Desc.unused_base                  = Desc.chunk;
   free_pool.pool_size              += Desc.available;
   next                              = Desc.next;
   Desc.next                         = free_pool.memory_descriptor_list;
   free_pool.memory_descriptor_list  = desc;
   desc = next;
  EndWhile;
 Pool.memory_descriptor_list = Null;
 Pool.pool_size              =
 Pool.used_bytes             = 0L;
 return;
EndCode
#undef Pool
#undef Desc

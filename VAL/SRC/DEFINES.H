/*                                 DEFINES.H                               */

#define Now                            get_time()

#define MAX_ELEMENT_SIZE               2052
#define MAX_OBJECT_FILE_READ_SIZE      2048
#define MAX_PUBLICS_IN_LIBRARY         8192
#define TEMP_STRING_LENGTH             256
/* The following string must be one more than TEMP_STRING_LENGTH */
#define TEMP_STRING_SIZE               257

#define AlignmentGap(exp, bound)       (-(exp And bound) And bound)
#define CanonicFrame(exp)              ((bit_16) (exp >> 4L))
#define Frame(exp)                     ((bit_32) ((*(*exp).segment).address & \
                                                  0xFFFFFFF0L))
#define Target(exp)                    ((bit_32) ((*exp).address-Frame(exp)))
#define TokenIs(exp)                   (compare_string(token,exp) == 0)
#define TokenStartsWith(exp)           (compare_short_string(token,exp) == 0)

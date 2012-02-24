#ifndef _PB_H_
#define _PB_H_

/* pb.h: Common parts for nanopb library.
 * Most of these are quite low-level stuff. For the high-level interface,
 * see pb_encode.h or pb_decode.h
 */
#include <pb_field.h>

/* This structure is used for giving the callback function.
 * It is stored in the message structure and filled in by the method that
 * calls pb_decode.
 *
 * The decoding callback will be given a limited-length stream
 * If the wire type was string, the length is the length of the string.
 * If the wire type was a varint/fixed32/fixed64, the length is the length
 * of the actual value.
 * The function may be called multiple times (especially for repeated types,
 * but also otherwise if the message happens to contain the field multiple
 * times.)
 *
 * The encoding callback will receive the actual output stream.
 * It should write all the data in one call, including the field tag and
 * wire type. It can write multiple fields.
 *
 * The callback can be null if you want to skip a field.
 */
typedef struct _pb_istream_t pb_istream_t;
typedef struct _pb_ostream_t pb_ostream_t;
typedef struct _pb_callback_t pb_callback_t;
struct _pb_callback_t {
    union {
        bool (*decode)(pb_istream_t *stream, const pb_field_t *field, void *arg);
        bool (*encode)(pb_ostream_t *stream, const pb_field_t *field, const void *arg);
    } funcs;
    
    /* Free arg for use by callback */
    void *arg;
};

/* Wire types. Library user needs these only in encoder callbacks. */
typedef enum {
    PB_WT_VARINT = 0,
    PB_WT_64BIT  = 1,
    PB_WT_STRING = 2,
    PB_WT_32BIT  = 5
} pb_wire_type_t;

#endif

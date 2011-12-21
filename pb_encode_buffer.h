#ifndef _PB_ENCODE_BUFFER_H_
#define _PB_ENCODE_BUFFER_H_

/* pb_encode_buffer.h: Functions to encode protocol buffers to an
 * in-memory array of bytes. Depends on pb_encode_buffer.c. The main
 * function is pb_encode_buffer.  You also need an output buffer,
 * structures and their field descriptions (just like with pb_decode
 * or pb_encode).
 */

#include <stdbool.h>
#include "pb.h"

/* Output stream for an in-memory buffer.
 */
struct _pb_strstream_t
{
    uint8_t *buffer;
    uint8_t *last;
};

pb_strstream_t pb_strstream_from_buffer(uint8_t *buf, size_t bufsize);
bool pb_buf_write(pb_strstream_t *stream, const uint8_t *buf, size_t count);

/* --- Helper functions ---
 * You may want to use these from your caller or callbacks.
 */

bool pb_encbuf_varint(pb_strstream_t *stream, uint64_t value);
bool pb_encbuf_tag(pb_strstream_t *stream, pb_wire_type_t wiretype, int field_number);
/* Encode tag based on LTYPE and field number defined in the field structure. */
bool pb_encbuf_tag_for_field(pb_strstream_t *stream, const pb_field_t *field);
/* Write length as varint and then the contents of buffer. */
bool pb_encbuf_string(pb_strstream_t *stream, const uint8_t *buffer, size_t size);

/* Encode struct to given output stream.
 * Returns true on success, false on any failure.
 * The actual struct pointed to by src_struct must match the description in msg.
 * All required fields in the struct are assumed to have been filled in.
 */
bool pb_encode_buffer(pb_strstream_t *stream, const pb_message_t *msg, const void *src_struct);

#endif

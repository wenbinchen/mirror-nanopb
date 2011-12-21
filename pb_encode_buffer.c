/* pb_encode_buffer.c - encode a protobuf to an in-memory array
 *
 * 2011 Michael Poole <mdpoole@troilus.org>
 * Part of nanopb, 2011 Petteri Aimonen <jpa@kapsi.fi>
 */

#include "pb_encode_buffer.h"
#include <string.h>

#ifdef __GNUC__
/* Verify that we remember to check all return values for proper error propagation */
#define checkreturn __attribute__((warn_unused_result))
#else
#define checkreturn
#endif

typedef bool (*pb_encoder_t)(pb_strstream_t *stream, const pb_field_t *field, const void *src) checkreturn;

static bool pb_encb_varint(pb_strstream_t *stream, const pb_field_t *field, const void *src);
static bool pb_encb_svarint(pb_strstream_t *stream, const pb_field_t *field, const void *src);
static bool pb_encb_fixed32(pb_strstream_t *stream, const pb_field_t *field, const void *src);
static bool pb_encb_fixed64(pb_strstream_t *stream, const pb_field_t *field, const void *src);
static bool pb_encb_bytes(pb_strstream_t *stream, const pb_field_t *field, const void *src);
static bool pb_encb_string(pb_strstream_t *stream, const pb_field_t *field, const void *src);
static bool pb_encb_submessage(pb_strstream_t *stream, const pb_field_t *field, const void *src);

/* --- Function pointers to field encoders ---
 * Order in the array must match pb_action_t LTYPE numbering.
 */
static const pb_encoder_t PB_ENCODERS[PB_LTYPES_COUNT] = {
    &pb_encb_varint,
    &pb_encb_svarint,
    &pb_encb_fixed32,
    &pb_encb_fixed64,
    
    &pb_encb_bytes,
    &pb_encb_string,
    &pb_encb_submessage
};

/* pb_strstream_t implementation */

pb_strstream_t pb_strstream_from_buffer(uint8_t *buf, size_t bufsize)
{
    pb_strstream_t stream;
    stream.buffer = buf;
    stream.last = buf + bufsize;
    return stream;
}

bool pb_buf_write(pb_strstream_t *stream, const uint8_t *buf, size_t count)
{
    if (stream->buffer + count > stream->last)
        return false;
    stream->last -= count;
    memcpy(stream->last, buf, count);
    return true;
}

/* Main encoding stuff */

/* This function is static for the same reason as the version in
 * pb_encode.c.
 */
static bool checkreturn encode_array(pb_strstream_t *stream, const pb_field_t *field,
                                     const void *pData, size_t count, pb_encoder_t func)
{
    const void *p;
    uint8_t *start;
    size_t size;
    int i;
    
    if (count == 0)
        return true;
    
    start = stream->last;
    p = (const char*)pData + field->data_size * count;
    
    if (PB_LTYPE(field->type) <= PB_LTYPE_LAST_PACKABLE)
    {
        /* Write the data (in reverse order). */
        for (i = 0; i < count; i++)
        {
            p = (const char*)p - field->data_size;
            if (!func(stream, field, p))
                return false;
        }
        
        /* Write the size. */
        size = start - stream->last;
        if (!pb_encbuf_varint(stream, size))
            return false;
        if (!pb_encbuf_tag(stream, PB_WT_STRING, field->tag))
            return false;
    }
    else
    {
        for (i = 0; i < count; i++)
        {
            p = (const char*)p - field->data_size;
            if (!func(stream, field, p))
                return false;
            if (!pb_encbuf_tag_for_field(stream, field))
                return false;
        }
    }
    
    return true;
}

bool checkreturn pb_encode_buffer(pb_strstream_t *stream, const pb_message_t *msg, const void *src_struct)
{
    const char *has_fields = src_struct;
    const uint8_t *pData = src_struct;
    const pb_field_t *field;
    pb_encoder_t func;
    size_t size;
    unsigned int i;

    /* msg->size includes trailing padding, so we must calculate the
     * offset of the last field by counting forward from the start.
     */
    for (i = 0; i < msg->field_count; i++)
    {
        field = &msg->fields[i];
        pData += field->data_offset;
        size = field->data_size;
        if (PB_HTYPE(field->type) == PB_HTYPE_ARRAY)
            size *= field->array_size;
        pData += size;
    }

    /* Iterate through the fields in reverse order.  Because we write
     * from the end of the buffer, the result is in canonical order.
     */
    for (i = msg->field_count; i > 0; pData -= field->data_offset)
    {
        field = &msg->fields[--i];
        func = PB_ENCODERS[PB_LTYPE(field->type)];
        size = field->data_size;
        if (PB_HTYPE(field->type) == PB_HTYPE_ARRAY)
            size *= field->array_size;
        pData -= size;
        
        switch (PB_HTYPE(field->type))
        {
        case PB_HTYPE_OPTIONAL:
            if (!(has_fields[i/8] & (1 << i%8)))
                break;
            if (PB_POINTER(field->type)
                && (PB_LTYPE(field->type) == PB_LTYPE_STRING
                        || PB_LTYPE(field->type) == PB_LTYPE_SUBMESSAGE)
                    && *(void**)pData == NULL)
                    break;
                /* else fall through to required case */

        case PB_HTYPE_REQUIRED:
            if (!func(stream, field, pData))
                return false;
            if (!pb_encbuf_tag_for_field(stream, field))
                return false;
            break;

        case PB_HTYPE_ARRAY:
            size = *(size_t*)(pData + field->size_offset);
            if (!encode_array(stream, field, pData, size, func))
                return false;
            break;

        case PB_HTYPE_CALLBACK:
        {
            pb_callback_t *callback = (pb_callback_t*)pData;
            if (callback->funcs.encode_buffer != NULL
                && !callback->funcs.encode_buffer(stream, field, callback->arg))
                return false;
            break;
        }
        }
    }
    
    return true;
}

/* Helper functions */

bool checkreturn pb_encbuf_varint(pb_strstream_t *stream, uint64_t value)
{
    if (stream->last == stream->buffer)
    {
        return false;
    }
    else if (value < 128)
    {
        *--stream->last = value;
        return true;
    }
    else
    {
        uint8_t buffer[10];
        int i = 0;

        while (value)
        {
            buffer[i] = (value & 0x7F) | 0x80;
            value >>= 7;
            i++;
        }
        buffer[i-1] &= 0x7F;
        return pb_buf_write(stream, buffer, i);
    }
}

bool checkreturn pb_encbuf_tag(pb_strstream_t *stream, pb_wire_type_t wiretype, int field_number)
{
    int tag = wiretype | (field_number << 3);
    return pb_encbuf_varint(stream, tag);
}

bool checkreturn pb_encbuf_tag_for_field(pb_strstream_t *stream, const pb_field_t *field)
{
    pb_wire_type_t wiretype;
    switch (PB_LTYPE(field->type))
    {
        case PB_LTYPE_VARINT:
        case PB_LTYPE_SVARINT:
            wiretype = PB_WT_VARINT;
            break;
        
        case PB_LTYPE_FIXED32:
            wiretype = PB_WT_32BIT;
            break;
        
        case PB_LTYPE_FIXED64:
            wiretype = PB_WT_64BIT;
            break;
        
        case PB_LTYPE_BYTES:
        case PB_LTYPE_STRING:
        case PB_LTYPE_SUBMESSAGE:
            wiretype = PB_WT_STRING;
            break;
        
        default:
            return false;
    }
    
    return pb_encbuf_tag(stream, wiretype, field->tag);
}

bool checkreturn pb_encbuf_string(pb_strstream_t *stream, const uint8_t *buffer, size_t size)
{
    if (!pb_buf_write(stream, buffer, size))
        return false;
    return pb_encbuf_varint(stream, size);
}

/* Field encoders */

/* Copy srcsize bytes of integer from src so that values are casted properly.
 * On little endian machine, copy to start of dest
 * On big endian machine, copy to end of dest
 * destsize must always be larger than srcsize
 * 
 * Note: This is the reverse of the endian_copy in pb_decode.c.
 */
static void endian_copy(uint64_t *dest, const void *src, size_t srcsize)
{
#ifdef __BIG_ENDIAN__
    memcpy((char*)dest + sizeof(*dest) - srcsize, src, srcsize);
#else
    memcpy(dest, src, srcsize);
#endif
}

static bool pb_encb_varint(pb_strstream_t *stream, const pb_field_t *field, const void *src)
{
    uint64_t value = 0;
    endian_copy(&value, src, field->data_size);
    return pb_encbuf_varint(stream, value);
}

static bool pb_encb_svarint(pb_strstream_t *stream, const pb_field_t *field, const void *src)
{
    uint64_t value = 0;
    uint64_t zigzagged;
    uint64_t signbitmask = (uint64_t)0x80 << (field->data_size * 8 - 8);
    uint64_t xormask = ((uint64_t)-1) >> (64 - field->data_size * 8);
    
    endian_copy(&value, src, field->data_size);
    if (value & signbitmask)
        zigzagged = ((value ^ xormask) << 1) | 1;
    else
        zigzagged = value << 1;
    
    return pb_encbuf_varint(stream, zigzagged);
}

static bool pb_encb_fixed32(pb_strstream_t *stream, const pb_field_t *field, const void *src)
{
#ifdef __BIG_ENDIAN__
    const uint8_t *bytes = (const uint8_t*)src;
    uint8_t lebytes[4] = {bytes[3], bytes[2], bytes[1], bytes[0]};
    src = lebytes;
#endif
    return pb_buf_write(stream, src, 4);
}

static bool pb_encb_fixed64(pb_strstream_t *stream, const pb_field_t *field, const void *src)
{
#ifdef __BIG_ENDIAN__
    const uint8_t *bytes = (const uint8_t*)src;
    uint8_t lebytes[8] = {bytes[7], bytes[6], bytes[5], bytes[4],
                          bytes[3], bytes[2], bytes[1], bytes[0]};
    src = lebytes;
#endif
    return pb_buf_write(stream, src, 8);
}

static bool pb_encb_bytes(pb_strstream_t *stream, const pb_field_t *field, const void *src)
{
    if ((field != NULL) && PB_POINTER(field->type)) {
        pb_bytes_t *bytes = (pb_bytes_t*)src;
        return pb_encbuf_string(stream, bytes->bytes, bytes->size);
    } else {
        pb_bytes_array_t *bytes = (pb_bytes_array_t*)src;
        return pb_encbuf_string(stream, bytes->bytes, bytes->size);
    }
}

static bool pb_encb_string(pb_strstream_t *stream, const pb_field_t *field, const void *src)
{
    size_t len;
    if ((field != NULL) && PB_POINTER(field->type))
        src = *(char**)src;
    len = src ? strlen((char*)src) : 0;
    return pb_encbuf_string(stream, (uint8_t*)src, len);
}

static bool pb_encb_submessage(pb_strstream_t *stream, const pb_field_t *field, const void *src)
{
    uint8_t *start;
    size_t size;
    
    if (field->ptr == NULL)
        return false;
    
    if (PB_POINTER(field->type)) {
        src = *(void**)src;
        if (src == NULL)
            return false;
    }
    
    start = stream->last;
    if (!pb_encode_buffer(stream, (const pb_message_t*)field->ptr, src))
        return false;
    
    size = start - stream->last;
    if (!pb_encbuf_varint(stream, size))
        return false;
    
    return true;
}

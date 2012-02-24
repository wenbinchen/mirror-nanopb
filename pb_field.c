#include "pb_field.h"

#ifdef __GNUC__
/* Verify that we remember to check all return values for proper error propagation */
#define checkreturn __attribute__((warn_unused_result))
#else
#define checkreturn
#endif

/**
 * pb_common_aligned_field_info array holds the common field info that can be shared
 * by any numeric fields whose previous field is memory aligned
 */
const pb_field_t pb_common_aligned_field_info[NUM_COMMON_FIELD_INFO] = {
	/* REQUIRED bool */
    {0, PB_HTYPE_REQUIRED | PB_LTYPE_VARINT, 0, 0, sizeof(bool), 0, 0},
    /* OPTIONAL bool */
    {0, PB_HTYPE_OPTIONAL | PB_LTYPE_VARINT, 1, -1, sizeof(bool), 0, 0},
	/* REQUIRED int32, uint32, enum */
    {0, PB_HTYPE_REQUIRED | PB_LTYPE_VARINT, 0, 0, sizeof(uint32_t), 0, 0},
    /* OPTIONAL int32, uint32, enum */
    {0, PB_HTYPE_OPTIONAL | PB_LTYPE_VARINT, 4, -4, sizeof(uint32_t), 0, 0},
	/* REQUIRED int64, uint64 */
    {0, PB_HTYPE_REQUIRED | PB_LTYPE_VARINT, 0, 0, sizeof(uint64_t), 0, 0},
    /* OPTIONAL int64, uint64 */
    {0, PB_HTYPE_OPTIONAL | PB_LTYPE_VARINT, 4, -4, sizeof(uint64_t), 0, 0},
    /* REQUIRED sint32 */
    {0, PB_HTYPE_REQUIRED | PB_LTYPE_SVARINT, 0, 0, sizeof(int32_t), 0, 0},
    /* OPTIONAL sint32 */
    {0, PB_HTYPE_OPTIONAL | PB_LTYPE_SVARINT, 4, -4, sizeof(int32_t), 0, 0},
    /* REQUIRED sint64 */
    {0, PB_HTYPE_REQUIRED | PB_LTYPE_SVARINT, 0, 0, sizeof(int64_t), 0, 0},
    /* OPTIONAL sint64 */
    {0, PB_HTYPE_OPTIONAL | PB_LTYPE_SVARINT, 4, -4, sizeof(int64_t), 0, 0},
    /* REQUIRED fixed32, sfixed32, float */
    {0, PB_HTYPE_REQUIRED | PB_LTYPE_FIXED32, 0, 0, sizeof(uint32_t), 0, 0},
    /* OPTIONAL fixed32, sfixed32, float */
    {0, PB_HTYPE_OPTIONAL | PB_LTYPE_FIXED32, 4, -4, sizeof(uint32_t), 0, 0},
    /* REQUIRED fixed64, sfixed64, double */
    {0, PB_HTYPE_REQUIRED | PB_LTYPE_FIXED64, 0, 0, sizeof(uint64_t), 0, 0},
    /* OPTIONAL fixed64, sfixed64, double */
    {0, PB_HTYPE_OPTIONAL | PB_LTYPE_FIXED64, 4, -4, sizeof(uint64_t), 0, 0}
};

void pb_field_init(pb_field_iterator_t *iter, const pb_field_info_t *fields, void *dest_struct)
{
    if (fields->field_keys == NULL) return;
    iter->start_key = iter->current_key = fields->field_keys;
    iter->field_info = fields->field_info;
    iter->current = *(PB_FIELD_INFO(iter->current_key, iter->field_info));
    iter->current.tag = PB_TAG_VAL(iter->current_key);
    iter->field_index = 0;
    iter->pData = (char*)dest_struct + iter->current.data_offset;
    iter->pSize = (char*)iter->pData + iter->current.size_offset;
    iter->dest_struct = dest_struct;
}

bool pb_field_next(pb_field_iterator_t *iter)
{
    bool notwrapped = true;
    size_t prev_size = iter->current.data_size;

    if (PB_HTYPE(iter->current.type) == PB_HTYPE_ARRAY)
        prev_size *= iter->current.array_size;

    iter->field_index++;
    if (PB_IS_LAST(iter->current_key))
    {
        iter->current_key = iter->start_key;
        iter->current = *(PB_FIELD_INFO(iter->current_key, iter->field_info));
        iter->current.tag = PB_TAG_VAL(iter->current_key);
        iter->field_index = 0;
        iter->pData = iter->dest_struct;
        prev_size = 0;
        notwrapped = false;
    } else {
        iter->current_key++;
        iter->current = *(PB_FIELD_INFO(iter->current_key, iter->field_info));
        iter->current.tag = PB_TAG_VAL(iter->current_key);
    }

    iter->pData = (char*)iter->pData + prev_size + iter->current.data_offset;
    iter->pSize = (char*)iter->pData + iter->current.size_offset;
    return notwrapped;
}

bool checkreturn pb_field_find(pb_field_iterator_t *iter, int tag)
{
    int start = iter->field_index;

    do {
        if (iter->current.tag == tag)
            return true;
        pb_field_next(iter);
    } while (iter->field_index != start);

    return false;
}

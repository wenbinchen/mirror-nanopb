/* Tests the decoding of all types. Currently only in the 'required' variety.
 * This is the counterpart of test_encode3.
 * Run e.g. ./test_encode3 | ./test_decode3
 */

#include <stdio.h>
#include <string.h>
#include <pb_decode.h>
#include "aligntype.pb.h"

#define TEST(x) if (!(x)) { \
    printf("Test " #x " failed.\n"); \
    return false; \
    }

/* This function is called once from main(), it handles
   the decoding and checks the fields. */
bool check_aligntypes(pb_istream_t *stream)
{
    AlignTypes aligntypes = {};
    
    if (!pb_decode(stream, AlignTypes_fields, &aligntypes))
        return false;
    
    TEST(aligntypes.req_bool          == true);
    TEST(aligntypes.req_int32         == 1001);
    TEST(strcmp(aligntypes.req_submsg.substuff1, "1002") == 0);
    TEST(aligntypes.req_submsg.substuff2 == 1002);
    TEST(aligntypes.req_int64         == 1003);
    TEST(strcmp(aligntypes.req_string, "1004") == 0);
    TEST(aligntypes.req_enum == MyEnum_Truth);
    TEST(aligntypes.req_bytes.size == 4);
    TEST(memcmp(aligntypes.req_bytes.bytes, "1005", 4) == 0);
    TEST(aligntypes.req_sint32        == 1006);
    TEST(aligntypes.req_bool_2        == true);
    TEST(aligntypes.req_sint64        == 1007);
    TEST(aligntypes.req_bool_3        == true);
    TEST(aligntypes.req_fixed32       == 1008);
    TEST(aligntypes.req_bool_4        == true);
    TEST(aligntypes.req_fixed64       == 1009);
    TEST(aligntypes.req_bool_5        == true);
    TEST(aligntypes.req_float         == 1010.0f);
    TEST(aligntypes.req_bool_5        == true);
    TEST(aligntypes.req_double        == 1011.0f);
   
    TEST(aligntypes.has_opt_bool      == true);
    TEST(aligntypes.opt_bool          == true);
    TEST(aligntypes.has_opt_int32     == true);
    TEST(aligntypes.opt_int32         == 1001);
    TEST(aligntypes.has_opt_submsg    == true);
    TEST(strcmp(aligntypes.opt_submsg.substuff1, "1002") == 0);
    TEST(aligntypes.opt_submsg.substuff2 == 1002);
    TEST(aligntypes.has_opt_int64     == true);
    TEST(aligntypes.opt_int64         == 1003);
    TEST(aligntypes.has_opt_string    == true);
    TEST(strcmp(aligntypes.opt_string, "1004") == 0);
    TEST(aligntypes.has_opt_enum      == true);
    TEST(aligntypes.opt_enum == MyEnum_Truth);
    TEST(aligntypes.has_opt_bytes     == true);
    TEST(aligntypes.opt_bytes.size == 4);
    TEST(memcmp(aligntypes.opt_bytes.bytes, "1005", 4) == 0);
    TEST(aligntypes.has_opt_sint32    == true);
    TEST(aligntypes.opt_sint32        == 1006);
    TEST(aligntypes.has_opt_bool_2    == true);
    TEST(aligntypes.opt_bool_2        == true);
    TEST(aligntypes.has_opt_sint64    == true);
    TEST(aligntypes.opt_sint64        == 1007);
    TEST(aligntypes.has_opt_bool_3    == true);
    TEST(aligntypes.opt_bool_3        == true);
    TEST(aligntypes.has_opt_fixed32   == true);
    TEST(aligntypes.opt_fixed32       == 1008);
    TEST(aligntypes.has_opt_bool_4    == true);
    TEST(aligntypes.opt_bool_4        == true);
    TEST(aligntypes.has_opt_fixed64   == true);
    TEST(aligntypes.opt_fixed64       == 1009);
    TEST(aligntypes.has_opt_bool_5    == true);
    TEST(aligntypes.opt_bool_5        == true);
    TEST(aligntypes.has_opt_float     == true);
    TEST(aligntypes.opt_float         == 1010.0f);
    TEST(aligntypes.has_opt_bool_6    == true);
    TEST(aligntypes.opt_bool_6        == true);
    TEST(aligntypes.has_opt_double    == true);
    TEST(aligntypes.opt_double        == 1011.0f);

    TEST(aligntypes.req_bool_aligned   == true);
    TEST(aligntypes.req_int32_aligned  == 1001);
    TEST(aligntypes.req_int64_aligned  == 1003);
    TEST(aligntypes.req_enum_aligned   == MyEnum_Truth);
    TEST(aligntypes.req_sint32_aligned == 1006);
    TEST(aligntypes.req_sint64_aligned == 1007);
    TEST(aligntypes.req_fixed32_aligned == 1008);
    TEST(aligntypes.req_fixed64_aligned == 1009);
    TEST(aligntypes.req_float_aligned  == 1010.0f);
    TEST(aligntypes.req_double_aligned == 1011.0f);

    TEST(aligntypes.has_opt_bool_aligned      == true);
    TEST(aligntypes.opt_bool_aligned          == true);
    TEST(aligntypes.has_opt_int32_aligned     == true);
    TEST(aligntypes.opt_int32_aligned         == 1001);
    TEST(aligntypes.has_opt_int64_aligned     == true);
    TEST(aligntypes.opt_int64_aligned         == 1003);
    TEST(aligntypes.has_opt_enum_aligned      == true);
    TEST(aligntypes.opt_enum_aligned == MyEnum_Truth);
    TEST(aligntypes.has_opt_sint32_aligned    == true);
    TEST(aligntypes.opt_sint32_aligned        == 1006);
    TEST(aligntypes.has_opt_sint64_aligned    == true);
    TEST(aligntypes.opt_sint64_aligned        == 1007);
    TEST(aligntypes.has_opt_fixed32_aligned   == true);
    TEST(aligntypes.opt_fixed32_aligned       == 1008);
    TEST(aligntypes.has_opt_fixed64_aligned   == true);
    TEST(aligntypes.opt_fixed64_aligned       == 1009);
    TEST(aligntypes.has_opt_float_aligned     == true);
    TEST(aligntypes.opt_float_aligned         == 1010.0f);
    TEST(aligntypes.has_opt_double_aligned    == true);
    TEST(aligntypes.opt_double_aligned        == 1011.0f);

    TEST(aligntypes.end == 1099);
    
    return true;
}

int main()
{
    /* Read the data into buffer */
    uint8_t buffer[512];
    size_t count = fread(buffer, 1, sizeof(buffer), stdin);
    
    /* Construct a pb_istream_t for reading from the buffer */
    pb_istream_t stream = pb_istream_from_buffer(buffer, count);
    
    /* Decode and print out the stuff */
    if (!check_aligntypes(&stream))
    {
        printf("Parsing failed.\n");
        return 1;
    } else {
        return 0;
    }
}

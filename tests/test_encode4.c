/* Attempts to test all the datatypes supported by ProtoBuf.
 * Currently only tests the 'required' variety.
 */

#include <stdio.h>
#include <pb_encode.h>
#include "aligntype.pb.h"

int main()
{
    /* Initialize the structure with constants */
    AlignTypes aligntypes = {
        true,
        1001,
        {"1002", 1002},
        1003,
        "1004",
        MyEnum_Truth,
        {4, "1005"},
        1006,
        true,
        1007,
        true,
        1008,
        true,
        1009,
        true,
        1010.0f,
        true,
        1011.0f,

        true,
        true,
        true,
        1001,
        true,
        {"1002", 1002},
        true,
        1003,
        true,
        "1004",
        true,
        MyEnum_Truth,
        true,
        {4, "1005"},
        true,
        1006,
        true,
        true,
        true,
        1007,
        true,
        true,
        true,
        1008,
        true,
        true,
        true,
        1009,
        true,
        true,
        true,
        1010.0f,
        true,
        true,
        true,
        1011.0f,
        
        true,
        1001,
        1003,
        MyEnum_Truth,
        1006,
        1007,
        1008,
        1009,
        1010.0f,
        1011.0f,

        true,
        true,
        true,
        1001,
        true,
        1003,
        true,
        MyEnum_Truth,
        true,
        1006,
        true,
        1007,
        true,
        1008,
        true,
        1009,
        true,
        1010.0f,
        true,
        1011.0f,

        1099
    };
    
    uint8_t buffer[512];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    
    /* Now encode it and check if we succeeded. */
    if (pb_encode(&stream, AlignTypes_fields, &aligntypes))
    {
        fwrite(buffer, 1, stream.bytes_written, stdout);
        return 0; /* Success */
    }
    else
    {
        return 1; /* Failure */
    }
}

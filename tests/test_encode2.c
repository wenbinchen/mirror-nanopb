/* Same as test_encode1.c, except writes directly to stdout.
 */

#include <stdio.h>
#include <pb_encode.h>
#include "person.pb.h"

/* This binds the pb_ostream_t into the stdout stream */
bool streamcallback(pb_ostream_t *stream, const uint8_t *buf, size_t count)
{
    FILE *file = (FILE*) stream->state;
    return fwrite(buf, 1, count, file) == count;
}

int main()
{
    /* Initialize the structure with constants */
    Person person = {{1 << Person_email_index},
        "Test Person 99", 99, "test@person.com",
        3, {{{1 << Person_PhoneNumber_type_index},
             "555-12345678", Person_PhoneType_MOBILE},
            {{1 << Person_PhoneNumber_type_index},
             "99-2342", 0},
            {{1 << Person_PhoneNumber_type_index},
             "1234-5678", Person_PhoneType_WORK},
        }};
    
    /* Prepare the stream, output goes directly to stdout */
    pb_ostream_t stream = {&streamcallback, stdout, SIZE_MAX, 0};
    
    /* Now encode it and check if we succeeded. */
    if (pb_encode(&stream, Person_msg, &person))
        return 0; /* Success */
    else
        return 1; /* Failure */
}

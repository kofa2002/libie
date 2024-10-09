#include <stdio.h>
#include <stdlib.h>
#include "iec61850_server.h"
#include "hal_thread.h"

int LLVMFuzzerTestOneInput(const char *data, size_t size) {
    int out;
    MmsValue* value = NULL;

    // Create a non-const copy of the input data
    uint8_t* non_const_data = (uint8_t*)malloc(size);
    if (non_const_data == NULL) {
        return -1; // Handle memory allocation failure
    }
    memcpy(non_const_data, data, size);

    value = MmsValue_decodeMmsData(non_const_data, 0, size, &out);

    if (value != NULL) {
        MmsValue_delete(value);
    }

    // Free the allocated non-const copy
    free(non_const_data);

    return 0;
}

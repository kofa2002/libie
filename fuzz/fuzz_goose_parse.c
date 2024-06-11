#include <stdio.h>
#include <stdlib.h>

#include "goose_receiver.h"

#define kMinInputLength 22

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size <= kMinInputLength) {
        return 0;
    }
    
    GooseReceiver receiver = GooseReceiver_create();
    GooseSubscriber subscriber = GooseSubscriber_create("fuzz", NULL);
    GooseReceiver_addSubscriber(receiver, subscriber);
    GooseReceiver_handleMessage(receiver, data, size);
    GooseReceiver_destroy(receiver);

    return 0;
}

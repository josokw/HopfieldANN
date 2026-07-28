#include "HopfieldContext.h"
#include <stdlib.h>

HopfieldContext *hopfield_context_create(void)
{
    return (HopfieldContext *)calloc(1, sizeof(HopfieldContext));
}

void hopfield_context_destroy(HopfieldContext *ctx)
{
    if (ctx != NULL) {
        free(ctx);
    }
}

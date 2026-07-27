#include "HopfieldContext.h"
#include <stdlib.h>
#include <string.h>

HopfieldContext *hopfield_context_create(void)
{
    HopfieldContext *ctx = (HopfieldContext *)calloc(1, sizeof(HopfieldContext));
    if (ctx != NULL) {
        ctx->nRows = 0;
        ctx->nColumns = 0;
        ctx->nPatterns = 0;
        ctx->patternSize = 0;
        ctx->nNoisyPatterns = 0;
    }
    return ctx;
}

void hopfield_context_destroy(HopfieldContext *ctx)
{
    if (ctx != NULL) {
        free(ctx);
    }
}

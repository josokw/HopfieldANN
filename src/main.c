#include "cli.h"
#include "HopfieldContext.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    HopfieldContext *ctx = hopfield_context_create();
    if (ctx == NULL) {
        fprintf(stderr, "Error: Failed to allocate context\n");
        return EXIT_FAILURE;
    }

    int result = run_cli(ctx, argc, argv);
    hopfield_context_destroy(ctx);
    return result;
}

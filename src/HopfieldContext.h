#ifndef HOPFIELDCONTEXT_H
#define HOPFIELDCONTEXT_H

#include "HopfieldConfig.h"
#include <stdbool.h>

typedef struct {
    /* Pattern storage (dynamically allocated, NULL until sized) */
    double **patterns;
    double **noisyPatterns;
    double **W;

    /* Metadata */
    int nRows;
    int nColumns;
    int nPatterns;
    int patternSize;
    int nNoisyPatterns;
} HopfieldContext;

typedef enum {
    HOPFIELD_OK = 0,
    HOPFIELD_ERR_FILE_NOT_FOUND,
    HOPFIELD_ERR_INVALID_FORMAT,
    HOPFIELD_ERR_INDEX_OUT_OF_RANGE,
    HOPFIELD_ERR_SIZE_EXCEEDED,
    HOPFIELD_ERR_OUT_OF_MEMORY
} HopfieldError;

/* Allocate an (rows x cols) matrix in one contiguous block with a row
   pointer array. Returns NULL on failure. */
double **allocMatrix(int rows, int cols);

/* Free a matrix returned by allocMatrix (NULL-safe). */
void freeMatrix(double **m);

/* Constructor - initializes to zero */
HopfieldContext *hopfield_context_create(void);

/* Destructor */
void hopfield_context_destroy(HopfieldContext *ctx);

/* (Re)allocate the pattern and weight storage to the given dimensions.
   Existing contents are lost. Returns false on invalid dimensions or
   allocation failure, leaving the context in a usable (zeroed) state. */
bool hopfield_context_resize(HopfieldContext *ctx, int nRows, int nColumns,
                             int nPatterns, int nNoisyPatterns);

/* (Re)allocate only the noisy-pattern storage. Existing patterns and the
   weight matrix are preserved. */
bool hopfield_context_set_noisy(HopfieldContext *ctx, int nNoisyPatterns);

#endif

#ifndef HOPFIELDCONTEXT_H
#define HOPFIELDCONTEXT_H

#include "HopfieldConfig.h"

typedef struct {
    /* Pattern storage */
    double patterns[NMAX_PATTERNS][NMAX_NEURONS];
    double noisyPatterns[NMAX_PATTERNS][NMAX_NEURONS];
    double W[NMAX_NEURONS][NMAX_NEURONS];

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
    HOPFIELD_ERR_SIZE_EXCEEDED
} HopfieldError;

/* Constructor - initializes to zero */
HopfieldContext *hopfield_context_create(void);

/* Destructor */
void hopfield_context_destroy(HopfieldContext *ctx);

#endif

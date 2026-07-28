/*---------------------------------------------------------------------------*/
/* Simulation Hopfield ANN */
/*---------------------------------------------------------------------------*/

#ifndef HOPFIELDIO_H
#define HOPFIELDIO_H

#include "HopfieldContext.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

HopfieldError readFile(HopfieldContext *ctx, const char fileName[]);
HopfieldError readNoisyFile(HopfieldContext *ctx, const char fileName[]);

bool showIndexedPattern(const HopfieldContext *ctx, int index);
bool showIndexedNoisyPattern(const HopfieldContext *ctx, int index);

bool showPattern(const HopfieldContext *ctx, const double pattern[]);
bool showPatternAndDifference(const HopfieldContext *ctx,
                              const double pattern[],
                              const double patternWithNoise[]);
bool showPatternAsVector(const HopfieldContext *ctx, const double pattern[]);

#ifdef __cplusplus
}
#endif

#endif

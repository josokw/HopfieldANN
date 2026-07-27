/*---------------------------------------------------------------------------*/
/* Simulation Hopfield ANN */
/*---------------------------------------------------------------------------*/

#ifndef HOPFIELDIO_H
#define HOPFIELDIO_H

#include "HopfieldContext.h"

#ifdef __cplusplus
extern "C" {
#endif

HopfieldError readFile(HopfieldContext *ctx, const char fileName[]);
HopfieldError readNoisyFile(HopfieldContext *ctx, const char fileName[]);

void showIndexedPattern(const HopfieldContext *ctx, int index);
void showIndexedNoisyPattern(const HopfieldContext *ctx, int index);

void showPattern(const HopfieldContext *ctx, const double pattern[]);
void showPatternAndDifference(const HopfieldContext *ctx,
                              const double pattern[],
                              const double patternWithNoise[]);
void showPatternAsVector(const HopfieldContext *ctx, const double pattern[]);

#ifdef __cplusplus
}
#endif

#endif

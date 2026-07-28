#ifndef HOPFIELDUTIL_H
#define HOPFIELDUTIL_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool equals(double d1, double d2);
int sign(double value);
int randomInt(int min, int max);
void copyPattern(const int patternSize, const double sourcePattern[],
                 double targetPattern[]);

#ifdef __cplusplus
}
#endif

#endif

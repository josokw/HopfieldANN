#include "HopfieldUtil.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>

static const double EPSILON = 1e-6;

bool equals(double d1, double d2)
{
   return fabs(d1 - d2) < EPSILON;
}

int sign(double value)
{
   return (value >= 0.0) ? 1 : -1;
}

int randomInt(int min, int max)
{
   assert(max >= min);
   int range = max - min + 1;
   int limit = RAND_MAX - (RAND_MAX % range);
   int r;

   do {
      r = rand();
   } while (r >= limit);
   return min + (r % range);
}

void copyPattern(const int patternSize, const double sourcePattern[],
                 double targetPattern[])
{
   for (int i = 0; i < patternSize; i++) {
      targetPattern[i] = sourcePattern[i];
   }
}

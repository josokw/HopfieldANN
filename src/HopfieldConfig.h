#ifndef HOPFIELDCONFIG_H
#define HOPFIELDCONFIG_H

#define MAX_ITERATIONS 1000
#define MAX_NOISE_PERCENT 100              /* hard cap: valid noise range is 0..100% */
#define MAX_INFORMATIVE_NOISE_PERCENT 50   /* above this, input anti-correlates with the
                                              stored pattern -> recall converges to the
                                              inverted pattern (W(-x) = -Wx) */
#define STORAGE_CAPACITY_FACTOR 0.138

#endif

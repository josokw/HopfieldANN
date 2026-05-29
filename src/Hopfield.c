#include "Hopfield.h"

double patterns[NMAX_PATTERNS][NMAX_NEURONS] = {{0.0}};
double noisyPatterns[NMAX_PATTERNS][NMAX_NEURONS] = {{0.0}};
// Weight connection matrix: is symmetric and has zero-diagonal.
double W[NMAX_NEURONS][NMAX_NEURONS] = {{0.0}};

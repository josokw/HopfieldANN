#include "Hopfield.h"

double Patterns[NMAX_PATTERNS][NMAX_NEURONS] = {0.0};
double NoisyPatterns[NMAX_PATTERNS][NMAX_NEURONS] = {0.0};
// Weight connection matrix: is symmetric and has zero-diagonal.
double W[NMAX_NEURONS][NMAX_NEURONS] = {0.0};

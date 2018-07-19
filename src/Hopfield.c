#include "Hopfield.h"

double Patterns[MAXP][MAXN] = {0.0};
double NoisyPatterns[MAXP][MAXN] = {0.0};
// Weight connection matrix: is symmetric and has zero-diagonal.
double W[MAXN][MAXN] = {0.0};

#include "Hopfield.h"

double Patterns[MAXP][MAXN];
double NoisyPatterns[MAXP][MAXN];
// Weight connection matrix: is symmetric and has zero-diagonal.
double W[MAXN][MAXN];

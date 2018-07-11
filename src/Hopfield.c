#include "Hopfield.h"

double Patterns[MAXP][MAXN];
double NoisyPatterns[MAXP][MAXN];
/* Symmetric, zero-diagonal connection matrix */
double J[MAXN][MAXN];

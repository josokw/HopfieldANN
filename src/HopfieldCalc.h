#ifndef HOPFIELDCALC_H
#define HOPFIELDCALC_H

#include "Hopfield.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool isSymmetric(const int patternSize, const double W[][MAXN]);
bool hasZeroDiagonal(const int patternSize, const double W[][MAXN]);
int storageCapacity(const int patternSize);

void learnW(const int nPatterns, const int patternSize, double W[][MAXN]);

int addNoise(const int patternSize, int PatNumber, double Pat[],
             int Chance);

void calcOut(const int patternSize, const double W[][MAXN],
             const double inputPattern[], double outputPattern[]);

void copyPattern(const int patternSize, double sourcePattern[],
                 double targetPattern[]);

double calcEnergy(const int patternSize, const double pattern[],
                  const double W[][MAXN]);

void calcAssociations(const int patternSize, const double W[][MAXN],
                      const double inputPattern[],
                      const double inputPatternWithNoise[],
                      double associationPattern[]);

#ifdef __cplusplus
}
#endif

#endif

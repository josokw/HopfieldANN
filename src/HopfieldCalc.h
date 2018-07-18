#ifndef HOPFIELDCALC_H
#define HOPFIELDCALC_H

#include "Hopfield.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

bool isSymmetric(const int patternSize, const double W[][MAXN]);
bool hasZeroDiagonal(const int patternSize, const double W[][MAXN]);

void learnW(const int maxPat, const int patSize, double W[][MAXN]);

int addNoise(const int PatSize, int PatNumber, double Pat[], int Chance);

void calcOut(const int PatSize, const double W[][MAXN],
             const double InPattern[],
             double OutPattern[]);

void copyPattern(const int patternSize, double sourcePattern[],
                 double targetPattern[]);

double calcEnergy(const int patternSize, const double Pattern[],
                  const double W[][MAXN]);

void calcAssociations(const int patternSize,
                      const double W[][MAXN],
                      const double InputPattern[],
                      const double InputPatternWithNoise[],
                      double AssociationPattern[]);

#ifdef __cplusplus
}
#endif

#endif

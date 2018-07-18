#ifndef HOPFIELDCALC_H
#define HOPFIELDCALC_H

#include "Hopfield.h"

#ifdef __cplusplus
extern "C"
{
#endif

void learnJ(const int maxPat, const int patSize, double J[][MAXN]);

int addNoise(const int PatSize, int PatNumber, double Pat[], int Chance);

void calcOut(const int PatSize, const double J[][MAXN],
             const double InPattern[],
             double OutPattern[]);

void copyPattern(const int patternSize, double sourcePattern[],
                 double targetPattern[]);

double calcEnergy(const int patternSize, const double Pattern[],
                  const double J[][MAXN]);

void calcAssociations(const int patternSize,
                      const double J[][MAXN],
                      const double InputPattern[],
                      const double InputPatternWithNoise[],
                      double AssociationPattern[]);

#ifdef __cplusplus
}
#endif

#endif

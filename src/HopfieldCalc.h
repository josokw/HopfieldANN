#ifndef HOPFIELDCALC_H
#define HOPFIELDCALC_H

#include "Hopfield.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool isSymmetric(const int patternSize, const double W[][NMAX_NEURONS]);
bool hasZeroDiagonal(const int patternSize,
                     const double W[][NMAX_NEURONS]);
int storageCapacity(const int patternSize);

void learnHebbian(const int nPatterns, const int patternSize,
                  double W[][NMAX_NEURONS]);

int addNoiseToPattern(const int patternSize, int PatNumber,
                      double pattern[], int Chance);

void calcOut(const int patternSize, const double W[][NMAX_NEURONS],
             const double inputPattern[], double outputPattern[]);

void copyPattern(const int patternSize, double sourcePattern[],
                 double targetPattern[]);

double calcEnergy(const int patternSize, const double pattern[],
                  const double W[][NMAX_NEURONS]);

void calcAssociatedPattern(const int patternSize,
                           const double W[][NMAX_NEURONS],
                           const double inputPattern[],
                           double associatedPattern[]);

void showAssociatedPattern(const int patternSize,
                           const double W[][NMAX_NEURONS],
                           const double inputPattern[],
                           const double inputPatternWithNoise[],
                           double associatedPattern[]);

#ifdef __cplusplus
}
#endif

#endif

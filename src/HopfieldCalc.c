#include "HopfieldCalc.h"
#include "HopfieldIO.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static const double EPSILON = 1e-6;

static int sign(double value)
{
   return (value >= 0) ? 1 : -1;
}

static int Random(int min, int max)
{
   return min + (rand() % (max - min + 1));
}

bool equals(double d1, double d2)
{
   return fabs(d1 - d2) < EPSILON;
}

bool isSymmetric(const int patternSize, const double W[][NMAX_NEURONS])
{
   assert(patternSize < NMAX_NEURONS);

   bool isSymmetric = true;

   for (int i = 1; i < patternSize; i++) {
      for (int j = i; j < patternSize; j++) {
         if (!equals(W[i][j], W[j][i])) {
            isSymmetric = false;
            break;
         }
      }
      if (!isSymmetric) {
         break;
      }
   }
   return isSymmetric;
}

bool hasZeroDiagonal(const int patternSize, const double W[][NMAX_NEURONS])
{
   assert(patternSize < NMAX_NEURONS);

   bool hasZD = true;

   for (int i = 0; i < patternSize; i++) {
      if (!equals(W[i][i], 0.0)) {
         hasZD = false;
         break;
      }
   }
   return hasZD;
}

int storageCapacity(const int patternSize)
{
   return 0.14 * patternSize;
}

void learnHebbian(const int nPatterns, const int patternSize,
                  double W[][NMAX_NEURONS])
{
   for (int row = 0; row < patternSize; row++) {
      for (int column = row; column < patternSize; column++) {
         if (row == column) {
            W[row][column] = 0.0;
         }
         else {
            for (int pat = 0; pat < nPatterns; pat++) {
               W[row][column] += Patterns[pat][row] *
                                 Patterns[pat][column] /
                                 (double)patternSize;
               W[column][row] = W[row][column];
            }
         }
      }
   }
   assert(hasZeroDiagonal(patternSize, W));
   assert(isSymmetric(patternSize, W));
}

int addNoiseToPattern(const int patternSize, int PatNumber,
                      double pattern[], int Chance)
{
   if (Chance < 0)
      Chance = 0;
   if (Chance > 75)
      Chance = 75;

   int Nnoise = patternSize * Chance / 100;
   int NoiseIndex = 0;
   int NoiseArray[NMAX_NEURONS] = {0};
   int n = 0;
   while (n < Nnoise) {
      NoiseIndex = Random(0, patternSize);
      if (NoiseArray[NoiseIndex] == 0) {
         NoiseArray[NoiseIndex] = 1;
         n++;
      }
   }
   for (int index = 0; index < patternSize; index++) {
      if (NoiseArray[index] == 1) {
         pattern[index] = -Patterns[PatNumber][index];
      }
      else {
         pattern[index] = Patterns[PatNumber][index];
      }
   }
   return Nnoise;
}

void calcOutputPattern(const int patternSize,
                       const double W[][NMAX_NEURONS],
                       const double inputPattern[], double outputPattern[])
{
   for (int outIndex = 0; outIndex < patternSize; outIndex++) {
      double delta = 0.0;
      for (int inIndex = 0; inIndex < patternSize; inIndex++) {
         delta += inputPattern[inIndex] * W[outIndex][inIndex];
      }
      outputPattern[outIndex] = sign(delta);
   }
}

void copyPattern(const int patternSize, const double *sourcePattern,
                 double *targetPattern)
{
   for (int i = 0; i < patternSize; i++) {
      targetPattern[i] = sourcePattern[i];
   }
}

double calcEnergy(const int patternSize, const double pattern[],
                  const double W[][NMAX_NEURONS])
{
   double energy = 0.0;

   for (int i = 0; i < patternSize; i++) {
      for (int j = 0; j < patternSize; j++) {
         energy += pattern[i] * pattern[j] * W[i][j];
      }
   }
   energy *= -0.5;

   return energy;
}

double calcAssociatedPattern(const int patternSize,
                             const double W[][NMAX_NEURONS],
                             const double inputPattern[],
                             double associatedPattern[])
{
   double pattern[patternSize];
   copyPattern(patternSize, inputPattern, pattern);

   double energy = calcEnergy(patternSize, pattern, W);
   double energyPrevious = 0.0;

   do {
      energyPrevious = energy;
      calcOutputPattern(patternSize, W, pattern, associatedPattern);
      energy = calcEnergy(patternSize, associatedPattern, W);
      copyPattern(patternSize, associatedPattern, pattern);
      printf("\n    Energy = %9.4f\n\n", energy);
   } while (!equals(energyPrevious, energy));
   showPattern(associatedPattern);

   return energy;
}

void showAssociatedPattern(const int patternSize,
                           const double W[][NMAX_NEURONS],
                           const double inputPattern[],
                           const double inputPatternWithNoise[],
                           double associatedPattern[])
{
   double patternWithNoise[patternSize];
   copyPattern(patternSize, inputPatternWithNoise, patternWithNoise);

   double energy = 0.0;
   double energyPrevious = 0.0;

   showPatternAndDifference(inputPattern, patternWithNoise);
   energy = calcEnergy(patternSize, patternWithNoise, W);
   printf("\n    Energy = %9.4f\n\n", energy);

   do {
      energyPrevious = energy;
      calcOutputPattern(patternSize, W, patternWithNoise,
                        associatedPattern);
      energy = calcEnergy(patternSize, associatedPattern, W);
      copyPattern(patternSize, associatedPattern, patternWithNoise);
      showPatternAndDifference(inputPattern, associatedPattern);
      printf("\n    Energy = %9.4f\n\n", energy);
   } while (!equals(energyPrevious, energy));
}

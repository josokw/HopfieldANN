#include "HopfieldCalc.h"
#include "HopfieldIO.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static const double EPSILON = 1e-6;

static int sign(double value)
{
   return (value >= 0.0) ? 1 : -1;
}

static int Random(int min, int max)
{
   assert(max >= min);
   // Avoid modulo bias by using a rejection sampling method.
   // Force statistical rigor.
   int range = max - min + 1;
   int limit = RAND_MAX - (RAND_MAX % range);
   int r;

   do {
      r = rand();
   } while (r >= limit);
   return min + (r % range);
}

bool equals(double d1, double d2)
{
   return fabs(d1 - d2) < EPSILON;
}

bool isSymmetric(const int patternSize, const double w[][NMAX_NEURONS])
{
   assert(patternSize > 0 && patternSize <= NMAX_NEURONS);

   bool symmetric = true;

   for (int i = 0; i < patternSize; i++) {
      for (int j = i + 1; j < patternSize; j++) {
         if (!equals(w[i][j], w[j][i])) {
            symmetric = false;
            break;
         }
      }
      if (!symmetric) {
         break;
      }
   }
   return symmetric;
}

bool hasZeroDiagonal(const int patternSize, const double w[][NMAX_NEURONS])
{
   assert(patternSize > 0 && patternSize <= NMAX_NEURONS);

   bool hasZD = true;

   for (int i = 0; i < patternSize; i++) {
      if (!equals(w[i][i], 0.0)) {
         hasZD = false;
         break;
      }
   }
   return hasZD;
}

int storageCapacity(const int patternSize)
{
   int cap = (int)(STORAGE_CAPACITY_FACTOR * patternSize);
   return cap < 1 ? 1 : cap;
}

void learnHebbian(const int nPatterns, const int patternSize,
                  double w[][NMAX_NEURONS])
{
   assert(nPatterns > 0 && nPatterns <= NMAX_PATTERNS);
   assert(patternSize > 0 && patternSize <= NMAX_NEURONS);

   for (int row = 0; row < patternSize; row++) {
      for (int column = 0; column < patternSize; column++) {
         w[row][column] = 0.0;
      }
   }
   for (int row = 0; row < patternSize; row++) {
      for (int column = row; column < patternSize; column++) {
         if (row == column) {
            w[row][column] = 0.0;
         }
         else {
            for (int pat = 0; pat < nPatterns; pat++) {
               w[row][column] += patterns[pat][row] *
                                 patterns[pat][column] /
                                 (double)patternSize;
            }
            w[column][row] = w[row][column];
         }
      }
   }
   assert(hasZeroDiagonal(patternSize, w));
   assert(isSymmetric(patternSize, w));
}

int addNoiseToPattern(const int patternSize, const int patNumber,
                      double pattern[], int chance)
{
   assert(patNumber >= 0 && patNumber < nPatterns);
   assert(patternSize > 0 && patternSize <= NMAX_NEURONS);

   if (chance < 0)
      chance = 0;
   if (chance > MAX_NOISE_PERCENT)
      chance = MAX_NOISE_PERCENT;

   int nNoise = patternSize * chance / 100;
   int noiseIndex = 0;
   int noiseArray[NMAX_NEURONS] = {0};
   int n = 0;
   while (n < nNoise) {
      noiseIndex = Random(0, patternSize - 1);
      if (noiseArray[noiseIndex] == 0) {
         noiseArray[noiseIndex] = 1;
         n++;
      }
   }
   for (int index = 0; index < patternSize; index++) {
      if (noiseArray[index] == 1) {
         pattern[index] = -patterns[patNumber][index];
      }
      else {
         pattern[index] = patterns[patNumber][index];
      }
   }
   return nNoise;
}

void calcOutputPattern(const int patternSize,
                       const double w[][NMAX_NEURONS],
                       const double inputPattern[], double outputPattern[])
{
   for (int outIndex = 0; outIndex < patternSize; outIndex++) {
      double delta = 0.0;
      for (int inIndex = 0; inIndex < patternSize; inIndex++) {
         delta += inputPattern[inIndex] * w[outIndex][inIndex];
      }
      outputPattern[outIndex] = sign(delta);
   }
}

void copyPattern(const int patternSize, const double sourcePattern[],
                 double targetPattern[])
{
   for (int i = 0; i < patternSize; i++) {
      targetPattern[i] = sourcePattern[i];
   }
}

double calcEnergy(const int patternSize, const double pattern[],
                  const double w[][NMAX_NEURONS])
{
   double energy = 0.0;

   for (int i = 0; i < patternSize; i++) {
      for (int j = 0; j < patternSize; j++) {
         energy += pattern[i] * pattern[j] * w[i][j];
      }
   }

   return -0.5 * energy;
}

double calcAssociatedPattern(const int patternSize,
                             const double w[][NMAX_NEURONS],
                             const double inputPattern[],
                             double associatedPattern[])
{
   double pattern[NMAX_NEURONS] = {0};
   copyPattern(patternSize, inputPattern, pattern);

   double energy = calcEnergy(patternSize, pattern, w);
   double energyPrevious = 0.0;
   int iter = 0;

   do {
      energyPrevious = energy;
      calcOutputPattern(patternSize, w, pattern, associatedPattern);
      energy = calcEnergy(patternSize, associatedPattern, w);
      copyPattern(patternSize, associatedPattern, pattern);
      printf("\n    Energy = %9.4f\n\n", energy);
      iter++;
   } while (!equals(energyPrevious, energy) && iter < MAX_ITERATIONS);
   showPattern(associatedPattern);

   return energy;
}

void showAssociatedPattern(const int patternSize,
                           const double w[][NMAX_NEURONS],
                           const double inputPattern[],
                           const double inputPatternWithNoise[],
                           double associatedPattern[])
{
   assert(patternSize > 0 && patternSize <= NMAX_NEURONS);

   double patternWithNoise[NMAX_NEURONS] = {0};
   copyPattern(patternSize, inputPatternWithNoise, patternWithNoise);

   double energy = 0.0;
   double energyPrevious = 0.0;
   int iter = 0;

   showPatternAndDifference(inputPattern, patternWithNoise);
   energy = calcEnergy(patternSize, patternWithNoise, w);
   printf("\n    Energy = %9.4f\n\n", energy);

   do {
      energyPrevious = energy;
      calcOutputPattern(patternSize, w, patternWithNoise,
                        associatedPattern);
      energy = calcEnergy(patternSize, associatedPattern, w);
      copyPattern(patternSize, associatedPattern, patternWithNoise);
      showPatternAndDifference(inputPattern, associatedPattern);
      printf("\n    Energy = %9.4f\n\n", energy);
      iter++;
   } while (!equals(energyPrevious, energy) && iter < MAX_ITERATIONS);
}

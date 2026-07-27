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

void learnHebbian(HopfieldContext *ctx)
{
   assert(ctx != NULL);
   assert(ctx->nPatterns > 0 && ctx->nPatterns <= NMAX_PATTERNS);
   assert(ctx->patternSize > 0 && ctx->patternSize <= NMAX_NEURONS);

   for (int row = 0; row < ctx->patternSize; row++) {
      for (int column = 0; column < ctx->patternSize; column++) {
         ctx->W[row][column] = 0.0;
      }
   }
   for (int row = 0; row < ctx->patternSize; row++) {
      for (int column = row; column < ctx->patternSize; column++) {
         if (row == column) {
            ctx->W[row][column] = 0.0;
         }
         else {
            for (int pat = 0; pat < ctx->nPatterns; pat++) {
               ctx->W[row][column] += ctx->patterns[pat][row] *
                                 ctx->patterns[pat][column] /
                                 (double)ctx->patternSize;
            }
            ctx->W[column][row] = ctx->W[row][column];
         }
      }
   }
   assert(hasZeroDiagonal(ctx->patternSize, ctx->W));
   assert(isSymmetric(ctx->patternSize, ctx->W));
}

int addNoiseToPattern(HopfieldContext *ctx, const int patNumber, int chance)
{
   assert(ctx != NULL);
   assert(patNumber >= 0 && patNumber < ctx->nPatterns);
   assert(ctx->patternSize > 0 && ctx->patternSize <= NMAX_NEURONS);

   if (chance < 0)
      chance = 0;
   if (chance > MAX_NOISE_PERCENT)
      chance = MAX_NOISE_PERCENT;

   int nNoise = ctx->patternSize * chance / 100;
   int noiseIndex = 0;
   int noiseArray[NMAX_NEURONS] = {0};
   int n = 0;
   while (n < nNoise) {
      noiseIndex = Random(0, ctx->patternSize - 1);
      if (noiseArray[noiseIndex] == 0) {
         noiseArray[noiseIndex] = 1;
         n++;
      }
   }
   /* We need a temporary pattern to store the noisy version */
   double noisyPattern[NMAX_NEURONS] = {0};
   for (int index = 0; index < ctx->patternSize; index++) {
      if (noiseArray[index] == 1) {
         noisyPattern[index] = -ctx->patterns[patNumber][index];
      }
      else {
         noisyPattern[index] = ctx->patterns[patNumber][index];
      }
   }
   /* Copy the noisy pattern to the output (caller must provide a buffer) */
   /* For now, we'll store it in the first noisy pattern slot */
   /* This is a temporary solution until Phase 2 separates concerns */
   for (int index = 0; index < ctx->patternSize; index++) {
      ctx->noisyPatterns[0][index] = noisyPattern[index];
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

double calcAssociatedPattern(HopfieldContext *ctx,
                             const double inputPattern[],
                             double associatedPattern[])
{
   assert(ctx != NULL);

   double pattern[NMAX_NEURONS] = {0};
   copyPattern(ctx->patternSize, inputPattern, pattern);

   double energy = calcEnergy(ctx->patternSize, pattern, ctx->W);
   double energyPrevious = 0.0;
   int iter = 0;

   do {
      energyPrevious = energy;
      calcOutputPattern(ctx->patternSize, ctx->W, pattern, associatedPattern);
      energy = calcEnergy(ctx->patternSize, associatedPattern, ctx->W);
      copyPattern(ctx->patternSize, associatedPattern, pattern);
      iter++;
   } while (!equals(energyPrevious, energy) && iter < MAX_ITERATIONS);

   return energy;
}

void showAssociatedPattern(HopfieldContext *ctx,
                           const double inputPattern[],
                           const double inputPatternWithNoise[],
                           double associatedPattern[])
{
   assert(ctx != NULL);
   assert(ctx->patternSize > 0 && ctx->patternSize <= NMAX_NEURONS);

   double patternWithNoise[NMAX_NEURONS] = {0};
   copyPattern(ctx->patternSize, inputPatternWithNoise, patternWithNoise);

   double energy = 0.0;
   double energyPrevious = 0.0;
   int iter = 0;

   showPatternAndDifference(ctx, inputPattern, patternWithNoise);
   energy = calcEnergy(ctx->patternSize, patternWithNoise, ctx->W);
   printf("\n    Energy = %9.4f\n\n", energy);

   do {
      energyPrevious = energy;
      calcOutputPattern(ctx->patternSize, ctx->W, patternWithNoise,
                        associatedPattern);
      energy = calcEnergy(ctx->patternSize, associatedPattern, ctx->W);
      copyPattern(ctx->patternSize, associatedPattern, patternWithNoise);
      showPatternAndDifference(ctx, inputPattern, associatedPattern);
      printf("\n    Energy = %9.4f\n\n", energy);
      iter++;
   } while (!equals(energyPrevious, energy) && iter < MAX_ITERATIONS);
}

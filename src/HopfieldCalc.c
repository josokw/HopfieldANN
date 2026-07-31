#include "HopfieldCalc.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

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

bool learnHebbian(HopfieldContext *ctx)
{
   if (ctx == NULL || ctx->nPatterns <= 0 || ctx->nPatterns > NMAX_PATTERNS ||
       ctx->patternSize <= 0 || ctx->patternSize > NMAX_NEURONS) {
      return false;
   }

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
   assert(hasZeroDiagonal(ctx->patternSize,
                          (const double (*)[NMAX_NEURONS])ctx->W));
   assert(isSymmetric(ctx->patternSize,
                       (const double (*)[NMAX_NEURONS])ctx->W));
   return true;
}

bool learnStorkey(HopfieldContext *ctx)
{
   if (ctx == NULL || ctx->nPatterns <= 0 || ctx->nPatterns > NMAX_PATTERNS ||
       ctx->patternSize <= 0 || ctx->patternSize > NMAX_NEURONS) {
      return false;
   }

   int N = ctx->patternSize;

   for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
         ctx->W[i][j] = 0.0;
      }
   }

   for (int pat = 0; pat < ctx->nPatterns; pat++) {
      double *xi = ctx->patterns[pat];

      double field[NMAX_NEURONS];
      for (int i = 0; i < N; i++) {
         field[i] = 0.0;
         for (int k = 0; k < N; k++) {
            field[i] += ctx->W[i][k] * xi[k];
         }
      }

      for (int i = 0; i < N; i++) {
         for (int j = i + 1; j < N; j++) {
            double h_ij = field[i] - ctx->W[i][j] * xi[j];
            double h_ji = field[j] - ctx->W[i][j] * xi[i];
            double delta = (xi[i] * xi[j] - xi[i] * h_ji - h_ij * xi[j]) /
                           (double)N;
            ctx->W[i][j] += delta;
            ctx->W[j][i] = ctx->W[i][j];
         }
         ctx->W[i][i] = 0.0;
      }
   }

   assert(hasZeroDiagonal(ctx->patternSize,
                            (const double (*)[NMAX_NEURONS])ctx->W));
   assert(isSymmetric(ctx->patternSize,
                        (const double (*)[NMAX_NEURONS])ctx->W));
   return true;
}

bool learnPseudoInverse(HopfieldContext *ctx)
{
   if (ctx == NULL || ctx->nPatterns <= 0 || ctx->nPatterns > NMAX_PATTERNS ||
       ctx->patternSize <= 0 || ctx->patternSize > NMAX_NEURONS) {
      return false;
   }

   const int N = ctx->patternSize;
   const int P = ctx->nPatterns;

   /* Gram matrix G[mu][nu] = (1/N) sum_i xi^mu_i * xi^nu_i */
   double G[NMAX_PATTERNS][NMAX_PATTERNS] = {{0}};
   for (int mu = 0; mu < P; mu++) {
      for (int nu = 0; nu < P; nu++) {
         double sum = 0.0;
         for (int i = 0; i < N; i++) {
            sum += ctx->patterns[mu][i] * ctx->patterns[nu][i];
         }
         G[mu][nu] = sum / (double)N;
      }
   }

   /* Invert G using Gauss-Jordan elimination with partial pivoting */
   double inv[NMAX_PATTERNS][NMAX_PATTERNS] = {{0}};
   for (int mu = 0; mu < P; mu++) {
      inv[mu][mu] = 1.0;
   }
   for (int col = 0; col < P; col++) {
      int pivot = col;
      for (int row = col + 1; row < P; row++) {
         if (fabs(G[row][col]) > fabs(G[pivot][col])) {
            pivot = row;
         }
      }
      if (fabs(G[pivot][col]) < 1e-12) {
         return false; /* singular: patterns are linearly dependent */
      }
      if (pivot != col) {
         for (int k = 0; k < P; k++) {
            double tmp = G[col][k];
            G[col][k] = G[pivot][k];
            G[pivot][k] = tmp;
            tmp = inv[col][k];
            inv[col][k] = inv[pivot][k];
            inv[pivot][k] = tmp;
         }
      }
      double pivotValue = G[col][col];
      for (int k = 0; k < P; k++) {
         G[col][k] /= pivotValue;
         inv[col][k] /= pivotValue;
      }
      for (int row = 0; row < P; row++) {
         if (row == col) {
            continue;
         }
         double factor = G[row][col];
         for (int k = 0; k < P; k++) {
            G[row][k] -= factor * G[col][k];
            inv[row][k] -= factor * inv[col][k];
         }
      }
   }

   /* M[mu][i] = sum_nu inv[mu][nu] * xi^nu_i */
   double M[NMAX_PATTERNS][NMAX_NEURONS];
   for (int mu = 0; mu < P; mu++) {
      for (int i = 0; i < N; i++) {
         double sum = 0.0;
         for (int nu = 0; nu < P; nu++) {
            sum += inv[mu][nu] * ctx->patterns[nu][i];
         }
         M[mu][i] = sum;
      }
   }

   /* W[i][j] = (1/N) sum_mu xi^mu_i * M[mu][j] */
   for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
         double sum = 0.0;
         for (int mu = 0; mu < P; mu++) {
            sum += ctx->patterns[mu][i] * M[mu][j];
         }
         ctx->W[i][j] = sum / (double)N;
      }
   }

   /* Enforce exact symmetry to cancel floating-point rounding */
   for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
         ctx->W[j][i] = ctx->W[i][j];
      }
   }

   assert(isSymmetric(N, (const double (*)[NMAX_NEURONS])ctx->W));
   return true;
}

int addNoiseToPattern(HopfieldContext *ctx, const int patNumber, int chance)
{
   if (ctx == NULL || patNumber < 0 || patNumber >= ctx->nPatterns ||
       ctx->patternSize <= 0 || ctx->patternSize > NMAX_NEURONS) {
      return -1;
   }

   if (chance < 0)
      chance = 0;
   if (chance > MAX_NOISE_PERCENT)
      chance = MAX_NOISE_PERCENT;

   int nNoise = ctx->patternSize * chance / 100;

   /* Fisher-Yates partial shuffle: pick nNoise distinct indices in O(nNoise) */
   int indices[NMAX_NEURONS];
   for (int i = 0; i < ctx->patternSize; i++) {
      indices[i] = i;
   }
   for (int i = 0; i < nNoise; i++) {
      int j = i + randomInt(0, ctx->patternSize - i - 1);
      int tmp = indices[i];
      indices[i] = indices[j];
      indices[j] = tmp;
   }

   /* Build noisy pattern in a temp buffer, flip selected indices */
   double noisyPattern[NMAX_NEURONS];
   copyPattern(ctx->patternSize, ctx->patterns[patNumber], noisyPattern);
   for (int i = 0; i < nNoise; i++) {
      noisyPattern[indices[i]] = -noisyPattern[indices[i]];
   }
   copyPattern(ctx->patternSize, noisyPattern,
               ctx->noisyPatterns[patNumber]);
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

void calcOutputPatternAsync(const int patternSize,
                           const double w[][NMAX_NEURONS],
                           double pattern[])
{
   int order[NMAX_NEURONS];
   for (int i = 0; i < patternSize; i++) {
      order[i] = i;
   }
   for (int i = 0; i < patternSize; i++) {
      int j = i + randomInt(0, patternSize - i - 1);
      int tmp = order[i];
      order[i] = order[j];
      order[j] = tmp;
   }
   for (int k = 0; k < patternSize; k++) {
      int idx = order[k];
      double delta = 0.0;
      for (int j = 0; j < patternSize; j++) {
         delta += pattern[j] * w[idx][j];
      }
      pattern[idx] = sign(delta);
   }
}

double calcOverlap(const int patternSize, const double pattern1[],
                   const double pattern2[])
{
   double sum = 0.0;
   for (int i = 0; i < patternSize; i++) {
      sum += pattern1[i] * pattern2[i];
   }
   return sum / (double)patternSize;
}

int calcHammingDistance(const int patternSize, const double pattern1[],
                        const double pattern2[])
{
   int dist = 0;
   for (int i = 0; i < patternSize; i++) {
      if (!equals(pattern1[i], pattern2[i])) {
         dist++;
      }
   }
   return dist;
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

bool convergePattern(HopfieldContext *ctx,
                     const double inputPattern[],
                     double outputPattern[],
                     ConvergenceCallback callback,
                     void *user_data,
                     double *finalEnergy)
{
   if (ctx == NULL || ctx->patternSize <= 0 ||
       ctx->patternSize > NMAX_NEURONS) {
      if (finalEnergy) *finalEnergy = 0.0;
      return false;
   }

   double pattern[NMAX_NEURONS] = {0};
   copyPattern(ctx->patternSize, inputPattern, pattern);

   double energy = calcEnergy(ctx->patternSize, pattern,
                             (const double (*)[NMAX_NEURONS])ctx->W);
   bool converged = false;
   int iter = 0;

   do {
      double previousPattern[NMAX_NEURONS];
      copyPattern(ctx->patternSize, pattern, previousPattern);
      calcOutputPatternAsync(ctx->patternSize,
                            (const double (*)[NMAX_NEURONS])ctx->W,
                            pattern);
      int flips = 0;
      for (int i = 0; i < ctx->patternSize; i++) {
         if (!equals(previousPattern[i], pattern[i])) {
            flips++;
         }
      }
      energy = calcEnergy(ctx->patternSize, pattern,
                          (const double (*)[NMAX_NEURONS])ctx->W);
      iter++;
      if (callback) callback(iter, energy, pattern, user_data);
      if (flips == 0) {
         converged = true;
         break;
      }
   } while (iter < MAX_ITERATIONS);

   copyPattern(ctx->patternSize, pattern, outputPattern);
   if (finalEnergy) *finalEnergy = energy;
   return converged;
}

bool calcAssociatedPattern(HopfieldContext *ctx,
                           const double inputPattern[],
                           double associatedPattern[],
                           double *finalEnergy)
{
   return convergePattern(ctx, inputPattern, associatedPattern,
                          NULL, NULL, finalEnergy);
}



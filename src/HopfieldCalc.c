#include "HopfieldCalc.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

bool isSymmetric(const int patternSize, const double *const w[])
{
   assert(patternSize > 0);

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

bool hasZeroDiagonal(const int patternSize, const double *const w[])
{
   assert(patternSize > 0);

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
   if (ctx == NULL || ctx->patterns == NULL || ctx->W == NULL ||
       ctx->nPatterns <= 0 || ctx->patternSize <= 0) {
      return false;
   }

   ctx->modernHopfield = false;

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
   assert(
      hasZeroDiagonal(ctx->patternSize, (const double *const *)ctx->W));
   assert(isSymmetric(ctx->patternSize, (const double *const *)ctx->W));
   return true;
}

bool learnStorkey(HopfieldContext *ctx)
{
   if (ctx == NULL || ctx->patterns == NULL || ctx->W == NULL ||
       ctx->nPatterns <= 0 || ctx->patternSize <= 0) {
      return false;
   }

   ctx->modernHopfield = false;

   int N = ctx->patternSize;

   for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
         ctx->W[i][j] = 0.0;
      }
   }

   double *field = (double *)malloc((size_t)N * sizeof(double));
   if (field == NULL) {
      return false;
   }

   for (int pat = 0; pat < ctx->nPatterns; pat++) {
      double *xi = ctx->patterns[pat];

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
            double delta =
               (xi[i] * xi[j] - xi[i] * h_ji - h_ij * xi[j]) / (double)N;
            ctx->W[i][j] += delta;
            ctx->W[j][i] = ctx->W[i][j];
         }
         ctx->W[i][i] = 0.0;
      }
   }

   free(field);

   assert(
      hasZeroDiagonal(ctx->patternSize, (const double *const *)ctx->W));
   assert(isSymmetric(ctx->patternSize, (const double *const *)ctx->W));
   return true;
}

bool learnPseudoInverse(HopfieldContext *ctx)
{
   if (ctx == NULL || ctx->patterns == NULL || ctx->W == NULL ||
       ctx->nPatterns <= 0 || ctx->patternSize <= 0) {
      return false;
   }

   ctx->modernHopfield = false;

   const int N = ctx->patternSize;
   const int P = ctx->nPatterns;

   double **G = allocMatrix(P, P);
   double **inv = allocMatrix(P, P);
   double **M = allocMatrix(P, N);
   if (G == NULL || inv == NULL || M == NULL) {
      freeMatrix(G);
      freeMatrix(inv);
      freeMatrix(M);
      return false;
   }

   /* Gram matrix G[mu][nu] = (1/N) sum_i xi^mu_i * xi^nu_i */
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
      if (fabs(G[pivot][col]) < PSEUDO_INVERSE_SINGULARITY_THRESHOLD) {
         freeMatrix(G); /* singular: patterns are linearly dependent */
         freeMatrix(inv);
         freeMatrix(M);
         return false;
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

   freeMatrix(G);
   freeMatrix(inv);
   freeMatrix(M);

   assert(isSymmetric(N, (const double *const *)ctx->W));
   return true;
}

/* Largest |eigenvalue| of the symmetric matrix w, computed by power
   iteration. This is the spectral norm ||w||_2 used to renormalize the
   coupling matrix. */
static double spectralNorm(const int patternSize, double *const w[])
{
   double *v = (double *)malloc((size_t)patternSize * sizeof(double));
   double *u = (double *)malloc((size_t)patternSize * sizeof(double));
   if (v == NULL || u == NULL) {
      free(v);
      free(u);
      return 0.0;
   }

   for (int i = 0; i < patternSize; i++) {
      v[i] = (randomInt(0, 1) == 0) ? 1.0 : -1.0;
   }

   double norm = 0.0;
   for (int iter = 0; iter < DAYDREAMING_NORM_ITERATIONS; iter++) {
      for (int i = 0; i < patternSize; i++) {
         double sum = 0.0;
         for (int j = 0; j < patternSize; j++) {
            sum += w[i][j] * v[j];
         }
         u[i] = sum;
      }
      norm = 0.0;
      for (int i = 0; i < patternSize; i++) {
         norm += u[i] * u[i];
      }
      norm = sqrt(norm);
      if (norm < 1e-12) {
         free(v);
         free(u);
         return 0.0;
      }
      for (int i = 0; i < patternSize; i++) {
         v[i] = u[i] / norm;
      }
   }

   free(v);
   free(u);
   return norm;
}

/* Daydreaming learning rule (Serricchio et al., "Daydreaming Hopfield
   Networks and their surprising effectiveness on correlated data", 2025).
   Starts from the Hebbian coupling matrix, then repeatedly reinforces a
   random stored pattern while unlearning the fixed point reached from a
   random configuration. Both terms are updated every step, so the
   procedure can be iterated indefinitely without destroying the stored
   patterns. */
bool learnDaydreaming(HopfieldContext *ctx)
{
   if (ctx == NULL || ctx->patterns == NULL || ctx->W == NULL ||
       ctx->nPatterns <= 0 || ctx->patternSize <= 0) {
      return false;
   }

   ctx->modernHopfield = false;

   const int N = ctx->patternSize;
   const int P = ctx->nPatterns;

   if (!learnHebbian(ctx)) {
      return false;
   }

   double *xi = (double *)malloc((size_t)N * sizeof(double));
   double *sigma = (double *)malloc((size_t)N * sizeof(double));
   double *fixed = (double *)malloc((size_t)N * sizeof(double));
   if (xi == NULL || sigma == NULL || fixed == NULL) {
      free(xi);
      free(sigma);
      free(fixed);
      return false;
   }

   const double tau = DAYDREAMING_TAU_FACTOR * (double)N;
   const double scale = 1.0 / (tau * (double)N);

   for (int epoch = 0; epoch < DAYDREAMING_EPOCHS; epoch++) {
      for (int step = 0; step < N; step++) {
         int mu = randomInt(0, P - 1);
         copyPattern(N, ctx->patterns[mu], xi);
         for (int i = 0; i < N; i++) {
            sigma[i] = (randomInt(0, 1) == 0) ? 1.0 : -1.0;
         }
         convergePattern(ctx, sigma, fixed, NULL, NULL, NULL);
         for (int i = 0; i < N; i++) {
            for (int j = i + 1; j < N; j++) {
               double delta =
                  scale * (xi[i] * xi[j] - fixed[i] * fixed[j]);
               ctx->W[i][j] += delta;
               ctx->W[j][i] = ctx->W[i][j];
            }
         }
      }

      double norm = spectralNorm(N, ctx->W);
      if (norm > 0.0) {
         for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
               ctx->W[i][j] /= norm;
            }
         }
      }
   }

   free(xi);
   free(sigma);
   free(fixed);

   assert(
      hasZeroDiagonal(ctx->patternSize, (const double *const *)ctx->W));
   assert(isSymmetric(ctx->patternSize, (const double *const *)ctx->W));
   return true;
}

/* Modern Hopfield learning rule (Ramsauer et al., "Hopfield Networks is
   All You Need", 2020). The stored patterns themselves are the memory;
   retrieval is one softmax-attention read-out, x <- Xi * softmax(beta *
   Xi^T x), whose energy E(x) = -lse(beta, Xi^T x) + 0.5*||x||^2 can store
   exponentially many patterns with exponentially small retrieval error. W
   is still filled with the Hebbian coupling matrix so the
   connection-matrix invariants hold, but retrieval uses the stored
   patterns directly. */
bool learnModernHopfield(HopfieldContext *ctx)
{
   if (ctx == NULL || ctx->patterns == NULL || ctx->W == NULL ||
       ctx->nPatterns <= 0 || ctx->patternSize <= 0) {
      return false;
   }

   if (!learnHebbian(ctx)) {
      return false;
   }

   ctx->modernHopfield = true;
   ctx->modernBeta = MODERN_BETA;

   assert(isSymmetric(ctx->patternSize, (const double *const *)ctx->W));
   assert(
      hasZeroDiagonal(ctx->patternSize, (const double *const *)ctx->W));
   return true;
}

/* Compute the M overlaps a_mu = beta * <xi^mu, pattern> and return their
 * max. */
static double computeOverlaps(const HopfieldContext *ctx,
                              const double pattern[], double overlaps[])
{
   const int P = ctx->nPatterns;
   const int N = ctx->patternSize;
   const double beta = ctx->modernBeta;

   double maxA = -HUGE_VAL;
   for (int mu = 0; mu < P; mu++) {
      double sum = 0.0;
      for (int i = 0; i < N; i++) {
         sum += ctx->patterns[mu][i] * pattern[i];
      }
      overlaps[mu] = beta * sum;
      if (overlaps[mu] > maxA) {
         maxA = overlaps[mu];
      }
   }
   return maxA;
}

/* Log-sum-exp of beta * Xi^T x computed from the pre-scaled overlaps,
   shifted by their max for numerical stability. */
static double lseFromOverlaps(const double overlaps[], int nPatterns,
                              double maxA)
{
   double sum = 0.0;
   for (int mu = 0; mu < nPatterns; mu++) {
      sum += exp(overlaps[mu] - maxA);
   }
   return maxA + log(sum);
}

double calcModernEnergy(const HopfieldContext *ctx, const double pattern[])
{
   if (ctx == NULL || ctx->patterns == NULL || ctx->nPatterns <= 0 ||
       ctx->patternSize <= 0) {
      return 0.0;
   }

   double *overlaps =
      (double *)malloc((size_t)ctx->nPatterns * sizeof(double));
   if (overlaps == NULL) {
      return 0.0;
   }

   double maxA = computeOverlaps(ctx, pattern, overlaps);
   double energy = -lseFromOverlaps(overlaps, ctx->nPatterns, maxA);
   for (int i = 0; i < ctx->patternSize; i++) {
      energy += 0.5 * pattern[i] * pattern[i];
   }

   free(overlaps);
   return energy;
}

/* Modern Hopfield retrieval: iterate x <- Xi * softmax(beta * Xi^T x)
   until the state stabilizes (max |x_i - prev_i| <
   MODERN_CONVERGENCE_EPSILON) or the iteration budget is exhausted. Clean
   inputs settle in a single update (the paper's key property); noisy
   inputs need one more. The final output is thresholded to +/- 1 so the
   binary display and recall-quality helpers stay valid; the iteration
   callback receives a thresholded copy for the same reason. */
static bool convergeModernPattern(HopfieldContext *ctx,
                                  const double inputPattern[],
                                  double outputPattern[],
                                  ConvergenceCallback callback,
                                  void *user_data, double *finalEnergy)
{
   const int N = ctx->patternSize;
   const int P = ctx->nPatterns;

   double *x = (double *)malloc((size_t)N * sizeof(double));
   double *prev = (double *)malloc((size_t)N * sizeof(double));
   double *overlaps = (double *)malloc((size_t)P * sizeof(double));
   double *weights = (double *)malloc((size_t)P * sizeof(double));
   double *display = (double *)malloc((size_t)N * sizeof(double));
   if (x == NULL || prev == NULL || overlaps == NULL || weights == NULL ||
       display == NULL) {
      free(x);
      free(prev);
      free(overlaps);
      free(weights);
      free(display);
      if (finalEnergy)
         *finalEnergy = 0.0;
      return false;
   }

   copyPattern(N, inputPattern, x);

   double energy = 0.0;
   bool converged = false;
   int iter = 0;

   do {
      copyPattern(N, x, prev);

      double maxA = computeOverlaps(ctx, x, overlaps);
      double norm = 0.0;
      for (int mu = 0; mu < P; mu++) {
         weights[mu] = exp(overlaps[mu] - maxA);
         norm += weights[mu];
      }
      for (int mu = 0; mu < P; mu++) {
         weights[mu] /= norm;
      }

      /* x <- Xi * softmax(beta * Xi^T x) */
      for (int i = 0; i < N; i++) {
         double sum = 0.0;
         for (int mu = 0; mu < P; mu++) {
            sum += ctx->patterns[mu][i] * weights[mu];
         }
         x[i] = sum;
      }

      /* E(x) = -lse(beta, Xi^T x) + 0.5 * ||x||^2, evaluated at the new
       * state */
      maxA = computeOverlaps(ctx, x, overlaps);
      energy = -lseFromOverlaps(overlaps, P, maxA);
      for (int i = 0; i < N; i++) {
         energy += 0.5 * x[i] * x[i];
      }

      iter++;
      if (callback) {
         for (int i = 0; i < N; i++) {
            display[i] = (double)sign(x[i]);
         }
         callback(iter, energy, display, user_data);
      }

      double maxDelta = 0.0;
      for (int i = 0; i < N; i++) {
         double delta = fabs(x[i] - prev[i]);
         if (delta > maxDelta) {
            maxDelta = delta;
         }
      }
      if (maxDelta < MODERN_CONVERGENCE_EPSILON) {
         converged = true;
         break;
      }
   } while (iter < MAX_ITERATIONS);

   for (int i = 0; i < N; i++) {
      outputPattern[i] = (double)sign(x[i]);
   }

   free(x);
   free(prev);
   free(overlaps);
   free(weights);
   free(display);
   if (finalEnergy)
      *finalEnergy = energy;
   return converged;
}

int addNoiseToPattern(HopfieldContext *ctx, const int patNumber,
                      int chance)
{
   if (ctx == NULL || ctx->patterns == NULL || ctx->patternSize <= 0 ||
       patNumber < 0 || patNumber >= ctx->nPatterns) {
      return -1;
   }

   if (ctx->noisyPatterns == NULL || patNumber >= ctx->nNoisyPatterns) {
      int required = patNumber + 1;
      if (ctx->noisyPatterns != NULL) {
         freeMatrix(ctx->noisyPatterns);
      }
      ctx->noisyPatterns = allocMatrix(required, ctx->patternSize);
      if (ctx->noisyPatterns == NULL) {
         ctx->nNoisyPatterns = 0;
         return -1;
      }
      ctx->nNoisyPatterns = required;
   }

   if (chance < 0)
      chance = 0;
   if (chance > MAX_NOISE_PERCENT)
      chance = MAX_NOISE_PERCENT;

   int nNoise = ctx->patternSize * chance / 100;

   int *indices = (int *)malloc((size_t)ctx->patternSize * sizeof(int));
   double *noisyPattern =
      (double *)malloc((size_t)ctx->patternSize * sizeof(double));
   if (indices == NULL || noisyPattern == NULL) {
      free(indices);
      free(noisyPattern);
      return -1;
   }

   /* Fisher-Yates partial shuffle: pick nNoise distinct indices in
    * O(nNoise) */
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
   copyPattern(ctx->patternSize, ctx->patterns[patNumber], noisyPattern);
   for (int i = 0; i < nNoise; i++) {
      noisyPattern[indices[i]] = -noisyPattern[indices[i]];
   }
   copyPattern(ctx->patternSize, noisyPattern,
               ctx->noisyPatterns[patNumber]);

   free(indices);
   free(noisyPattern);
   return nNoise;
}

void calcOutputPattern(const int patternSize, const double *const w[],
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

void calcOutputPatternAsync(const int patternSize, const double *const w[],
                            double pattern[])
{
   int *order = (int *)malloc((size_t)patternSize * sizeof(int));
   if (order == NULL) {
      return;
   }
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
   free(order);
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
                  const double *const w[])
{
   double energy = 0.0;

   for (int i = 0; i < patternSize; i++) {
      for (int j = 0; j < patternSize; j++) {
         energy += pattern[i] * pattern[j] * w[i][j];
      }
   }

   return -0.5 * energy;
}

bool convergePattern(HopfieldContext *ctx, const double inputPattern[],
                     double outputPattern[], ConvergenceCallback callback,
                     void *user_data, double *finalEnergy)
{
   if (ctx == NULL || ctx->patternSize <= 0) {
      if (finalEnergy)
         *finalEnergy = 0.0;
      return false;
   }

   if (ctx->modernHopfield) {
      return convergeModernPattern(ctx, inputPattern, outputPattern,
                                   callback, user_data, finalEnergy);
   }

   if (ctx->W == NULL) {
      if (finalEnergy)
         *finalEnergy = 0.0;
      return false;
   }

   double *pattern =
      (double *)malloc((size_t)ctx->patternSize * sizeof(double));
   double *previousPattern =
      (double *)malloc((size_t)ctx->patternSize * sizeof(double));
   if (pattern == NULL || previousPattern == NULL) {
      free(pattern);
      free(previousPattern);
      if (finalEnergy)
         *finalEnergy = 0.0;
      return false;
   }

   copyPattern(ctx->patternSize, inputPattern, pattern);

   double energy =
      calcEnergy(ctx->patternSize, pattern, (const double *const *)ctx->W);
   bool converged = false;
   int iter = 0;

   do {
      copyPattern(ctx->patternSize, pattern, previousPattern);
      calcOutputPatternAsync(ctx->patternSize,
                             (const double *const *)ctx->W, pattern);
      int flips = 0;
      for (int i = 0; i < ctx->patternSize; i++) {
         if (!equals(previousPattern[i], pattern[i])) {
            flips++;
         }
      }
      energy = calcEnergy(ctx->patternSize, pattern,
                          (const double *const *)ctx->W);
      iter++;
      if (callback)
         callback(iter, energy, pattern, user_data);
      if (flips == 0) {
         converged = true;
         break;
      }
   } while (iter < MAX_ITERATIONS);

   copyPattern(ctx->patternSize, pattern, outputPattern);
   free(pattern);
   free(previousPattern);
   if (finalEnergy)
      *finalEnergy = energy;
   return converged;
}

bool calcAssociatedPattern(HopfieldContext *ctx,
                           const double inputPattern[],
                           double associatedPattern[], double *finalEnergy)
{
   return convergePattern(ctx, inputPattern, associatedPattern, NULL, NULL,
                          finalEnergy);
}

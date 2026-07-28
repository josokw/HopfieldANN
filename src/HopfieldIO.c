#include "HopfieldIO.h"
#include "HopfieldCalc.h"

#include <stdio.h>

HopfieldError readFile(HopfieldContext *ctx, const char fileName[])
{
   if (ctx == NULL) {
      return HOPFIELD_ERR_INVALID_FORMAT;
   }

   FILE *hfDataFile = fopen(fileName, "r");
   if (hfDataFile == NULL) {
      return HOPFIELD_ERR_FILE_NOT_FOUND;
   }
   if (fscanf(hfDataFile, "%d %d %d", &ctx->nRows, &ctx->nColumns,
              &ctx->nPatterns) != 3) {
      fclose(hfDataFile);
      return HOPFIELD_ERR_INVALID_FORMAT;
   }

   if (ctx->nRows <= 0 || ctx->nColumns <= 0) {
      fclose(hfDataFile);
      return HOPFIELD_ERR_INVALID_FORMAT;
   }

   if (ctx->nPatterns <= 0 || ctx->nPatterns > NMAX_PATTERNS) {
      fclose(hfDataFile);
      return HOPFIELD_ERR_SIZE_EXCEEDED;
   }

   if (ctx->nRows > NMAX_NEURONS || ctx->nColumns > NMAX_NEURONS ||
       ctx->nRows > NMAX_NEURONS / ctx->nColumns) {
      fclose(hfDataFile);
      return HOPFIELD_ERR_SIZE_EXCEEDED;
   }
   ctx->patternSize = ctx->nColumns * ctx->nRows;

   // Line has maximum size if pattern is flat: height = 1
   char line[NMAX_NEURONS + 1] = {'\0'};

   for (int nP = 0; nP < ctx->nPatterns; nP++) {
      for (int nR = 0; nR < ctx->nRows; nR++) {
         if (fgets(line, NMAX_NEURONS, hfDataFile) == NULL) {
            fclose(hfDataFile);
            return HOPFIELD_ERR_INVALID_FORMAT;
         }
         if (line[0] != '\n' && line[0] != '\r') {
            for (int nC = 0; nC < ctx->nColumns; nC++) {
               if (line[nC] == '*') {
                  ctx->patterns[nP][nR * ctx->nColumns + nC] = 1;
               }
               else {
                  ctx->patterns[nP][nR * ctx->nColumns + nC] = -1;
               }
            }
         }
         else {
            nR--;
         }
      }
   }
   fclose(hfDataFile);
   return HOPFIELD_OK;
}

bool showIndexedPattern(const HopfieldContext *ctx, int index)
{
   if (ctx == NULL || index >= ctx->nPatterns || index < 0) {
      fprintf(stderr, "\n\tERROR: index %d out of range\n\n", index);
      return false;
   }
   return showPatternAndDifference(ctx, ctx->patterns[index],
                                   ctx->patterns[index]);
}

HopfieldError readNoisyFile(HopfieldContext *ctx, const char fileName[])
{
   if (ctx == NULL) {
      return HOPFIELD_ERR_INVALID_FORMAT;
   }

   char line[NMAX_NEURONS + 1] = {'\0'};
   int nR = 0;
   int nC = 0;
   int nP = 0;
   int nNoisyRows = 0;
   int nNoisyColumns = 0;

   FILE *hfDataFile = fopen(fileName, "r");

   if (hfDataFile == NULL) {
      return HOPFIELD_ERR_FILE_NOT_FOUND;
   }

   if (fscanf(hfDataFile, "%d %d %d", &nNoisyRows, &nNoisyColumns,
               &ctx->nNoisyPatterns) != 3) {
      fclose(hfDataFile);
      return HOPFIELD_ERR_INVALID_FORMAT;
   }

   if (nNoisyRows <= 0 || nNoisyColumns <= 0) {
      fclose(hfDataFile);
      return HOPFIELD_ERR_INVALID_FORMAT;
   }

   if (ctx->nRows != nNoisyRows || ctx->nColumns != nNoisyColumns) {
      fclose(hfDataFile);
      return HOPFIELD_ERR_INVALID_FORMAT;
   }

   if (ctx->nNoisyPatterns <= 0 || ctx->nNoisyPatterns > NMAX_PATTERNS) {
      fclose(hfDataFile);
      return HOPFIELD_ERR_SIZE_EXCEEDED;
   }

   for (nP = 0; nP < ctx->nNoisyPatterns; nP++) {
      for (nR = 0; nR < ctx->nRows; nR++) {
         if (fgets(line, NMAX_NEURONS, hfDataFile) == NULL) {
            fclose(hfDataFile);
            return HOPFIELD_ERR_INVALID_FORMAT;
         }
         if (line[0] != '\n' && line[0] != '\r') {
            for (nC = 0; nC < ctx->nColumns; nC++) {
               if (line[nC] == '*') {
                  ctx->noisyPatterns[nP][nR * ctx->nColumns + nC] = 1.0;
               }
               else {
                  ctx->noisyPatterns[nP][nR * ctx->nColumns + nC] = -1.0;
               }
            }
         }
         else {
            nR--;
         }
      }
   }

   fclose(hfDataFile);
   return HOPFIELD_OK;
}

bool showIndexedNoisyPattern(const HopfieldContext *ctx, int index)
{
   if (ctx == NULL || index >= ctx->nNoisyPatterns || index < 0) {
      fprintf(stderr, "\n\tERROR: index %d out of range\n\n", index);
      return false;
   }
   return showPatternAndDifference(ctx, ctx->noisyPatterns[index],
                                   ctx->noisyPatterns[index]);
}

bool showPattern(const HopfieldContext *ctx, const double pattern[])
{
   if (ctx == NULL) {
      return false;
   }

   for (int nR = 0; nR < ctx->nRows; nR++) {
      for (int nC = 0; nC < ctx->nColumns; nC++) {
         if (equals(pattern[nR * ctx->nColumns + nC], 1.0)) {
            printf("*");
         }
         else {
            if (equals(pattern[nR * ctx->nColumns + nC], -1.0)) {
               printf(".");
            }
            else {
               fprintf(stderr,
                       "\n\tERROR: pattern value %+f out of range\n\n",
                       pattern[nR * ctx->nColumns + nC]);
               return false;
            }
         }
      }
      puts("");
   }
   return true;
}

bool showPatternAndDifference(const HopfieldContext *ctx,
                              const double pattern[],
                              const double patternWithNoise[])
{
   if (ctx == NULL) {
      return false;
   }

   for (int nR = 0; nR < ctx->nRows; nR++) {
      for (int nC = 0; nC < ctx->nColumns; nC++) {
         if (equals(patternWithNoise[nR * ctx->nColumns + nC], +1.0)) {
            printf("*");
         }
         else {
            if (equals(patternWithNoise[nR * ctx->nColumns + nC], -1.0)) {
               printf(".");
            }
            else {
               fprintf(stderr,
                       "\n\tERROR: pattern value %+f out of range\n\n",
                       patternWithNoise[nR * ctx->nColumns + nC]);
               return false;
            }
         }
      }
      printf("    ");
      for (int nC = 0; nC < ctx->nColumns; nC++) {
         if (equals(patternWithNoise[nR * ctx->nColumns + nC],
                    pattern[nR * ctx->nColumns + nC])) {
            printf(" ");
         }
         else {
            printf("#");
         }
      }
      printf("\n");
   }
   return true;
}

bool showPatternAsVector(const HopfieldContext *ctx, const double pattern[])
{
   if (ctx == NULL) {
      return false;
   }

   for (int n = 0; n < ctx->nRows * ctx->nColumns; n++) {
      if (equals(pattern[n], +1.0)) {
         printf("*");
      }
      else {
         if (equals(pattern[n], -1.0)) {
            printf(".");
         }
         else {
            fprintf(stderr,
                    "\n\tERROR: pattern value %+f out of range\n\n",
                    pattern[n]);
            return false;
         }
      }
   }
   return true;
}

void showAssociatedPattern(HopfieldContext *ctx,
                            const double inputPattern[],
                            const double inputPatternWithNoise[],
                            double associatedPattern[])
{
   if (ctx == NULL || ctx->patternSize <= 0 ||
       ctx->patternSize > NMAX_NEURONS) {
      return;
   }

   double patternWithNoise[NMAX_NEURONS] = {0};
   copyPattern(ctx->patternSize, inputPatternWithNoise, patternWithNoise);

   double energy = 0.0;
   double energyPrevious = 0.0;
   int iter = 0;

   showPatternAndDifference(ctx, inputPattern, patternWithNoise);
   energy = calcEnergy(ctx->patternSize, patternWithNoise,
                        (const double (*)[NMAX_NEURONS])ctx->W);
   printf("\n    Energy = %9.4f\n\n", energy);

   do {
      energyPrevious = energy;
      calcOutputPatternAsync(ctx->patternSize,
                            (const double (*)[NMAX_NEURONS])ctx->W,
                            patternWithNoise);
      energy = calcEnergy(ctx->patternSize, patternWithNoise,
                          (const double (*)[NMAX_NEURONS])ctx->W);
      showPatternAndDifference(ctx, inputPattern, patternWithNoise);
      printf("\n    Energy = %9.4f\n\n", energy);
      iter++;
   } while (!equals(energyPrevious, energy) && iter < MAX_ITERATIONS);

   copyPattern(ctx->patternSize, patternWithNoise, associatedPattern);
   double overlap = calcOverlap(ctx->patternSize, inputPattern,
                                associatedPattern);
   int hamming = calcHammingDistance(ctx->patternSize, inputPattern,
                                     associatedPattern);
   printf("\n    Recall quality: overlap = %7.4f, "
          "Hamming distance = %d / %d\n",
          overlap, hamming, ctx->patternSize);
}

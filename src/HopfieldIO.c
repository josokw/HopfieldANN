#include "HopfieldIO.h"
#include "HopfieldCalc.h"

#include <stdio.h>

static HopfieldError read_pattern_rows(FILE *fh, int nRows, int nColumns,
                                        double storage[][NMAX_NEURONS],
                                        int patternIndex)
{
   char line[NMAX_NEURONS + 1];

   for (int nR = 0; nR < nRows; nR++) {
      if (fgets(line, sizeof(line), fh) == NULL) {
         if (ferror(fh)) {
            return HOPFIELD_ERR_INVALID_FORMAT;
         }
         return HOPFIELD_ERR_INVALID_FORMAT;
      }
      if (line[0] == '\n' || line[0] == '\r') {
         nR--;
         continue;
      }
      if (line[0] == '\0') {
         return HOPFIELD_ERR_INVALID_FORMAT;
      }
      for (int nC = 0; nC < nColumns; nC++) {
         if (line[nC] == '*') {
            storage[patternIndex][nR * nColumns + nC] = 1.0;
         }
         else {
            storage[patternIndex][nR * nColumns + nC] = -1.0;
         }
      }
   }
   return HOPFIELD_OK;
}

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

   for (int nP = 0; nP < ctx->nPatterns; nP++) {
      HopfieldError err = read_pattern_rows(hfDataFile, ctx->nRows,
                                             ctx->nColumns, ctx->patterns, nP);
      if (err != HOPFIELD_OK) {
         fclose(hfDataFile);
         return err;
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

   for (int nP = 0; nP < ctx->nNoisyPatterns; nP++) {
      HopfieldError err = read_pattern_rows(hfDataFile, ctx->nRows,
                                             ctx->nColumns,
                                             ctx->noisyPatterns, nP);
      if (err != HOPFIELD_OK) {
         fclose(hfDataFile);
         return err;
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

struct ShowIterationData {
   const HopfieldContext *ctx;
   const double *inputPattern;
};

static void show_iteration(int iteration, double energy,
                            const double pattern[], void *user_data)
{
   (void)iteration;
   const struct ShowIterationData *data = user_data;
   showPatternAndDifference(data->ctx, data->inputPattern, pattern);
   printf("\n    Energy = %9.4f\n\n", energy);
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

   struct ShowIterationData cb_data = {ctx, inputPattern};

   showPatternAndDifference(ctx, inputPattern, inputPatternWithNoise);
   double initialEnergy = calcEnergy(ctx->patternSize, inputPatternWithNoise,
                                     (const double (*)[NMAX_NEURONS])ctx->W);
   printf("\n    Energy = %9.4f\n\n", initialEnergy);

   convergePattern(ctx, inputPatternWithNoise, associatedPattern,
                   show_iteration, &cb_data);

   double overlap = calcOverlap(ctx->patternSize, inputPattern,
                                associatedPattern);
   int hamming = calcHammingDistance(ctx->patternSize, inputPattern,
                                     associatedPattern);
   printf("\n    Recall quality: overlap = %7.4f, "
          "Hamming distance = %d / %d\n",
          overlap, hamming, ctx->patternSize);
}

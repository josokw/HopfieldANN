#include "HopfieldIO.h"
#include "HopfieldCalc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Read the "rows columns patterns" header and reject any trailing non-blank
   content on the header line, which would otherwise be parsed as pattern
   data. The header's own line ending is consumed. */
static HopfieldError read_pattern_header(FILE *fh, int *nRows, int *nColumns,
                                         int *nPatterns)
{
   if (fscanf(fh, "%d %d %d", nRows, nColumns, nPatterns) != 3) {
      return HOPFIELD_ERR_INVALID_FORMAT;
   }
   int c;
   while ((c = fgetc(fh)) != '\n' && c != EOF) {
      if (c != ' ' && c != '\t' && c != '\r') {
         return HOPFIELD_ERR_INVALID_FORMAT;
      }
   }
   return HOPFIELD_OK;
}

static HopfieldError read_pattern_rows(FILE *fh, int nRows, int nColumns,
                                       double *const storage[],
                                       int patternIndex)
{
   /* Content + optional CR + LF + NUL. A full CRLF row (nColumns + 2 chars)
      fits exactly; anything longer fails the length check below. */
   size_t bufSize = (size_t)nColumns + 3;
   if (bufSize > INT_MAX) {
      return HOPFIELD_ERR_OUT_OF_MEMORY;
   }
   char *line = (char *)malloc(bufSize);
   if (line == NULL) {
      return HOPFIELD_ERR_OUT_OF_MEMORY;
   }

   int nR = 0;
   while (nR < nRows) {
      if (fgets(line, (int)bufSize, fh) == NULL) {
         free(line);
         return HOPFIELD_ERR_INVALID_FORMAT;
      }
      size_t len = strlen(line);
      if (len > 0 && line[len - 1] == '\n') {
         line[--len] = '\0';
      }
      if (len > 0 && line[len - 1] == '\r') {
         line[--len] = '\0';
      }
      if (len == 0) {
         /* Blank lines separate patterns (and end the header). Skip them
            without advancing the row counter, so leading blanks do not
            shift the first real row. */
         continue;
      }
      if (len != (size_t)nColumns) {
         free(line);
         return HOPFIELD_ERR_INVALID_FORMAT;
      }
      for (int nC = 0; nC < nColumns; nC++) {
         if (line[nC] == '*') {
            storage[patternIndex][nR * nColumns + nC] = 1.0;
         }
         else if (line[nC] == '.') {
            storage[patternIndex][nR * nColumns + nC] = -1.0;
         }
         else {
            free(line);
            return HOPFIELD_ERR_INVALID_FORMAT;
         }
      }
      nR++;
   }
   free(line);
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
   HopfieldError err = read_pattern_header(hfDataFile, &ctx->nRows,
                                           &ctx->nColumns, &ctx->nPatterns);
   if (err != HOPFIELD_OK) {
      fclose(hfDataFile);
      return err;
   }

   if (ctx->nRows <= 0 || ctx->nColumns <= 0) {
      fclose(hfDataFile);
      return HOPFIELD_ERR_INVALID_FORMAT;
   }

   if (ctx->nPatterns <= 0) {
      fclose(hfDataFile);
      return HOPFIELD_ERR_SIZE_EXCEEDED;
   }

   if (!hopfield_context_resize(ctx, ctx->nRows, ctx->nColumns,
                                ctx->nPatterns, 0)) {
      fclose(hfDataFile);
      return HOPFIELD_ERR_OUT_OF_MEMORY;
   }

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

   HopfieldError err = read_pattern_header(hfDataFile, &nNoisyRows,
                                           &nNoisyColumns,
                                           &ctx->nNoisyPatterns);
   if (err != HOPFIELD_OK) {
      fclose(hfDataFile);
      return err;
   }

   if (nNoisyRows <= 0 || nNoisyColumns <= 0) {
      fclose(hfDataFile);
      return HOPFIELD_ERR_INVALID_FORMAT;
   }

   if (ctx->nRows != nNoisyRows || ctx->nColumns != nNoisyColumns) {
      fclose(hfDataFile);
      return HOPFIELD_ERR_INVALID_FORMAT;
   }

   if (ctx->nNoisyPatterns <= 0) {
      fclose(hfDataFile);
      return HOPFIELD_ERR_SIZE_EXCEEDED;
   }

   if (!hopfield_context_set_noisy(ctx, ctx->nNoisyPatterns)) {
      fclose(hfDataFile);
      return HOPFIELD_ERR_OUT_OF_MEMORY;
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
   if (ctx == NULL || ctx->patternSize <= 0) {
      return;
   }

   struct ShowIterationData cb_data = {ctx, inputPattern};

   showPatternAndDifference(ctx, inputPattern, inputPatternWithNoise);
   double initialEnergy;
   if (ctx->modernHopfield) {
      initialEnergy = calcModernEnergy(ctx, inputPatternWithNoise);
   }
   else {
      initialEnergy = calcEnergy(ctx->patternSize, inputPatternWithNoise,
                                 (const double *const *)ctx->W);
   }
   printf("\n    Energy = %9.4f\n\n", initialEnergy);

   double finalEnergy;
   convergePattern(ctx, inputPatternWithNoise, associatedPattern,
                   show_iteration, &cb_data, &finalEnergy);

   double overlap = calcOverlap(ctx->patternSize, inputPattern,
                                associatedPattern);
   int hamming = calcHammingDistance(ctx->patternSize, inputPattern,
                                     associatedPattern);
   printf("\n    Recall quality: overlap = %7.4f, "
          "Hamming distance = %d / %d\n",
          overlap, hamming, ctx->patternSize);
}

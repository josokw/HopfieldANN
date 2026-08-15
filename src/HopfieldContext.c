#include "HopfieldContext.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

double **allocMatrix(int rows, int cols)
{
   if (rows <= 0 || cols <= 0) {
      return NULL;
   }

   size_t rowCount = (size_t)rows;
   size_t colCount = (size_t)cols;
   if (rowCount > SIZE_MAX / sizeof(double *) ||
       colCount > SIZE_MAX / sizeof(double) ||
       rowCount * colCount > SIZE_MAX / sizeof(double)) {
      return NULL;
   }

   size_t totalBytes =
      rowCount * sizeof(double *) + rowCount * colCount * sizeof(double);
   double **m = (double **)malloc(totalBytes);
   if (m == NULL) {
      return NULL;
   }
   memset(m, 0, totalBytes);

   double *data = (double *)(m + rowCount);
   for (size_t r = 0; r < rowCount; r++) {
      m[r] = data + r * colCount;
   }
   return m;
}

void freeMatrix(double **m)
{
   free(m);
}

HopfieldContext *hopfield_context_create(void)
{
   return (HopfieldContext *)calloc(1, sizeof(HopfieldContext));
}

void hopfield_context_destroy(HopfieldContext *ctx)
{
   if (ctx != NULL) {
      freeMatrix(ctx->patterns);
      freeMatrix(ctx->noisyPatterns);
      freeMatrix(ctx->W);
      free(ctx);
   }
}

bool hopfield_context_resize(HopfieldContext *ctx, int nRows, int nColumns,
                             int nPatterns, int nNoisyPatterns)
{
   if (ctx == NULL || nRows <= 0 || nColumns <= 0 || nPatterns < 0 ||
       nNoisyPatterns < 0) {
      return false;
   }

   long long sizeLL = (long long)nRows * nColumns;
   if (sizeLL <= 0 || sizeLL > INT_MAX) {
      return false;
   }
   int patternSize = (int)sizeLL;

   double **patterns = NULL;
   if (nPatterns > 0) {
      patterns = allocMatrix(nPatterns, patternSize);
      if (patterns == NULL) {
         return false;
      }
   }

   double **noisyPatterns = NULL;
   if (nNoisyPatterns > 0) {
      noisyPatterns = allocMatrix(nNoisyPatterns, patternSize);
      if (noisyPatterns == NULL) {
         freeMatrix(patterns);
         return false;
      }
   }

   double **W = allocMatrix(patternSize, patternSize);
   if (W == NULL) {
      freeMatrix(patterns);
      freeMatrix(noisyPatterns);
      return false;
   }

   freeMatrix(ctx->patterns);
   freeMatrix(ctx->noisyPatterns);
   freeMatrix(ctx->W);

   ctx->patterns = patterns;
   ctx->noisyPatterns = noisyPatterns;
   ctx->W = W;
   ctx->nRows = nRows;
   ctx->nColumns = nColumns;
   ctx->patternSize = patternSize;
   ctx->nPatterns = nPatterns;
   ctx->nNoisyPatterns = nNoisyPatterns;
   return true;
}

bool hopfield_context_set_noisy(HopfieldContext *ctx, int nNoisyPatterns)
{
   if (ctx == NULL || nNoisyPatterns < 0 || ctx->patternSize <= 0) {
      return false;
   }

   double **noisyPatterns = NULL;
   if (nNoisyPatterns > 0) {
      noisyPatterns = allocMatrix(nNoisyPatterns, ctx->patternSize);
      if (noisyPatterns == NULL) {
         return false;
      }
   }

   freeMatrix(ctx->noisyPatterns);
   ctx->noisyPatterns = noisyPatterns;
   ctx->nNoisyPatterns = nNoisyPatterns;
   return true;
}

bool hopfield_save_weights(const HopfieldContext *ctx,
                           const char *filename)
{
   if (ctx == NULL || filename == NULL) {
      return false;
   }

   FILE *f = fopen(filename, "wb");
   if (f == NULL) {
      return false;
   }

   const char magic[4] = {'H', 'O', 'P', 'F'};
   const uint8_t version = 1;

   if (fwrite(magic, 1, 4, f) != 4) {
      fclose(f);
      return false;
   }
   if (fwrite(&version, 1, 1, f) != 1) {
      fclose(f);
      return false;
   }
   if (fwrite(&ctx->patternSize, sizeof(int), 1, f) != 1) {
      fclose(f);
      return false;
   }
   if (fwrite(&ctx->nRows, sizeof(int), 1, f) != 1) {
      fclose(f);
      return false;
   }
   if (fwrite(&ctx->nColumns, sizeof(int), 1, f) != 1) {
      fclose(f);
      return false;
   }
   if (fwrite(&ctx->nPatterns, sizeof(int), 1, f) != 1) {
      fclose(f);
      return false;
   }
   if (fwrite(&ctx->modernHopfield, sizeof(bool), 1, f) != 1) {
      fclose(f);
      return false;
   }
   if (fwrite(&ctx->modernBeta, sizeof(double), 1, f) != 1) {
      fclose(f);
      return false;
   }

   for (int i = 0; i < ctx->patternSize; i++) {
      if (fwrite(ctx->W[i], sizeof(double), ctx->patternSize, f) !=
          (size_t)ctx->patternSize) {
         fclose(f);
         return false;
      }
   }

   fclose(f);
   return true;
}

bool hopfield_load_weights(HopfieldContext *ctx, const char *filename)
{
   if (ctx == NULL || filename == NULL) {
      return false;
   }

   FILE *f = fopen(filename, "rb");
   if (f == NULL) {
      return false;
   }

   char magic[4];
   uint8_t version;
   int patternSize, nRows, nColumns, nPatterns;
   bool modernHopfield;
   double modernBeta;

   if (fread(magic, 1, 4, f) != 4) {
      fclose(f);
      return false;
   }
   if (magic[0] != 'H' || magic[1] != 'O' || magic[2] != 'P' ||
       magic[3] != 'F') {
      fclose(f);
      return false;
   }
   if (fread(&version, 1, 1, f) != 1) {
      fclose(f);
      return false;
   }
   if (version != 1) {
      fclose(f);
      return false;
   }
   if (fread(&patternSize, sizeof(int), 1, f) != 1) {
      fclose(f);
      return false;
   }
   if (fread(&nRows, sizeof(int), 1, f) != 1) {
      fclose(f);
      return false;
   }
   if (fread(&nColumns, sizeof(int), 1, f) != 1) {
      fclose(f);
      return false;
   }
   if (fread(&nPatterns, sizeof(int), 1, f) != 1) {
      fclose(f);
      return false;
   }
   if (fread(&modernHopfield, sizeof(bool), 1, f) != 1) {
      fclose(f);
      return false;
   }
   if (fread(&modernBeta, sizeof(double), 1, f) != 1) {
      fclose(f);
      return false;
   }

   /* If context already has matching dimensions, preserve patterns and
    * just load W */
   bool dimensionsMatch =
      (ctx->nRows == nRows && ctx->nColumns == nColumns &&
       ctx->nPatterns == nPatterns && ctx->patternSize == patternSize);

   if (!dimensionsMatch) {
      if (!hopfield_context_resize(ctx, nRows, nColumns, nPatterns, 0)) {
         fclose(f);
         return false;
      }
   }

   ctx->modernHopfield = modernHopfield;
   ctx->modernBeta = modernBeta;

   for (int i = 0; i < patternSize; i++) {
      if (fread(ctx->W[i], sizeof(double), patternSize, f) !=
          (size_t)patternSize) {
         fclose(f);
         return false;
      }
   }

   fclose(f);
   return true;
}

#include "HopfieldContext.h"

#include <limits.h>
#include <stdint.h>
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

// Simulation Hopfield ANN

#include "HopfieldIO.h"
#include "Hopfield.h"

#include <stdio.h>
#include <stdlib.h>

static FILE *hfDataFile = NULL;
int nRows = 0;
int nColumns = 0;
int nPatterns = 0;
int patSize = 0;
int nNoisyRows = 0;
int nNoisyColumns = 0;
int nNoisyPatterns = 0;

void readFile(const char fileName[])
{
   char line[MAXN] = {'\0'};

   hfDataFile = fopen(fileName, "r");
   if (hfDataFile == NULL)
   {
      fprintf(stderr, "\n\n\tERROR: file '%s' can not be opened\n\n", fileName);
      getchar();
      exit(EXIT_FAILURE);
   }
   if (fscanf(hfDataFile, "%d %d %d", &nRows, &nColumns, &nPatterns) != 3)
   {
      fprintf(stderr, "\n\n\tERROR: format error in file '%s'\n\n", fileName);
      getchar();
      exit(EXIT_FAILURE);
   }
   patSize = nColumns * nRows;

   if (patSize > MAXN)
   {
      fprintf(stderr, "\n\tERROR: size input vector must be < %d\n\n", MAXN);
      getchar();
      exit(EXIT_FAILURE);
   }

   for (int nP = 0; nP < nPatterns; nP++)
   {
      for (int nR = 0; nR < nRows; nR++)
      {
         if (fgets(line, MAXN, hfDataFile) != NULL)
         {
            if (line[0] != '\n' && line[0] != '\r')
            {
               for (int nC = 0; nC < nColumns; nC++)
               {
                  if (line[nC] == '*')
                  {
                     Patterns[nP][nR * nColumns + nC] = 1;
                  }
                  else
                  {
                     Patterns[nP][nR * nColumns + nC] = -1;
                  }
               }
            }
            else
            {
               nR--;
            }
         }
      }
   }
   fclose(hfDataFile);
}

void showIndexedPattern(int index)
{
   if (index >= nPatterns || index < 0)
   {
      fprintf(stderr, "\n\tERROR: index %d out of range\n\n", index);
      getchar();
      exit(EXIT_FAILURE);
   }
   showPatternAndDifference(Patterns[index], Patterns[index]);
}

void readNoisyFile(const char fileName[])
{
   char line[MAXN] = {'\0'};
   int nR = 0;
   int nC = 0;
   int nP = 0;

   hfDataFile = fopen(fileName, "r");
   if (hfDataFile == NULL)
   {
      fprintf(stderr, "\n\n\tERROR: file '%s' can not be opened\n\n", fileName);
      getchar();
      exit(EXIT_FAILURE);
   }
   if (fscanf(hfDataFile, "%d %d %d", &nNoisyRows, &nNoisyColumns, &nNoisyPatterns) != 3)
   {
      fprintf(stderr, "\n\n\tERROR: format error in file '%s'\n\n", fileName);
      getchar();
      exit(EXIT_FAILURE);
   }
   if (nRows != nNoisyRows || nColumns != nNoisyColumns)
   {
      fprintf(stderr, "\n\n\tERROR: format error in file '%s'\n\n", fileName);
      getchar();
      exit(EXIT_FAILURE);
   }

   if (patSize > MAXN)
   {
      fprintf(stderr, "\n\tERROR: size input vector must be < %d\n\n", MAXN);
      getchar();
      exit(EXIT_FAILURE);
   }

   for (nP = 0; nP < nPatterns; nP++)
   {
      for (nR = 0; nR < nRows; nR++)
      {
         if (fgets(line, MAXN, hfDataFile) != NULL)
         {
            if (line[0] != '\n' && line[0] != '\r')
            {
               for (nC = 0; nC < nColumns; nC++)
               {
                  if (line[nC] == '*')
                  {
                     NoisyPatterns[nP][nR * nColumns + nC] = 1;
                  }
                  else
                  {
                     NoisyPatterns[nP][nR * nColumns + nC] = -1;
                  }
               }
            }
            else
            {
               nR--;
            }
         }
      }
   }

   fclose(hfDataFile);
}

void showIndexedNoisyPattern(int index)
{
   if (index >= nNoisyPatterns || index < 0)
   {
      fprintf(stderr, "\n\tERROR: index %d out of range\n\n", index);
      getchar();
      exit(EXIT_FAILURE);
   }
   showPatternAndDifference(NoisyPatterns[index], NoisyPatterns[index]);
}

void showPattern(const double pattern[])
{
   for (int nR = 0; nR < nRows; nR++)
   {
      for (int nC = 0; nC < nColumns; nC++)
      {
         if (pattern[nR * nColumns + nC] > 0.0)
         {
            printf("*");
         }
         else
         {
            if (pattern[nR * nColumns + nC] < 0.0)
            {
               printf(".");
            }
            else
            {
               fprintf(stderr, "\n\tERROR: pattern value %+f out of range\n\n",
                       pattern[nR * nColumns + nC]);
               getchar();
               exit(EXIT_FAILURE);
            }
         }
      }
      puts("");
   }
}

void showPatternAndDifference(const double pattern[],
                              const double patternWithNoise[])
{
   for (int nR = 0; nR < nRows; nR++)
   {
      for (int nC = 0; nC < nColumns; nC++)
      {
         if (patternWithNoise[nR * nColumns + nC] == +1.0)
         {
            printf("*");
         }
         else
         {
            if (patternWithNoise[nR * nColumns + nC] == -1.0)
            {
               printf(".");
            }
            else
            {
               fprintf(stderr, "\n\tERROR: pattern value %+f out of range\n\n",
                       pattern[nR * nColumns + nC]);
               getchar();
               exit(EXIT_FAILURE);
            }
         }
      }
      printf("    ");
      for (int nC = 0; nC < nColumns; nC++)
      {
         if (patternWithNoise[nR * nColumns + nC] == pattern[nR * nColumns + nC])
         {
            printf(" ");
         }
         else
         {
            printf("#");
         }
      }
      printf("\n");
   }
}

void showPatternAsVector(const double pattern[])
{
   int n;

   for (n = 0; n < nRows * nColumns; n++)
   {
      if (pattern[n] == +1.0)
      {
         printf("*");
      }
      else
      {
         if (pattern[n] == -1.0)
         {
            printf(".");
         }
         else
         {
            fprintf(stderr, "\n\tERROR: pattern value %+f out of range\n\n", pattern[n]);
            getchar();
            exit(EXIT_FAILURE);
         }
      }
   }
}

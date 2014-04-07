/*---------------------------------------------------------------------------*/
/* Simulation Hopfield ANN                                                   */
/*---------------------------------------------------------------------------*/

#include "HopfieldCalc.h"
#include "HopfieldIO.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

static int Sign(double Value)
{
   if (Value >= 0)
   {
      return 1;
   }
   return -1;
}

static int Random(int min, int max)
{
   static int count = 0;
   if (count == 0)
   {
      srand((unsigned int)time(NULL));
      count++;
   }

   /* Previous implementation causes a run time bug using gcc in Lunix */
   /* Generates only 0 values, RAND_MAX == MAX_INT in gcc              */
   /* return min + (int)((max - min + 1)* rand() / (RAND_MAX + 1.0));  */
   return min + (rand() % (max - min + 1)) + 1;
}

void LearnJ(int MaxPat, int PatSize, double J[][MAXN])
{
   int row;
   int column;
   int Pat;

   for (row = 0; row < PatSize; row++)
     for (column=row; column < PatSize; column++)
     {
         J[row][column] = 0.0;
         if (row != column)
         {
             for (Pat = 0; Pat < MaxPat; Pat++)
             {
                 J[row][column] += Patterns[Pat][row] * Patterns[Pat][column]
                                   / (double)PatSize;
                 J[column][row] = J[row][column];
             }
         }
     }
}

int AddNoise(int PatSize, int PatNumber, double Pat[], int Chance)
{
    int index;
    int n;
    int NoiseIndex;
    int NoiseArray[MAXN] = {0};
    int Nnoise;

    if (Chance < 0) Chance = 0;
    if (Chance > 75) Chance = 75;

    Nnoise = PatSize * Chance / 100;
    n = 0;
    while (n < Nnoise)
    {
        NoiseIndex = Random(0, PatSize);
        if (NoiseArray[NoiseIndex] == 0)
        {
            NoiseArray[NoiseIndex] = 1;
            n++;
        }
    }
    for (index = 0; index < PatSize; index++)
    {
        if (NoiseArray[index] == 1)
        {
            Pat[index] = -Patterns[PatNumber][index];
        }
        else
        {
            Pat[index] = Patterns[PatNumber][index];
        }
    }
    return Nnoise;
}

void CalcOut(int PatSize, const double J[][MAXN], const double InPattern[], double OutPattern[])
{
    int outIndex;
    int inIndex;
    double Delta;

    for (outIndex = 0; outIndex < PatSize; outIndex++)
    {
        Delta = 0.0;
        for (inIndex = 0; inIndex < PatSize; inIndex++)
        {
            Delta += InPattern[inIndex] * J[outIndex][inIndex];
        }
        OutPattern[outIndex] = Sign(Delta);
    }
}

void CopyPattern(int PatSize, const double sourcePattern[], double targetPattern[])
{
    int index;

    for (index = 0; index < PatSize; index++)
    {
        targetPattern[index] = sourcePattern[index];
    }
}

double CalcEnergy(int PatSize, const double Pattern[], const double J[][MAXN])
{
    double Energy = 0.0;
    int i;
    int j;

    for (i = 0; i < PatSize; i++)
    {
       for (j = 0; j < PatSize; j++)
       {
           Energy += Pattern[i] * Pattern[j] * J[i][j];
       }
    }
    Energy *= -0.5;

    return Energy;
}

void CalcAssociations(int PatSize,
                      const double J[][MAXN],
                      const double InputPattern[],
                      const double InputPatternWithNoise[],
                      double AssociationPattern[])
{
    double En = 0.0;
    double En_1 = 0.0;

    showPatternAndDifference(InputPattern, InputPatternWithNoise);
    En = CalcEnergy(PatSize, InputPatternWithNoise, J);
    printf("\n---- Energy: %f\n\n", En);

    do
    {
       En_1 = En;
       CalcOut(PatSize, J, InputPatternWithNoise, AssociationPattern);
       En = CalcEnergy(PatSize, AssociationPattern, J);
       CopyPattern(PatSize, AssociationPattern, InputPatternWithNoise);
       showPatternAndDifference(InputPattern, AssociationPattern);
       printf("\n---- Energy: %f\n\n", En);
    }
    while (fabs(En_1 - En) > 1e-9);
}

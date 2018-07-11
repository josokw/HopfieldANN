#include "HopfieldCalc.h"
#include "HopfieldIO.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static int Sign(double value)
{
    return (value >= 0) ? 1 : -1;
}

static int Random(int min, int max)
{
    return min + (rand() % (max - min + 1));
}

void LearnJ(int MaxPat, int PatSize, double J[][MAXN])
{
    for (int row = 0; row < PatSize; row++)
    {
        for (int column = row; column < PatSize; column++)
        {
            J[row][column] = 0.0;
            if (row != column)
            {
                for (int Pat = 0; Pat < MaxPat; Pat++)
                {
                    J[row][column] += Patterns[Pat][row] *
                                      Patterns[Pat][column] /
                                      (double)PatSize;
                    J[column][row] = J[row][column];
                }
            }
        }
    }
}

int AddNoise(int PatSize, int PatNumber, double Pat[], int Chance)
{
    int n;
    int NoiseIndex;
    int NoiseArray[MAXN] = {0};
    int Nnoise;

    if (Chance < 0)
        Chance = 0;
    if (Chance > 75)
        Chance = 75;

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
    for (int index = 0; index < PatSize; index++)
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

void CalcOut(int PatSize, const double J[][MAXN], const double InPattern[],
             double OutPattern[])
{
    for (int outIndex = 0; outIndex < PatSize; outIndex++)
    {
        double delta = 0.0;
        for (int inIndex = 0; inIndex < PatSize; inIndex++)
        {
            delta += InPattern[inIndex] * J[outIndex][inIndex];
        }
        OutPattern[outIndex] = Sign(delta);
    }
}

void CopyPattern(int PatSize, const double sourcePattern[],
                 double targetPattern[])
{
    int index;

    for (index = 0; index < PatSize; index++)
    {
        targetPattern[index] = sourcePattern[index];
    }
}

double CalcEnergy(int PatSize,
                  const double Pattern[], const double J[][MAXN])
{
    double energy = 0.0;

    for (int i = 0; i < PatSize; i++)
    {
        for (int j = 0; j < PatSize; j++)
        {
            energy += Pattern[i] * Pattern[j] * J[i][j];
        }
    }
    energy *= -0.5;

    return energy;
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
    } while (fabs(En_1 - En) > 1e-9);
}

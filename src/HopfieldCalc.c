#include "HopfieldCalc.h"
#include "HopfieldIO.h"

#include <assert.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

static const double EPSILON = 1e-8;

static bool equals(double d1, double d2)
{
    return fabs(d1 - d2) < EPSILON;
}

static int sign(double value)
{
    return (value >= 0) ? 1 : -1;
}

static int Random(int min, int max)
{
    return min + (rand() % (max - min + 1));
}

bool isSymmetric(const int patternSize, const double W[][MAXN])
{
    assert(patternSize < MAXN);

    bool isSymmetric = true;

    for (int i = 1; i < patternSize; i++)
    {
        for (int j = i; j < patternSize; j++)
        {
            if (!equals(W[i][j], W[j][i]))
            {
                isSymmetric = false;
                break;           
            }
        }
        if (!isSymmetric)
        {
            break;
        }
    }
    return isSymmetric;
}

bool hasZeroDiagonal(const int patternSize, const double W[][MAXN])
{
    assert(patternSize < MAXN);

    bool hasZD = true;

    for (int i = 0; i < patternSize; i++)
    {
        if (!equals(W[i][i], 0.0))
        {
            hasZD = false;
            break;
        }
    }
    return hasZD;
}

int storageCapacity(const int patternSize)
{
    return 0.14 * patternSize;
}

void learnW(const int MaxPat, const int patternSize, double W[][MAXN])
{
    for (int row = 0; row < patternSize; row++)
    {
        for (int column = row; column < patternSize; column++)
        {
            if (row == column)
            {
                W[row][column] = 0.0;
            }
            else
            {
                for (int Pat = 0; Pat < MaxPat; Pat++)
                {
                    W[row][column] += Patterns[Pat][row] *
                                      Patterns[Pat][column] /
                                      (double)patternSize;
                    W[column][row] = W[row][column];
                }
            }
        }
    }
    assert(hasZeroDiagonal(patternSize, W));
    assert(isSymmetric(patternSize, W));
}

int addNoise(const int patternSize, int PatNumber, double Pat[], int Chance)
{
    int n;
    int NoiseIndex;
    int NoiseArray[MAXN] = {0};
    int Nnoise;

    if (Chance < 0) Chance = 0;
    if (Chance > 75) Chance = 75;

    Nnoise = patternSize * Chance / 100;
    n = 0;
    while (n < Nnoise)
    {
        NoiseIndex = Random(0, patternSize);
        if (NoiseArray[NoiseIndex] == 0)
        {
            NoiseArray[NoiseIndex] = 1;
            n++;
        }
    }
    for (int index = 0; index < patternSize; index++)
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

void calcOut(int patternSize, 
             const double W[][MAXN], 
             const double InPattern[],
             double OutPattern[])
{
    for (int outIndex = 0; outIndex < patternSize; outIndex++)
    {
        double delta = 0.0;
        for (int inIndex = 0; inIndex < patternSize; inIndex++)
        {
            delta += InPattern[inIndex] * W[outIndex][inIndex];
        }
        OutPattern[outIndex] = sign(delta);
    }
}

void copyPattern(const int patternSize, 
                 double sourcePattern[],
                 double targetPattern[])
{
    for (int i = 0; i < patternSize; i++)
    {
        targetPattern[i] = sourcePattern[i];
    }
}

double calcEnergy(const int patternSize,
                  const double Pattern[], 
                  const double W[][MAXN])
{
    double energy = 0.0;

    for (int i = 0; i < patternSize; i++)
    {
        for (int j = 0; j < patternSize; j++)
        {
            energy += Pattern[i] * Pattern[j] * W[i][j];
        }
    }
    energy *= -0.5;

    return energy;
}

void calcAssociations(const int patternSize,
                      const double W[][MAXN],
                      const double InputPattern[],
                      const double InputPatternWithNoise[],
                      double AssociationPattern[])
{
    double energy = 0.0;
    double En_1 = 0.0;

    showPatternAndDifference(InputPattern, InputPatternWithNoise);
    energy = calcEnergy(patternSize, InputPatternWithNoise, W);
    printf("\n    Energy = %9.4f\n\n", energy);

    do
    {
        En_1 = energy;
        calcOut(patternSize, W, InputPatternWithNoise, AssociationPattern);
        energy = calcEnergy(patternSize, AssociationPattern, W);
        copyPattern(patternSize, AssociationPattern, InputPatternWithNoise);
        showPatternAndDifference(InputPattern, AssociationPattern);
        printf("\n    Energy = %9.4f\n\n", energy);
    } while (!equals(En_1, energy));
}

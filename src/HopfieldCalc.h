/*---------------------------------------------------------------------------*/
/* Simulation Hopfield ANN                                                   */
/*---------------------------------------------------------------------------*/

#ifndef HOPFIELDCALC_H
#define HOPFIELDCALC_H

#include "Hopfield.h"

#ifdef __cplusplus
extern "C" {
#endif

void LearnJ(int MaxPat, int PatSize, double J[][MAXN]);

int AddNoise(int PatSize, int PatNumber, double Pat[], int Chance);

void CalcOut(int PatSize, const double J[][MAXN], const double InPattern[],
             double OutPattern[]);

void CopyPattern(int PatSize, const double sourcePattern[],
                 double targetPattern[]);

double CalcEnergy(int PatSize, const double Pattern[], const double J[][MAXN]);

void CalcAssociations(int Patsize,
                      const double J[][MAXN],
                      const double InputPattern[],
                      const double InputPatternWithNoise[],
                      double AssociationPattern[]);

#ifdef __cplusplus
}
#endif

#endif

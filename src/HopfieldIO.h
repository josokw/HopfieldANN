/*---------------------------------------------------------------------------*/
/* Simulation Hopfield ANN                                                   */
/*---------------------------------------------------------------------------*/

#ifndef HOPFIELDIO_H
#define HOPFIELDIO_H

extern int nRows;
extern int nColumns;
extern int nPatterns;
extern int patSize;
extern int nNoisyPatterns;

#ifdef __cplusplus
extern "C" {
#endif

void readFile(const char fileName[]);
void readNoisyFile(const char fileName[]);

void showIndexedPattern(int index);
void showIndexedNoisyPattern(int index);

void showPattern(const double pattern[]);
void showPatternAndDifference(const double pattern[], 
                              const double patternWithNoise[]);
void showPatternAsVector(const double pattern[]);

#ifdef __cplusplus
}
#endif

#endif

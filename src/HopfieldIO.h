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
void showPattern(const double Pattern[]);
void showPatternAndDifference(const double Pattern[], const double PatternWithNoise[]);
void showPatternAsVector(const double Pattern[]);

#ifdef __cplusplus
}
#endif

#endif

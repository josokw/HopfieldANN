/*---------------------------------------------------------------------------*/
/* Simulation Hopfield ANN                                                   */
/*---------------------------------------------------------------------------*/

#ifndef HOPFIELD_H
#define HOPFIELD_H

/* Max number of neurons */
#define MAXN 1000
/* Max number of patterns */
#define MAXP 25

extern double Patterns[][MAXN];
extern double NoisyPatterns[][MAXN];
extern double W[][MAXN];

#endif

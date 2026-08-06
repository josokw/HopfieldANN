#ifndef HOPFIELDCONFIG_H
#define HOPFIELDCONFIG_H

#define MAX_ITERATIONS 1000
#define MAX_NOISE_PERCENT 100              /* hard cap: valid noise range is 0..100% */
#define MAX_INFORMATIVE_NOISE_PERCENT 50   /* above this, input anti-correlates with the
                                              stored pattern -> recall converges to the
                                              inverted pattern (W(-x) = -Wx) */
#define STORAGE_CAPACITY_FACTOR 0.138

/* Daydreaming learning rule (Serricchio et al., 2025). */
#define DAYDREAMING_EPOCHS 10            /* training epochs (each = N steps) */
#define DAYDREAMING_TAU_FACTOR 1.0       /* tau = factor * N (paper uses tau = N) */
#define DAYDREAMING_NORM_ITERATIONS 50   /* power-iteration steps for the spectral norm */

/* Modern Hopfield learning rule (Ramsauer et al., 2020). */
#define MODERN_BETA 1.0                  /* inverse softmax temperature: larger =
                                            sharper (near-argmax) retrieval */
#define MODERN_CONVERGENCE_EPSILON 1e-6  /* max |x_i - prev_i| that ends iteration */

#endif

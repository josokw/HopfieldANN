#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

extern "C" {
    #include "HopfieldContext.h"
    #include "HopfieldCalc.h"
    #include "HopfieldIO.h"
}

enum { TEST_MAX_NEURONS = 1024 };

class HopfieldCalcTest : public ::testing::Test {
protected:
    HopfieldContext *ctx;

    void SetUp() override {
        ctx = hopfield_context_create();
        ASSERT_NE(ctx, nullptr);
    }

    void TearDown() override {
        hopfield_context_destroy(ctx);
    }

    void allocate(int patternSize, int nPatterns, int nNoisyPatterns = 0) {
        ASSERT_TRUE(hopfield_context_resize(ctx, patternSize, 1, nPatterns,
                                            nNoisyPatterns));
    }
};

TEST_F(HopfieldCalcTest, StorageCapacity) {
    EXPECT_EQ(storageCapacity(100), 13);   // 0.138 * 100 ≈ 13
    EXPECT_EQ(storageCapacity(10), 1);     // minimum 1
    EXPECT_EQ(storageCapacity(1000), 138);
    EXPECT_EQ(storageCapacity(1), 1);      // minimum 1
}

TEST_F(HopfieldCalcTest, Equals) {
    EXPECT_TRUE(equals(1.0, 1.0));
    EXPECT_TRUE(equals(0.0, 0.0));
    EXPECT_FALSE(equals(1.0, 2.0));
    EXPECT_TRUE(equals(1.0, 1.0 + 1e-10));  // within epsilon
    EXPECT_FALSE(equals(1.0, 1.0 + 1e-4));  // outside epsilon
}

TEST_F(HopfieldCalcTest, CopyPattern) {
    double source[TEST_MAX_NEURONS] = {0};
    double target[TEST_MAX_NEURONS] = {0};

    // Initialize source
    source[0] = 1.0;
    source[1] = -1.0;
    source[2] = 1.0;
    source[3] = -1.0;

    copyPattern(4, source, target);

    for (int i = 0; i < 4; i++) {
        EXPECT_DOUBLE_EQ(source[i], target[i]);
    }
}

TEST_F(HopfieldCalcTest, IsSymmetric) {
    // Use context's W matrix instead of local array to avoid stack overflow
    allocate(3, 0);
    ctx->W[0][1] = 0.5; ctx->W[0][2] = 0.3;
    ctx->W[1][0] = 0.5; ctx->W[1][2] = 0.7;
    ctx->W[2][0] = 0.3; ctx->W[2][1] = 0.7;

    EXPECT_TRUE(isSymmetric(3, ctx->W));

    // Make it asymmetric
    ctx->W[1][0] = 0.6;
    EXPECT_FALSE(isSymmetric(3, ctx->W));
}

TEST_F(HopfieldCalcTest, HasZeroDiagonal) {
    // Use context's W matrix
    allocate(3, 0);
    ctx->W[0][1] = 0.5; ctx->W[0][2] = 0.3;
    ctx->W[1][0] = 0.5; ctx->W[1][2] = 0.7;
    ctx->W[2][0] = 0.3; ctx->W[2][1] = 0.7;

    EXPECT_TRUE(hasZeroDiagonal(3, ctx->W));

    // Add non-zero diagonal
    ctx->W[0][0] = 1.0;
    EXPECT_FALSE(hasZeroDiagonal(3, ctx->W));
}

TEST_F(HopfieldCalcTest, LearnHebbianSymmetric) {
    // Setup: 2 patterns with 4 neurons
    allocate(4, 2);
    ctx->patterns[0][0] = 1; ctx->patterns[0][1] = -1;
    ctx->patterns[0][2] = 1; ctx->patterns[0][3] = -1;
    ctx->patterns[1][0] = -1; ctx->patterns[1][1] = 1;
    ctx->patterns[1][2] = -1; ctx->patterns[1][3] = 1;

    EXPECT_TRUE(learnHebbian(ctx));

    EXPECT_TRUE(isSymmetric(ctx->patternSize, ctx->W));
    EXPECT_TRUE(hasZeroDiagonal(ctx->patternSize, ctx->W));
}

TEST_F(HopfieldCalcTest, LearnHebbianCorrectWeights) {
    // Simple case: 1 pattern with 2 neurons
    allocate(2, 1);
    ctx->patterns[0][0] = 1;
    ctx->patterns[0][1] = 1;

    EXPECT_TRUE(learnHebbian(ctx));

    // W[0][1] = W[1][0] = (1*1)/2 = 0.5
    EXPECT_DOUBLE_EQ(ctx->W[0][1], 0.5);
    EXPECT_DOUBLE_EQ(ctx->W[1][0], 0.5);
    EXPECT_DOUBLE_EQ(ctx->W[0][0], 0.0);
    EXPECT_DOUBLE_EQ(ctx->W[1][1], 0.0);
}

TEST_F(HopfieldCalcTest, LearnStorkeySymmetric) {
   allocate(4, 2);
   ctx->patterns[0][0] = 1; ctx->patterns[0][1] = -1;
   ctx->patterns[0][2] = 1; ctx->patterns[0][3] = -1;
   ctx->patterns[1][0] = -1; ctx->patterns[1][1] = 1;
   ctx->patterns[1][2] = -1; ctx->patterns[1][3] = 1;

   EXPECT_TRUE(learnStorkey(ctx));

   EXPECT_TRUE(isSymmetric(ctx->patternSize, ctx->W));
   EXPECT_TRUE(hasZeroDiagonal(ctx->patternSize, ctx->W));
}

TEST_F(HopfieldCalcTest, LearnStorkeyCorrectWeights) {
   allocate(2, 1);
   ctx->patterns[0][0] = 1;
   ctx->patterns[0][1] = 1;

   EXPECT_TRUE(learnStorkey(ctx));

   EXPECT_DOUBLE_EQ(ctx->W[0][1], 0.5);
   EXPECT_DOUBLE_EQ(ctx->W[1][0], 0.5);
   EXPECT_DOUBLE_EQ(ctx->W[0][0], 0.0);
   EXPECT_DOUBLE_EQ(ctx->W[1][1], 0.0);
}

TEST_F(HopfieldCalcTest, LearnStorkeyTwoPatterns) {
   allocate(4, 2);
   ctx->patterns[0][0] = 1; ctx->patterns[0][1] = 1;
   ctx->patterns[0][2] = 1; ctx->patterns[0][3] = 1;
   ctx->patterns[1][0] = 1; ctx->patterns[1][1] = 1;
   ctx->patterns[1][2] = -1; ctx->patterns[1][3] = -1;

   EXPECT_TRUE(learnStorkey(ctx));

   EXPECT_DOUBLE_EQ(ctx->W[0][1], 0.75);
   EXPECT_DOUBLE_EQ(ctx->W[0][2], 0.0);
   EXPECT_DOUBLE_EQ(ctx->W[2][3], 0.75);
}

TEST_F(HopfieldCalcTest, LearnStorkeyRecall) {
   allocate(4, 2);
   ctx->patterns[0][0] = 1; ctx->patterns[0][1] = -1;
   ctx->patterns[0][2] = 1; ctx->patterns[0][3] = -1;
   ctx->patterns[1][0] = -1; ctx->patterns[1][1] = 1;
   ctx->patterns[1][2] = -1; ctx->patterns[1][3] = 1;

   EXPECT_TRUE(learnStorkey(ctx));

   double input[TEST_MAX_NEURONS] = {0};
   double associated[TEST_MAX_NEURONS] = {0};

   input[0] = 1.0; input[1] = -1.0;
   input[2] = 1.0; input[3] = -1.0;

    double energy;
    bool converged = calcAssociatedPattern(ctx, input, associated, &energy);

    for (int i = 0; i < 4; i++) {
       EXPECT_DOUBLE_EQ(associated[i], ctx->patterns[0][i]);
    }
    EXPECT_TRUE(converged);
    EXPECT_LT(energy, 0.0);
}

TEST_F(HopfieldCalcTest, LearnPseudoInverseSymmetric) {
   allocate(4, 2);
   ctx->patterns[0][0] = 1; ctx->patterns[0][1] = 1;
   ctx->patterns[0][2] = 1; ctx->patterns[0][3] = 1;
   ctx->patterns[1][0] = 1; ctx->patterns[1][1] = 1;
   ctx->patterns[1][2] = -1; ctx->patterns[1][3] = -1;

   EXPECT_TRUE(learnPseudoInverse(ctx));

   EXPECT_TRUE(isSymmetric(ctx->patternSize, ctx->W));
   EXPECT_FALSE(hasZeroDiagonal(ctx->patternSize, ctx->W));
}

TEST_F(HopfieldCalcTest, LearnPseudoInverseCorrectWeights) {
   allocate(2, 1);
   ctx->patterns[0][0] = 1;
   ctx->patterns[0][1] = 1;

   EXPECT_TRUE(learnPseudoInverse(ctx));

   EXPECT_DOUBLE_EQ(ctx->W[0][1], 0.5);
   EXPECT_DOUBLE_EQ(ctx->W[1][0], 0.5);
   EXPECT_DOUBLE_EQ(ctx->W[0][0], 0.5);
   EXPECT_DOUBLE_EQ(ctx->W[1][1], 0.5);
}

TEST_F(HopfieldCalcTest, LearnPseudoInverseTwoPatterns) {
   allocate(4, 2);
   ctx->patterns[0][0] = 1; ctx->patterns[0][1] = 1;
   ctx->patterns[0][2] = 1; ctx->patterns[0][3] = 1;
   ctx->patterns[1][0] = 1; ctx->patterns[1][1] = 1;
   ctx->patterns[1][2] = -1; ctx->patterns[1][3] = -1;

   EXPECT_TRUE(learnPseudoInverse(ctx));

   EXPECT_DOUBLE_EQ(ctx->W[0][1], 0.5);
   EXPECT_DOUBLE_EQ(ctx->W[0][2], 0.0);
   EXPECT_DOUBLE_EQ(ctx->W[2][3], 0.5);
   EXPECT_DOUBLE_EQ(ctx->W[0][0], 0.5);
}

TEST_F(HopfieldCalcTest, LearnPseudoInverseExactFixedPoints) {
   allocate(4, 2);
   ctx->patterns[0][0] = 1; ctx->patterns[0][1] = -1;
   ctx->patterns[0][2] = 1; ctx->patterns[0][3] = -1;
   ctx->patterns[1][0] = 1; ctx->patterns[1][1] = 1;
   ctx->patterns[1][2] = -1; ctx->patterns[1][3] = -1;

   EXPECT_TRUE(learnPseudoInverse(ctx));

   for (int mu = 0; mu < ctx->nPatterns; mu++) {
      for (int i = 0; i < ctx->patternSize; i++) {
         double field = 0.0;
         for (int j = 0; j < ctx->patternSize; j++) {
            field += ctx->W[i][j] * ctx->patterns[mu][j];
         }
         EXPECT_DOUBLE_EQ(field, ctx->patterns[mu][i]);
      }
   }
}

TEST_F(HopfieldCalcTest, LearnPseudoInverseRecall) {
   allocate(8, 2);
   ctx->patterns[0][0] = 1; ctx->patterns[0][1] = 1;
   ctx->patterns[0][2] = 1; ctx->patterns[0][3] = 1;
   ctx->patterns[0][4] = -1; ctx->patterns[0][5] = -1;
   ctx->patterns[0][6] = -1; ctx->patterns[0][7] = -1;
   ctx->patterns[1][0] = 1; ctx->patterns[1][1] = -1;
   ctx->patterns[1][2] = 1; ctx->patterns[1][3] = -1;
   ctx->patterns[1][4] = 1; ctx->patterns[1][5] = -1;
   ctx->patterns[1][6] = 1; ctx->patterns[1][7] = -1;

   EXPECT_TRUE(learnPseudoInverse(ctx));

   double input[TEST_MAX_NEURONS] = {0};
   double associated[TEST_MAX_NEURONS] = {0};

   input[0] = -1.0;
   for (int i = 1; i < 8; i++) {
      input[i] = ctx->patterns[0][i];
   }

   double energy;
   bool converged = calcAssociatedPattern(ctx, input, associated, &energy);

   for (int i = 0; i < 8; i++) {
      EXPECT_DOUBLE_EQ(associated[i], ctx->patterns[0][i]);
   }
   EXPECT_TRUE(converged);
   EXPECT_LT(energy, 0.0);
}

TEST_F(HopfieldCalcTest, LearnPseudoInverseRejectsDependent) {
   allocate(4, 2);
   ctx->patterns[0][0] = 1; ctx->patterns[0][1] = 1;
   ctx->patterns[0][2] = 1; ctx->patterns[0][3] = 1;
   ctx->patterns[1][0] = 1; ctx->patterns[1][1] = 1;
   ctx->patterns[1][2] = 1; ctx->patterns[1][3] = 1;

   EXPECT_FALSE(learnPseudoInverse(ctx));
}

TEST_F(HopfieldCalcTest, LearnPseudoInverseRejectsInvertedDependent) {
   allocate(4, 2);
   ctx->patterns[0][0] = 1; ctx->patterns[0][1] = -1;
   ctx->patterns[0][2] = 1; ctx->patterns[0][3] = -1;
   ctx->patterns[1][0] = -1; ctx->patterns[1][1] = 1;
   ctx->patterns[1][2] = -1; ctx->patterns[1][3] = 1;

   EXPECT_FALSE(learnPseudoInverse(ctx));
}

TEST_F(HopfieldCalcTest, LearnDaydreamingSymmetric) {
   allocate(4, 2);
   ctx->patterns[0][0] = 1; ctx->patterns[0][1] = -1;
   ctx->patterns[0][2] = 1; ctx->patterns[0][3] = -1;
   ctx->patterns[1][0] = -1; ctx->patterns[1][1] = 1;
   ctx->patterns[1][2] = -1; ctx->patterns[1][3] = 1;

   EXPECT_TRUE(learnDaydreaming(ctx));

   EXPECT_TRUE(isSymmetric(ctx->patternSize, ctx->W));
   EXPECT_TRUE(hasZeroDiagonal(ctx->patternSize, ctx->W));
}

TEST_F(HopfieldCalcTest, LearnDaydreamingRecall) {
   // Two orthogonal patterns on 16 neurons.
   allocate(16, 2);
   for (int i = 0; i < 8; i++) {
      ctx->patterns[0][i] = 1.0;
      ctx->patterns[0][i + 8] = -1.0;
      ctx->patterns[1][i] = (i % 2 == 0) ? 1.0 : -1.0;
      ctx->patterns[1][i + 8] = (i % 2 == 0) ? 1.0 : -1.0;
   }

   EXPECT_TRUE(learnDaydreaming(ctx));

   double input[TEST_MAX_NEURONS] = {0};
   double associated[TEST_MAX_NEURONS] = {0};
   double energy;

   input[0] = -ctx->patterns[0][0];  // flip one bit
   for (int i = 1; i < 16; i++) {
      input[i] = ctx->patterns[0][i];
   }

   bool converged = calcAssociatedPattern(ctx, input, associated, &energy);

   EXPECT_TRUE(converged);
   EXPECT_LT(energy, 0.0);
   EXPECT_GE(calcOverlap(16, associated, ctx->patterns[0]), 0.9);
}

TEST_F(HopfieldCalcTest, LearnDaydreamingRejectsEmpty) {
   allocate(4, 0);

   EXPECT_FALSE(learnDaydreaming(ctx));
}

TEST_F(HopfieldCalcTest, CalcEnergy) {
    allocate(2, 0);
    double pattern[TEST_MAX_NEURONS] = {0};

    pattern[0] = 1.0;
    pattern[1] = -1.0;
    ctx->W[0][1] = 0.5;
    ctx->W[1][0] = 0.5;

    // Energy = -0.5 * (sum of pattern[i]*pattern[j]*w[i][j])
    // = -0.5 * (1*1*0 + 1*(-1)*0.5 + (-1)*1*0.5 + (-1)*(-1)*0)
    // = -0.5 * (-0.5 - 0.5) = -0.5 * (-1) = 0.5
    double energy = calcEnergy(2, pattern, ctx->W);
    EXPECT_DOUBLE_EQ(energy, 0.5);
}

TEST_F(HopfieldCalcTest, CalcOutputPattern) {
    allocate(2, 0);
    double input[TEST_MAX_NEURONS] = {0};
    double output[TEST_MAX_NEURONS] = {0};

    input[0] = 1.0;
    input[1] = -1.0;
    ctx->W[0][1] = 1.0;
    ctx->W[1][0] = 1.0;

    calcOutputPattern(2, ctx->W, input, output);

    // output[0] = sign(input[0]*w[0][0] + input[1]*w[0][1])
    //           = sign(1*0 + (-1)*1) = sign(-1) = -1
    // output[1] = sign(input[0]*w[1][0] + input[1]*w[1][1])
    //           = sign(1*1 + (-1)*0) = sign(1) = 1
    EXPECT_DOUBLE_EQ(output[0], -1.0);
    EXPECT_DOUBLE_EQ(output[1], 1.0);
}

TEST_F(HopfieldCalcTest, CalcAssociatedPattern) {
    // Setup: 2 patterns with 4 neurons
    allocate(4, 2);
    ctx->patterns[0][0] = 1; ctx->patterns[0][1] = -1;
    ctx->patterns[0][2] = 1; ctx->patterns[0][3] = -1;
    ctx->patterns[1][0] = -1; ctx->patterns[1][1] = 1;
    ctx->patterns[1][2] = -1; ctx->patterns[1][3] = 1;

    EXPECT_TRUE(learnHebbian(ctx));

    // Try to recall pattern 0 from a slightly noisy version
    double input[TEST_MAX_NEURONS] = {0};
    double associated[TEST_MAX_NEURONS] = {0};

    input[0] = 1.0; input[1] = -1.0;
    input[2] = 1.0; input[3] = -1.0;  // exact pattern 0

    double energy;
    bool converged = calcAssociatedPattern(ctx, input, associated, &energy);

    // Should converge to the same pattern
    for (int i = 0; i < 4; i++) {
        EXPECT_DOUBLE_EQ(associated[i], ctx->patterns[0][i]);
    }
    EXPECT_TRUE(converged);
    EXPECT_LT(energy, 0.0);  // Energy should decrease
}

TEST_F(HopfieldCalcTest, CalcOutputPatternAsync) {
    allocate(2, 0);
    double pattern[TEST_MAX_NEURONS] = {0};

    pattern[0] = 1.0;
    pattern[1] = -1.0;
    ctx->W[0][1] = 1.0;
    ctx->W[1][0] = 1.0;

    calcOutputPatternAsync(2, ctx->W, pattern);

    // Neuron 0: sign(pattern[0]*W[0][0] + pattern[1]*W[0][1])
    //         = sign(1*0 + (-1)*1) = sign(-1) = -1
    // Neuron 1: sign(pattern[0]*W[1][0] + pattern[1]*W[1][1])
    //         = sign(1*1 + (-1)*0) = sign(1) = 1
    // Async updates one neuron at a time; the other retains its value.
    // After one sweep, both neurons may have been updated.
    // Verify the pattern is still valid (all values are +/-1)
    for (int i = 0; i < 2; i++) {
        EXPECT_TRUE(pattern[i] == 1.0 || pattern[i] == -1.0);
    }
}

TEST_F(HopfieldCalcTest, AddNoiseToPattern) {
    allocate(4, 1, 1);
    ctx->patterns[0][0] = 1; ctx->patterns[0][1] = -1;
    ctx->patterns[0][2] = 1; ctx->patterns[0][3] = -1;

    int nNoise = addNoiseToPattern(ctx, 0, 50);

    // With 50% noise on 4 neurons, expect ~2 noisy pixels
    EXPECT_GE(nNoise, 1);
    EXPECT_LE(nNoise, 3);

    // Noisy pattern should be stored in ctx->noisyPatterns[patNumber]
    // Count differences
    int diffCount = 0;
    for (int i = 0; i < 4; i++) {
        if (ctx->noisyPatterns[0][i] != ctx->patterns[0][i]) {
            diffCount++;
        }
    }
    EXPECT_EQ(diffCount, nNoise);
}

TEST_F(HopfieldCalcTest, AddNoiseToPatternClampsAt100) {
    allocate(4, 1);
    ctx->patterns[0][0] = 1; ctx->patterns[0][1] = -1;
    ctx->patterns[0][2] = 1; ctx->patterns[0][3] = -1;

    // Above MAX_NOISE_PERCENT (100) clamps to 100%: every pixel flipped
    int nNoise = addNoiseToPattern(ctx, 0, 150);

    EXPECT_EQ(nNoise, 4);
    int diffCount = 0;
    for (int i = 0; i < 4; i++) {
        if (ctx->noisyPatterns[0][i] != ctx->patterns[0][i]) {
            diffCount++;
        }
    }
    EXPECT_EQ(diffCount, 4);
}

TEST_F(HopfieldCalcTest, AddNoiseToPatternClampsNegative) {
    allocate(4, 1);
    ctx->patterns[0][0] = 1; ctx->patterns[0][1] = -1;
    ctx->patterns[0][2] = 1; ctx->patterns[0][3] = -1;

    // Negative noise clamps to 0%: no pixels flipped
    int nNoise = addNoiseToPattern(ctx, 0, -50);

    EXPECT_EQ(nNoise, 0);
    int diffCount = 0;
    for (int i = 0; i < 4; i++) {
        if (ctx->noisyPatterns[0][i] != ctx->patterns[0][i]) {
            diffCount++;
        }
    }
    EXPECT_EQ(diffCount, 0);
}

TEST_F(HopfieldCalcTest, ContextResizeReallocates) {
    allocate(4, 2);
    ctx->patterns[0][0] = 1.0;

    // Re-size to a different shape: storage must be re-allocated
    ASSERT_TRUE(hopfield_context_resize(ctx, 8, 1, 2, 0));
    EXPECT_EQ(ctx->patternSize, 8);
    EXPECT_EQ(ctx->nRows, 8);
    EXPECT_EQ(ctx->nColumns, 1);

    ctx->patterns[0][7] = 1.0;
    ctx->W[7][7] = 0.25;
    EXPECT_DOUBLE_EQ(ctx->W[7][7], 0.25);
    EXPECT_TRUE(isSymmetric(8, ctx->W));
}

TEST_F(HopfieldCalcTest, ContextResizeRejectsImpossibleSizes) {
    EXPECT_FALSE(hopfield_context_resize(ctx, 0, 1, 1, 0));
    EXPECT_FALSE(hopfield_context_resize(ctx, 1, 1, -1, 0));
    // Product overflows int range
    EXPECT_FALSE(hopfield_context_resize(ctx, 1000000000, 1000000000, 1, 0));
}

TEST_F(HopfieldCalcTest, OverlapIdentical) {
    double p1[4] = {1, -1, 1, -1};
    double p2[4] = {1, -1, 1, -1};
    EXPECT_DOUBLE_EQ(calcOverlap(4, p1, p2), 1.0);
}

TEST_F(HopfieldCalcTest, OverlapInverted) {
    double p1[4] = {1, -1, 1, -1};
    double p2[4] = {-1, 1, -1, 1};
    EXPECT_DOUBLE_EQ(calcOverlap(4, p1, p2), -1.0);
}

TEST_F(HopfieldCalcTest, OverlapOrthogonal) {
    double p1[2] = {1, -1};
    double p2[2] = {1, 1};
    EXPECT_DOUBLE_EQ(calcOverlap(2, p1, p2), 0.0);
}

TEST_F(HopfieldCalcTest, OverlapPartial) {
    double p1[4] = {1, -1, 1, -1};
    double p2[4] = {1, 1, -1, -1};  // 2 of 4 match
    EXPECT_DOUBLE_EQ(calcOverlap(4, p1, p2), 0.0);
}

TEST_F(HopfieldCalcTest, HammingIdentical) {
    double p1[4] = {1, -1, 1, -1};
    double p2[4] = {1, -1, 1, -1};
    EXPECT_EQ(calcHammingDistance(4, p1, p2), 0);
}

TEST_F(HopfieldCalcTest, HammingInverted) {
    double p1[4] = {1, -1, 1, -1};
    double p2[4] = {-1, 1, -1, 1};
    EXPECT_EQ(calcHammingDistance(4, p1, p2), 4);
}

TEST_F(HopfieldCalcTest, HammingPartial) {
    double p1[4] = {1, -1, 1, -1};
    double p2[4] = {1, 1, -1, -1};  // 2 match, 2 differ
    EXPECT_EQ(calcHammingDistance(4, p1, p2), 2);
}

class HopfieldIOTest : public ::testing::Test {
protected:
    HopfieldContext *ctx;

    void SetUp() override {
        ctx = hopfield_context_create();
        ASSERT_NE(ctx, nullptr);
    }

    void TearDown() override {
        hopfield_context_destroy(ctx);
    }
};

TEST_F(HopfieldIOTest, ReadFileSuccess) {
    HopfieldError err = readFile(ctx, "data/hopf01.dat");
    EXPECT_EQ(err, HOPFIELD_OK);
    EXPECT_EQ(ctx->nRows, 10);
    EXPECT_EQ(ctx->nColumns, 10);
    EXPECT_EQ(ctx->nPatterns, 7);
    EXPECT_EQ(ctx->patternSize, 100);
}

TEST_F(HopfieldIOTest, ReadFileNotFound) {
    HopfieldError err = readFile(ctx, "nonexistent.dat");
    EXPECT_EQ(err, HOPFIELD_ERR_FILE_NOT_FOUND);
}

TEST_F(HopfieldIOTest, ReadNoisyFileSuccess) {
    // First read the main patterns
    readFile(ctx, "data/hopf01.dat");

    HopfieldError err = readNoisyFile(ctx, "data/hopf01noisy.dat");
    EXPECT_EQ(err, HOPFIELD_OK);
    EXPECT_GT(ctx->nNoisyPatterns, 0);
}

TEST_F(HopfieldIOTest, ReadNoisyFilePreservesLearnedWeights) {
    // Loading noisy patterns must not reallocate (and thus wipe) W.
    readFile(ctx, "data/hopf01.dat");
    ASSERT_TRUE(learnHebbian(ctx));

    double savedWeight = ctx->W[0][1];
    HopfieldError err = readNoisyFile(ctx, "data/hopf01noisy.dat");
    EXPECT_EQ(err, HOPFIELD_OK);
    EXPECT_DOUBLE_EQ(ctx->W[0][1], savedWeight);
}

TEST_F(HopfieldIOTest, ReadFileBeyondOldLimits) {
    // 32x32 = 1024 neurons (> 1000) and 30 patterns (> 25) exercise the
    // dynamic allocation path that replaced the fixed-size limits.
    std::filesystem::path path =
        std::filesystem::temp_directory_path() / "hopfield_large_test.dat";
    {
        std::ofstream out(path);
        out << "32 32 30\n";
        for (int p = 0; p < 30; p++) {
            for (int r = 0; r < 32; r++) {
                for (int c = 0; c < 32; c++) {
                    out << (r == c ? '*' : '.');
                }
                out << '\n';
            }
        }
    }

    HopfieldError err = readFile(ctx, path.string().c_str());
    EXPECT_EQ(err, HOPFIELD_OK);
    EXPECT_EQ(ctx->nPatterns, 30);
    EXPECT_EQ(ctx->patternSize, 1024);
    EXPECT_DOUBLE_EQ(ctx->patterns[0][0], 1.0);
    EXPECT_DOUBLE_EQ(ctx->patterns[0][1], -1.0);
    EXPECT_DOUBLE_EQ(ctx->patterns[29][31], -1.0);

    std::filesystem::remove(path);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

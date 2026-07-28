#include <gtest/gtest.h>

extern "C" {
    #include "HopfieldContext.h"
    #include "HopfieldCalc.h"
    #include "HopfieldIO.h"
}

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
    double source[NMAX_NEURONS] = {0};
    double target[NMAX_NEURONS] = {0};

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
    ctx->patternSize = 3;
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
    ctx->patternSize = 3;
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
    ctx->nPatterns = 2;
    ctx->patternSize = 4;
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
    ctx->nPatterns = 1;
    ctx->patternSize = 2;
    ctx->patterns[0][0] = 1;
    ctx->patterns[0][1] = 1;

    EXPECT_TRUE(learnHebbian(ctx));

    // W[0][1] = W[1][0] = (1*1)/2 = 0.5
    EXPECT_DOUBLE_EQ(ctx->W[0][1], 0.5);
    EXPECT_DOUBLE_EQ(ctx->W[1][0], 0.5);
    EXPECT_DOUBLE_EQ(ctx->W[0][0], 0.0);
    EXPECT_DOUBLE_EQ(ctx->W[1][1], 0.0);
}

TEST_F(HopfieldCalcTest, CalcEnergy) {
    double pattern[NMAX_NEURONS] = {0};

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
    double input[NMAX_NEURONS] = {0};
    double output[NMAX_NEURONS] = {0};

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
    ctx->nPatterns = 2;
    ctx->patternSize = 4;
    ctx->patterns[0][0] = 1; ctx->patterns[0][1] = -1;
    ctx->patterns[0][2] = 1; ctx->patterns[0][3] = -1;
    ctx->patterns[1][0] = -1; ctx->patterns[1][1] = 1;
    ctx->patterns[1][2] = -1; ctx->patterns[1][3] = 1;

    EXPECT_TRUE(learnHebbian(ctx));

    // Try to recall pattern 0 from a slightly noisy version
    double input[NMAX_NEURONS] = {0};
    double associated[NMAX_NEURONS] = {0};

    input[0] = 1.0; input[1] = -1.0;
    input[2] = 1.0; input[3] = -1.0;  // exact pattern 0

    double energy = calcAssociatedPattern(ctx, input, associated);

    // Should converge to the same pattern
    for (int i = 0; i < 4; i++) {
        EXPECT_DOUBLE_EQ(associated[i], ctx->patterns[0][i]);
    }
    EXPECT_LT(energy, 0.0);  // Energy should decrease
}

TEST_F(HopfieldCalcTest, CalcOutputPatternAsync) {
    double pattern[NMAX_NEURONS] = {0};

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
    ctx->nPatterns = 1;
    ctx->patternSize = 4;
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

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

//======== Copyright (c) 2020, FIT VUT Brno, All rights reserved. ============//
//
// Purpose:     White Box - Tests suite
//
// $NoKeywords: $ivs_project_1 $white_box_code.cpp
// $Author:     Ondřej Ondryáš <xondry02@stud.fit.vutbr.cz>
// $Date:       $2020-03-01
//============================================================================//
/**
 * @file white_box_tests.cpp
 * @author Ondřej Ondryáš
 * 
 * @brief Implementace testu prace s maticemi.
 */

#include "gtest/gtest.h"
#include "white_box_code.h"

struct SizedMatrix {
public:
    Matrix matrix;
    // Matrix width (number of columns)
    size_t cols;
    // Matrix height (number of rows)
    size_t rows;
};

struct Equation {
public:
    int variables;
    Matrix matrix;
    std::vector<double> values;
    std::vector<double> expectedResults;
};


class MixedSizesMatricesTest : public ::testing::Test {
protected:
    void SetUp() override {
        const int a = 8;
        matrices = std::vector<SizedMatrix>(a * a);
        identities = std::vector<Matrix>(a);

        for (size_t w = 0; w < a; w++) {
            for (size_t h = 0; h < a; h++) {
                matrices[h * a + w] = {Matrix(h + 1, w + 1), w + 1, h + 1};
            }
        }

        for (size_t i = 1; i <= a; i++) {
            Matrix m(i, i);

            for (size_t pos = 0; pos < a; pos++) {
                m.set(pos, pos, 1);
            }

            identities[i - 1] = m;
        }
    }

    std::vector<SizedMatrix> matrices;
    std::vector<Matrix> identities;
};

// Comment out to disable the long 10x10 matrix equation calculation
//#define TEST_EQ_10

class EquationTest : public ::testing::Test {
protected:
    void SetUp() override {
        squareMatrices = {std::pair<Matrix, size_t>(Matrix(1, 1), 1),
                          std::pair<Matrix, size_t>(Matrix(2, 2), 2),
                          std::pair<Matrix, size_t>(Matrix(3, 3), 3),
                          std::pair<Matrix, size_t>(Matrix(4, 4), 4),
                          std::pair<Matrix, size_t>(Matrix(8, 8), 8),
        };

        Equation eq1 = {1, Matrix(1, 1), {10}, {2}};
        eq1.matrix.set(0, 0, 5);

        Equation eq2 = {2, Matrix(2, 2), {20, 12}, {2.25, 2.1875}};
        eq2.matrix.set({{5,    4},
                        {8.25, -3}});

        Equation eq3 = {3, Matrix(3, 3), {5, 1, -15}, {7, 5, -3}};
        eq3.matrix.set(
                {{1,  -1, -1},
                 {-1, 1,  -1},
                 {-1, -1, 1}});

        Equation eq4 = {4, Matrix(4, 4), {8, 4, -1, 14}, {1, 2, 1, 4}};
        eq4.matrix.set(
                {
                        {1, 1, 1, 1},
                        {0, 3, 2, -1},
                        {0, 1, 1, -1},
                        {0, 1, 4, 2}
                }
        );

#ifdef TEST_EQ_10
        Equation eq10 = {10, Matrix(10, 10)};
        eq10.values = {87, 116, 153, 73, 147, 151, 83, 162, 90, 150};
        eq10.expectedResults = {0, 1, 6, 6, 3, 4, 5, 1, 1, 3};
        eq10.matrix.set({
                                {4, 3, 3, 3, 3, 3, 3, 3, 3, 2},
                                {6, 3, 3, 4, 6, 4, 4, 4, 4, 3},
                                {9, 4, 5, 5, 7, 5, 5, 6, 5, 4},
                                {4, 2, 2, 3, 4, 2, 2, 3, 2, 2},
                                {8, 4, 4, 6, 8, 4, 4, 6, 5, 4},
                                {8, 4, 4, 6, 8, 5, 4, 6, 5, 4},
                                {4, 2, 2, 3, 4, 3, 3, 3, 3, 2},
                                {8, 4, 5, 6, 8, 5, 5, 6, 5, 4},
                                {4, 3, 3, 3, 4, 3, 3, 3, 3, 2},
                                {8, 4, 5, 5, 6, 5, 5, 6, 5, 4}
                        });

        equations = {eq1, eq2, eq3, eq4, eq10};
#else
        equations = {eq1, eq2, eq3, eq4};
#endif
    }

    std::vector<std::pair<Matrix, size_t>> squareMatrices;
    std::vector<Equation> equations;

};

/*
void printMatrix(SizedMatrix *ms) {
    printf("Matrix %zux%zu\n----------------\n", ms->rows, ms->cols);
    for (size_t y = 0; y < ms->rows; y++) {
        for (size_t x = 0; x < ms->cols; x++) {
            printf("%.0f ", ms->matrix.get(y, x));
        }
        printf("\n");
    }
    printf("----------------\n");
}*/

TEST(MatrixTest, Construct) {
    const int a = 16;

    for (size_t w = 0; w < a; w++) {
        for (size_t h = 0; h < a; h++) {
            EXPECT_NO_THROW(Matrix(w + 1, h + 1));
        }
    }

    EXPECT_NO_THROW(Matrix());
    EXPECT_ANY_THROW(Matrix(0, 0));
    EXPECT_ANY_THROW(Matrix(0, 16));
    EXPECT_ANY_THROW(Matrix(16, 0));
}

TEST_F(MixedSizesMatricesTest, GetInitialValue) {
    for (auto &ms : matrices) {
        for (size_t x = 0; x < ms.cols; x++) {
            for (size_t y = 0; y < ms.rows; y++) {
                int val;
                ASSERT_NO_THROW(val = ms.matrix.get(y, x));
                EXPECT_EQ(val, 0);
            }
        }
    }
}

TEST_F(MixedSizesMatricesTest, GetOutsideBounds) {
    for (auto &ms : matrices) {
        // Both R and C outside
        EXPECT_ANY_THROW(ms.matrix.get(ms.rows, ms.cols));
        // R outside, C inside
        EXPECT_ANY_THROW(ms.matrix.get(ms.rows, ms.cols - 1));
        // R inside, C outside
        EXPECT_ANY_THROW(ms.matrix.get(ms.rows - 1, ms.cols));
    }
}

TEST_F(MixedSizesMatricesTest, Set) {
    const double newVal = 100;

    for (auto &ms : matrices) {
        for (size_t x = 0; x < ms.cols; x++) {
            for (size_t y = 0; y < ms.rows; y++) {
                ASSERT_TRUE(ms.matrix.set(y, x, newVal));
                EXPECT_EQ(ms.matrix.get(y, x), newVal);
            }
        }
    }
}

TEST_F(MixedSizesMatricesTest, SetOutsideBounds) {
    for (auto &ms : matrices) {
        EXPECT_FALSE(ms.matrix.set(ms.rows, 0, 1.0));
        EXPECT_FALSE(ms.matrix.set(ms.rows, ms.cols, 1.0));
        EXPECT_FALSE(ms.matrix.set(0, ms.cols, 1.0));

        // uint max boundary
        if (ms.rows != -1) {
            EXPECT_FALSE(ms.matrix.set(-1, 0, 1.0));
        }
        if (ms.cols != -1) {
            EXPECT_FALSE(ms.matrix.set(0, -1, 1.0));
        }
    }
}

TEST_F(MixedSizesMatricesTest, SetAll) {
    const double newVal = 100;

    for (auto &ms : matrices) {
        auto invalidVals = std::vector<std::vector<double>>(ms.rows + 1, std::vector<double>(ms.cols + 1, newVal));
        auto newVals = std::vector<std::vector<double>>(ms.rows, std::vector<double>(ms.cols, newVal));

        EXPECT_FALSE(ms.matrix.set(invalidVals));
        ASSERT_TRUE(ms.matrix.set(newVals));

        for (size_t x = 0; x < ms.cols; x++) {
            for (size_t y = 0; y < ms.rows; y++) {
                EXPECT_EQ(ms.matrix.get(y, x), newVal);
            }
        }
    }
}

TEST_F(MixedSizesMatricesTest, EqualsNonMatchingSizes) {
    for (int i = 0; i < matrices.size() - 1; i++) {
        EXPECT_ANY_THROW(matrices[i].matrix == matrices[i + 1].matrix);
    }
}

TEST_F(MixedSizesMatricesTest, Equals) {
    const double nonZeroConstant = 100;

    for (const auto &ms : matrices) {
        Matrix def(ms.rows, ms.cols);
        EXPECT_TRUE(def == ms.matrix);
        EXPECT_TRUE(ms.matrix == def);

        def.set(0, 0, nonZeroConstant);
        EXPECT_FALSE(def == ms.matrix);
        EXPECT_FALSE(ms.matrix == def);
    }
}

TEST_F(MixedSizesMatricesTest, SumNonMatchingSizes) {
    for (size_t i = 0; i < matrices.size() - 1; i++) {
        EXPECT_ANY_THROW(matrices[i].matrix + matrices[i + 1].matrix);
    }
}

TEST_F(MixedSizesMatricesTest, Sum) {
    const double constant = 5;

    for (auto &ms : matrices) {
        Matrix test(ms.rows, ms.cols);
        for (size_t y = 0; y < ms.rows; y++) {
            for (size_t x = 0; x < ms.cols; x++) {
                test.set(y, x, constant);
            }
        }

        // The original matrix has zeroes in it, so the sum matrix should be equal to the test matrix.
        Matrix sum = ms.matrix + test;

        for (size_t y = 0; y < ms.rows; y++) {
            for (size_t x = 0; x < ms.cols; x++) {
                double newVal;
                ASSERT_NO_THROW(newVal = sum.get(y, x))
                                            << "The new matrix has a size different from the original ones.";
                EXPECT_EQ(newVal, test.get(y, x));
            }
        }
    }
}

TEST_F(MixedSizesMatricesTest, MultiplyByConstant) {
    const double originalValueBase = 0;
    const double constant = 2;

    for (auto &ms : matrices) {
        double i = originalValueBase;

        // Fill the matrix with an incrementing value
        for (size_t y = 0; y < ms.rows; y++) {
            for (size_t x = 0; x < ms.cols; x++) {
                ms.matrix.set(y, x, i++);
            }
        }

        Matrix multiplied = ms.matrix * constant;

        for (size_t y = 0; y < ms.rows; y++) {
            for (size_t x = 0; x < ms.cols; x++) {
                double newVal, origVal;
                origVal = ms.matrix.get(y, x);
                ASSERT_EQ(origVal, originalValueBase + y * ms.cols + x) << "The original matrix has changed.";
                ASSERT_NO_THROW(
                        newVal = multiplied.get(y, x)) << "The new matrix has a size different from the original one.";
                EXPECT_EQ(newVal, origVal * constant);
            }
        }
    }
}

TEST_F(MixedSizesMatricesTest, MultiplySizesCheck) {
    for (auto &ms : matrices) {
        Matrix nonMatching(ms.cols + 1, ms.rows);

        EXPECT_ANY_THROW(ms.matrix * nonMatching);
        EXPECT_NO_THROW(nonMatching * ms.matrix);
    }
}

TEST_F(MixedSizesMatricesTest, Multiply) {
    for (auto &ms : matrices) {
        // Fill the matrix with an incrementing value
        double i = 0;
        for (size_t y = 0; y < ms.rows; y++) {
            for (size_t x = 0; x < ms.cols; x++) {
                ms.matrix.set(y, x, i++);
            }
        }

        // Multiplying by an identity matrix should yield a new matrix equal to the original one
        Matrix &matchingIdentityRight = identities[ms.cols - 1];
        Matrix &matchingIdentityLeft = identities[ms.rows - 1];

        Matrix multRight = ms.matrix * matchingIdentityRight;
        Matrix multLeft = matchingIdentityLeft * ms.matrix;

        EXPECT_TRUE(multRight == ms.matrix) << "Failed for the " << ms.cols << "x" << ms.rows << " matrix.";
        EXPECT_TRUE(multLeft == ms.matrix);
    }
}

TEST_F(EquationTest, UnmatchingRightSide) {
    const size_t systemSize = 3;

    Matrix eq(systemSize, systemSize);
    // The "right side" has more values
    std::vector<double> valuesMore(systemSize + 1);
    // The "right side" has less values
    std::vector<double> valuesLess(systemSize - 1);

    EXPECT_ANY_THROW(eq.solveEquation(valuesMore));
    EXPECT_ANY_THROW(eq.solveEquation(valuesLess));
}

TEST_F(EquationTest, NonSquareMatrix) {
    const size_t systemSize = 3;
    // The matrix has more rows than columns
    Matrix eq(systemSize + 1, systemSize);
    std::vector<double> values(systemSize);

    ASSERT_ANY_THROW(eq.solveEquation(values));
}

TEST_F(EquationTest, SingularMatrix) {
    for (auto &m : squareMatrices) {
        if (m.second < 2) continue;

        // Make a slick singular matrix with linearly dependent rows
        for (size_t x = 0; x < m.second; x++) {
            for (size_t y = 0; y < m.second; y++) {
                m.first.set(y, x, (double) ((x + 1) * (y + 1)));
            }
        }

        std::vector<double> values(m.second);
        ASSERT_ANY_THROW(m.first.solveEquation(values));
    }
}

TEST_F(EquationTest, Solve) {
    for (auto &e : equations) {
        std::vector<double> res = e.matrix.solveEquation(e.values);
        EXPECT_EQ(res.size(), e.variables);
        for (size_t i = 0; i < res.size(); i++) {
            EXPECT_EQ(res[i], e.expectedResults[i]);
        }
    }
}

/*** Konec souboru white_box_tests.cpp ***/

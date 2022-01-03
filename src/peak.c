#include <stdbool.h>
#include <math.h>
#include "peak.h"

bool
equals(const double a, const double b)
{
    if (b == 0) {
        return fabs(a) < TOLRAT;
    }
    double ratio = a / b;
    return fabs(ratio - 1) < TOLRAT;
}

bool
mat_equal(const struct matrix A, const struct matrix B)
{
    if (A.len1 != B.len1 || A.len2 != B.len2) {
        WARNING("incompatible matrices\n\tmat_equal %zdx%zd vs %zdx%zd\n", A.len1, A.len2, B.len1, B.len2);
        return false;
    }
    for (size_t i = 0; i < A.len1; i++) {
        for (size_t j = 0; j < A.len2; j++) {
            if (!equals(mat_get(A, i, j), mat_get(B, i, j))) {
                return false;
            }
        }
    }
    return true;
}

size_t
min2(const size_t x, const size_t y) 
{
    return x < y ? x : y;
}

size_t
min3(const size_t x, const size_t y, const size_t z) 
{
    return min2(min2(x, y), z);
}

double
peak_sim_measure_L2(const struct matarray m1, const struct matarray m2, size_t n) 
{
    // 2 4xn matrices (output of `peak_stat`), number of peaks -> similarity of them btwn
    // 0 and 1
    // TODO
    return 0;
}

struct matrix
peak_stat(const struct matarray matrices) 
{
    // 5 matrices of spectra, and number of peaks -> nx4 matrix of peaks
    // TODO
    struct matrix A;
    return A;
}

double
cos_sim_L2(const struct vec u, const struct vec v)
{
    // add 1e-4 to all std
    double v0 = vec_get(v, 0),        v1 = vec_get(v, 1),
           v2 = vec_get(v, 2) + 1e-4, v3 = vec_get(v, 3) + 1e-4,
           u0 = vec_get(u, 0),        u1 = vec_get(u, 1),
           u2 = vec_get(u, 2) + 1e-4, u3 = vec_get(u, 3) + 1e-4;
    double tmp2 = (2*u2*v2) / (u2*u2 + v2*v2);
    double tmp3 = (2*u3*v3) / (u3*u3 + v3*v3);
    double a = sqrt(tmp2) * sqrt(tmp3);
    double bx = (u0-v0) * (u0-v0) / (u2*u2 + v2*v2);
    double by = (u1-v1) * (u1-v1) / (u3*u3 + v3*v3);
    return a * exp(-0.5 * (bx + by));
}

struct matarray
peak_sort(const struct matarray matrices, size_t n)
{
    // 5 matrices of spectra, number of peaks -> n matrices s.t. ith matrix is 5
    // pts assoc. with ith largest peak

    // reset n
    for (int i = 0; i < matrices.length; i++) {
        n = min2(n, matarr_get(matrices, i).len1);
    }

    struct matarray P = matarr_zeros(n);
    struct matarray matrices_copy = matarr_copy(matrices);
    for (size_t i = 0; i < n; i++) {
        // find largest point left in all replicates
        double maxy = -INFINITY;
        double maxx = 0;
        for (size_t j = 0; j < matrices_copy.length; j++) {
            struct matrix mj = matarr_get(matrices_copy, j);
            struct vec ys = vec_from_col(mj, 1);
            size_t argmax = vec_argmax(ys);
            if (vec_get(ys, argmax) > maxy) {
                maxx = mat_get(mj, argmax, 0);
                maxy = mat_get(mj, argmax, 1);
            }
        }

        // find points in replicates closest to p
        struct matrix peak = mat_zeros(matrices_copy.length, 2);
        for (size_t j = 0; j < matrices_copy.length; j++) {
            struct matrix mj = matarr_get(matrices_copy, j);
            double mindist = INFINITY;
            
            // get closest pt in mj matrix
            size_t rowargmin = 0;
            for (size_t row = 0; row < mj.len1; row++) {
                double dx = maxx - mat_get(mj, row, 0);
                double dy = maxy - mat_get(mj, row, 1);
                double dist = dx*dx + dy*dy;
                if (dist < mindist) {
                    rowargmin = row;
                    mindist = dist;
                }
            }

            // assign smallest point to peak mat
            mat_set(peak, j, 0, mat_get(mj, rowargmin, 0));
            mat_set(peak, j, 1, mat_get(mj, rowargmin, 1));
            // set the point to (-1, -1) so it doesn't become another max
            mat_set(mj, rowargmin, 0, -INFINITY);
            mat_set(mj, rowargmin, 1, -INFINITY);
        }

        // make the next peak a new matrix in P
        matarr_set(P, i, peak);
    }
    matarr_free(matrices_copy);
    return P;
}


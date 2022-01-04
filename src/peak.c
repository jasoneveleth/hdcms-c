#include <stdbool.h>
#include <math.h>
#include "peak.h"

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

// 2 4xn matrices (output of `peak_stat`), number of peaks -> similarity of them btwn 0 and 1
double
peak_sim_measure_L2(const struct matarray m1, const struct matarray m2, size_t n) 
{
    // TODO
    return 0;
}

// 5 matrices of spectra, and number of peaks -> nx4 matrix of peaks
struct matrix
peak_stat(const struct matarray matrices, size_t n) 
{
    // reset n
    for (size_t i = 0; i < matrices.length; i++) {
        n = min2(n, matarr_get(matrices, i).len1);
    }

    struct matrix B = mat_zeros(n, 4);
    struct matarray P = peak_sort(matrices, n);
    for (size_t i = 0; i < P.length; i++) {
        struct matrix M = matarr_get(P, i);
        struct vec x = vec_from_col(M, 0);
        struct vec y = vec_from_col(M, 1);
        mat_set(B, i, 0, vec_mean(x));
        mat_set(B, i, 1, vec_mean(y));
        mat_set(B, i, 2, vec_std(x));
        mat_set(B, i, 3, vec_std(y));
    }
    matarr_free(P);
    return B;
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

// 5 matrices of spectra, number of peaks -> n matrices s.t. ith matrix is 5
// pts assoc. with ith largest peak
struct matarray
peak_sort(const struct matarray matrices, size_t n)
{
    // reset n
    for (size_t i = 0; i < matrices.length; i++) {
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

            // set y value of the point to -inf so it never matches or is a max
            mat_set(mj, rowargmin, 1, -INFINITY);
        }

        // make the next peak a new matrix in P
        matarr_set(P, i, peak);
    }
    matarr_free(matrices_copy);
    return P;
}


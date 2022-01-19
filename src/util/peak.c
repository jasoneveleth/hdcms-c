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
peak_sim_measure_L2(const struct matrix m1, const struct matrix m2, size_t n) 
{
    n = min3(m1.len1, m2.len1, n);
    if (n == 0) {
        WARNING("peak_sim_measure_L2 one of the matrices has %d length\n", 0);
        return 0;
    }
    struct matrix m1_copy = mat_copy(m1);
    struct matrix m2_copy = mat_copy(m2);
    struct vec values = vec_zeros(n);
    struct vec weights = vec_zeros(n);

    for (size_t i = 0; i < n; i++) {
        struct vec m1_ys = vec_from_col(m1_copy, 1);
        struct vec m2_ys = vec_from_col(m2_copy, 1);
        size_t m1_argmax = vec_argmax(m1_ys);
        size_t m2_argmax = vec_argmax(m2_ys);

        struct matrix *matrix_without_max_peak_p;
        struct vec u; // peak with max y value
        bool m1_has_larger_peak = mat_get(m1_copy, m1_argmax, 1) >= mat_get(m2_copy, m2_argmax, 1);
        matrix_without_max_peak_p = m1_has_larger_peak ? &m2_copy : &m1_copy;
        u = m1_has_larger_peak
            ? vec_from_row(m1_copy, m1_argmax)
            : vec_from_row(m2_copy, m2_argmax);

        struct vec sim_scores = vec_zeros(matrix_without_max_peak_p->len1);
        for (size_t j = 0; j < matrix_without_max_peak_p->len1; j++) {
            struct vec v = vec_from_row(*matrix_without_max_peak_p, j);
            vec_set(sim_scores, j, cos_sim_L2(u, v));
        }

        // argmax from values that aren't -inf
        double max = -inf;
        size_t score_argmax = 0;
        for (size_t j = 0; j < matrix_without_max_peak_p->len1; j++) {
            double y = mat_get(*matrix_without_max_peak_p, j, 1);
            double sim = vec_get(sim_scores, j);
            if (y != -inf && max < sim) {
                max = sim;
                score_argmax = j;
            }
        }

        struct vec v = vec_from_row(*matrix_without_max_peak_p, score_argmax);
        // -inf * -inf * 0
        double sim = vec_get(sim_scores, score_argmax);
        vec_set(values, i, vec_get(u, 1) * vec_get(v, 1) * sim);
        vec_set(weights, i, vec_get(u, 1) * vec_get(v, 1));

        // cause peaks to never appear again
        vec_set(u, 0, -inf);
        vec_set(u, 1, -inf);
        vec_set(v, 0, -inf);
        vec_set(v, 1, -inf);

        vec_free(sim_scores);
    }
    double ret = vec_sum(values)/vec_sum(weights);
    mat_free(m1_copy);
    mat_free(m2_copy);
    vec_free(values);
    vec_free(weights);
    return ret;
}

// 5 matrices of spectra, and number of peaks -> nx4 matrix of peaks
struct matrix
peak_stat(const struct matarray matrices, size_t n) 
{
    if (matrices.length <= 0) {
        WARNING("peak_stat got %zd length array\n", matrices.length);
        struct matrix m = {0, 0, 0, NULL, false};
        return m;
    }

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
    // assert input correct
    if (u.length != 4) {
        WARNING("%s vec size not equal to 4\n", __FUNCTION__);
        fprintf(stderr, "vec:\t");
        vec_fprintf(stderr, u);
        return 0;
    }
    if (v.length != 4) {
        WARNING("%s vec size not equal to 4\n", __FUNCTION__);
        fprintf(stderr, "vec:\t");
        vec_fprintf(stderr, v);
        return 0;
    }

    // add 1e-4 to all std, to avoid the 0 std div 0 error
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
        double maxy = -inf;
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
            double mindist = inf;
            
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
            mat_set(mj, rowargmin, 1, -inf);
        }

        // make the next peak a new matrix in P
        matarr_set(P, i, peak);
    }
    matarr_free(matrices_copy);
    return P;
}


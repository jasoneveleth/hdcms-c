#include <stdbool.h>
#include <math.h>
#include "array.h"
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

size_t
max(size_t x, size_t y)
{
    return x < y ? y : x;
}

// (2 4xn matrices (output of `peak_stat`), number of peaks) -> similarity of them btwn 0 and 1
double
peak_sim_measure_L2(const struct matrix m1, const struct matrix m2, double desingularization, size_t n) 
{
    n = min3(m1.len1, m2.len1, n);
    if (n == 0) {
        WARNING("%s\n", "peak_sim_measure_L2 one of the matrices has 0 length");
        return 0;
    }
    struct matrix m1_copy = mat_copy(m1);
    struct matrix m2_copy = mat_copy(m2);
    struct vec weighted_sum = vec_zeros(n);
    struct vec sum = vec_zeros(n);

    for (size_t i = 0; i < n; i++) {
        struct vec m1_ys = vec_from_col(m1_copy, 1);
        struct vec m2_ys = vec_from_col(m2_copy, 1);
        size_t m1_argmax = vec_argmax(m1_ys);
        size_t m2_argmax = vec_argmax(m2_ys);

        struct matrix *matrix_without_max_peak_p;
        struct vec u; // peak with max y value
        bool m1_has_larger_peak = mat_get(m1_copy, m1_argmax, 1) >= mat_get(m2_copy, m2_argmax, 1);
        double m1_max_y = vec_get(m1_ys, m1_argmax);
        double m2_max_y = vec_get(m2_ys, m2_argmax);
        if (m1_max_y > m2_max_y) {
            matrix_without_max_peak_p = &m2_copy;
            u = vec_from_row(m1_copy, m1_argmax);
        } else if (m1_max_y < m2_max_y) {
            matrix_without_max_peak_p = &m1_copy;
            u = vec_from_row(m2_copy, m2_argmax);
        } else {
            // equal, break tie with lexigraphic sort
            size_t i = 0;
            for (; vec_get(m1_ys, i) == vec_get(m2_ys, i); i++) {
                if (i >= n - 1) {
                    break;
                }
            }
            if (mat_get(m1_copy, i, 1) > mat_get(m2_copy, i, 1)) {
                matrix_without_max_peak_p = &m2_copy;
                u = vec_from_row(m1_copy, m1_argmax);
            } else {
                matrix_without_max_peak_p = &m1_copy;
                u = vec_from_row(m2_copy, m2_argmax);
            }
        }

        struct vec sim_scores = vec_zeros(matrix_without_max_peak_p->len1);
        for (size_t j = 0; j < matrix_without_max_peak_p->len1; j++) {
            struct vec v = vec_from_row(*matrix_without_max_peak_p, j);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
            // this is oaky because we set it to be this
            if (vec_get(v, 1) == -inf) { // this peak has already been used
#pragma GCC diagnostic pop
                vec_set(sim_scores, j, -inf);
            } else {
                vec_set(sim_scores, j, cos_sim_L2(u, v, desingularization));
            }
        }

        size_t score_argmax = vec_argmax(sim_scores);
        struct vec v = vec_from_row(*matrix_without_max_peak_p, score_argmax);
        double sim = vec_get(sim_scores, score_argmax);
        vec_set(weighted_sum, i, vec_get(u, 1) * vec_get(v, 1) * sim);
        vec_set(sum, i, vec_get(u, 1) * vec_get(v, 1));

        // cause peaks to never appear again
        vec_set(u, 1, -inf);
        vec_set(v, 1, -inf);

        vec_free(sim_scores);
    }
    double ret = vec_sum(weighted_sum)/vec_sum(sum);
    mat_free(m1_copy);
    mat_free(m2_copy);
    vec_free(weighted_sum);
    vec_free(sum);
    return ret;
}

// (matrices of spectra, number of peaks) -> nx4 matrix of peaks
struct matrix
peak_stat(const struct matarray replicates, size_t n, double xtol) 
{
    if (replicates.length <= 0) {
        WARNING("peak_stat got %zd length array\n", replicates.length);
        struct matrix m = {0, 0, 0, NULL, false};
        return m;
    }

    struct matarray P = peak_sort(replicates, n, xtol);
    struct matrix B = mat_zeros(P.length, 4);
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
cos_sim_L2(const struct vec u, const struct vec v, double desingularization)
{
    // assert input correct
    if (u.length != 4) {
        fprintf(stderr, "vec:\t");
        vec_fprintf(stderr, u);
        WARNING("%s vec size not equal to 4 (maybe forgot to pass --1d)\n", __func__);
        exit(1);
        // unreachable
    }
    if (v.length != 4) {
        fprintf(stderr, "vec:\t");
        vec_fprintf(stderr, v);
        WARNING("%s vec size not equal to 4 (maybe forgot to pass --1d)\n", __func__);
        exit(1);
        // unreachable
    }

    // add 1e-4 to all std, to avoid the 0 std div 0 error
    double v0 = vec_get(v, 0),        v1 = vec_get(v, 1),
           v2 = vec_get(v, 2) + desingularization, v3 = vec_get(v, 3) + desingularization,
           u0 = vec_get(u, 0),        u1 = vec_get(u, 1),
           u2 = vec_get(u, 2) + desingularization, u3 = vec_get(u, 3) + desingularization;
    double tmp2 = (2*u2*v2) / (u2*u2 + v2*v2);
    double tmp3 = (2*u3*v3) / (u3*u3 + v3*v3);
    double a = sqrt(tmp2) * sqrt(tmp3);
    double bx = (u0-v0) * (u0-v0) / (u2*u2 + v2*v2);
    double by = (u1-v1) * (u1-v1) / (u3*u3 + v3*v3);
    return a * exp(-0.5 * (bx + by));
}

// (matrices of spectra, number of peaks) -> n matrices s.t. ith matrix is the
// pts assoc. with ith largest peak
struct matarray
peak_sort(const struct matarray replicates, size_t n, double xtol)
{
    struct matarray P = matarr_zeros(n);

    // check for zero length matrices
    for (size_t j = 0; j < replicates.length; j++) {
        struct matrix mj = matarr_get(replicates, j);
        if (mj.len2 == 0) {
            WARNING("%s\n", "peak_sort given a matrix with dimension (<something>, 0)");
            matarr_free(P);
            return matarr_zeros(0);
        }
    }

    struct matarray replicates_copy = matarr_copy(replicates);

    size_t i;
    for (i = 0; i < n; i++) {
        // find largest point left in all replicates
        double maxy = -inf;
        double maxx = 0;
        bool found_peak = false;
        for (size_t j = 0; j < replicates_copy.length; j++) {
            struct matrix mj = matarr_get(replicates_copy, j);
            struct vec ys = vec_from_col(mj, 1);
            size_t argmax = vec_argmax(ys);
            if (vec_get(ys, argmax) > maxy) {
                maxx = mat_get(mj, argmax, 0);
                maxy = mat_get(mj, argmax, 1);
                found_peak = true;
            }
        }

        // we've exhausted all replicates and there are no peaks left
        if (!found_peak) {
            break;
        }

        // find points in replicates closest to p
        struct matrix peak_points = mat_zeros(replicates_copy.length, 2);
        for (size_t j = 0; j < replicates_copy.length; j++) {
            struct matrix mj = matarr_get(replicates_copy, j);

            // find closest pt in mj matrix (within xtol)
            double mindist = inf;
            size_t rowargmin = -1;
            bool found = false;
            for (size_t row = 0; row < mj.len1; row++) {
                if (fabs(mat_get(mj, row, 0) - maxx) > xtol) {
                    continue;
                }
                double dx = maxx - mat_get(mj, row, 0);
                double dy = maxy - mat_get(mj, row, 1);
                double dist = dx*dx + dy*dy;
                if (dist < mindist) {
                    found = true;
                    rowargmin = row;
                    mindist = dist;
                }
            }

            if (!found) {
                mat_set(peak_points, j, 0, maxx);
                mat_set(peak_points, j, 1, 0);
            } else {
                // assign closest point to peak
                mat_set(peak_points, j, 0, mat_get(mj, rowargmin, 0));
                mat_set(peak_points, j, 1, mat_get(mj, rowargmin, 1));

                // set y value of the point to -inf so it never matches or is a max
                mat_set(mj, rowargmin, 1, -inf);
            }

        }

        // add the peak points to P
        matarr_set(P, i, peak_points);
    }

    // if we exhausted the replicates before we got n peaks, then make a smaller
    // matarray
    if (i != n) {
        struct matarray a = matarr_zeros(i);
        for (size_t j = 0; j < i; j++) {
            matarr_set(a, j, matarr_get(P, j));
        }
        P.is_owner = false;
        matarr_free(P);
        P = a;
    }

    matarr_free(replicates_copy);
    return P;
}


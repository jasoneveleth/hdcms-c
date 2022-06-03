#include <stdbool.h>
#include <math.h>
#include "oned.h"

struct matrix 
bin_stat_1D(const struct matarray A, double width)
{
    double num_bins = floor(900./width);
    width = 900./num_bins; // XXX unneeded
    struct matrix M = mat_zeros(A.length, num_bins);
    for (size_t i = 0; i < A.length; i++) {
        struct matrix spectra = matarr_get(A, i);
        struct vec bin_heights = spec_vec(spectra, width);
        vec_to_row(M, bin_heights, i);
        vec_free(bin_heights);
    }

    struct matrix B = mat_zeros(num_bins, 2);
    for (size_t i = 0; i < num_bins; i++) {
        struct vec ith_bin = vec_from_col(M, i);
        mat_set(B, i, 0, vec_mean(ith_bin));
        mat_set(B, i, 1, vec_std(ith_bin));
    }
    mat_free(M);
    return B;
}

static size_t
get_bin(const struct vec bins, double val)
{
    size_t i = 0;
    while (vec_get(bins, i) <= val) {
        i++;
    }
    return i == 0 ? 0 : i - 1;
}

struct vec 
spec_vec(const struct matrix m, double width)
{
    double n = floor(900./width);
    width = 900./n;
    struct vec t = vec_linspace(0, 900. * ((n - 1) / n), n);
    struct vec v = vec_zeros(n);

    for (size_t i = 0; i < m.len1; i++) {
        size_t i_max = get_bin(t, mat_get(m, i, 0));
        vec_set(v, i_max, mat_get(m, i, 1));
    }
    vec_free(t);
    return v;
}

void
scaled_data(const struct matrix m)
{
    struct vec v = vec_from_col(m, 1);
    double max = vec_max(v);
    vec_invscale(v, max);
}

double 
prob_dot_prod(const struct matrix u, const struct matrix v)
{
    // input are nx2 matrices of (mean, std) pairs

    struct vec u_mean = vec_from_col(u, 0);
    struct vec u_std = vec_from_col(u, 1);
    struct vec v_mean = vec_from_col(v, 0);
    struct vec v_std = vec_from_col(v, 1);

    // add 1e-4 so we don't have 0 std
    vec_add_const(u_std, 1e-4);
    vec_add_const(v_std, 1e-4);

    struct vec weights = vec_copy(u_std);
    vec_scale(weights, 2);
    vec_multiply(weights, v_std);

    // sq_sum = u_std^2 + v_std^2       <-- all pointwise
    struct vec sq_sum = vec_copy(u_std);
    struct vec tmp = vec_copy(v_std);
    vec_square(sq_sum);
    vec_square(tmp);
    vec_add(sq_sum, tmp);
    vec_free(tmp);

    vec_divide(weights, sq_sum);
    vec_sqrt(weights);

    struct vec tmp3 = vec_copy(u_mean);
    vec_sub(tmp3, v_mean);
    vec_square(tmp3);
    vec_scale(tmp3, -0.5);
    vec_divide(tmp3, sq_sum);

    vec_exp(tmp3);

    vec_multiply(weights, tmp3);
    vec_free(sq_sum);
    vec_free(tmp3);

    vec_multiply(weights, v_mean);
    double denom = sqrt(vec_dot(u_mean, u_mean)) * sqrt(vec_dot(v_mean, v_mean));
    double ans = vec_dot(weights, u_mean) / (sqrt(vec_dot(u_mean, u_mean)) * sqrt(vec_dot(v_mean, v_mean)));

    vec_free(weights);
    return ans;
}


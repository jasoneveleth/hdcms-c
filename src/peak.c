#include <stdbool.h>
#include <math.h>
#include "peak.h"

bool
equals(double a, double b)
{
    double ratio = a / b;
    return fabs(ratio - 1) < TOLRAT;
}

size_t
min2(size_t x, size_t y) 
{
    return x < y ? x : y;
}

size_t
min3(size_t x, size_t y, size_t z) 
{
    return min2(min2(x, y), z);
}

double
peak_sim_measure_L2(struct array m1, struct array m2, size_t n) 
{
    // 2 4xn matrices (output of `peak_stat`), number of peaks -> similarity of them btwn
    // 0 and 1
    // TODO
    return 0;
}

struct matrix
peak_stat(struct array matrices) 
{
    // 5 matrices of spectra, and number of peaks -> nx4 matrix of peaks
    // TODO
    struct matrix A;
    return A;
}

double
cos_sim_L2(struct vec u, struct vec v)
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

struct array
peak_sort(struct array matrices, size_t n)
{
    // 5 matrices of spectra, number of peaks -> n matrices s.t. ith matrix is 5
    // pts assoc. with ith largest peak
    // TODO
    struct array arr;
    return arr;
}


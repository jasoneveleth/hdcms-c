#include <stdio.h>
#include "array.h"

double
peak_sim_measure_L2() {
    // 2 4xn matrices (output of `peak_stat`), number of peaks -> similarity of them btwn
    // 0 and 1
    // TODO
    return 0;
}

struct matrix
peak_stat() {
    // 5 matrices of spectra, and number of peaks -> nx4 matrix of peaks
    // TODO
    struct matrix A;
    return A;
}

double
cos_sim_L2(struct vec u, struct vec v)
{
    // 2 4-length vectors -> cos of angle between gaussians those vectors represent
    // TODO
    return 0;
}

struct array
peak_sort()
{
    // 5 matrices of spectra, number of peaks -> n matrices s.t. ith matrix is 5
    // pts assoc. with ith largest peak
    // TODO
    struct array arr;
    return arr;
}


int main()
{
    return 0;
}

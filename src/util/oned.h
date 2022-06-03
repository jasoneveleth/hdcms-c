#ifndef ONED_H
#define ONED_H
#include <stdbool.h>
#include "array.h"

#define E (2.718281828459045235360287471352)

size_t min2(const size_t x, const size_t y);
size_t min3(const size_t x, const size_t y, const size_t z);

struct matrix bin_stat_1D(const struct matarray A, double width);
void scaled_data(const struct matrix m);

// this function bins the spectra m (an array of (x,y) pairs) into 900 bins
struct vec spec_vec(const struct matrix m, double width);
double prob_dot_prod(const struct matrix u, const struct matrix v);

#endif // ONED_H

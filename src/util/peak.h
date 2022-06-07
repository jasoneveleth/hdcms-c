#ifndef PEAK_H
#define PEAK_H
#include <stdbool.h>
#include "array.h"

size_t min2(const size_t x, const size_t y);
size_t min3(const size_t x, const size_t y, const size_t z);

double peak_sim_measure_L2(const struct matrix m1, const struct matrix m2, size_t n);
double cos_sim_L2(const struct vec u, const struct vec v);
struct matarray peak_sort(const struct matarray matrices, size_t n);
struct matrix peak_stat(const struct matarray matrices, size_t n);

#endif // PEAK_H

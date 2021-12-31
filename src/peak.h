#ifndef PEAK_H
#define PEAK_H
#include <stdbool.h>
#include "array.h"

#define E (2.718281828459045235360287471352)
// tolerance ratio
#define TOLRAT (0.0001)

bool equals(const double a, const double b);
size_t min2(const size_t x, const size_t y);
size_t min3(const size_t x, const size_t y, const size_t z);

double peak_sim_measure_L2(const struct matarray m1, const struct matarray m2, size_t n);
double cos_sim_L2(const struct vec u, const struct vec v);
struct matarray peak_sort(const struct matarray matrices, size_t n);
struct matrix peak_stat(const struct matarray matrices);

#endif // PEAK_H

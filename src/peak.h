#ifndef PEAK_H
#define PEAK_H
#include <stdbool.h>
#include "array.h"

#define E (2.718281828459045235360287471352)
// tolerance ratio
#define TOLRAT (0.0001)

bool equals(double a, double b);
size_t min2(size_t x, size_t y);
size_t min3(size_t x, size_t y, size_t z);

double peak_sim_measure_L2(struct array m1, struct array m2, size_t n);
double cos_sim_L2(struct vec u, struct vec v);
struct array peak_sort(struct array matrices, size_t n);
struct matrix peak_stat(struct array matrices);

#endif // PEAK_H

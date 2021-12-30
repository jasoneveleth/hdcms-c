#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "array.h"

double
vec_max(const struct vec v)
{
    double max = -INFINITY;
    for (size_t i = 0; i < v.length; i++) {
        double ith = v.data[i * v.stride];
        if (ith > max) {
            max = ith;
        }
    }
    return max;
}

double
vec_min(const struct vec v)
{
    double min = INFINITY;
    for (size_t i = 0; i < v.length; i++) {
        double ith = v.data[i * v.stride];
        if (ith < min) {
            min = ith;
        }
    }
    return min;
}

/* kahan summation */
double
vec_sum(const struct vec v)
{
    double sum = 0;
    double old_low_bits = 0;
    for (size_t i = 0; i < v.length; i++) {
        double y = vec_get(v, i) - old_low_bits; // = high - low - oldlow = newhigh - newlow
        double t = sum + y;                      // = sum + newhigh
        old_low_bits = (t - sum) - y;            // = (newhigh) - (newhigh - newlow)
        sum = t;
    }
    return sum;
}

struct vec
vec_from_data(double *data, size_t len)
{
    struct vec v;
    v.data = data;
    v.is_owner = true;
    v.length = len;
    v.stride = 1;
    return v;
}

void
vec_printf(const char *const format, const struct vec v)
{
    for (size_t i = 0; i < v.length; i++) {
        printf(format, vec_get(v, i));
    }
    printf("\n");
}

void
vec_scale(struct vec v, const double c)
{
    for (size_t i = 0; i < v.length; i++) {
        vec_set(v, i, vec_get(v, i) * c);
    }
}

struct vec
vec_read(FILE *file, const char *const format)
{
    // TODO
    struct vec v;
    return v;
}

void
vec_write(FILE *file, const struct vec v)
{
    // TODO
}

double
vec_get(const struct vec v, size_t i)
{
    return v.data[v.stride * i];
}

void
vec_set(struct vec v, size_t i, double a)
{
    v.data[v.stride * i] = a;
}

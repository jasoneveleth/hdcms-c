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
        double ith = vec_get(v, i);
        if (ith > max) {
            max = ith;
        }
    }
    return max;
}

void
mat_set(struct matrix m, size_t i, size_t j, double x)
{
    m.data[i * m.physlen1 + j] = x;
}

struct matrix
mat_zeros(size_t len1, size_t len2)
{
    return mat_from_data(calloc(len1 * len2, sizeof(double)), len1, len2, len1);
}

double
vec_argmax(const struct vec v)
{
    double max = -INFINITY;
    size_t argmax = 0;
    for (size_t i = 0; i < v.length; i++) {
        double ith = vec_get(v, i);
        if (ith > max) {
            max = ith;
            argmax = i;
        }
    }
    return argmax;
}

double
vec_min(const struct vec v)
{
    double min = INFINITY;
    for (size_t i = 0; i < v.length; i++) {
        double ith = vec_get(v, i);
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

struct vec
vec_zeros(size_t len)
{
    double *data = calloc(len, sizeof(double));
    return vec_from_data(data, len);
}

double
mat_get(const struct matrix m, size_t i, size_t j)
{
    return m.data[i * m.physlen1 + j];
}

struct matrix
mat_from_data(double *data, size_t len1, size_t len2, size_t physlen1)
{
    struct matrix m;
    m.is_owner = 1;
    m.data = data;
    m.len1 = len1;
    m.len2 = len2;
    m.physlen1 = physlen1;
    return m;
}

struct matrix
mat_copy(struct matrix m)
{
    double *newdata = malloc(m.len1 * m.len2 * sizeof(double));
    for (size_t i = 0; i < m.len1; i++) {
        for (size_t j = 0; j < m.len2; j++) {
            newdata[i * m.len1 + j] = mat_get(m, i, j);
        }
    }
    return mat_from_data(newdata, m.len1, m.len2, m.len1);
}

struct matarray
matarr_copy(const struct matarray old)
{
    struct matarray new;
    for (size_t i = 0; i < old.length; i++)
    {
        matarr_set(new, i, mat_copy(matarr_get(old, i)));
    }
    return new;
}

struct vec
vec_from_col(struct matrix m, size_t col)
{
    struct vec v;
    v.data = m.data + col;
    v.is_owner = false;
    v.length = m.len2;
    v.stride = m.physlen1;
    return v;
}

struct matarray
matarr_zeros(size_t len)
{
    struct matarray arr;
    arr.length = len;
    arr.data = calloc(len, sizeof(struct matarray));
    return arr;
}

void
matarr_set(const struct matarray arr, size_t i, struct matrix m)
{
    arr.data[i] = m;
}

struct matrix
matarr_get(const struct matarray arr, size_t i)
{
    return arr.data[i];
}

void
vec_set(struct vec v, size_t i, double a)
{
    v.data[v.stride * i] = a;
}

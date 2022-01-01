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

struct matarray
matarr_from_data(struct matrix *data, size_t len)
{
    struct matarray arr;
    arr.length = len;
    arr.data = data;
    return arr;
}

void
mat_set(struct matrix m, const size_t i, const size_t j, const double x)
{
    m.data[i * m.physlen + j] = x;
}

struct matrix
mat_zeros(size_t len1, size_t len2)
{
    return mat_from_data(calloc(len1 * len2, sizeof(double)), len1, len2, len2, true);
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
vec_from_data(double *data, size_t len, int is_owner)
{
    struct vec v;
    v.data = data;
    v.is_owner = is_owner;
    v.length = len;
    v.stride = 1;
    return v;
}

void
vec_printf(const struct vec v)
{
    printf("[");
    for (size_t i = 0; i < v.length; i++) {
        printf("%6g", vec_get(v, i));
        if (i != v.length - 1)
            printf(", ");
    }
    printf("]\n");
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
    return vec_from_data(data, len, true);
}

double
mat_get(const struct matrix m, size_t i, size_t j)
{
    return m.data[i * m.physlen + j];
}

struct matrix
mat_from_data(double *data, size_t len1, size_t len2, size_t physlen, int is_owner)
{
    struct matrix m;
    m.is_owner = is_owner;
    m.data = data;
    m.len1 = len1;
    m.len2 = len2;
    m.physlen = physlen;
    return m;
}

struct matrix
mat_copy(struct matrix m)
{
    double *newdata = calloc(m.len1 * m.len2, sizeof(double));
    for (size_t i = 0; i < m.len1; i++) {
        for (size_t j = 0; j < m.len2; j++) {
            newdata[i * m.len2 + j] = mat_get(m, i, j);
        }
    }
    return mat_from_data(newdata, m.len1, m.len2, m.len2, true);
}

struct matarray
matarr_copy(const struct matarray old)
{
    struct matarray new;
    new.length = old.length;
    new.data = calloc(old.length, sizeof(struct matrix));
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
    v.length = m.len1;
    v.stride = m.physlen;
    return v;
}

struct matarray
matarr_zeros(size_t len)
{
    struct matarray arr;
    arr.length = len;
    arr.data = calloc(len, sizeof(struct matrix));
    return arr;
}

void
matarr_free(struct matarray arr)
{
    free(arr.data);
}

void
matarr_printf(const struct matarray arr)
{
    for (size_t j = 0; j < arr.length; j++) {
        printf("%zd:\n", j);
        mat_printf(matarr_get(arr, j));
    }
}

void
mat_printf(const struct matrix m)
{
    printf("[");
    for (size_t i = 0; i < m.len1; i++) {
        if (i != 0) printf(" ");
        printf("[");
        for (size_t j = 0; j < m.len2; j++) {
            printf("%6g, ", mat_get(m, i, j));
        }
        printf("]");
        if (i != m.len1 - 1) printf("\n");
    }
    printf("]");
    printf("\n");
}

void
mat_free(struct matrix m)
{
    if (m.is_owner)
        free(m.data);
}

void
vec_free(struct vec v)
{
    if (v.is_owner)
        free(v.data);
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

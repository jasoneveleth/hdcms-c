#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "array.h"

double
vec_max(struct vec *v)
{
    double max = -INFINITY;
    for (size_t i = 0; i < v->length; i++) {
        double ith = v->data[i * v->stride];
        if (ith > max) {
            max = ith;
        }
    }
    return max;
}

double
vec_min(struct vec *v)
{
    double min = INFINITY;
    for (size_t i = 0; i < v->length; i++) {
        double ith = v->data[i * v->stride];
        if (ith < min) {
            min = ith;
        }
    }
    return min;
}

double
vec_sum(struct vec *v)
{
    // TODO
    return 0;
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
vec_printf(char *format, struct vec v)
{
    for (size_t i = 0; i < v.length; i++) {
        printf(format, vec_get(v, i));
    }
    printf("\n");
}

struct vec
vec_read(FILE *file, char *format)
{
    // TODO
    struct vec v;
    return v;
}

void
vec_write(FILE *file, struct vec *v)
{
    // TODO
}

double
vec_get(struct vec v, size_t i)
{
    return v.data[v.stride * i];
}

void
vec_set(struct vec v, size_t i, double a)
{
    v.data[v.stride * i] = a;
}

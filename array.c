#include <math.h>
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

struct vec
vec_fread(FILE *file, char *format)
{
    return NULL;
}


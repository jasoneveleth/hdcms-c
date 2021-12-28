#ifndef ARRAY_H
#define ARRAY_H
#include <stdlib.h>

struct vec
{
    size_t length;
    size_t stride;
    double *data;
    int is_owner;
};

struct matrix
{
    size_t len1;
    size_t len2;
    size_t physlen1;
    double *data;
    int is_owner;
};

double vec_max(struct vec *v);
double vec_min(struct vec *v);

#endif // ARRAY_H

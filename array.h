#ifndef ARRAY_H
#define ARRAY_H
#include <stdlib.h>
#include <stdio.h>

struct array
{
    size_t length;
    size_t elewidth;
    void *data;
};

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
double vec_sum(struct vec *v);
struct vec vec_fread(FILE *file, char *format);
void vec_write(FILE *file, struct vec *v);
double vec_get(struct vec v, size_t i);
void vec_set(struct vec v, size_t i, double a);


#endif // ARRAY_H

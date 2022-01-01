#ifndef ARRAY_H
#define ARRAY_H
#include <stdlib.h>
#include <stdio.h>

#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

struct matarray
{
    size_t length;
    struct matrix *data;
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
    size_t physlen;
    double *data;
    int is_owner;
};

double vec_max(const struct vec v);
double vec_min(const struct vec v);
double vec_sum(const struct vec v);
struct vec vec_fread(FILE *file, const char *const format);
void vec_write(FILE *file, const struct vec v);
double vec_get(const struct vec v, size_t i);
void vec_set(struct vec v, size_t i, double a);
struct vec vec_from_data(double *data, size_t len);
void vec_printf(const char *const format, const struct vec v);
void vec_scale(struct vec v, const double c);
struct matrix matarr_get(const struct matarray arr, size_t i);
struct vec vec_zeros(size_t len);
void matarr_set(const struct matarray arr, size_t i, struct matrix m);
struct matarray matarr_copy(const struct matarray old);
struct matarray matarr_zeros(size_t len);
struct vec vec_from_col(struct matrix m, size_t col);
double mat_get(const struct matrix m, size_t i, size_t j);
double vec_argmax(const struct vec v);
struct matrix mat_from_data(double *data, size_t len1, size_t len2, size_t physlen);
struct matrix mat_zeros(size_t len1, size_t len2);
void mat_set(struct matrix m, const size_t i, const size_t j, const double x);
struct matarray matarr_from_data(struct matrix *data, size_t len);


#endif // ARRAY_H

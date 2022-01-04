#ifndef ARRAY_H
#define ARRAY_H
#include <stdlib.h>
#include <stdio.h>

#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

#define WARNING(fmt, ...) fprintf(stderr, "%s:%d: " RED "WARNING: " RESET fmt, __FILE__, __LINE__, __VA_ARGS__)

// tolerance ratio
#define TOLRAT (0.0001)

struct matarray
{
    size_t length;
    struct matrix *data;
    int is_owner;
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
struct vec vec_from_data(double *data, size_t len, int is_owner);
void vec_printf(const struct vec v);
void vec_scale(struct vec v, const double c);
struct matrix matarr_get(const struct matarray arr, size_t i);
struct vec vec_zeros(size_t len);
void matarr_set(const struct matarray arr, size_t i, struct matrix m);
struct matarray matarr_copy(const struct matarray old);
struct matarray matarr_zeros(size_t len);
struct vec vec_from_col(struct matrix m, size_t col);
double mat_get(const struct matrix m, size_t i, size_t j);
double vec_argmax(const struct vec v);
struct matrix mat_from_data(double *data, size_t len1, size_t len2, size_t physlen, int is_owner);
struct matrix mat_zeros(size_t len1, size_t len2);
void mat_set(struct matrix m, const size_t i, const size_t j, const double x);
struct matarray matarr_from_data(struct matrix *data, size_t len, const bool is_owner);
void matarr_free(struct matarray arr);
void mat_free(struct matrix m);
void vec_free(struct vec v);
void mat_printf(const struct matrix m);
void matarr_printf(const struct matarray arr);
void vec_add_const(struct vec v, const double a);
void vec_square(struct vec v);
struct vec vec_copy(const struct vec v);
double vec_mean(const struct vec v);
double vec_std(const struct vec v);
bool equals(const double a, const double b);
bool mat_equal(const struct matrix A, const struct matrix B);
bool matarr_equal(const struct matarray arr, const struct matarray arr2);
bool vec_equal(const struct vec v1, const struct vec v2);


#endif // ARRAY_H

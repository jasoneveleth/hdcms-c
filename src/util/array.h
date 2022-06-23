#ifndef ARRAY_H
#define ARRAY_H
#include "../fixwindows.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

#if !defined(RED) || !defined(RESET)
#define RED ""
#define RESET ""
#endif
#define WARNING(fmt, ...) fprintf(stderr, "%s:%d: " RED "WARNING: " RESET fmt, __FILE__, __LINE__, __VA_ARGS__)

// tolerance ratio
#define TOLRAT (1e-5)
#define TOLABS (1e-6)

#define inf ((double)INFINITY)

/* 
 * This is a data structure to hold a list of matrices. It stores a pointer to
 * the array of matrix structs, and has a length and boolean of whether this
 * struct owns the data in the pointer.
 */
struct matarray
{
    size_t length;
    struct matrix *data;
    long is_owner;
};

/* 
 * This is a vector data structure. It has a pointer to the array of doubles,
 * and has a length (which we do bounds checking on in assert()'s). It also has
 * a stride which is the gap in memory between doubles. This means to calculate
 * the address of an index 73 element, you dereference v->data[stride * 73].
 */
struct vec
{
    size_t length;
    size_t stride;
    double *data;
    long is_owner;
};

/*
 * This is a matrix data structure, it stores len1 which is the number of rows,
 * and len2 which is the number of columns. We bounds check on that in
 * assert()'s. We also store a physlen, which is the number of double in each
 * row in memory. We store this in case len2 and physlen don't agree, we need to
 * still calculate the correct offset into the data pointer. Look at mat_get()
 * to see how the offset to get a specific (i,j) index is calculated.
 */
struct matrix
{
    size_t len1;
    size_t len2;
    size_t physlen;
    double *data;
    long is_owner;
};

/* vec */
// retrive element at index in the vector
static inline double
vec_get(const struct vec v, size_t i)
{
    assert(i < v.length && "trying to get an index outside bounds in vector");
    return v.data[v.stride * i];
}
// *mutates* element at index
static inline void
vec_set(struct vec v, size_t i, double a)
{
    assert(i < v.length && "trying to set an index outside bounds in vector");
    v.data[v.stride * i] = a;
}
// maximum value in vector
double vec_max(const struct vec v);
// minimum value in vector
double vec_min(const struct vec v);
// uses a kahan summation to accumulate with quad precision
double vec_sum(const struct vec v);
// initializes a vector structure with data pointer
struct vec vec_from_data(double data[], size_t len, int is_owner);
// write vector to stdout
void vec_printf(const struct vec v);
// write vector to a file
void vec_fprintf(FILE *fp, const struct vec v);
// *mutates* the vector by multiplying each element by a
void vec_scale(struct vec v, const double a);
// *mutates* the vector by dividing each element by a
void vec_invscale(struct vec v, const double a);
// initializes vector full of zeros
struct vec vec_zeros(size_t len);
// return the index of the largest element of the array
size_t vec_argmax(const struct vec v);
// destructor for vector
void vec_free(struct vec v);
// *mutates* vector by adding a constant `a` to each element
void vec_add_const(struct vec v, const double a);
// *mutates* vector v by element-wise subtracting values of u
void vec_sub(struct vec v, const struct vec u);
// *mutates* vector v by element-wise squaring
void vec_square(struct vec v);
// *mutates* vector v by element-wise square-rooting
void vec_sqrt(struct vec v);
// *mutates* vector v by element-wise multiplying values of u
void vec_multiply(struct vec v, const struct vec u);
// *mutates* vector v by element-wise dividing values of u
void vec_divide(struct vec v, const struct vec u);
// *mutates* vector v by raising each value to the power of Euler's constant e
void vec_exp(struct vec v);
// returns dot product of two vectors
double vec_dot(const struct vec v, const struct vec u);
// *mutates* vector v by element-wise addition of u
void vec_add(struct vec v, const struct vec u);
// returns a copy of vector v which has its own copied pointer
struct vec vec_copy(const struct vec v);
// returns the arithmetic mean of the elements of v
double vec_mean(const struct vec v);
// returns the sample standard deviation of the elements of v
double vec_std(const struct vec v);
// returns whether two vectors are equal (using a tolerance)
bool vec_equal(const struct vec v1, const struct vec v2);
// takes file pointer, and reads contents into a vector
struct vec vec_fscanf(FILE *file);
// takes path name and reads file into a vector using the default format string "%lg"
struct vec vec_from_file(const char *path);
// *mutates* v by setting all the values to a
void vec_set_all(struct vec v, const double a);
// returns a vector of equally-spaced values of size num_steps from start to end
struct vec vec_linspace(double start, double end, double num_steps);

/* vec and mat */
// returns a vector which points to one of the matrix's columns
struct vec vec_from_col(const struct matrix m, const size_t col);
// returns a vector which points to one of the matrix's rows
struct vec vec_from_row(const struct matrix m, const size_t row);
// *mutates* the matrix by setting one of its rows to be the contents of v
void vec_to_row(struct matrix m, const struct vec v, const size_t row);

/* matarr */
// retrives a value from array of matrices at index i
static inline struct matrix
matarr_get(const struct matarray arr, size_t i)
{
    assert(i < arr.length && "trying to get an index outside bounds in matarray");
    return arr.data[i];
}
// *mutates* matrix array by setting index i to struct matrix
static inline void
matarr_set(const struct matarray arr, size_t i, struct matrix m)
{
    assert(i < arr.length && "trying to set an index outside bounds in matarray");
    arr.data[i] = m;
}
// duplicates the data in the matrix array into a new array
struct matarray matarr_copy(const struct matarray old);
// initializes empty matrix array of the length len
struct matarray matarr_zeros(size_t len);
// initializes a matrix array with data from pointer
struct matarray matarr_from_data(struct matrix data[], size_t len, const bool is_owner);
// destructor for matrix array
void matarr_free(struct matarray arr);
// print the matrix array to stdout
void matarr_printf(const struct matarray arr);
// printf the stats of the matrices in the array to stdout
void matarr_stats_printf(struct matarray arr);
// check for equality between matrix arrays
bool matarr_equal(const struct matarray arr, const struct matarray arr2);

/* mat */
// *mutates* matrix by setting elemetn (i,j) to x
static inline void
mat_set(struct matrix m, const size_t i, const size_t j, const double a)
{
    assert(i < m.len1 && "trying to set an index outside num_rows in matrix");
    assert(j < m.len2 && "trying to set an index outside num_cols in matrix");
    m.data[i * m.physlen + j] = a;
}
// retrieve element at index (i,j)
static inline double
mat_get(const struct matrix m, size_t i, size_t j)
{
    assert(i < m.len1 && "trying to get an index outside num_rows in matrix");
    assert(j < m.len2 && "trying to get an index outside num_cols in matrix");
    return m.data[i * m.physlen + j];
}
// initalizes matrix from pointer
struct matrix mat_from_data(double data[], size_t len1, size_t len2, size_t physlen, int is_owner);
// initalizes empty matrix of size (len1, len2)
struct matrix mat_zeros(size_t len1, size_t len2);
// destructor for matrix
void mat_free(struct matrix m);
// print properties of matrix to stdout
void mat_stats_printf(struct matrix m);
// print matrix in human readable form to stdout
void mat_printf(const struct matrix m);
// print matrix in human readable form to file
void mat_fprintf(FILE * restrict file, const struct matrix m);
// check for equality between 
bool mat_equal(const struct matrix m1, const struct matrix m2);
// duplicate the data in matrix and return struct with that data
struct matrix mat_copy(struct matrix m);
// reads from file pointer a human readable matrix, returns that matrix
struct matrix mat_fscanf(FILE *file);
// takes pathname and reads file using mat_fscanf()
struct matrix mat_from_file(const char *path);

/* safely handle failures of system functions */
// wrapper for calloc(), which requests memory, and returns a pointer to it
void *safe_calloc(size_t num, size_t size);
// wrapper for realloc(), which resizes the memory pointed to by ptr
void *safe_realloc(void *ptr, size_t size);
// wrapper for fopen() which opens a path and returns a file pointer.
FILE *safe_fopen(const char *const path, const char *const mode);
// wrapper for freopn() which changes file stream to point to file at path
FILE * safe_freopen(const char *path, const char *mode, FILE *stream);

// reads one line from a file, removes newline if any
char *read_line(FILE *fp);
char *find_token(char **resumer, const char *const sep);

// returns the bits of a float as unsigned 64 bit int
uint64_t d2z(const double a);
// takes 64 bits and interprets them as a double (and returns it)
double z2d(const uint64_t a);
// this function checks if two doubles are within a tolerance
bool equals(const double a, const double b);

#endif // ARRAY_H

#ifndef ARRAY_H
#define ARRAY_H
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

// escape codes for terminals
#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

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
    int is_owner;
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
    int is_owner;
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
    int is_owner;
};

/* vec */
// maximum value in vector
double vec_max(const struct vec v);
double vec_min(const struct vec v);
// uses a kahan summation to accumulate with quad precision
double vec_sum(const struct vec v);
// retrive element at index in the vector
double vec_get(const struct vec v, size_t i);
// *mutates* element at index
void vec_set(struct vec v, size_t i, double a);
// initializes a vector structure with data pointer
struct vec vec_from_data(double *data, size_t len, int is_owner);
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
// *mutates* vector by adding a constant a to each element
void vec_add_const(struct vec v, const double a);
// *mutates* vector v by element-wise subtracting values of u
void vec_sub(struct vec v, struct vec u);
// *mutates* vector v by element-wise squaring
void vec_square(struct vec v);
// *mutates* vector v by element-wise square-rooting
void vec_sqrt(struct vec v);
// *mutates* vector v by element-wise multiplying values of u
void vec_multiply(struct vec v, struct vec u);
// *mutates* vector v by element-wise dividing values of u
void vec_divide(struct vec v, struct vec u);
// *mutates* vector v by raising each value to the power of Euler's constant e
void vec_exp(struct vec v);
// returns dot product of two vectors
double vec_dot(const struct vec v, const struct vec u);
// *mutates* vector v by element-wise addition of u
void vec_add(struct vec v, struct vec u);
// returns a copy of vector v which has its own copied pointer
struct vec vec_copy(const struct vec v);
// returns the arithmetic mean of the elements of v
double vec_mean(const struct vec v);
// returns the sample standard deviation of the elements of v
double vec_std(const struct vec v);
// returns whether two vectors are equal (using a tolerance)
bool vec_equal(const struct vec v1, const struct vec v2);
// takes file pointer, and reads contents into a vector
// if the format string is NULL, then it uses the format "%lg"
struct vec vec_fscanf(FILE *file, const char *const format);
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
struct matrix matarr_get(const struct matarray arr, size_t i);
// *mutates* matrix array by setting index i to struct matrix
void matarr_set(const struct matarray arr, size_t i, struct matrix m);
// duplicates the data in the matrix array into a new array
struct matarray matarr_copy(const struct matarray old);
// initializes empty matrix array of the length len
struct matarray matarr_zeros(size_t len);
// initializes a matrix array with data from pointer
struct matarray matarr_from_data(struct matrix *data, size_t len, const bool is_owner);
// destructor for matrix array
void matarr_free(struct matarray arr);
// print the matrix array to stdout
void matarr_printf(const struct matarray arr);
// check for equality between matrix arrays
bool matarr_equal(const struct matarray arr, const struct matarray arr2);

/* mat */
double mat_get(const struct matrix m, size_t i, size_t j);
struct matrix mat_from_data(double *data, size_t len1, size_t len2, size_t physlen, int is_owner);
struct matrix mat_zeros(size_t len1, size_t len2);
void mat_set(struct matrix m, const size_t i, const size_t j, const double x);
void mat_free(struct matrix m);
void mat_printf(const struct matrix m);
void mat_fprintf(FILE * restrict file, const struct matrix m);
bool mat_equal(const struct matrix A, const struct matrix B);
struct matrix mat_copy(struct matrix m);
struct matrix mat_read(FILE *file);
struct matrix mat_from_file(const char *path);

void *safe_calloc(size_t num, size_t size);
void *safe_realloc(void *ptr, size_t size);
FILE *safe_fopen(const char *path, const char *mode);

char *read_line(FILE *fp);

/* 
 * This function takes a double and returns it's bits as an unsigned 64 bit int,
 * which is very useful when trying to generate bitwise identical floats to
 * MATLAB
 */
double z2d(const uint64_t a);
// this function takes 64 bits and interprets them as a double
uint64_t d2z(const double a);
// this function checks if two doubles are within a tolerance
bool equals(const double a, const double b);

#endif // ARRAY_H

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "array.h"

void *
safe_calloc(size_t num, size_t size)
{
    void *ret = calloc(num, size);
    if (ret == NULL) {
        perror("calloc");
        abort();
    }
    return ret;
}

void *
safe_realloc(void *ptr, size_t size)
{
    void *ret = realloc(ptr, size);
    if (ret == NULL) {
        perror("realloc");
        abort();
    }
    return ret;
}

FILE *
safe_fopen(const char * restrict path, const char * restrict mode)
{
    FILE *ret = fopen(path, mode);
    if (ret == NULL) {
        fprintf(stderr, "%s: ", path);
        perror("");
        abort();
    }
    return ret;
}

bool
equals(const double a, const double b)
{
    if (b == 0) {
        return fabs(a) < TOLABS;
    }
    double ratio = a / b;
    return fabs(ratio - 1) < TOLRAT;
}

bool
matarr_equal(const struct matarray arr, const struct matarray arr2)
{
    if (arr.length != arr2.length) {
        WARNING("incompatible matrix arrays\n\tmatarr_equal %zd vs %zd\n", arr.length, arr2.length);
        return false;
    }
    for (size_t i = 0; i < arr.length; i++) {
        if (!mat_equal(matarr_get(arr, i), matarr_get(arr2, i))) {
            return false;
        }
    }
    return true;
}

bool
mat_equal(const struct matrix A, const struct matrix B)
{
    if (A.len1 != B.len1 || A.len2 != B.len2) {
        WARNING("incompatible matrices\n\tmat_equal %zdx%zd vs %zdx%zd\n", A.len1, A.len2, B.len1, B.len2);
        return false;
    }
    for (size_t i = 0; i < A.len1; i++) {
        for (size_t j = 0; j < A.len2; j++) {
            if (!equals(mat_get(A, i, j), mat_get(B, i, j))) {
                return false;
            }
        }
    }
    return true;
}

bool
vec_equal(const struct vec v1, const struct vec v2)
{
    if (v1.length != v2.length) {
        WARNING("incompatible vectors\n\tvec_equal %zdvs%zd\n", v1.length, v2.length);
        return false;
    }
    bool ret = true;
    for (size_t i = 0; i < v1.length; i++) {
        if (!equals(vec_get(v1, i), vec_get(v2, i))) {
            printf("%zd: %14e %14e\n", i, vec_get(v1, i), vec_get(v2, i));
            ret = false;
        }
    }
    return ret;
}

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
matarr_from_data(struct matrix *data, size_t len, const bool is_owner)
{
    struct matarray arr;
    arr.length = len;
    arr.data = data;
    arr.is_owner = is_owner;
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
    return mat_from_data(safe_calloc(len1 * len2, sizeof(double)), len1, len2, len2, true);
}

double
vec_mean(const struct vec v) 
{
    double sum = vec_sum(v);
    return sum/v.length;
}

void
vec_square(struct vec v) 
{
    for (size_t i = 0; i < v.length; i++) {
        const double ele = vec_get(v, i);
        vec_set(v, i, ele * ele);
    }
}

struct vec
vec_copy(const struct vec v)
{
    struct vec vcopy;
    vcopy.data = safe_calloc(v.length, sizeof(double));
    vcopy.is_owner = true;
    vcopy.length = v.length;
    vcopy.stride = 1;
    for (size_t i = 0; i < v.length; i++) {
        vec_set(vcopy, i, vec_get(v, i));
    }
    return vcopy;
}

double
vec_std(const struct vec v) 
{
    if (v.length < 2) {
            return 0;
    }
    struct vec vcopy = vec_copy(v);
    double mean = vec_mean(vcopy);
    vec_add_const(vcopy, -mean);
    vec_square(vcopy);
    double variance = vec_sum(vcopy)/(v.length - 1);
    vec_free(vcopy);
    return sqrt(variance);
}

void
vec_add_const(struct vec v, const double a)
{
    for (size_t i = 0; i < v.length; i++) {
        vec_set(v, i, vec_get(v, i) + a);
    }
}

size_t
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
vec_fprintf(FILE *fp, const struct vec v)
{
    fprintf(fp, "[");
    for (size_t i = 0; i < v.length; i++) {
        fprintf(fp, "%6e", vec_get(v, i));
        if (i != v.length - 1)
            fprintf(fp, ", ");
    }
    fprintf(fp, "]\n");
}

void
vec_printf(const struct vec v)
{
    vec_fprintf(stdout, v);
}

void
vec_scale(struct vec v, const double c)
{
    for (size_t i = 0; i < v.length; i++) {
        vec_set(v, i, vec_get(v, i) * c);
    }
}

struct vec
vec_read(FILE *file, const char *const argformat)
{
    const char *const format = (argformat == NULL) ? "%lg" : argformat;
    struct vec v;
    size_t len_of_vec = 0;
    size_t allocd = 1;
    double *data = safe_calloc(allocd, sizeof(double));

    while (!feof(file)) {
        char *line = read_line(file);

        double ele = 0;
        int num_matches = sscanf(line, format, &ele);
        if (num_matches == EOF) { // empty line
            free(line);
            continue;
        }
        if (num_matches == 0) { // invalid input
            WARNING("vec_read: line didn't match\n\t`%s`\n\tsetting ele to 0.0\n", line);
        }
        if (len_of_vec == allocd) {
            allocd = len_of_vec * 2;
            data = safe_realloc(data, allocd * sizeof(double));
        }
        data[len_of_vec++] = ele;
        free(line);
    }
    v.data = data;
    v.is_owner = true;
    v.length = len_of_vec;
    v.stride = 1;
    return v;
}

double
safe_strtod(const char *const token)
{
    if (token == NULL) {
        WARNING("no token recieved, setting ele to %g\n", 0.);
        return 0;
    }
    char *endptr;
    double ele = strtod(token, &endptr);
    if (token == endptr) {
        WARNING("unknown token\n\t\"%s\"\n\tsetting ele to 0.0\n", token);
    }
    return ele;
}

char *
find_token(char **resumer, const char *const sep)
{
    if ((*resumer) == NULL) {
        return NULL;
    }

    char *ret = &((*resumer)[strspn(*resumer, sep)]);
    char *next = &ret[strcspn(ret, sep)];
    if (next[0] == '\0') {
        *resumer = NULL;
    } else {
        next[0] = '\0';
        *resumer = &next[1];
    }
    return ret;
}

struct matrix
mat_read(FILE *file)
{
    size_t num_cols = 0;
    size_t allocd = 1;
    double *data = safe_calloc(allocd, sizeof(double));
    char *sep = " \t,";

    // handle first line, get # of columns
    char *first_line = read_line(file);
    char *resumer = first_line;
    char *token;
    while ((token = find_token(&resumer, sep))) {
        if (allocd <= num_cols) {
            allocd *= 2;
            data = safe_realloc(data, allocd * sizeof(double));
        }
        double ele = safe_strtod(token);
        data[num_cols++] = ele;
    }
    free(first_line);

    // handle the rest of the rows, assume the # of columns never changes
    size_t col_zero = num_cols; // ptr offset to col_zero of this row
    size_t num_rows = 1; // we read in the first row already
    while (!feof(file)) {
        char *line = read_line(file);
        if (line[0] == '\0') { // skip empty lines
            free(line);
            continue;
        }
        if (col_zero + num_cols >= allocd) {
            allocd *= 2;
            data = safe_realloc(data, allocd * sizeof(double));
        }
        char *resumer = line;
        for (int i = 0; i < num_cols; i++) {
            char *token = find_token(&resumer, sep);
            data[col_zero + i] = safe_strtod(token);
        }
        col_zero += num_cols;
        num_rows++;
        free(line);
    }

    struct matrix m;
    m.data = data;
    m.len1 = num_rows;
    m.len2 = num_cols;
    m.physlen = num_cols;
    m.is_owner = true;
    return m;
}

char *
read_line(FILE *fp)
{
    size_t allocd = 2;
    char *line = safe_calloc(allocd, sizeof(char));
    size_t len = 0;
    int c = getc(fp);
    while (c != EOF && c != '\n') {
        if (len >= allocd) {
            allocd *= 2;
            line = safe_realloc(line, allocd);
        }
        line[len++] = (char)c;
        c = getc(fp);
    }

    // handle possible error
    if (ferror(fp)) {
        perror("getc");
        abort();
    }

    // handle ending
    if (len > 0 && line[len - 1] == '\r') { // handle CRLF delimiter
        line[len - 1] = '\0';
    }
    line[len] = '\0';
    return line;
}

void
vec_write(FILE *file, const struct vec v)
{
    // TODO
}

void
mat_write(FILE *fp)
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
    double *data = safe_calloc(len, sizeof(double));
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
    double *newdata = safe_calloc(m.len1 * m.len2, sizeof(double));
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
    new.data = safe_calloc(old.length, sizeof(struct matrix));
    new.is_owner = true;
    for (size_t i = 0; i < old.length; i++)
    {
        matarr_set(new, i, mat_copy(matarr_get(old, i)));
    }
    return new;
}

void
vec_set_all(struct vec v, const double d)
{
    for (size_t i = 0; i < v.length; i++) {
        vec_set(v, i, d);
    }
}

struct vec
vec_from_col(const struct matrix m, const size_t col)
{
    struct vec v;
    v.data = m.data + col;
    v.is_owner = false;
    v.length = m.len1;
    v.stride = m.physlen;
    return v;
}

struct vec
vec_from_row(const struct matrix m, const size_t row)
{
    struct vec v;
    v.data = m.data + m.physlen * row;
    v.is_owner = false;
    v.length = m.len2;
    v.stride = 1;
    return v;
}

struct matarray
matarr_zeros(size_t len)
{
    return matarr_from_data(safe_calloc(len, sizeof(struct matrix)), len, true);
}

void
matarr_free(struct matarray arr)
{
    for (size_t i = 0; i < arr.length; i++) {
        mat_free(matarr_get(arr, i));
    }
    if (arr.is_owner)
        free(arr.data);
}

void
matarr_printf(const struct matarray arr)
{
    if (arr.length == 0) {
        printf("empty array");
    }
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
            printf("%6e, ", mat_get(m, i, j));
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

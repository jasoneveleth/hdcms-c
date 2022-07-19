#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "array.h"

#define BUF_SIZE_INIT 120

#define KAHAN_INIT(_SUM)                 \
    _SUM = 0;                            \
    double _SUM ## _low = 0              \

// read documentation for vec_sum
#define KAHAN_INCR(_SUM, _INCR)                           \
        double a ## _SUM = _INCR + _SUM ## _low;          \
        double new_ ## _SUM = _SUM + a ## _SUM;           \
        _SUM ## _low = a ## _SUM - (new_ ## _SUM - _SUM); \
        _SUM = new_ ## _SUM

/* 
 * This function takes a double and returns it's bits as an unsigned 64 bit int,
 * which is very useful when trying to generate bitwise identical floats to
 * MATLAB
 */
uint64_t
d2z(const double a)
{
    union {double d; uint64_t z;} mem = {.d = a};
    return mem.z;
}

double
z2d(const uint64_t a)
{
    union {double d; uint64_t z;} mem = {.z = a};
    return mem.d;
}

void *
safe_calloc(size_t num, size_t size)
{
    void *ret = calloc(num, size);
    if (ret == NULL) {
        perror("calloc");
        exit(1);
    }
    return ret;
}

void *
safe_realloc(void *ptr, size_t size)
{
    void *ret = realloc(ptr, size);
    if (ret == NULL) {
        perror("realloc");
        exit(1);
    }
    return ret;
}

FILE *
safe_fopen(const char * restrict path, const char * restrict mode)
{
    FILE *ret = fopen(path, mode);
    if (ret == NULL) {
        perror(path);
        exit(1);
    }
    return ret;
}

FILE *
safe_freopen(const char *path, const char *mode, FILE *stream)
{
    FILE *ret = freopen(path, mode, stream);
    if (ret == NULL) {
        perror("freopen");
        exit(1);
    }
    return ret;
}

/* 
 * This function starts with a buffer size of 2, and reads characters into the
 * buffer, properly avoiding buffer overflow. While there are more characters
 * left in the line (no EOF or newline), then keep increasing buffer size. We
 * only reallocate the buffer by doubling to make the reallocation amortized O(1).
 */
char *
read_line(FILE *fp)
{
    size_t allocd = BUF_SIZE_INIT;
    char *line = safe_calloc(allocd, sizeof(char));
    size_t len = 0;

    flockfile(fp); // lock for getc()
    int c = getc_unlocked(fp);
    while (c != EOF && c != '\n') {
        if (len + 1 >= allocd) { // +1 so we have room for NUL byte terminator
            allocd *= 2;
            line = safe_realloc(line, allocd * sizeof(*line));
        }
        line[len++] = (char)c;
        c = getc_unlocked(fp);
    }
    funlockfile(fp); // unlock for getc()

    // handle possible error
    if (ferror(fp)) {
        perror("getc_unlocked");
        exit(1);
    }

    // handle ending
    if (len > 0 && line[len - 1] == '\r') { // handle crlf delimiter
        line[len - 1] = '\0';
    }
    line[len] = '\0';
    return line;
}

/*
 * This function safely converts a string to a double.
 */
static double
safe_strtod(const char *const token)
{
    if (token == NULL) {
        const double default_val = 0.;
        WARNING("no token recieved, setting ele to %g\n", default_val);
        return default_val;
    }
    char *endptr;
    double ele = strtod(token, &endptr);
    if (token == endptr) {
        WARNING("%s: unknown token\n\t\"%s\"\n\tsetting ele to 0.0\n", __func__, token);
    }
    if (*endptr != '\0') {
        WARNING("%s: part of the string wasn't valid (likely\n"
            "trailing whitespace/commas)\n"
            "\t\"%s\" -> %g\n"
            "\tyou can ignore this if the value is right\n", __func__, token, ele);
    }
    return ele;
}

/*
 * This is a custom tokenizer to avoid using strtok (which uses global state).
 * It takes a pointer to the string it is tokenizing `resumer`, and a string of
 * the characters which are separators of tokens. It is built to run
 * incrementally, each time returning a pointer to the next token, and
 * incrementing the pointer to the string it is tokenizing.
 *
 * It uses strspn() to calculate the offset into the `resumer` pointer of a new
 * token (so it will ignore leading separators), and assigns a pointer to that
 * offset `tkn_begin`. Then, it calculates the location of the end of the token
 * by calling strcspn(). 
 *
 * Then, we need to check if this is the last token. If the beginning of the
 * next token is the NUL character, then our token is the last token. Note that
 * this handles both the case where there is no trailing separators, and when
 * there are trailing separators. If this token is the last token, we set the
 * string we are tokenizing to NULL, otherwise  point the resumer to the
 * character after the end of the token. We terminate the token with a NUL
 * character, and return the beginning of the token.
 *
 * For example:

    char buf[] = "   one    two,    three   ";
    char *string = buf;
    char *c1 = find_token(&string, " ,");
    char *c2 = find_token(&string, " ,");
    char *c3 = find_token(&string, " ,");
    
    assert(strcmp(c1, "one") == 0);
    assert(strcmp(c2, "two") == 0);
    assert(strcmp(c3, "three") == 0);
    assert(string == NULL);
    char res[] = "   one\0   two\0    three\0   ";
    assert(memcmp(res, buf, sizeof(buf)));

 */
char *
find_token(char **resumer, const char *const sep)
{
    if ((*resumer) == NULL) {
        return NULL;
    }

    char *tkn_begin = &((*resumer)[strspn(*resumer, sep)]);
    char *tkn_end = &tkn_begin[strcspn(tkn_begin, sep)];
    char *next_tkn_begin = &tkn_end[strspn(tkn_end, sep)];

    if (next_tkn_begin[0] == '\0') {
        *resumer = NULL;
    } else {
        *resumer = &tkn_end[1];
    }
    // this is redundant if we're on the last token and there are no trailing
    // separators, but that's okay
    tkn_end[0] = '\0';

    return tkn_begin;
}

bool
equals(const double a, const double b)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
    // this is okay because we do only care when its 0
    if (b == 0) {
#pragma GCC diagnostic pop
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
mat_equal(const struct matrix m1, const struct matrix m2)
{
    if (m1.len1 != m2.len1 || m1.len2 != m2.len2) {
        WARNING("incompatible matrices\n\tmat_equal %zdx%zd vs %zdx%zd\n", m1.len1, m1.len2, m2.len1, m2.len2);
        return false;
    }
    bool are_equal = true;
    for (size_t i = 0; i < m1.len1; i++) {
        for (size_t j = 0; j < m1.len2; j++) {
            if (!equals(mat_get(m1, i, j), mat_get(m2, i, j))) {
                are_equal = false;
            }
        }
    }
    return are_equal;
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
    double max = -inf;
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
    double *data = safe_calloc(v.length, sizeof(double));
    for (size_t i = 0; i < v.length; i++) {
        data[i] = vec_get(v, i);
    }
    return vec_from_data(data, v.length, true);
}

/*
 * Read the `vec_sum` documentation for explanation of the kahan sum.
 * Notice that `sqsum / (n-1)` is the standard deviation. However, we can
 * approximate the error term with: ((\sum x_i)^2 / n) / (n-1). This is
 * formula 1.7 in 
 *
 * Algorithms for Computing the Sample Variance: Analysis and Recommendations
 * by Tony F. Chan, et al.
 * https://www.jstor.org/stable/2683386
 */
double
vec_std(const struct vec v) 
{
    if (v.length < 2) {
        return 0;
    }
    double mean = vec_mean(v);
    double sum, sqsum;

    KAHAN_INIT(sum);
    KAHAN_INIT(sqsum);
    for (size_t i = 0; i < v.length; i++) {
        double diff = vec_get(v, i) - mean;
        KAHAN_INCR(sum, diff);
        KAHAN_INCR(sqsum, diff * diff);
    }
    double variance = (sqsum - (sum * sum) / v.length) / (v.length - 1);
    return sqrt(variance);
}

void 
vec_multiply(struct vec v, const struct vec u)
{
    if (v.length != u.length) {
        WARNING("%s: incorrect dims\n\t%zu > %zu\n", __func__, v.length, u.length);
        exit(1);
    }
    for (size_t i = 0; i < v.length; i++) {
        vec_set(v, i, vec_get(v, i) * vec_get(u, i));
    }
}

double
vec_dot(const struct vec v, const struct vec u)
{
    if (v.length != u.length) {
        WARNING("%s: incorrect dims\n\t%zu > %zu\n", __func__, v.length, u.length);
        exit(1);
    }

    double sum = 0;
    for (size_t i = 0; i < v.length; i++) {
        sum = fma(vec_get(v, i), vec_get(u, i), sum);
    }
    return sum;
}

void 
vec_divide(struct vec v, const struct vec u)
{
    if (v.length != u.length) {
        WARNING("%s: incorrect dims\n\t%zu > %zu\n", __func__, v.length, u.length);
        exit(1);
    }
    for (size_t i = 0; i < v.length; i++) {
        vec_set(v, i, vec_get(v, i) / vec_get(u, i));
    }
}

void 
vec_add(struct vec v, const struct vec u)
{
    if (v.length != u.length) {
        WARNING("%s: incorrect dims\n\t%zu > %zu\n", __func__, v.length, u.length);
        exit(1);
    }
    for (size_t i = 0; i < v.length; i++) {
        vec_set(v, i, vec_get(v, i) + vec_get(u, i));
    }
}

void
vec_sub(struct vec v, const struct vec u)
{
    if (v.length != u.length) {
        WARNING("%s: incorrect dims\n\t%zu > %zu\n", __func__, v.length, u.length);
        exit(1);
    }
    for (size_t i = 0; i < v.length; i++) {
        vec_set(v, i, vec_get(v, i) - vec_get(u, i));
    }
}

void
vec_sqrt(struct vec v)
{
    for (size_t i = 0; i < v.length; i++) {
        vec_set(v, i, sqrt(vec_get(v,i)));
    }
}

void
vec_exp(struct vec v)
{
    for (size_t i = 0; i < v.length; i++) {
        vec_set(v, i, exp(vec_get(v,i)));
    }
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
    double max = -inf;
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
    double min = inf;
    for (size_t i = 0; i < v.length; i++) {
        double ith = vec_get(v, i);
        if (ith < min) {
            min = ith;
        }
    }
    return min;
}

/*
 * Kahan summation. If you think about it in terms of overlap of the significant
 * figures of the doubles, our operations look something like this:

   |========hi=========||=========low==========|
              |========a=========|     = v[i] + low
              |==a_hi==||==a_lo==|
   |====(hi + a)=======|

   Notice that:

     hi + a
   = hi + (a_hi + a_lo)
   = hi + a_hi            // since a_lo bits are all lost
 */
double
vec_sum(const struct vec v)
{
    double hi = 0;
    double low = 0;
    for (size_t i = 0; i < v.length; i++) {
        double a = vec_get(v, i) + low;
        double hi_plus_a = hi + a;

        // double a_hi = hi_plus_a - hi;
        // double a_lo = a - a_hi;

        low = a - (hi_plus_a - hi); // a_lo = a - a_hi; which we assign to low
        hi  = hi_plus_a;
    }
    return hi;
}

void 
vec_to_row(struct matrix m, const struct vec v, const size_t row)
{
    if (m.len2 != v.length) {
        WARNING("%s: incorrect dims\n\t%zu > %zu\n", __func__, m.len2, v.length);
        exit(1);
    }
    for (size_t i = 0; i < m.len2; i++) {
        mat_set(m, row, i, vec_get(v, i));
    }
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
            fprintf(fp, ",\n");
    }
    fprintf(fp, "]\n");
}

void
vec_printf(const struct vec v)
{
    vec_fprintf(stdout, v);
}

void
vec_scale(struct vec v, const double a)
{
    for (size_t i = 0; i < v.length; i++) {
        vec_set(v, i, vec_get(v, i) * a);
    }
}

void
vec_invscale(struct vec v, const double a)
{
    for (size_t i = 0; i < v.length; i++) {
        vec_set(v, i, vec_get(v, i) / a);
    }
}

/*
 * This function returns a vector of length n, of numbers 0 through n-1.
 */
static struct vec
vec_arange(size_t n)
{
    if (n <= 0) {
        WARNING("%s: %zu <= 0\n", __func__, n);
        return vec_zeros(1);
    }

    struct vec v = vec_zeros(n);
    for (size_t i = 0; i < n; i++) {
        vec_set(v, i, (double)i);
    }
    return v;
}

struct vec 
vec_linspace(double start, double end, double num_steps)
{
    if (start > end) {
        WARNING("%s: start > end\n\t%g > %g\n", __func__, start, end);
    }
    num_steps = floor(num_steps);
    struct vec v = vec_arange((size_t)num_steps);

    vec_scale(v, end - start);
    vec_invscale(v, num_steps - 1);
    vec_add_const(v, start);

    vec_set(v, 0, start); // in case the first term didn't round correctly
    vec_set(v, (size_t)num_steps - 1, end); // in case the last term didn't round correctly
    return v;
}

struct vec
vec_fscanf(FILE *file)
{
    size_t len_of_vec = 0;
    size_t allocd = BUF_SIZE_INIT;
    double *data = safe_calloc(allocd, sizeof(double));

    while (!feof(file)) {
        char *line = read_line(file);

        // empty line
        // line starts with '#'
        if (*line == '\0' || *line == '#') { 
            free(line);
            continue;
        }

        double ele = safe_strtod(line);
        if (len_of_vec == allocd) {
            allocd *= 2;
            data = safe_realloc(data, allocd * sizeof(*data));
        }
        data[len_of_vec++] = ele;
        free(line);
    }
    return vec_from_data(data, len_of_vec, true);
}

struct vec
vec_from_file(const char *path)
{
    FILE *file = safe_fopen(path, "r");
    struct vec v = vec_fscanf(file);
    fclose(file);
    return v;
}

/*
 * This function takes a file pointer and reads the first line to figure out how
 * many values in each row. Then it assumes that number will be the same for
 * the rest, and reads the rest of the lines as rows of a matrix.
 */
struct matrix
mat_fscanf(FILE *file)
{
    size_t num_cols = 0;
    size_t allocd = BUF_SIZE_INIT;
    double *data = safe_calloc(allocd, sizeof(double));
    char *sep = " \t,";

    // handle header and first line, get # of columns
    char *line = read_line(file);
    // empty line || line starts with '#'
    while (*line == '\0' || *line == '#') { 
        free(line);
        line = read_line(file);
    }
    char *resumer = line;
    char *token;
    while ((token = find_token(&resumer, sep))) {
        if (allocd <= num_cols) {
            allocd *= 2;
            data = safe_realloc(data, allocd * sizeof(*data));
        }
        double ele = safe_strtod(token);
        data[num_cols++] = ele;
    }
    free(line);

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
            data = safe_realloc(data, allocd * sizeof(*data));
        }
        resumer = line;
        for (size_t i = 0; i < num_cols; i++) {
            token = find_token(&resumer, sep);
            data[col_zero + i] = safe_strtod(token);
        }
        col_zero += num_cols;
        num_rows++;
        free(line);
    }
    return mat_from_data(data, num_rows, num_cols, num_cols, true);
}

struct matrix
mat_from_file(const char *path)
{
    FILE *file = safe_fopen(path, "r");
    struct matrix m = mat_fscanf(file);
    fclose(file);
    return m;
}

struct vec
vec_zeros(size_t len)
{
    double *data = safe_calloc(len, sizeof(double));
    return vec_from_data(data, len, true);
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
    struct matrix *data = safe_calloc(old.length, sizeof(struct matrix));
    for (size_t i = 0; i < old.length; i++)
    {
        data[i] = mat_copy(matarr_get(old, i));
    }
    return matarr_from_data(data, old.length, true);
}

void
vec_set_all(struct vec v, const double a)
{
    for (size_t i = 0; i < v.length; i++) {
        vec_set(v, i, a);
    }
}

struct vec
vec_from_col(const struct matrix m, const size_t col)
{
    struct vec v = vec_from_data(m.data + col, m.len1, false);
    v.stride = m.physlen;
    return v;
}

struct vec
vec_from_row(const struct matrix m, const size_t row)
{
    return vec_from_data(m.data + m.physlen * row, m.len2, false);
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
matarr_stats_printf(struct matarray arr)
{
    for (size_t i = 0; i < arr.length; i++) {
        mat_stats_printf(matarr_get(arr, i));
    }
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
mat_fprintf(FILE * restrict file, const struct matrix m)
{
    fprintf(file, "[");
    for (size_t i = 0; i < m.len1; i++) {
        if (i != 0) fprintf(file, " ");
        fprintf(file, "[");
        for (size_t j = 0; j < m.len2; j++) {
            fprintf(file, "%6e, ", mat_get(m, i, j));
        }
        fprintf(file, "]");
        if (i != m.len1 - 1) fprintf(file, "\n");
    }
    fprintf(file, "]");
    fprintf(file, "\n");
}

void
mat_stats_printf(struct matrix m)
{
    printf("len1    : %zu\n", m.len1);
    printf("len2    : %zu\n", m.len2);
    printf("physlen : %zu\n", m.physlen);
    printf("data    : %p\n", (void *)m.data);
    printf("is_owner: %s\n", m.is_owner ? "true" : "false");
}

void
mat_printf(const struct matrix m)
{
    mat_fprintf(stdout, m);
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


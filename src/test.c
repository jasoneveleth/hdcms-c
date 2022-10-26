#include "fixwindows.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include "test.h"
#include "../data/data.c"
#include "util/peak.h"
#include "util/array.h"
#include "util/bin.h"

typedef bool (*testfunc)(void);
// the division and stuff I did before causes the float 899.9 to be 0x408c1f3333333334, rather than 0x408c1f3333333333, so all my tests are wrong if I do 899.9
#define end (899.90000000000009094947)

// path from current working directory of shell running the test executable
// CANNOT INCLUDE '%' CHARACTER BECAUSE USED IN snprintf()
#ifndef DATA_DIR
#define DATA_DIR "../data/"
#endif

/* This makes compiler verify that `format` is a string literal.
 * The numbers are the index of the arguemnts starting from 1.
 * The (string literal) format is arg number 3, and the compiler should verify
 * the format literal against all the variadic args (which start at arg 4).
 */
#ifdef __GNUC__
#if __has_attribute(__format__)
__attribute__((__format__ (__printf__, 3, 4)))
#endif
#endif
static size_t 
safe_snprintf(char *restrict buf, size_t size, const char *restrict format, ...)
{
    int result;
    va_list args;

    va_start(args, format);
    result = vsnprintf(buf, size, format, args);
    if (result < 0) {
        fprintf(stderr, "%s:%d: snprintf error\n", __FILE__, __LINE__);
        perror("snprintf");
        abort();
    }
    va_end(args);

    return (size_t)result;
}

// replaces stderr with null device, returns the new file descriptor for stderr
static int 
suppress_stderr(void) {
    int stderr_fileno = fileno(stderr);

    int ret = dup(stderr_fileno);
    int nullfd = open(NULL_DEVICE, O_WRONLY);
    if (nullfd == -1) {
        WARNING("%s", "");
        perror("open " NULL_DEVICE);
    }
    dup2(nullfd, stderr_fileno);
    close(nullfd);

    return ret;
}

// set stderr to `fd`
static void
resume_stderr(int fd) {
    int stderr_fileno = fileno(stderr);
    dup2(fd, stderr_fileno);
    close(fd);
}

// ==================+ END OF HELPER FUNCTIONS +==================

static bool
test_cos_sim_L2(void)
{
    printf(__func__);
    double udata[] = {91, 0.9, 0.1, 0.08};
    double vdata[] = {90, 1, 0.09, 0.11};
    struct vec u = vec_from_data(udata, 4, false);
    struct vec v = vec_from_data(vdata, 4, false);
    double sim = cos_sim_L2(u, v, 1e-4);
    vec_free(u);
    vec_free(v);
    return equals(sim, 7.9235e-13);
}

/*
 * A = analyte_peak_stats(12, 1)
 * B = analyte_peak_stats(11, 1)
 * (first)
 *      i = 3, 
 *      u = 1.0e+02 *
            3.882976388000000   0.002553558503045   0.000017707525801   0.000093945978461,
 *      B_copy(j, :) = 1.0e+02 *
                    3.882982086000000   0.002463295137625   0.000007833634533   0.000323634722072
 *
 *      sim: 0.5993
 */
static bool
test_cos_sim_3rd_highest_of_12_11_30V(void)
{
    printf(__func__);
    double udata[] = {3.882976388000000, 0.002553558503045, 0.000017707525801, 0.000093945978461};
    double vdata[] = {3.882982086000000, 0.002463295137625, 0.000007833634533, 0.000323634722072};
    struct vec u = vec_from_data(udata, 4, false);
    struct vec v = vec_from_data(vdata, 4, false);
    vec_scale(u, 1e2);
    vec_scale(v, 1e2);
    double sim1 = cos_sim_L2(u, v, 1e-4);
    vec_free(u);
    vec_free(v);
    return equals(sim1, 0.599328450629485);
}

/*
 * A = analyte_peak_stats(12, 1)
 * B = analyte_peak_stats(11, 1)
 *  (second)
 *      i = 11,
 *      u = 1.0e02 *
 *              3.893009976000000   0.000527014227888   0.000015185034738   0.000069406478369
 *      B_copy(j, :) = 1.0e02 *
 *              3.893016630000000   0.000479664131948   0.000006732993391   0.000109354061818
 *
 *      sim: 0.7344
 */
static bool
test_cos_sim_11th_highest_of_12_11_30V(void) 
{
    printf(__func__);
    double udata2[] = {3.893009976000000, 0.000527014227888, 0.000015185034738, 0.000069406478369};
    double vdata2[] = {3.893016630000000, 0.000479664131948, 0.000006732993391, 0.000109354061818};
    struct vec u = vec_from_data(udata2, 4, false);
    struct vec v = vec_from_data(vdata2, 4, false);
    vec_scale(u, 1e2);
    vec_scale(v, 1e2);
    double sim2 = cos_sim_L2(u, v, 1e-4);
    vec_free(u);
    vec_free(v);
    return equals(sim2, 0.734449667053694);
}

static bool
test_argmax(void)
{
    printf(__func__);
    double m2data[] = {83, 0.23,
        92, 0.47,
        79, 0.61,
        110, 0.5};
    struct matrix m = mat_from_data(m2data, 4, 2, 2, false);
    struct vec ys = vec_from_col(m, 1);
    size_t argmax = vec_argmax(ys);
    // double x = mat_get(m, argmax, 0);
    double y = mat_get(m, argmax, 1);
    mat_free(m);
    return equals(y, 0.61);
}

static bool
test_peak_sort_simple(void)
{
    printf(__func__);
    size_t n = 20;
    double m1data[] = {72, 0.68,
        97, 0.43,
        89, 0.27,
        100, 0.1};
    double m2data[] = {83, 0.23,
        92, 0.47,
        79, 0.61,
        110, 0.5};
    struct matrix m1 = mat_from_data(m1data, 4, 2, 2, false);
    struct matrix m2 = mat_from_data(m2data, 4, 2, 2, false);

    struct matrix adata[] = {m1, m2};
    struct matarray input = matarr_from_data(adata, 2, false);
    struct matarray output = peak_sort(input, n);

    // answer
    double peak0data[] = {72, 0.68, 79, 0.61};
    double peak1data[] = {100, 0.1, 110, 0.5};
    double peak2data[] = {89, 0.27, 92, 0.47};
    double peak3data[] = {97, 0.43, 83, 0.23};
    struct matrix peak0 = mat_from_data(peak0data, 2, 2, 2, false);
    struct matrix peak1 = mat_from_data(peak1data, 2, 2, 2, false);
    struct matrix peak2 = mat_from_data(peak2data, 2, 2, 2, false);
    struct matrix peak3 = mat_from_data(peak3data, 2, 2, 2, false);

    // each is equal
    bool b0 = mat_equal(peak0, matarr_get(output, 0));
    bool b1 = mat_equal(peak1, matarr_get(output, 1));
    bool b2 = mat_equal(peak2, matarr_get(output, 2));
    bool b3 = mat_equal(peak3, matarr_get(output, 3));

    mat_free(peak0);
    mat_free(peak1);
    mat_free(peak2);
    mat_free(peak3);
    matarr_free(input);
    matarr_free(output);
    return b0 && b1 && b2 && b3;
}

static bool
test_peak_sort_zeros(void)
{
    printf(__func__);
    size_t n = 20;
    double m1data[] = {72, 0.68,
        97, 0.43,
        89, 0.27,
        100, 0.1};
    double m2data[] = {83, 0.23,
        92, 0.47,
        79, 0.61,
        110, 0.5};
    double m3data[] = {0, 0,
        0, 0,
        0, 0,
        0, 0};
    double m4data[] = {0, 0,
        0, 0,
        0, 0,
        0, 0};
    double m5data[] = {0, 0,
        0, 0,
        0, 0,
        0, 0};
    struct matrix m1 = mat_from_data(m1data, 4, 2, 2, false);
    struct matrix m2 = mat_from_data(m2data, 4, 2, 2, false);
    struct matrix m3 = mat_from_data(m3data, 4, 2, 2, false);
    struct matrix m4 = mat_from_data(m4data, 4, 2, 2, false);
    struct matrix m5 = mat_from_data(m5data, 4, 2, 2, false);

    struct matrix adata[] = {m1, m2, m3, m4, m5};
    struct matarray input = matarr_from_data(adata, 5, false);
    struct matarray output = peak_sort(input, n);

    // answer
    double peak0data[] = {72, 0.68, 79, 0.61, 0, 0, 0, 0, 0, 0};
    double peak1data[] = {100, 0.1, 110, 0.5, 0, 0, 0, 0, 0, 0};
    double peak2data[] = {89, 0.27, 92, 0.47, 0, 0, 0, 0, 0, 0};
    double peak3data[] = {97, 0.43, 83, 0.23, 0, 0, 0, 0, 0, 0};
    struct matrix peak0 = mat_from_data(peak0data, 5, 2, 2, false);
    struct matrix peak1 = mat_from_data(peak1data, 5, 2, 2, false);
    struct matrix peak2 = mat_from_data(peak2data, 5, 2, 2, false);
    struct matrix peak3 = mat_from_data(peak3data, 5, 2, 2, false);

    // each is equal
    struct matrix solutiondata[] = {peak0, peak1, peak2, peak3};
    struct matarray solution = matarr_from_data(solutiondata, 4, false);
    bool ret = matarr_equal(solution, output);

    matarr_free(input);
    matarr_free(output);
    matarr_free(solution);
    return ret;
}

static bool 
test_peak_sort_real2(void)
{
    printf(__func__);
    size_t n = 2;
    // from data.c
    double *m1data = m13_1_2_data;
    double *m2data = m13_2_2_data;
    double *m3data = m13_3_2_data;
    double *m4data = m13_4_2_data;
    double *m5data = m13_5_2_data;
    struct matrix m1 = mat_from_data(m1data, 15, 2, 2, false);
    struct matrix m2 = mat_from_data(m2data, 22, 2, 2, false);
    struct matrix m3 = mat_from_data(m3data, 13, 2, 2, false);
    struct matrix m4 = mat_from_data(m4data, 18, 2, 2, false);
    struct matrix m5 = mat_from_data(m5data, 18, 2, 2, false);

    struct matrix adata[] = {m1, m2, m3, m4, m5};
    struct matarray input = matarr_from_data(adata, 5, false);
    struct matarray output = peak_sort(input, n);

    // answer
    struct matrix peak0 = mat_from_data(p_m13_2__0_data, 5, 2, 2, false);
    struct matrix peak1 = mat_from_data(p_m13_2__1_data, 5, 2, 2, false);

    struct matrix solutiondata[] = {peak0, peak1};
    struct matarray solution = matarr_from_data(solutiondata, 2, false);
    bool ret = matarr_equal(solution, output);

    matarr_free(input);
    matarr_free(output);
    matarr_free(solution);
    return ret;
}

static bool
test_peak_sort_real15(void)
{
    printf(__func__);
    size_t n = 15;
    // from data.c
    double *m1data = m13_1_2_data;
    double *m2data = m13_2_2_data;
    double *m3data = m13_3_2_data;
    double *m4data = m13_4_2_data;
    double *m5data = m13_5_2_data;

    struct matrix m1 = mat_from_data(m1data, 15, 2, 2, false);
    struct matrix m2 = mat_from_data(m2data, 22, 2, 2, false);
    struct matrix m3 = mat_from_data(m3data, 13, 2, 2, false);
    struct matrix m4 = mat_from_data(m4data, 18, 2, 2, false);
    struct matrix m5 = mat_from_data(m5data, 18, 2, 2, false);

    struct matrix adata[] = {m1, m2, m3, m4, m5};
    struct matarray input = matarr_from_data(adata, 5, false);
    struct matarray output = peak_sort(input, n);

    // answer
    struct matrix peak0 = mat_from_data(p_m13_2__0_data, 5, 2, 2, false);
    struct matrix peak1 = mat_from_data(p_m13_2__1_data, 5, 2, 2, false);
    struct matrix peak2 = mat_from_data(p_m13_2__2_data, 5, 2, 2, false);
    struct matrix peak3 = mat_from_data(p_m13_2__3_data, 5, 2, 2, false);
    struct matrix peak4 = mat_from_data(p_m13_2__4_data, 5, 2, 2, false);
    struct matrix peak5 = mat_from_data(p_m13_2__5_data, 5, 2, 2, false);
    struct matrix peak6 = mat_from_data(p_m13_2__6_data, 5, 2, 2, false);
    struct matrix peak7 = mat_from_data(p_m13_2__7_data, 5, 2, 2, false);
    struct matrix peak8 = mat_from_data(p_m13_2__8_data, 5, 2, 2, false);
    struct matrix peak9 = mat_from_data(p_m13_2__9_data, 5, 2, 2, false);
    struct matrix peak10 = mat_from_data(p_m13_2__10_data, 5, 2, 2, false);
    struct matrix peak11 = mat_from_data(p_m13_2__11_data, 5, 2, 2, false);
    struct matrix peak12 = mat_from_data(p_m13_2__12_data, 5, 2, 2, false);
    struct matrix solutiondata[] = {peak0, peak1, peak2, peak3, peak4, peak5, peak6, peak7, peak8, peak9, peak10, peak11, peak12};
    struct matarray solution = matarr_from_data(solutiondata, 13, false);
    bool ret = matarr_equal(solution, output);

    matarr_free(input);
    matarr_free(output);
    matarr_free(solution);
    return ret;
}

static bool
test_peak_stat_real(void)
{
    printf(__func__);
    size_t n = 15;
    // from data.c
    double *m1data = m13_1_2_data;
    double *m2data = m13_2_2_data;
    double *m3data = m13_3_2_data;
    double *m4data = m13_4_2_data;
    double *m5data = m13_5_2_data;

    struct matrix m1 = mat_from_data(m1data, 15, 2, 2, false);
    struct matrix m2 = mat_from_data(m2data, 22, 2, 2, false);
    struct matrix m3 = mat_from_data(m3data, 13, 2, 2, false);
    struct matrix m4 = mat_from_data(m4data, 18, 2, 2, false);
    struct matrix m5 = mat_from_data(m5data, 18, 2, 2, false);

    struct matrix adata[] = {m1, m2, m3, m4, m5};
    struct matarray input = matarr_from_data(adata, 5, false);
    struct matrix output = peak_stat(input, n);

    struct matrix sol = mat_from_data(stat_m13_i_2, 13, 4, 4, false);
    bool ret = mat_equal(output, sol);
    mat_free(output);
    mat_free(sol);
    matarr_free(input);
    return ret;
}

static bool
test_vec_read_simple(void)
{
    printf(__func__);
    struct vec output = vec_from_file(DATA_DIR "vec_read_simple.txt");
    double data[] = {728, 283910, 812931, 72891};
    struct vec sol = vec_from_data(data, 4, false);
    bool ret = vec_equal(output, sol);
    vec_free(output);
    vec_free(sol);
    return ret;
}

static bool
test_cos_sim_extra1(void)
{
    printf(__func__);
    double adata[] = {90, 0.5, 0.001, 0.02};
    double bdata[] = {100.001, 0.908, 0.004, 0.01};
    struct vec u = vec_from_data(adata, 4, false);
    struct vec v = vec_from_data(bdata, 4, false);
    return equals(cos_sim_L2(u, v, 1e-4), 0);
}

static bool
test_cos_sim_extra2(void)
{
    printf(__func__);
    double adata[] = {100, 0.9, 0.003, 0.008};
    double bdata[] = {100.001, 0.908, 0.004, 0.01};
    struct vec u = vec_from_data(adata, 4, false);
    struct vec v = vec_from_data(bdata, 4, false);
    return equals(cos_sim_L2(u, v, 1e-4), 7.856829001024321e-01);
}

static bool
test_peak_sim_measure_simple(void)
{
    printf(__func__);
    double adata[] = {90, 0.5, 0.001, 0.02,
        100, 0.9, 0.003, 0.008};
    double bdata[] = {90.001, 0.497, 0.002, 0.015,
        100.001, 0.908, 0.004, 0.01};
    struct matrix A = mat_from_data(adata, 2, 4, 4, false);
    struct matrix B = mat_from_data(bdata, 2, 4, 4, false);
    double sim = peak_sim_measure_L2(A, B, 1e-4, 2);
    bool ret = equals(sim, 7.906904116230999e-01);
    mat_free(A);
    mat_free(B);
    return ret;
}

static bool
test_peak_sim_measure_complex(void)
{
    printf(__func__);
    double adata[] = {78.946, 0.961, 0.5, 0.02,  
        79.854,  0.286, 0.5, 0.02,  
        101.909, 0.259, 0.5, 0.02, 
        90.903,  0.649, 0.5, 0.02,  
        85.208,  0.556, 0.5, 0.02,  
        96.322,  0.615, 0.5, 0.02,  
        61.448,  0.846, 0.5, 0.02,  
        82.000,  0.959, 0.5, 0.02,  
        82.683,  0.169, 0.5, 0.02,  
        111.682, 0.261, 0.5, 0.02};
    double bdata[] = {77.293, 0.168, 0.5, 0.02,
        78.210,  0.852, 0.5, 0.02,
        100.486, 0.629, 0.5, 0.02,
        89.976,  0.380, 0.5, 0.02,
        84.043,  0.490, 0.5, 0.02,
        95.330,  0.855, 0.5, 0.02,
        60.852,  0.589, 0.5, 0.02,
        81.939,  0.715, 0.5, 0.02,
        81.005,  0.837, 0.5, 0.02,
        110.329, 0.322, 0.5, 0.02};
    struct matrix A = mat_from_data(adata, 10, 4, 4, false);
    struct matrix B = mat_from_data(bdata, 10, 4, 4, false);
    double sim = peak_sim_measure_L2(A, B, 1e-4, 10);
    bool ret = equals(sim, 0.00045443404003044953);
    mat_free(A);
    mat_free(B);
    return ret;
}

static bool
test_peak_sim_measure_complex2(void)
{
    printf(__func__);
    double adata[] = {
        85.208,  0.556, 0.5, 0.02,  
        61.448,  0.846, 0.5, 0.02,  
        111.682, 0.261, 0.5, 0.02,
        78.946,  0.961, 0.5, 0.02,  
        96.322,  0.615, 0.5, 0.02,  
        101.909, 0.259, 0.5, 0.02, 
        82.000,  0.959, 0.5, 0.02,  
        82.683,  0.169, 0.5, 0.02,  
        90.903,  0.649, 0.5, 0.02,  
        79.854,  0.286, 0.5, 0.02,  
    };
    double bdata[] = {
        110.329, 0.322, 0.5, 0.02,
        81.005,  0.837, 0.5, 0.02,
        84.043,  0.490, 0.5, 0.02,
        81.939,  0.715, 0.5, 0.02,
        60.852,  0.589, 0.5, 0.02,
        95.330,  0.855, 0.5, 0.02,
        100.486, 0.629, 0.5, 0.02,
        89.976,  0.380, 0.5, 0.02,
        78.210,  0.852, 0.5, 0.02,
        77.293,  0.168, 0.5, 0.02,
    };
    struct matrix A = mat_from_data(adata, 10, 4, 4, false);
    struct matrix B = mat_from_data(bdata, 10, 4, 4, false);
    double sim = peak_sim_measure_L2(A, B, 1e-4, 10);
    bool ret = equals(sim, 0.00045443404003044953);
    mat_free(A);
    mat_free(B);
    return ret;
}

static bool
test_peak_sim_measure_n_less_than_both(void)
{
    printf(__func__);
    double adata[] = {
        85.208,  0.556, 0.5, 0.02,  
        61.448,  0.846, 0.5, 0.02,  
        111.682, 0.261, 0.5, 0.02,
        78.946,  0.961, 0.5, 0.02,  
        96.322,  0.615, 0.5, 0.02,  
        101.909, 0.259, 0.5, 0.02, 
        82.000,  0.959, 0.5, 0.02,  
        82.683,  0.169, 0.5, 0.02,  
        90.903,  0.649, 0.5, 0.02,  
        79.854,  0.286, 0.5, 0.02,  
    };
    double bdata[] = {
        110.329, 0.322, 0.5, 0.02,
        81.005,  0.837, 0.5, 0.02,
        84.043,  0.490, 0.5, 0.02,
        81.939,  0.715, 0.5, 0.02,
        60.852,  0.589, 0.5, 0.02,
        95.330,  0.855, 0.5, 0.02,
        100.486, 0.629, 0.5, 0.02,
        89.976,  0.380, 0.5, 0.02,
        78.210,  0.852, 0.5, 0.02,
        77.293,  0.168, 0.5, 0.02,
    };
    struct matrix A = mat_from_data(adata, 10, 4, 4, false);
    struct matrix B = mat_from_data(bdata, 10, 4, 4, false);
    double sim = peak_sim_measure_L2(A, B, 1e-4, 4);
    bool ret = equals(sim, 0.00012678008971000085);
    mat_free(A);
    mat_free(B);
    return ret;
}

static bool
test_peak_sim_measure_real(void)
{
    printf(__func__);
    struct matrix A = mat_from_data(stat_m13_i_2, 13, 4, 4, false);
    struct matrix B = mat_from_data(stat_m12_i_3, 25, 4, 4, false);
    double sim = peak_sim_measure_L2(A, B, 1e-4, 20);
    bool ret = equals(sim, 0);
    mat_free(A);
    mat_free(B);
    return ret;
}

static bool
test_vec_read_real(void)
{
    printf(__func__);
    struct vec output = vec_from_file(DATA_DIR "vec_read_real.txt");
    double data[] = {
        9.192224975486361e-05,
        1.600568648962115e-04,
        8.915716460218152e-05,
        1.864599152657470e-04,
        1.665837996925374e-02,
        6.442454361497599e-03,
        2.005903786313059e-04,
        8.257818095599679e-03,
        1.517282306016281e-01,
        7.761167437959599e-04,
        4.627274746573855e-01,
        1.106639055873704e-04,
        7.421074649294409e-01};
    struct vec sol = vec_from_data(data, 13, false);
    bool ret = vec_equal(output, sol);
    vec_free(output);
    vec_free(sol);
    return ret;
}

static bool
test_edgecase_contains_0_peak_sort(void)
{
    printf(__func__);
    struct matrix m = mat_zeros(0, 0);
    struct matrix adata[] = {m};
    struct matarray arr = matarr_from_data(adata, 1, false);
    struct matarray output = peak_sort(arr, 5);
    bool ret = output.length == 0;
    mat_free(m);
    matarr_free(output);
    return ret;
}

static bool
test_edgecase_0_peak_sort(void)
{
    printf(__func__);
    struct matarray m = matarr_zeros(0);
    struct matarray output = peak_sort(m, 5);
    bool ret = output.length == 5;
    matarr_free(m);
    matarr_free(output);
    return ret;
}

static bool
test_edgecase_1_peak_sort(void)
{
    printf(__func__);
    double mdata[] = {83, 0.23,
        92, 0.47,
        79, 0.61,
        110, 0.5};
    struct matrix m = mat_from_data(mdata, 4, 2, 2, false);
    struct matarray arr = matarr_zeros(1);
    matarr_set(arr, 0, m);
    struct matarray output = peak_sort(arr, 5);
    // output should be 4 matrices each of which is 1 pt
    double sol1data[] = {79, 0.61};
    double sol2data[] = {110, 0.5};
    double sol3data[] = {92, 0.47};
    double sol4data[] = {83, 0.23};
    struct matrix sol1 = mat_from_data(sol1data, 1, 2, 2, false);
    struct matrix sol2 = mat_from_data(sol2data, 1, 2, 2, false);
    struct matrix sol3 = mat_from_data(sol3data, 1, 2, 2, false);
    struct matrix sol4 = mat_from_data(sol4data, 1, 2, 2, false);
    struct matarray solarr = matarr_zeros(4);
    matarr_set(solarr, 0, sol1);
    matarr_set(solarr, 1, sol2);
    matarr_set(solarr, 2, sol3);
    matarr_set(solarr, 3, sol4);
    bool ret = matarr_equal(solarr, output);
    matarr_free(solarr);
    matarr_free(arr);
    matarr_free(output);
    return ret;
}

static bool
test_edge_case_1_peak_stat(void)
{
    printf(__func__);
    double mdata[] = {83, 0.23,
        92, 0.47,
        79, 0.61,
        110, 0.5};
    struct matrix m = mat_from_data(mdata, 4, 2, 2, false);
    struct matarray arr = matarr_zeros(1);
    matarr_set(arr, 0, m);
    struct matrix output = peak_stat(arr, 5);
    double soldata[] = {
        79, 0.61, 0, 0,
        110, 0.5, 0, 0,
        92, 0.47, 0, 0,
        83, 0.23, 0, 0,
    };
    struct matrix sol = mat_from_data(soldata, 4, 4, 4, false);
    bool ret = mat_equal(sol, output);
    matarr_free(arr);
    mat_free(output);
    return ret;
}

static bool
test_edgecase_contains_0_peak_stat(void)
{
    printf(__func__);
    struct matrix m = mat_zeros(0, 0);
    struct matrix adata[] = {m};
    struct matarray arr = matarr_from_data(adata, 1, false);
    struct matrix output = peak_stat(arr, 5);
    bool ret = output.len1 == 0;
    mat_free(output);
    mat_free(m);
    return ret;
}

static bool
test_edgecase_0_peak_stat(void)
{
    printf(__func__);
    struct matarray m = matarr_zeros(0);
    int fd = suppress_stderr();
    struct matrix output = peak_stat(m, 5);
    resume_stderr(fd);
    bool ret = output.len1 == 0 && output.len2 == 0;
    matarr_free(m);
    return ret;
}

static bool
test_edge_case_0_peak_sim(void)
{
    printf(__func__);
    struct matrix m = {0, 0, 0, NULL, false};
    double ndata[] = {420, 69, 0, 0};
    struct matrix n = {1, 4, 4, ndata, false};
    int fd = suppress_stderr();
    double d = peak_sim_measure_L2(m, n, 1e-4, 5);
    resume_stderr(fd);
    return equals(d, 0);
}

// we want it to exit(1) now -- don't test
// static bool
// test_edge_case_0_cos_sim()
// {
//     printf(__func__);
//     struct vec v = {0, 0, NULL, false};
//     struct vec u = {0, 0, NULL, false};
//     int fd = suppress_stderr();
//     double d = cos_sim_L2(u, v);
//     resume_stderr(fd);
//     return equals(d, 0);
// }

static bool
test_read_line_w_newline(void) 
{
    printf(__func__);
    FILE *file = safe_fopen(DATA_DIR "vec_read_simple.txt", "r");
    char *line = read_line(file);
    fclose(file);
    char *correct = "728";
    bool ret = strcmp(correct, line) == 0;
    free(line);
    return ret;
}

static bool
test_read_line_wo_newline(void) 
{
    printf(__func__);
    FILE *file = safe_fopen(DATA_DIR "oneline.txt", "r");
    char *line = read_line(file);
    fclose(file);
    char *correct = "23.324872 378.3247832";
    bool ret = strcmp(correct, line) == 0;
    free(line);
    return ret;
}

static bool
test_read_vec_no_eof_eol(void)
{
    printf(__func__);
    struct vec v = vec_from_file(DATA_DIR "no_newline.txt");
    double vdata[] = {
        34832749324832,
        32874789327894,
    };
    struct vec w = vec_from_data(vdata, 2, false);
    bool ret = vec_equal(v, w);
    vec_free(v);
    return ret;
}

static bool
test_mat_read_no_extra_newln_ints(void)
{
    printf(__func__);
    struct matrix m = mat_from_file(DATA_DIR "mat_no_extra_newln_ints.txt");
    double ndata[] = {
        7, 62, 40, 1,
        75, 92, 31, 12,
        56, 7, 43, 97,
        38, 11, 20, 13,
    };
    struct matrix n = mat_from_data(ndata, 4, 4, 4, false);
    bool ret = mat_equal(m, n);
    mat_free(m);
    return ret;
}

static bool
test_mat_read_blank_lines_ints(void)
{
    printf(__func__);
    struct matrix m = mat_from_file(DATA_DIR "mat_blank_line_ints.txt");
    double ndata[] = {
        7, 62, 40, 1,
        75, 92, 31, 12,
        56, 7, 43, 97,
        38, 11, 20, 13,
    };
    struct matrix n = mat_from_data(ndata, 4, 4, 4, false);
    bool ret = mat_equal(m, n);
    mat_free(m);
    return ret;
}

static bool
test_mat_read_ints(void)
{
    printf(__func__);
    struct matrix m = mat_from_file(DATA_DIR "mat_ints.txt");
    double ndata[] = {
        7, 62, 40, 1,
        75, 92, 31, 12,
        56, 7, 43, 97,
        38, 11, 20, 13,
    };
    struct matrix n = mat_from_data(ndata, 4, 4, 4, false);
    bool ret = mat_equal(m, n);
    mat_free(m);
    return ret;
}

static bool
test_mat_read_trailing_white_space(void)
{
    printf(__func__);
    struct matrix m = mat_from_file(DATA_DIR "mat_trailing_white_space.txt");
    double ndata[] = {
        5, 4, 10,
        1, 3, 8
    };
    struct matrix n = mat_from_data(ndata, 2, 3, 3, false);
    bool ret = mat_equal(m, n);
    mat_free(m);
    return ret;
}

static bool
test_mat_read_leading_white_space(void)
{
    printf(__func__);
    struct matrix m = mat_from_file(DATA_DIR "mat_leading_white_space.txt");
    double ndata[] = {
        5, 4, 10,
        1, 3, 8
    };
    struct matrix n = mat_from_data(ndata, 2, 3, 3, false);
    bool ret = mat_equal(m, n);
    mat_free(m);
    return ret;
}

static bool
test_analytes_normal_1_1_1(void)
{
    printf(__func__);
    struct matrix m = mat_from_file(DATA_DIR "analytes_normal_1_1_1.txt");
    double ndata[] = {
        9.806176300000000e+01, 3.145928075948926e-03,
        1.460602040000000e+02, 2.329312500803309e-03,
        1.470934360000000e+02, 1.760176593771673e-03,
        1.611076270000000e+02, 5.989167583700663e-03,
        1.631236780000000e+02, 9.245590634716005e-03,
        1.721704990000000e+02, 1.623929745919967e-03,
        1.731082990000000e+02, 1.716185893224221e-03,
        1.750878940000000e+02, 3.626154545864540e-03,
        1.760955720000000e+02, 4.633017227898267e-03,
        1.764790310000000e+02, 5.517298044959624e-04,
        1.771031860000000e+02, 1.000000000000000e+00,
        1.776327970000000e+02, 4.197740027687843e-02,
        1.781062900000000e+02, 1.179060896008900e-01,
        1.785196560000000e+02, 2.986597386105169e-02,
        1.791092180000000e+02, 6.208226618371004e-03,
        1.793977400000000e+02, 1.823665996056958e-02,
        1.798315620000000e+02, 1.809208812622298e-02,
        1.802809700000000e+02, 1.501559198435195e-02,
        1.807024250000000e+02, 5.075867317690202e-03,
        1.811546280000000e+02, 8.837032542836262e-03,
        1.815752490000000e+02, 2.773157745488899e-03,
        1.820030670000000e+02, 2.072589167730331e-03,
        1.824862730000000e+02, 1.203950204472203e-03,
        1.825371380000000e+02, 3.550403031288723e-04,
        1.911190650000000e+02, 1.199347681415036e-02,
        1.921218590000000e+02, 2.178193683938017e-03,
        1.931342630000000e+02, 3.683777544768389e-03,
        1.960982110000000e+02, 9.228911009587162e-03,
        1.971020240000000e+02, 1.318855708889322e-03,
        3.532001830000000e+02, 4.001965101575201e-03,
    };
    struct matrix n = mat_from_data(ndata, 30, 2, 2, false);
    bool ret = mat_equal(m, n);
    mat_free(m);
    return ret;
}

static bool
test_similarity_analysis(void)
{
    printf(__func__);
    double soldata[] = {
        2.714820552749603e-11,     3.133250698158485e-03,     3.624474914739576e-08,
        1.274166485409238e-07,     6.966236433131534e-06,     8.158015509877866e-13,
        2.200872268233036e-02,                         0,     2.204101096985747e-04,
        8.963423883881168e-01,     4.810626370915526e-16,     5.518881843724404e-19,
        2.459234783200257e-04,     4.308393317187379e-06,     1.266741377548182e-03,
        6.570605538696780e-02,     1.212102981190234e-05,     1.963276486504726e-10,
        9.445490457064037e-03,     9.054240490062887e-01,     8.650218111500563e-01,
    };
    struct matrix sol = mat_from_data(soldata, 7, 3, 3, false);

    // these loops initialize analyte_peak_stats to the 14 compounds at each
    // energy level (30V, 60V, 90V), we store the 2D mat array as a flat matarray
    struct matarray analyte_peak_stats = matarr_zeros(14 * 3);
    for (size_t i = 0; i < 14; i++) {
        for (size_t j = 0; j < 3; j++) {
            struct matarray replicates = matarr_zeros(5);
            // this loop reads the 5 replicates into `replicates`
            for (size_t k = 0; k < 5; k++) {
                // calling with NULL returns the length of the string
                size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "analytes_normal_%zd_%zd_%zd.txt", i+1, k+1, j+1);
                char *buf = safe_calloc(bufsz + 1, sizeof(char));

                safe_snprintf(buf, bufsz + 1, DATA_DIR "analytes_normal_%zd_%zd_%zd.txt", i+1, k+1, j+1);
                struct matrix m = mat_from_file(buf);
                free(buf);

                matarr_set(replicates, k, m);
            }
            struct matrix p = peak_stat(replicates, 15);
            matarr_free(replicates);
            matarr_set(analyte_peak_stats, i * 3 + j, p);
        }
    }
    struct matrix similarity_measures = mat_zeros(7, 3);
    for (size_t i = 0; i < 7; i++) {
        for (size_t j = 0; j < 3; j++) {
            struct matrix A = matarr_get(analyte_peak_stats, (2*i) * 3 + j);
            struct matrix B = matarr_get(analyte_peak_stats, (2*i + 1) * 3 + j);
            mat_set(similarity_measures, i, j, peak_sim_measure_L2(A, B, 1e-4, 25));
        }
    }
    matarr_free(analyte_peak_stats);
    bool ret = mat_equal(similarity_measures, sol);
    mat_free(similarity_measures);
    return ret;
}

static bool
test_mat_read_cm_data(void)
{
    printf(__func__);
    struct matrix m = mat_from_data(cm1_1_4, 231, 2, 2, 0);
    struct matrix k = mat_from_file(DATA_DIR "CM1_1_4.txt");
    bool ret = mat_equal(k, m);
    mat_free(k);
    mat_free(m);
    return ret;
}

static bool
test_scaled_data(void)
{
    printf(__func__);
    struct matrix m = mat_from_file(DATA_DIR "CM1_1_4.txt");
    struct matrix sol = mat_from_data(cm1_1_4_scaled, 231, 2, 2, 0);
    scaled_data(m);
    bool ret = mat_equal(m, sol);
    mat_free(m);
    return ret;
}

static bool
test_spec_vec(void)
{
    printf(__func__);

    struct vec sol = vec_from_file(DATA_DIR "spec_vec_CM1_1_4.txt");

    struct matrix m = mat_from_file(DATA_DIR "CM1_1_4.txt");

    scaled_data(m);
    struct vec v = spec_vec(m, 0, end, 9000);
    bool ret = vec_equal(v, sol);
    vec_free(v);
    vec_free(sol);
    mat_free(m);
    return ret;
}

static bool
test_spec_vec_all(void)
{
    printf(__func__);
    bool ret = true;
    for (size_t i = 0; i < 10; i++) {
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "spec_vec_CM1_1_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "spec_vec_CM1_1_%ld.txt", i+1); // i+1 for 1-indexed
        struct vec sol = vec_from_file(filename);
        free(filename);

        bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_1_%ld.txt", i+1); // i+1 for 1-indexed
        filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_1_%ld.txt", i+1); // i+1 for 1-indexed
        struct matrix m = mat_from_file(filename);
        free(filename);

        scaled_data(m);
        struct vec v = spec_vec(m, 0, end, 9000);
        ret = ret && vec_equal(v, sol);
        vec_free(v);
        vec_free(sol);
        mat_free(m);
    }
    return ret;
}

static bool
test_spec_vec_all_matarr(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_1_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_1_%ld.txt", i+1); // i+1 for 1-indexed
        matarr_set(A, i, mat_from_file(filename));
        free(filename);
    }

    bool ret = true;
    for (size_t i = 0; i < 10; i++) {
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "spec_vec_CM1_1_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "spec_vec_CM1_1_%ld.txt", i+1); // i+1 for 1-indexed
        struct vec sol = vec_from_file(filename);
        free(filename);

        struct matrix m = matarr_get(A, i);
        scaled_data(m);
        struct vec v = spec_vec(m, 0, end, 9000);
        ret = ret && vec_equal(v, sol);
        vec_free(v);
        vec_free(sol);
    }
    matarr_free(A);
    return ret;
}

static bool
test_bin_stat(void)
{
    printf(__func__);

    struct matrix sol = mat_from_file(DATA_DIR "bin_stats_CM1_1.txt");

    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_1_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_1_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats = bin_stat_1D(A, 0, end, 9000);

    bool ret = mat_equal(sol, bin_stats);
    mat_free(sol);
    mat_free(bin_stats);
    matarr_free(A);
    return ret;
}

static bool
test_prob_dot_prob_through(void)
{
    printf(__func__);
    double sol = 0.034024882840827;

    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_1_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_1_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_3_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_3_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(sol, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}

static bool
test_prob_dot_prob(void)
{
    printf(__func__);
    double sol = 0.034024882840827;

    struct matrix bin_stats_1 = mat_from_file(DATA_DIR "bin_stats_CM1_1.txt");

    struct matrix bin_stats_3 = mat_from_file(DATA_DIR "bin_stats_CM1_3.txt");

    double ans = prob_dot_prod(bin_stats_1, bin_stats_3, 1e-4);
    bool ret = equals(sol, ans);
    mat_free(bin_stats_1);
    mat_free(bin_stats_3);
    return ret;
}

static bool
test_CM1_25_and_CM1_28_prob_dot_prod(void)
{
    printf(__func__);
    double sol = 0.041264598345041;
    struct matrix bin_stats_25 = mat_from_file(DATA_DIR "bin_stats_CM1_25.txt");

    struct matrix bin_stats_28 = mat_from_file(DATA_DIR "bin_stats_CM1_28.txt");

    bool ret = equals(sol, prob_dot_prod(bin_stats_25, bin_stats_28, 1e-4));
    mat_free(bin_stats_25);
    mat_free(bin_stats_28);
    return ret;
}

static bool
test_spec_vec_10_CM1_28(void)
{
    printf(__func__);
    struct matrix A_1 = mat_from_file(DATA_DIR "CM1_28_1.txt");
    struct matrix A_2 = mat_from_file(DATA_DIR "CM1_28_2.txt");
    struct matrix A_3 = mat_from_file(DATA_DIR "CM1_28_3.txt");
    struct matrix A_4 = mat_from_file(DATA_DIR "CM1_28_4.txt");
    struct matrix A_5 = mat_from_file(DATA_DIR "CM1_28_5.txt");
    struct matrix A_6 = mat_from_file(DATA_DIR "CM1_28_6.txt");
    struct matrix A_7 = mat_from_file(DATA_DIR "CM1_28_7.txt");
    struct matrix A_8 = mat_from_file(DATA_DIR "CM1_28_8.txt");
    struct matrix A_9 = mat_from_file(DATA_DIR "CM1_28_9.txt");
    struct matrix A_10 = mat_from_file(DATA_DIR "CM1_28_10.txt");
    scaled_data(A_1);
    scaled_data(A_2);
    scaled_data(A_3);
    scaled_data(A_4);
    scaled_data(A_5);
    scaled_data(A_6);
    scaled_data(A_7);
    scaled_data(A_8);
    scaled_data(A_9);
    scaled_data(A_10);
    struct vec A_1_spec = spec_vec(A_1, 0, end, 9000);
    struct vec A_2_spec = spec_vec(A_2, 0, end, 9000);
    struct vec A_3_spec = spec_vec(A_3, 0, end, 9000);
    struct vec A_4_spec = spec_vec(A_4, 0, end, 9000);
    struct vec A_5_spec = spec_vec(A_5, 0, end, 9000);
    struct vec A_6_spec = spec_vec(A_6, 0, end, 9000);
    struct vec A_7_spec = spec_vec(A_7, 0, end, 9000);
    struct vec A_8_spec = spec_vec(A_8, 0, end, 9000);
    struct vec A_9_spec = spec_vec(A_9, 0, end, 9000);
    struct vec A_10_spec = spec_vec(A_10, 0, end, 9000);

    struct vec sol_spec_vec_A_1 = vec_from_file(DATA_DIR "spec_vec_CM1_28_1.txt");
    struct vec sol_spec_vec_A_2 = vec_from_file(DATA_DIR "spec_vec_CM1_28_2.txt");
    struct vec sol_spec_vec_A_3 = vec_from_file(DATA_DIR "spec_vec_CM1_28_3.txt");
    struct vec sol_spec_vec_A_4 = vec_from_file(DATA_DIR "spec_vec_CM1_28_4.txt");
    struct vec sol_spec_vec_A_5 = vec_from_file(DATA_DIR "spec_vec_CM1_28_5.txt");
    struct vec sol_spec_vec_A_6 = vec_from_file(DATA_DIR "spec_vec_CM1_28_6.txt");
    struct vec sol_spec_vec_A_7 = vec_from_file(DATA_DIR "spec_vec_CM1_28_7.txt");
    struct vec sol_spec_vec_A_8 = vec_from_file(DATA_DIR "spec_vec_CM1_28_8.txt");
    struct vec sol_spec_vec_A_9 = vec_from_file(DATA_DIR "spec_vec_CM1_28_9.txt");
    struct vec sol_spec_vec_A_10 = vec_from_file(DATA_DIR "spec_vec_CM1_28_10.txt");

    bool ret = true;
    ret = ret && vec_equal(A_1_spec, sol_spec_vec_A_1);
    ret = ret && vec_equal(A_2_spec, sol_spec_vec_A_2);
    ret = ret && vec_equal(A_3_spec, sol_spec_vec_A_3);
    ret = ret && vec_equal(A_4_spec, sol_spec_vec_A_4);
    ret = ret && vec_equal(A_5_spec, sol_spec_vec_A_5);
    ret = ret && vec_equal(A_6_spec, sol_spec_vec_A_6);
    ret = ret && vec_equal(A_7_spec, sol_spec_vec_A_7);
    ret = ret && vec_equal(A_8_spec, sol_spec_vec_A_8);
    ret = ret && vec_equal(A_9_spec, sol_spec_vec_A_9);
    ret = ret && vec_equal(A_10_spec, sol_spec_vec_A_10);

    vec_free(A_1_spec);
    vec_free(A_2_spec);
    vec_free(A_3_spec);
    vec_free(A_4_spec);
    vec_free(A_5_spec);
    vec_free(A_6_spec);
    vec_free(A_7_spec);
    vec_free(A_8_spec);
    vec_free(A_9_spec);
    vec_free(A_10_spec);
    mat_free(A_1);
    mat_free(A_2);
    mat_free(A_3);
    mat_free(A_4);
    mat_free(A_5);
    mat_free(A_6);
    mat_free(A_7);
    mat_free(A_8);
    mat_free(A_9);
    mat_free(A_10);
    vec_free(sol_spec_vec_A_1);
    vec_free(sol_spec_vec_A_2);
    vec_free(sol_spec_vec_A_3);
    vec_free(sol_spec_vec_A_4);
    vec_free(sol_spec_vec_A_5);
    vec_free(sol_spec_vec_A_6);
    vec_free(sol_spec_vec_A_7);
    vec_free(sol_spec_vec_A_8);
    vec_free(sol_spec_vec_A_9);
    vec_free(sol_spec_vec_A_10);
    return ret;
}

static bool
test_bin_stats_CM1_28(void)
{
    printf(__func__);
    struct matarray L = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_28_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_28_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix A_i = mat_from_file(filename);
        free(filename);
        scaled_data(A_i);
        matarr_set(L, i, A_i);
    }
    struct matrix sol = mat_from_file(DATA_DIR "bin_stats_CM1_28.txt");
    struct matrix bin_stats = bin_stat_1D(L, 0, end, 9000);
    bool ret = mat_equal(sol, bin_stats);
    mat_free(bin_stats);
    mat_free(sol);
    matarr_free(L);
    return ret;
}

static bool
test_CM1_25_and_CM1_28(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_25_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_25_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_28_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_28_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.041264598345041, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}

static bool
test_CM1_25_and_CM1_10(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_25_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_25_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_10_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_10_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.072582556629134, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_27_and_CM1_4(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_27_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_27_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_4_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_4_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.021623181634144, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_1_and_CM1_11(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_1_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_1_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_11_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_11_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.04089280788519, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_24_and_CM1_21(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_24_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_24_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_21_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_21_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.12515913619196, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_25_and_CM1_21(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_25_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_25_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_21_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_21_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.066069968923634, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_23_and_CM1_8(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_23_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_23_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_8_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_8_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.001484153831093, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_10_and_CM1_21(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_10_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_10_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_21_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_21_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.06845171025487, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_10_and_CM1_22(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_10_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_10_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_22_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_22_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.036359745578049, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_10_and_CM1_7(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_10_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_10_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_7_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_7_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.029871109665094, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_11_and_CM1_12(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_11_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_11_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_12_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_12_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.084960498296299, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_11_and_CM1_3(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_11_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_11_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_3_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_3_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.093213135837302, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_12_and_CM1_18(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_12_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_12_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_18_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_18_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.016897263730676, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_12_and_CM1_23(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_12_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_12_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_23_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_23_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.056214831376217, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_13_and_CM1_12(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_13_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_13_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_12_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_12_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.004101523575941, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_14_and_CM1_3(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_14_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_14_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_3_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_3_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.057425859524548, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_15_and_CM1_12(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_15_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_15_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_12_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_12_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.055218397676643, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_15_and_CM1_13(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_15_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_15_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_13_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_13_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.013231966438739, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_18_and_CM1_5(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_18_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_18_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_5_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_5_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.012781966810721, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_20_and_CM1_11(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_20_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_20_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_11_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_11_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.072328503941529, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_20_and_CM1_12(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_20_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_20_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_12_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_12_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.07456522293608, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_20_and_CM1_17(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_20_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_20_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_17_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_17_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.094404829387605, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_20_and_CM1_20(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_20_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_20_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_20_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_20_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(1, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_22_and_CM1_17(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_22_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_22_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_17_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_17_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.03751249943288, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_23_and_CM1_22(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_23_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_23_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_22_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_22_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.022740246848618, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_24_and_CM1_11(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_24_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_24_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_11_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_11_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.051801154974774, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_26_and_CM1_1(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_26_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_26_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_1_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_1_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.022912835862035, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_26_and_CM1_11(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_26_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_26_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_11_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_11_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.040749418848459, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_26_and_CM1_12(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_26_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_26_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_12_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_12_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.110433675124284, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_26_and_CM1_2(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_26_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_26_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_2_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_2_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.023472628814994, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_27_and_CM1_16(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_27_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_27_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_16_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_16_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.057995289632189, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_27_and_CM1_28(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_27_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_27_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_28_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_28_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.035569551931446, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_28_and_CM1_1(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_28_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_28_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_1_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_1_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.022519129110172, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_28_and_CM1_20(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_28_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_28_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_20_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_20_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.281531338126355, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_1_and_CM1_8(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_1_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_1_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_8_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_8_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.010513748623968, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_2_and_CM1_2(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_2_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_2_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_2_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_2_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(1.0, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_2_and_CM1_26(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_2_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_2_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_26_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_26_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.023472628814994, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_2_and_CM1_6(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_2_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_2_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_6_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_6_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.021362257050611, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_3_and_CM1_18(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_3_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_3_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_18_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_18_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.004160752445243, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_3_and_CM1_20(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_3_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_3_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_20_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_20_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.102379914691872, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_3_and_CM1_6(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_3_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_3_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_6_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_6_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.028129541037335, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_4_and_CM1_6(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_4_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_4_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_6_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_6_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.018767371051262, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_5_and_CM1_12(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_5_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_5_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_12_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_12_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.063673316564264, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_5_and_CM1_20(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_5_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_5_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_20_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_20_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.084359053077657, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_7_and_CM1_9(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_7_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_7_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_9_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_9_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.105884314043279, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_8_and_CM1_19(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_8_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_8_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_19_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_19_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.009837382753993, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_9_and_CM1_3(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_9_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_9_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_3_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_3_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.109741556620843, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_9_and_CM1_6(void)
{
    printf(__func__);
    struct matarray A = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_9_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_9_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(A, i, m);
        free(filename);
    }
    struct matrix bin_stats_A = bin_stat_1D(A, 0, end, 9000);

    struct matarray B = matarr_zeros(10);
    for (size_t i = 0; i < 10; i++) {
        // filename
        size_t bufsz = safe_snprintf(NULL, 0, DATA_DIR "CM1_6_%ld.txt", i+1); // i+1 for 1-indexed
        char *filename = safe_calloc(bufsz + 1, sizeof(char));
        safe_snprintf(filename, bufsz + 1, DATA_DIR "CM1_6_%ld.txt", i+1); // i+1 for 1-indexed

        struct matrix m = mat_from_file(filename);
        scaled_data(m);
        matarr_set(B, i, m);
        free(filename);
    }
    struct matrix bin_stats_B = bin_stat_1D(B, 0, end, 9000);

    bool ret = equals(0.012490393307203, prob_dot_prod(bin_stats_A, bin_stats_B, 1e-4));
    mat_free(bin_stats_A);
    mat_free(bin_stats_B);
    matarr_free(A);
    matarr_free(B);
    return ret;
}
static bool
test_CM1_10_vs_21_sanity_check(void)
{
    printf(__func__);
    struct matrix m = mat_from_file(DATA_DIR "CM1_10_1.txt");
    struct matrix m2 = mat_from_file(DATA_DIR "CM1_10_3.txt");
    struct matrix m3 = mat_from_file(DATA_DIR "CM1_10_4.txt");
    struct matrix n = mat_from_file(DATA_DIR "CM1_21_1.txt");
    struct matrix n2 = mat_from_file(DATA_DIR "CM1_21_3.txt");
    struct matrix n3 = mat_from_file(DATA_DIR "CM1_21_4.txt");
    scaled_data(m);
    scaled_data(m2);
    scaled_data(m3);
    scaled_data(n);
    scaled_data(n2);
    scaled_data(n3);
    struct matrix data[3];
    data[0] = m;
    data[1] = m2;
    data[2] = m3;
    struct matrix ndata[3];
    ndata[0] = n;
    ndata[1] = n2;
    ndata[2] = n3;
    struct matarray arr = matarr_from_data(data, 3, false);
    struct matarray arr2 = matarr_from_data(ndata, 3, false);
    struct matrix a = bin_stat_1D(arr, 0, end, 9000);
    struct matrix a2 = bin_stat_1D(arr2, 0, end, 9000);
    double d = prob_dot_prod(a, a2, 1e-4);
    mat_free(a);
    mat_free(a2);
    matarr_free(arr);
    matarr_free(arr2);
    return equals(0.015949719716551063, d);
}

static bool
test_symmetric_1d(void)
{
    printf(__func__);
    struct matrix m_10_1 = mat_from_file(DATA_DIR "CM1_10_1.txt");
    struct matrix m_10_2 = mat_from_file(DATA_DIR "CM1_10_2.txt");
    struct matrix m_10_3 = mat_from_file(DATA_DIR "CM1_10_3.txt");
    struct matrix m_10_4 = mat_from_file(DATA_DIR "CM1_10_4.txt");
    struct matrix m_10_5 = mat_from_file(DATA_DIR "CM1_10_5.txt");
    struct matrix m_10_6 = mat_from_file(DATA_DIR "CM1_10_6.txt");
    struct matrix m_3_1 = mat_from_file(DATA_DIR "CM1_3_1.txt");
    struct matrix m_3_2 = mat_from_file(DATA_DIR "CM1_3_2.txt");
    struct matrix m_3_3 = mat_from_file(DATA_DIR "CM1_3_3.txt");
    struct matrix m_3_4 = mat_from_file(DATA_DIR "CM1_3_4.txt");
    struct matrix m_3_5 = mat_from_file(DATA_DIR "CM1_3_5.txt");
    struct matrix m_3_6 = mat_from_file(DATA_DIR "CM1_3_6.txt");

    struct matrix data_10[6];
    data_10[0] = m_10_1;
    data_10[1] = m_10_2;
    data_10[2] = m_10_3;
    data_10[3] = m_10_4;
    data_10[4] = m_10_5;
    data_10[5] = m_10_6;

    struct matrix data_3[6];
    data_3[0] = m_3_1;
    data_3[1] = m_3_2;
    data_3[2] = m_3_3;
    data_3[3] = m_3_4;
    data_3[4] = m_3_5;
    data_3[5] = m_3_6;

    struct matarray arr = matarr_from_data(data_10, 6, 0);
    struct matarray arr2 = matarr_from_data(data_3, 6, 0);
    struct matrix a = bin_stat_1D(arr, 0, end, 9000);
    struct matrix a2 = bin_stat_1D(arr2, 0, end, 9000);
    bool ret = prob_dot_prod(a, a2, 1e-4) == prob_dot_prod(a2, a, 1e-4);
    bool ret2 = equals(prob_dot_prod(a, a2, 1e-4), 0.14003071140822);
    mat_free(a);
    mat_free(a2);
    matarr_free(arr);
    matarr_free(arr2);
    return ret && ret2;
}

static bool
test_peak_stat_all_through(void)
{
    printf(__func__);
    struct matrix m_10_1 = mat_from_file(DATA_DIR "analytes_normal_10_1_1.txt");
    struct matrix m_10_2 = mat_from_file(DATA_DIR "analytes_normal_10_2_1.txt");
    struct matrix m_10_3 = mat_from_file(DATA_DIR "analytes_normal_10_3_1.txt");
    struct matrix m_10_4 = mat_from_file(DATA_DIR "analytes_normal_10_4_1.txt");
    struct matrix m_10_5 = mat_from_file(DATA_DIR "analytes_normal_10_5_1.txt");
    struct matrix m_9_1 = mat_from_file(DATA_DIR  "analytes_normal_9_1_1.txt");
    struct matrix m_9_2 = mat_from_file(DATA_DIR  "analytes_normal_9_2_1.txt");
    struct matrix m_9_3 = mat_from_file(DATA_DIR  "analytes_normal_9_3_1.txt");
    struct matrix m_9_4 = mat_from_file(DATA_DIR  "analytes_normal_9_4_1.txt");
    struct matrix m_9_5 = mat_from_file(DATA_DIR  "analytes_normal_9_5_1.txt");

    struct matrix data_10[5];
    data_10[0] = m_10_1;
    data_10[1] = m_10_2;
    data_10[2] = m_10_3;
    data_10[3] = m_10_4;
    data_10[4] = m_10_5;

    struct matrix data_9[5];
    data_9[0] = m_9_1;
    data_9[1] = m_9_2;
    data_9[2] = m_9_3;
    data_9[3] = m_9_4;
    data_9[4] = m_9_5;

    struct matarray arr = matarr_from_data(data_10, 5, 0);
    struct matarray arr2 = matarr_from_data(data_9, 5, 0);
    struct matrix a = peak_stat(arr, 10);
    struct matrix a2 = peak_stat(arr2, 10);
    FILE *f = fopen("/tmp/mat", "w");
    mat_fprintf(f, a);
    fclose(f);
    FILE *f2 = fopen("/tmp/mat2", "w");
    mat_fprintf(f, a2);
    fclose(f2);
    double d = peak_sim_measure_L2(a, a2, 1e-4, 10);
    double d2 = peak_sim_measure_L2(a2, a, 1e-4, 10);
    bool ret = equals(d, 201.282585841773e-006) && equals(d2, 240.632397866844e-006);
    mat_free(a);
    mat_free(a2);
    matarr_free(arr);
    matarr_free(arr2);
    return ret;
}

static bool
simple(void)
{
    printf(__func__);
    return 0 == 0;
}

int main(int argc, char *argv[])
{
    if (argc > 1) {
        WARNING("%s\n", "too many parameters");
        for (int i = 1; i < argc; i++) {
            printf("ignored: %s\n", argv[i]);
        }
    }
    int ret = EXIT_SUCCESS;
    testfunc tests[] = {
        simple, 

        // 2D
        test_cos_sim_L2,
        test_cos_sim_3rd_highest_of_12_11_30V,
        test_cos_sim_11th_highest_of_12_11_30V, 
        test_argmax,
        test_peak_sort_simple,
        test_peak_sort_zeros,
        test_peak_sort_real2,
        test_peak_sort_real15,
        test_peak_stat_real,
        test_peak_sim_measure_simple,
        test_cos_sim_extra1,
        test_cos_sim_extra2,
        test_peak_sim_measure_complex,
        test_peak_sim_measure_complex2,
        test_peak_sim_measure_real,
        test_peak_sim_measure_n_less_than_both,
        test_edgecase_0_peak_sort,
        test_edgecase_1_peak_sort,
        test_edgecase_contains_0_peak_sort,
        test_edge_case_1_peak_stat,
        test_edgecase_contains_0_peak_stat,
        test_edgecase_0_peak_stat,
        test_edge_case_0_peak_sim,
        // test_edge_case_0_cos_sim,
        test_read_line_w_newline,
        test_read_line_wo_newline,
        test_vec_read_simple,
        test_vec_read_real,
        test_read_vec_no_eof_eol,
        test_mat_read_no_extra_newln_ints,
        test_mat_read_blank_lines_ints,
        test_mat_read_ints,
        test_mat_read_trailing_white_space,
        test_mat_read_leading_white_space,
        test_analytes_normal_1_1_1,
        test_similarity_analysis,

        // 1D
        test_mat_read_cm_data,
        test_scaled_data,
        test_spec_vec,
        test_spec_vec_all,
        test_spec_vec_all_matarr,
        test_spec_vec_10_CM1_28,
        test_bin_stat,
        test_bin_stats_CM1_28,
        test_prob_dot_prob,
        test_prob_dot_prob_through,
        test_CM1_25_and_CM1_28_prob_dot_prod,
        test_CM1_25_and_CM1_28,
        test_CM1_25_and_CM1_10,
        test_CM1_27_and_CM1_4,
        test_CM1_1_and_CM1_11,
        test_CM1_24_and_CM1_21,
        test_CM1_25_and_CM1_21,
        test_CM1_23_and_CM1_8,
        test_CM1_10_and_CM1_21,
        test_CM1_10_and_CM1_22,
        test_CM1_10_and_CM1_7,
        test_CM1_11_and_CM1_12,
        test_CM1_11_and_CM1_3,
        test_CM1_12_and_CM1_18,
        test_CM1_12_and_CM1_23,
        test_CM1_13_and_CM1_12,
        test_CM1_14_and_CM1_3,
        test_CM1_15_and_CM1_12,
        test_CM1_15_and_CM1_13,
        test_CM1_18_and_CM1_5,
        test_CM1_20_and_CM1_11,
        test_CM1_20_and_CM1_12,
        test_CM1_20_and_CM1_17,
        test_CM1_20_and_CM1_20,
        test_CM1_22_and_CM1_17,
        test_CM1_23_and_CM1_22,
        test_CM1_24_and_CM1_11,
        test_CM1_26_and_CM1_1,
        test_CM1_26_and_CM1_11,
        test_CM1_26_and_CM1_12,
        test_CM1_26_and_CM1_2,
        test_CM1_27_and_CM1_16,
        test_CM1_27_and_CM1_28,
        test_CM1_28_and_CM1_1,
        test_CM1_28_and_CM1_20,
        test_CM1_1_and_CM1_8,
        test_CM1_2_and_CM1_2,
        test_CM1_2_and_CM1_26,
        test_CM1_2_and_CM1_6,
        test_CM1_3_and_CM1_18,
        test_CM1_3_and_CM1_20,
        test_CM1_3_and_CM1_6,
        test_CM1_4_and_CM1_6,
        test_CM1_5_and_CM1_12,
        test_CM1_5_and_CM1_20,
        test_CM1_7_and_CM1_9,
        test_CM1_8_and_CM1_19,
        test_CM1_9_and_CM1_3,
        test_CM1_9_and_CM1_6,
        test_CM1_10_vs_21_sanity_check,
        test_symmetric_1d,
        test_peak_stat_all_through,
    };

    const size_t len = sizeof(tests)/sizeof(tests[0]);
    size_t num_passed = 0;
    size_t num_failed = 0;
    for (size_t i = 0; i < len; i++) {
        bool passed = tests[i]();
        if (passed) {
            num_passed += 1;
            printf(GREEN " OK" RESET "\n");
        } else {
            num_failed += 1;
            printf(RED " FAILED" RESET "\n");
            ret = EXIT_FAILURE;
        }
    }
    puts(GREEN "======================");
    puts("Testsuite summary");
    puts("======================" RESET);

    printf(BOLD "total: %zu\n" RESET, len);

    if (num_passed) printf(GREEN);
    printf("pass: %zu\n" RESET, num_passed);

    if (num_failed) printf(RED);
    printf("fail: %zu\n" RESET, num_failed);
    puts(GREEN "======================" RESET);

    return ret;

}


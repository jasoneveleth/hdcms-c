#include <stdio.h>
#include <stdbool.h>
#include "test.h"
#include "../src/peak.h"

bool
test_cos_sim_L2()
{
    printf(__FUNCTION__);
    double udata[] = {91, 0.9, 0.1, 0.08};
    double vdata[] = {90, 1, 0.09, 0.11};
    struct vec u = vec_from_data(udata, 4, false);
    struct vec v = vec_from_data(vdata, 4, false);
    double sim = cos_sim_L2(u, v);
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
bool
test_cos_sim_3rd_highest_of_12_11_30V()
{
    printf(__FUNCTION__);
    double udata[] = {3.882976388000000, 0.002553558503045, 0.000017707525801, 0.000093945978461};
    double vdata[] = {3.882982086000000, 0.002463295137625, 0.000007833634533, 0.000323634722072};
    struct vec u = vec_from_data(udata, 4, false);
    struct vec v = vec_from_data(vdata, 4, false);
    vec_scale(u, 1e2);
    vec_scale(v, 1e2);
    double sim1 = cos_sim_L2(u, v);
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
bool
test_cos_sim_11th_highest_of_12_11_30V() 
{
    printf(__FUNCTION__);
    double udata2[] = {3.893009976000000, 0.000527014227888, 0.000015185034738, 0.000069406478369};
    double vdata2[] = {3.893016630000000, 0.000479664131948, 0.000006732993391, 0.000109354061818};
    struct vec u = vec_from_data(udata2, 4, false);
    struct vec v = vec_from_data(vdata2, 4, false);
    vec_scale(u, 1e2);
    vec_scale(v, 1e2);
    double sim2 = cos_sim_L2(u, v);
    vec_free(u);
    vec_free(v);
    return equals(sim2, 0.734449667053694);
}

bool
test_argmax()
{
    printf(__FUNCTION__);
    double m2data[] = {83, 0.23,
        92, 0.47,
        79, 0.61,
        110, 0.5};
    struct matrix m = mat_from_data(m2data, 4, 2, 2, false);
    struct vec ys = vec_from_col(m, 1);
    size_t argmax = vec_argmax(ys);
    double x = mat_get(m, argmax, 0);
    double y = mat_get(m, argmax, 1);
    mat_free(m);
    return equals(y, 0.61);
}

bool
test_peak_sort_simple()
{
    printf(__FUNCTION__);
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

bool
test_peak_sort_zeros()
{
    printf(__FUNCTION__);
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

bool test_peak_sort_real2()
{
    printf(__FUNCTION__);
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

bool
test_peak_sort_real15()
{
    printf(__FUNCTION__);
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

bool
test_peak_stat_real()
{
    printf(__FUNCTION__);
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

bool
simple() 
{
    printf(__FUNCTION__);
    return 0 == 0;
}

int main() 
{
    int ret = EXIT_SUCCESS;
    puts("================");
    testfunc tests[] = {
        simple, 
        test_cos_sim_L2,
        test_cos_sim_3rd_highest_of_12_11_30V,
        test_cos_sim_11th_highest_of_12_11_30V, 
        test_argmax,
        test_peak_sort_simple,
        test_peak_sort_zeros,
        test_peak_sort_real2,
        test_peak_sort_real15,
        test_peak_stat_real,
    };
    const size_t len = sizeof(tests)/sizeof(tests[0]);
    for (size_t i = 0; i < len; i++) {
        bool passed = tests[i]();
        if (passed) {
            printf(GREEN " OK" RESET "\n");
        } else {
            printf(RED " FAILED" RESET "\n");
            ret = EXIT_FAILURE;
        }
    }
    puts("================");
    return ret;
}


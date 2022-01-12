#include <stdio.h>
#include <stdbool.h>
#include "test.h"
#include "../util/peak.h"
#include "../util/array.h"

// path from current working directory of shell running the test executable
#define TESTDIR "../src/test/"

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
test_vec_read_simple()
{
    printf(__FUNCTION__);
    FILE *fptr = safe_fopen(TESTDIR "vec_read_simple.txt", "r");
    struct vec output = vec_read(fptr, "%lg");
    fclose(fptr);
    double data[] = {728, 283910, 812931, 72891};
    struct vec sol = vec_from_data(data, 4, false);
    bool ret = vec_equal(output, sol);
    vec_free(output);
    vec_free(sol);
    return ret;
}

bool
test_cos_sim_extra1()
{
    printf(__FUNCTION__);
    double adata[] = {90, 0.5, 0.001, 0.02};
    double bdata[] = {100.001, 0.908, 0.004, 0.01};
    struct vec u = vec_from_data(adata, 4, false);
    struct vec v = vec_from_data(bdata, 4, false);
    return equals(cos_sim_L2(u, v), 0);
}

bool
test_cos_sim_extra2()
{
    printf(__FUNCTION__);
    double adata[] = {100, 0.9, 0.003, 0.008};
    double bdata[] = {100.001, 0.908, 0.004, 0.01};
    struct vec u = vec_from_data(adata, 4, false);
    struct vec v = vec_from_data(bdata, 4, false);
    return equals(cos_sim_L2(u, v), 7.856829001024321e-01);
}

bool
test_peak_sim_measure_simple()
{
    printf(__FUNCTION__);
    double adata[] = {90, 0.5, 0.001, 0.02,
        100, 0.9, 0.003, 0.008};
    double bdata[] = {90.001, 0.497, 0.002, 0.015,
        100.001, 0.908, 0.004, 0.01};
    struct matrix A = mat_from_data(adata, 2, 4, 4, false);
    struct matrix B = mat_from_data(bdata, 2, 4, 4, false);
    double sim = peak_sim_measure_L2(A, B, 2);
    bool ret = equals(sim, 7.906904116230999e-01);
    mat_free(A);
    mat_free(B);
    return ret;
}

bool
test_peak_sim_measure_complex()
{
    printf(__FUNCTION__);
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
    double sim = peak_sim_measure_L2(A, B, 10);
    bool ret = equals(sim, 0.00045443404003044953);
    mat_free(A);
    mat_free(B);
    return ret;
}

bool
test_peak_sim_measure_complex2()
{
    printf(__FUNCTION__);
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
    double sim = peak_sim_measure_L2(A, B, 10);
    bool ret = equals(sim, 0.00045443404003044953);
    mat_free(A);
    mat_free(B);
    return ret;
}

bool
test_peak_sim_measure_n_less_than_both()
{
    printf(__FUNCTION__);
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
    double sim = peak_sim_measure_L2(A, B, 4);
    bool ret = equals(sim, 0.00012678008971000085);
    mat_free(A);
    mat_free(B);
    return ret;
}

bool
test_peak_sim_measure_real()
{
    printf(__FUNCTION__);
    struct matrix A = mat_from_data(stat_m13_i_2, 13, 4, 4, false);
    struct matrix B = mat_from_data(stat_m12_i_3, 25, 4, 4, false);
    double sim = peak_sim_measure_L2(A, B, 20);
    bool ret = equals(sim, 0);
    mat_free(A);
    mat_free(B);
    return ret;
}

bool
test_vec_read_real()
{
    printf(__FUNCTION__);
    FILE *fptr = safe_fopen(TESTDIR "vec_read_real.txt", "r");
    struct vec output = vec_read(fptr, "%lg");
    fclose(fptr);
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

bool
test_edgecase_contains_0_peak_sort()
{
    printf(__FUNCTION__);
    struct matrix m = mat_zeros(0, 0);
    struct matrix adata[] = {m};
    struct matarray arr = matarr_from_data(adata, 1, false);
    struct matarray output = peak_sort(arr, 5);
    bool ret = output.length == 0;
    mat_free(m);
    matarr_free(output);
    return ret;
}

bool
test_edgecase_0_peak_sort()
{
    printf(__FUNCTION__);
    struct matarray m = matarr_zeros(0);
    struct matarray output = peak_sort(m, 5);
    bool ret = output.length == 5;
    matarr_free(m);
    matarr_free(output);
    return ret;
}

bool
test_edgecase_1_peak_sort()
{
    printf(__FUNCTION__);
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

bool
test_edge_case_1_peak_stat()
{
    printf(__FUNCTION__);
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

bool
test_edgecase_contains_0_peak_stat()
{
    printf(__FUNCTION__);
    struct matrix m = mat_zeros(0, 0);
    struct matrix adata[] = {m};
    struct matarray arr = matarr_from_data(adata, 1, false);
    struct matrix output = peak_stat(arr, 5);
    bool ret = output.len1 == 0;
    mat_free(output);
    mat_free(m);
    return ret;
}

bool
test_edgecase_0_peak_stat()
{
    printf(__FUNCTION__);
    struct matarray m = matarr_zeros(0);
    freopen(NULL_DEVICE, "w", stderr);
    struct matrix output = peak_stat(m, 5);
    freopen(CONSOLE, "w", stderr);
    bool ret = output.len1 == 0 && output.len2 == 0;
    matarr_free(m);
    return ret;
}

bool
test_edge_case_0_peak_sim()
{
    printf(__FUNCTION__);
    struct matrix m = {0, 0, 0, NULL, false};
    double ndata[] = {420, 69, 0, 0};
    struct matrix n = {1, 4, 4, ndata, false};
    freopen(NULL_DEVICE, "w", stderr);
    double d = peak_sim_measure_L2(m, n, 5);
    freopen(CONSOLE, "w", stderr);
    return equals(d, 0);
}

bool
test_edge_case_0_cos_sim()
{
    printf(__FUNCTION__);
    struct vec v = {0, 0, NULL, false};
    struct vec u = {0, 0, NULL, false};
    freopen(NULL_DEVICE, "w", stderr);
    double d = cos_sim_L2(u, v);
    freopen(CONSOLE, "w", stderr);
    return equals(d, 0);
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
        test_vec_read_simple,
        test_vec_read_real,
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
        test_edge_case_0_cos_sim,
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


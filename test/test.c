#include <stdio.h>
#include <stdbool.h>
#include "test.h"
#include "../src/peak.h"

bool
test_cos_sim_L2()
{
    printf("test_cos_sim_L2");
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
    printf("test_cos_sim_3rd_highest_of_12_11_30V");
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
    printf("test_cos_sim_11th_highest_of_12_11_30V");
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
test_peak_sort_simple()
{
    printf("test_peak_sort_simple");
    size_t n = 20;
    double m1data[] = {83, 0.23,
                   92, 0.47,
                   79, 0.61,
                   110, 0.5};
    double m2data[] = {72, 0.68,
                   97, 0.43,
                   89, 0.27,
                   100, 0.1};
    struct matrix m1 = mat_from_data(m1data, 4, 2, 2, false);
    struct matrix m2 = mat_from_data(m2data, 4, 2, 2, false);

    struct matrix adata[] = {m1, m2};
    struct matarray arr = matarr_from_data(adata, 2);
    struct matarray ans = peak_sort(arr, n);

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
    bool b0 = mat_equal(peak0, matarr_get(ans, 0));
    bool b1 = mat_equal(peak1, matarr_get(ans, 1));
    bool b2 = mat_equal(peak2, matarr_get(ans, 2));
    bool b3 = mat_equal(peak3, matarr_get(ans, 3));

    matarr_free(arr);
    mat_free(m1);
    mat_free(m2);
    mat_free(peak0);
    mat_free(peak1);
    mat_free(peak2);
    mat_free(peak3);
    return b0 && b1 && b2 && b3;
}


bool
simple() 
{
    printf("simple test");
    return 0 == 0;
}

int main() 
{
    int ret = EXIT_SUCCESS;
    puts("================");
    testfunc tests[] = {simple, 
        test_cos_sim_L2,
        test_cos_sim_3rd_highest_of_12_11_30V,
        test_cos_sim_11th_highest_of_12_11_30V, 
        test_peak_sort_simple
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


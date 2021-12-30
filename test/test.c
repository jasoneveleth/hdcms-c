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
    struct vec u = vec_from_data(udata, 4);
    struct vec v = vec_from_data(vdata, 4);
    double sim = cos_sim_L2(u, v);
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
    struct vec u = vec_from_data(udata, 4);
    struct vec v = vec_from_data(vdata, 4);
    vec_scale(u, 1e2);
    vec_scale(v, 1e2);
    double sim1 = cos_sim_L2(u, v);
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
    struct vec u = vec_from_data(udata2, 4);
    struct vec v = vec_from_data(vdata2, 4);
    vec_scale(u, 1e2);
    vec_scale(v, 1e2);
    double sim2 = cos_sim_L2(u, v);
    return equals(sim2, 0.734449667053694);
}


bool
simple() 
{
    printf("simple test");
    return 0 == 0;
}

int main() 
{
    testfunc tests[] = {simple, test_cos_sim_L2, test_cos_sim_3rd_highest_of_12_11_30V, test_cos_sim_11th_highest_of_12_11_30V};
    const size_t len = sizeof(tests)/sizeof(tests[0]);
    for (size_t i = 0; i < len; i++) {
        bool passed = tests[i]();
        if (passed) {
            printf(GREEN " OK" RESET "\n");
        } else {
            printf(RED " FAILED" RESET "\n");
        }
    }

    return 0;
}


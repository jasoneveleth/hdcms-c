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

bool
simple() 
{
    printf("simple test");
    return 0 == 0;
}

int main() 
{
    testfunc tests[] = {simple, test_cos_sim_L2};
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


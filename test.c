#include "test.h"
#include <stdio.h>

int
simple() {
    printf("simple test");
    if (0 != 0) {
        return 1;
    }
    return 0;
}

int main() 
{
    testfunc tests[] = {simple};
    const size_t len = sizeof(tests)/sizeof(tests[0]);
    for (size_t i = 0; i < len; i++) {
        int ret = tests[i]();
        if (ret) {
            printf(RED " ERROR" RESET "\n");
        } else {
            printf(GREEN " OK" RESET "\n");
        }
    }

    return 0;
}


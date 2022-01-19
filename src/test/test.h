#ifndef TEST_H
#define TEST_H
#include <stdbool.h>

#ifdef _WIN32
#define NULL_DEVICE "NUL:"
#define CONSOLE "CON:"
#else
#define NULL_DEVICE "/dev/null"
#define CONSOLE "/dev/tty"
#endif

typedef bool (*testfunc)(void);

extern double m13_1_2_data[];
extern double m13_2_2_data[];
extern double m13_3_2_data[];
extern double m13_4_2_data[];
extern double m13_5_2_data[];

extern double p_m13_2__0_data[];
extern double p_m13_2__1_data[];
extern double p_m13_2__2_data[];
extern double p_m13_2__3_data[];
extern double p_m13_2__4_data[];
extern double p_m13_2__5_data[];
extern double p_m13_2__6_data[];
extern double p_m13_2__7_data[];
extern double p_m13_2__8_data[];
extern double p_m13_2__9_data[];
extern double p_m13_2__10_data[];
extern double p_m13_2__11_data[];
extern double p_m13_2__12_data[];

extern double stat_m13_i_2[];
extern double stat_m12_i_3[];

#endif // TEST_H

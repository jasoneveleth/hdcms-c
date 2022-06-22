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

#endif // TEST_H

#ifndef FIXWINDOWS_H
#define FIXWINDOWS_H

#ifdef _WIN32
#define NULL_DEVICE "NUL:"
#else
#define NULL_DEVICE "/dev/null"
#endif

#ifdef _WIN32
#define RED ""
#define GREEN ""
#define BOLD ""
#define RESET ""
#else
#define RED "\033[31m"
#define GREEN "\033[32m"
#define BOLD "\033[1m"
#define RESET "\033[0m"
#endif

// assuming all non-windows are posix
#ifdef _WIN32
#define flockfile(x) do {} while (0)
#define funlockfile(x) do {} while (0)
#define getc_unlocked getc
#endif

#ifdef _WIN32
#define HAS_WINDOWS 1
#else
#define HAS_WINDOWS 0
#endif

// windows can't even compile its own headers without warnings (missing _WIN32_WINNT_WIN10_TH2, ...)
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#ifdef _WIN32
#define noreturn
#else
#include <stdnoreturn.h>
#endif

#endif // FIXWINDOWS_H

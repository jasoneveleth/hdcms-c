#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <fcntl.h>
#include <stdnoreturn.h>
#include <getopt.h>
#include "main.h"
#include "../util/array.h"
#include "../util/bin.h" // oned
#include "../util/peak.h" // twod

#define DEFAULT_WIDTH (0.1)

enum mode {ONED, TWOD};
static char *argv0;
static double width;
static int mflag;

noreturn static void
usage() {
    printf("usage: %s [--1d|--2d] [-h] [-q] --list=path1[,path2,...] FILE ...\n", argv0);
    printf("\n");
    printf("The default mode is --2d, which is for high resolution spectra.\n");
    printf("The argument --list is a list of pathnames to replicate files of a compound.\n");
    printf("The argument FILE should contain a list of pathnames to replicate files of a compound.\n");
    exit(2);
}

static struct matrix
compare_1d(const struct matarray arr)
{
    struct matrix m = mat_zeros(arr.length, arr.length);
    for (size_t i = 0; i < arr.length; i++) {
        for (size_t j = 0; j < arr.length; j++) {
            mat_set(m, i, j, prob_dot_prod(matarr_get(arr, i), matarr_get(arr, j)));
        }
    }
    return m;
}

static struct matrix
compare_2d(const struct matarray arr)
{
    struct matrix m = mat_zeros(arr.length, arr.length);
    for (size_t i = 0; i < arr.length; i++) {
        for (size_t j = 0; j < arr.length; j++) {
            size_t n = matarr_get(arr, i).len1; // >= longest possible length
            mat_set(m, i, j, peak_sim_measure_L2(matarr_get(arr, i), matarr_get(arr, j), n));
        }
    }
    return m;
}

static inline bool
file_readable(const char *const path)
{
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror(path);
        return false;
    }
    close(fd);
    return true;
}

static struct matrix
filenames_to_stats(char *str)
{
    struct matarray arr = matarr_zeros(2);

    char *path;
    size_t i;
    for (i = 0; (path = find_token(&str, " ,\n\t")); i++) {
        if ((size_t)i == arr.length) { // resize
            arr.length *= 2;
            arr.data = safe_realloc(arr.data, arr.length * sizeof(*arr.data));
        }
        struct matrix m = mat_from_file(path);
        scaled_data(m); // this is redundant in TWOD since the data we are given is scaled
        matarr_set(arr, i, m);
    }
    arr.length = i; // wastes some of the malloc'd mem but that's okay
    struct matrix ret;
    if (mflag == ONED) {
        ret = bin_stat_1D(arr, width);
    } else if (mflag == TWOD) {
        size_t n = matarr_get(arr, 0).len1; // >= longest possible length
        ret = peak_stat(arr, n);
    } else {
        printf("\n");
        usage();
    }
    matarr_free(arr);
    return ret;
}

static void
list_file(const char *const filename, const struct matarray arr, const size_t i)
{
    // https://stackoverflow.com/questions/22059189/read-a-file-as-byte-array
    FILE *fileptr;
    char *buffer;

    fileptr = safe_fopen(filename, "rb"); // Open the file in binary mode

    fseek(fileptr, 0, SEEK_END); // Jump to the end of the file
    size_t len =
        (size_t)ftell(fileptr); // Get the current byte offset in the file
    rewind(fileptr);            // Jump back to the beginning of the file

    buffer = malloc(len + 1);            // Enough memory for the file (and NUL)
    fread(buffer, len, 1, fileptr);      // Read in the entire file
    fclose(fileptr);                     // Close the file
    buffer[len] = '\0';                  // NUL terminate the string

    struct matrix bin_stats = filenames_to_stats(buffer);
    free(buffer);
    matarr_set(arr, i, bin_stats);
}

static void
list_option(char *str, struct matarray arr, const size_t i)
{
    struct matrix bin_stats = filenames_to_stats(str);
    matarr_set(arr, i, bin_stats);
}

static void
print_comparison(const struct matrix m)
{
    printf("╭────┬");
    for (size_t i = 0; i < m.len1 - 1; i++) {
        printf("──────────┬");
    }
    printf("──────────╮\n");

    printf("│%4s│", "x");
    for (size_t i = 0; i < m.len1; i++) {
        printf("%10zu│", i);
    }
    printf("\n");

    printf("├────┼");
    for (size_t i = 0; i < m.len1 - 1; i++) {
        printf("──────────┼");
    }
    printf("──────────┤");

    printf("\n");
    for (size_t i = 0; i < m.len1; i++) {
        printf("│%4zu│", i);
        for (size_t j = 0; j < m.len1; j++) {
            printf(" %8.6f │", mat_get(m, i, j));
        }
        printf("\n");
    }

    printf("╰────┴");
    for (size_t i = 0; i < m.len1 - 1; i++) {
        printf("──────────┴");
    }
    printf("──────────╯\n");
}

static void
print_comparison_no_utf8(const struct matrix m)
{
    printf("|%4s|", "x");
    for (size_t i = 0; i < m.len1; i++) {
        printf("%10zu|", i);
    }
    printf("\n");

    printf("|----|");
    for (size_t i = 0; i < m.len1 - 1; i++) {
        printf("----------|");
    }
    printf("----------|");

    printf("\n");
    for (size_t i = 0; i < m.len1; i++) {
        printf("|%4zu|", i);
        for (size_t j = 0; j < m.len1; j++) {
            printf(" %8.6f |", mat_get(m, i, j));
        }
        printf("\n");
    }
}

int 
main(int argc, char *argv[])
{
    argv0 = argv[0];

    int ch;
    size_t nreplicates;

    /* options descriptor */
    static struct option longopts[] = {
        { "1d",         no_argument, /*changes*/NULL,           ONED },
        { "1D",         no_argument, /*changes*/NULL,           ONED },
        { "2d",         no_argument, /*changes*/NULL,           TWOD },
        { "2D",         no_argument, /*changes*/NULL,           TWOD },
        { "list",       required_argument,      NULL,           'l' },
        { "help",       no_argument,            NULL,           'h' },
        { "quiet",      no_argument,            NULL,           'q' },
        { "width",      required_argument,      NULL,           'w' },
        { 0,            0,                      0,              0 }
    };
    // set 1d / 2d flag (you aren't allow to initilize a structure with non
    // const expression)
    longopts[0].flag = longopts[1].flag = longopts[2].flag = longopts[3].flag = &mflag;

    mflag = TWOD;
    nreplicates = 0;
    char **replicates = malloc((size_t)argc * sizeof(*replicates)); // can't be larger than argc
    width = DEFAULT_WIDTH;
    while ((ch = getopt_long(argc, argv, "hqw:", longopts, NULL)) != -1) {
        switch (ch) {
            case 'w':
                ; // need an expression immediately after case statement
                char *endptr;
                width = strtod(optarg, &endptr);
                if (optarg == endptr) {
                    width = DEFAULT_WIDTH;
                    WARNING("unknown width: `%s`, setting to default (%g)\n", optarg, width);
                }
                break;
            case 'l':
                replicates[nreplicates] = optarg;
                nreplicates++;
                break;
            case 0: // when we get to a toggle long option (1d/2d)
                break;
            case 'h':
            case '?':
            case ':':
            default:
                usage(); 
                // impossible to get here
        }
    }
    argc -= optind;
    argv += optind;

    struct matarray replicate_stats = matarr_zeros(2);

    for (size_t i = 0; i < nreplicates; i++) {
        // deal with list reallocation
        if ((size_t)i == replicate_stats.length) {
            replicate_stats.length *= 2;
            replicate_stats.data = safe_realloc(replicate_stats.data, replicate_stats.length * sizeof(*replicate_stats.data));
        }

        printf("%zu: %s\n", i, replicates[i]);
        list_option(replicates[i], replicate_stats, i);
    }

    free(replicates);

    for (int i = 0; i < argc; i++) {
        if (!file_readable(argv[i])) { // check if file exists
            printf("unrecognized argument / not file: `%s`\n\n", argv[i]);
            usage();
        }
        printf("%zu: %s\n", nreplicates, argv[i]);

        // deal with list reallocation
        if ((size_t)i == replicate_stats.length) {
            replicate_stats.length *= 2;
            replicate_stats.data = safe_realloc(replicate_stats.data, replicate_stats.length * sizeof(*replicate_stats.data));
        }

        list_file(argv[i], replicate_stats, nreplicates);
        nreplicates++;
    }

    replicate_stats.length = nreplicates; // wastes some of the malloc'd mem but that's okay

    // done parsing args

    struct matrix comparison;
    if (mflag == ONED)
        comparison = compare_1d(replicate_stats);
    else
        comparison = compare_2d(replicate_stats);
    matarr_free(replicate_stats);
#ifdef _WIN32
    print_comparison_no_utf8(comparison);
#else
    print_comparison(comparison);
#endif
    mat_free(comparison);
    return 0;
}


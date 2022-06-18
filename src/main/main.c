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
compare_1d(struct matarray arr)
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
compare_2d(struct matarray arr)
{
    struct matrix m = mat_zeros(arr.length, arr.length);
    for (size_t i = 0; i < arr.length; i++) {
        for (size_t j = 0; j < arr.length; j++) {
            // exploit that -1 is all 1's in twos complement to get huge value
            mat_set(m, i, j, peak_sim_measure_L2(matarr_get(arr, i), matarr_get(arr, j), (size_t)-1));
        }
    }
    return m;
}

static inline bool
file_readable(char *path)
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
        if (i == arr.length) { // resize
            arr.length *= 2;
            arr.data = safe_realloc(arr.data, arr.length * sizeof(*arr.data));
        }
        struct matrix m = mat_from_file(path);
        scaled_data(m); // this is redundant in TWOD since the data we are given is scaled
        matarr_set(arr, i, m);
    }
    arr.length = i; // cuts off some of the malloc'd mem but that's okay
    struct matrix ret;
    if (mflag == ONED) {
        ret = bin_stat_1D(arr, width);
    } else if (mflag == TWOD) {
        ret = peak_stat(arr, (size_t)-1); // explot 2's complement, largest size_t
    } else {
        printf("\n");
        usage();
    }
    matarr_free(arr);
    return ret;
}

static void
list_file(char *filename, struct matarray arr, size_t i)
{
    // deal with list reallocation
    if (i == arr.length) {
        arr.length *= 2;
        arr.data = safe_realloc(arr.data, arr.length);
    }

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
list_option(char *str, struct matarray arr, size_t i)
{
    // deal with list reallocation
    if (i == arr.length) {
        arr.length *= 2;
        arr.data = safe_realloc(arr.data, arr.length);
    }

    struct matrix bin_stats = filenames_to_stats(str);
    matarr_set(arr, i, bin_stats);
    mat_free(bin_stats);
}

static void
print_comparison(struct matrix m)
{
    printf("|%10s|", "x");
    for (size_t i = 0; i < m.len1; i++) {
        printf("%10zu|", i);
    }
    printf("\n");
    printf("|----------|");
    for (size_t i = 0; i < m.len1; i++) {
        printf("----------|");
    }
    printf("\n");
    for (size_t i = 0; i < m.len1; i++) {
        printf("|%10zu|", i);
        for (size_t j = 0; j < m.len1; j++) {
            printf("%10.4e|", mat_get(m, i, j));
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
        list_file(argv[i], replicate_stats, nreplicates);
        nreplicates++;
    }

    // done parsing args

    struct matrix comparison;
    if (mflag == ONED)
        comparison = compare_1d(replicate_stats);
    else
        comparison = compare_2d(replicate_stats);
    matarr_free(replicate_stats);
    print_comparison(comparison);
    mat_free(comparison);
    return 0;
}


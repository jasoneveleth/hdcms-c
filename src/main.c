#include "fixwindows.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <getopt.h>
#include "main.h"
#include "util/array.h"
#include "util/bin.h" // oned
#include "util/peak.h" // twod

#define DEFAULT_WIDTH (0.1)

enum mode {ONED, TWOD};
static char *argv0;
static double width = DEFAULT_WIDTH; // defaults
static int mflag = ONED; // defaults

noreturn static void
usage(void) {
    printf("USAGE:\n");
    printf("\t%s [--1d|--2d] [-h] --list=path1[,path2,...] FILE ...\n", argv0);
    printf("\n");
    printf("ARGS:\n");
    printf("\t%-30sA file with a list of replicate measurements.\n", "FILE");
    printf("\t%-30sA list of replicate measurements.\n", "--list=path1[,path2,...]");
    printf("\n");
    printf("Enter as many compounds as you want in either \"--list\" or \"FILE\" format.\n");
    printf("One compound per \"--list\" or \"FILE\" entry. The measurements must be\n");
    printf("text files with 1 coordinate per line\n");
    printf("\n");
    printf("OPTIONS:\n");
    printf("\t%-30sSwitch for low resolution data (default).\n", "--1d, --low-res");
    printf("\t%-30sSwitch for high resolution data.\n", "--2d, --high-res");
    printf("\t%-30sShow this help message\n", "-h, --help");
    printf("\n");
    exit(2);
}

static inline double
compare_compound(const struct matrix m1, const struct matrix m2, int mflag)
{
    if (mflag == ONED) {
        return prob_dot_prod(m1, m2);
    } else if (mflag == TWOD) {
        size_t n = m2.len1; // >= longest possible length
        return peak_sim_measure_L2(m1, m2, n);
    } else {
        printf("\n");
        usage();
    }
}

static struct matrix
compare_all(const struct matarray arr, int mflag)
{
    struct matrix m = mat_zeros(arr.length, arr.length);
    for (size_t i = 0; i < arr.length; i++) {
        for (size_t j = 0; j <= i; j++) {
            double sim = compare_compound(matarr_get(arr, i), matarr_get(arr, j), mflag);
            mat_set(m, i, j, sim);
            mat_set(m, j, i, sim);
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
filenames_to_stats(char *str, int mflag)
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
    struct matrix stats;
    if (mflag == ONED) {
        stats = bin_stat_1D(arr, width);
    } else if (mflag == TWOD) {
        size_t n = matarr_get(arr, 0).len1; // >= longest possible length
        stats = peak_stat(arr, n);
    } else {
        printf("\n");
        usage();
    }
    matarr_free(arr);
    return stats;
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

    struct matrix stats = filenames_to_stats(buffer, mflag);
    free(buffer);
    matarr_set(arr, i, stats);
}

static void
list_option(char *str, struct matarray arr, const size_t i)
{
    struct matrix stats = filenames_to_stats(str, mflag);
    matarr_set(arr, i, stats);
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
    printf("------");
    for (size_t i = 0; i < m.len1; i++) {
        printf("-----------");
    }
    printf("\n");

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

    printf("------");
    for (size_t i = 0; i < m.len1; i++) {
        printf("-----------");
    }
    printf("\n");
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
        { "low-res",    no_argument, /*changes*/NULL,           ONED },
        { "2d",         no_argument, /*changes*/NULL,           TWOD },
        { "2D",         no_argument, /*changes*/NULL,           TWOD },
        { "high-res",   no_argument, /*changes*/NULL,           TWOD },
        { "list",       required_argument,      NULL,           'l' },
        { "help",       no_argument,            NULL,           'h' },
        { "quiet",      no_argument,            NULL,           'q' },
        { "width",      required_argument,      NULL,           'w' },
        { 0,            0,                      0,              0 }
    };
    // set 1d / 2d flag (you aren't allow to initilize a structure with non
    // const expression)
    longopts[0].flag = longopts[1].flag = longopts[2].flag = longopts[3].flag = longopts[4].flag = longopts[5].flag = &mflag;

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

    struct matrix comparison = compare_all(replicate_stats, mflag);
    matarr_free(replicate_stats);

    if (HAS_WINDOWS)
        print_comparison_no_utf8(comparison);
    else
        print_comparison(comparison);

    mat_free(comparison);
    return 0;
}


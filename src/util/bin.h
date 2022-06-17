#ifndef BIND_H
#define BIND_H
#include <stdbool.h>
#include "array.h"

/* minimum of 2 and 3 size_t's respectively */
size_t min2(const size_t x, const size_t y);
size_t min3(const size_t x, const size_t y, const size_t z);

/*
 * Scales input data (y-values) between 0 and 1
 *
 * input: nx2 matrix of a spectra measurement coordinates
 * output: none
 *
 * Note: mutates the matrix in place.
 */
void scaled_data(const struct matrix m);

/*
 * This function bins the spectra m (an array of (x,y) pairs) into 9000 bins
 *
 * inputs: nx2 matrix of spectra measurement coordinates
 * outputs: vector of the hieght of the spectra in 9000 bins
 *
 * Takes each coordinate and finds the bin it belongs to. If two elements are
 * in the same bin, we take the one we encountered last. Note that we use
 * (900. / width) as the number of bins, a well tested width to use is 0.1.
 */
struct vec spec_vec(const struct matrix m, double width);

/*
 * This function returns summary statistics for the replicates (each of the
 * bins's mean/std)
 *
 * inputs: array of replicate spetra
 * outpus: 9000x2 array of mean and std for each bin of the binned replicate
 *         measurements.
 *
 * The strategy here is too make a temporary matrix which stores all the outputs
 * of the `spec_vec` calls as rows and call vec_mean and vec_std on the columns.
 */
struct matrix bin_stat_1D(const struct matarray A, double width);

/*
 * This measures the similarity between bin_stats.
 *
 * inputs: two nx2 arrays which are the mean and stadard deviation of each of
 *         the bins
 * output: double representing the similarity between them
 *
 * `prob_dot_prod` treats each of the rows as represeting two real-valued
 * functions (1D gaussian distributions, with those parameters), which it
 * measures the angle betwee in L2. Read the `peak.h` documentation to get a
 * better understanding of how the metric in L2 space works. We end up with a
 * very similar formula as the 2D case:

     $$angle(u_i, v_i, s_{u_i}, s_{v_i}) = \sqrt{\frac{2s_{v_i}s_{u_i}}{s_{v_i}^2 + s_{u_i}^2}}e^{-\frac{1}{2}\left(\frac{(u_i-v_i)^2}{s_{v_i}^2 + s_{u_i}^2}\right)}$$
 
 * The return value of the routine is this expression:

     $$\frac{\sum_{i=1}^{n}u_iv_i angle(u_i, v_i, s_{u_i}, s_{v_i})}{\sum_{i=1}^{n}u_iv_i}$$

 * , which is the weighted average of the products of the means by the
 * similarity of the functions.
 */
double prob_dot_prod(const struct matrix u, const struct matrix v);

#endif // BIN_H


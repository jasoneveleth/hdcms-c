#ifndef PEAK_H
#define PEAK_H
#include <stdbool.h>
#include "array.h"

// min of two values
size_t min2(const size_t x, const size_t y);
// min of three values
size_t min3(const size_t x, const size_t y, const size_t z);


/*
 * # READ THIS FIRST #
 * 
 * It is important to understand at a high level what is happening here. The goal
 * of this file is to compare replicate measurements of two compounds. It turns
 * those compounds into "peak statistics" which are essentially the important
 * features of the mass spectra. Each of those peaks is a function in 
 * $$L^2(\mathbb{R}^2)$$, so let's break that down. The $$\mathbb{R}^2$$ means that
 * the functions are of two variables which we can call x and y. These variables
 * are real valued. The $$L^2$$ part means the functions have a finite integral
 * when you square them. Thus, we are interested in using the peaks as functions in
 * the space $$L^2(\mathbb{R}^2)$$, where we can measure their similarity.
 * 
 * To meausre two function's similarities, we can use this formula to assist in taking the
 * "angle" between them: $$<f,g> = \int \int_{\mathbb{R}^2} f(x) g(x) dx dy$$. You
 * can pretend that we are taking the inner product of these functions as if they were
 * vectors. Recall that to get $$cos(\theta)$$, you need to take
 * $$\frac{<f,g>}{||f|| ||g||} = \frac{<f,g>}{\sqrt{<f,f>} \sqrt{<f,f>}}$$.
 *
 * Coming back to the peak statistics, we can summarize the peaks by their mean
 * and standard deviation for x and y values. This gives 4 values `u` which
 * summarize a peak. We interpret those values as the function:
 *
 * u = { x_mean, y_mean, x_std, y_std }
 *
 * $$f(x,y) = e^{-(1/2)[(\frac{x-u_1}{u_3})^2 + (\frac{y-u_2}{u_4})^2]}$$
 * 
 * This is the function in $$L^2$$ we are using. This strategy underpins the
 * explanations below.
 */

/*
 * inputs: two nx4 matrices which are peak statistics
 * output: the similarity of the two matrices
 *
 * The input matrices are generated by calls to `peak_stat`. The columns are
 * x_mean, y_mean, x_std, y_std. Keep in mind these matrices num_rows can be
 * greater than or less than `n`, and we will use the min of `m1.len2`, `m2.len2`,
 * and `n` as n moving forward. 
 *
 * The `desingularization` is the amount we add to the standard deviation to avoid
 * divide by 0 errors.
 *
 * The `xtol` is the amount of distance in the x direction that we permit for
 * being part of the same peak.
 *
 * We compare the most important (highest) peak in both matrices with all the
 * peaks in the other matrix. We select the best peak in the other matrix by
 * similarity. Repeat until n peaks have been matched. We use the similalrity
 * scores as the weights for a weighted average. We are calculating the weighted
 * average of the products of the y-values of matched peaks. We return that
 * weighted average. Which is a similarity measure between the two compounds.
 */
double peak_sim_measure_L2(const struct matrix m1, const struct matrix m2, double desingularization, size_t n, double xtol);

/*
 * Measures the similarity of two peaks.
 *
 * inputs: two peak statistics u and v
 *              u = { u_1, u_2, u_3, u_4 }
 *              v = { v_1, v_2, v_3, v_4 }
 * output: similarity of the two peaks
 *
 * This function first adds desingularization, to both the stdev's to avoid having an
 * ill-conditioned peak. Then, measures the angle between the two functions:

    $$f_1(x,y) = e^{-(1/2)[(\frac{x-u_1}{u_3})^2 + (\frac{y-u_2}{u_4})^2]}$$
    $$f_2(x,y) = e^{-(1/2)[(\frac{x-v_1}{v_3})^2 + (\frac{y-v_2}{v_4})^2]}$$

 * We simply evaluate the inner product of the two functions (i.e. we
 * integrate), and divide by their norm (recall we want $$\frac{<f,g>}{||f|| ||g||}$$),
 * which remarkably gives the expression:
  
    $$\sqrt{\frac{2 u_2 v_2}{u_2^2 + v_2^2}} \sqrt{\frac{2 u_3 v_3}{u_3^2 + v_3^2}} e^{-\frac{1}{2} \left(\frac{(u_0 - v_0)^2}{u_2^2 + v_2^2} + \frac{(u_1 - v_1)^2}{u_3^2 + v_3^2}\right)}$$

 * `cos_sim_L2` is just evaluating that expression.
 */
double cos_sim_L2(const struct vec u, const struct vec v, double desingularization);

/*
 * Creates a list of peaks with each element of the list being the coordinates
 * of that peak in each of the replicate spectra.
 *
 * inputs: array of matrices of spectra
 *         number of peaks
 * outputs: n matrices s.t. ith matrix is pts assoc. with ith largest peak
 *
 * We first find the largest coordinate by y-value in all the replicates, then
 * find the nearest coordinate in the other replicates to it. We say that
 * collection of coordinates is a peak, and put it first into the list we will
 * return. We remove the coordinates we used, and repeat the process again. (We
 * don't actually remove the peak, since that would be expensive, rather we set
 * it's y-value to -inf so it is never the closest or highest peak again.)
 *
 * This is a helper routine for peak_stat.
 *
 * See abox for `xtol` description.
 */
struct matarray peak_sort(const struct matarray replicates, size_t n, double xtol);

/*
 * Creates the peak statistics matrix from a list of peaks.
 *
 * inputs: array of matrices of spectra
 *         number of peaks
 * outputs: nx4 matrix of the mean and standard deviations for x and y for each of the n peaks
 *
 * We call `peak_sort` to get a list of the coordinates of the peaks. Then we
 * find the mean of the x-values of the coordinates, mean of the y-values, std
 * of the x-values, std of the y-values. These are the 4 columns of the matrix
 * we are returning. The ith row of the matrix we return is the statistics for
 * the ith peak from the call to `peak_sort`.
 *
 * See abox for `xtol` description.
 */
struct matrix peak_stat(const struct matarray replicates, size_t n, double xtol);

#endif // PEAK_H

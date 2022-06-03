# Note on stability

One of the issues with the 1D case is the fact that we are
binning. It isn't a problem in teh way you expect: that there is
something you miss by having important stuff between bins (also
the fact that our algorithm doesn't select the max, but rather
the last number in the bin). The problem is the `linspace()`
function. If you don't have bit perfect doubles coming out of
your `linspace` function you get very different results. For
example, MATLAB uses a different `linspace` than the one you
would expect (multiplying the step function), or the one that you
would next expect (addition of stepsize), or the next (symmetric
subtraction of multiples of stepsize). So if the library used one
linspace and you're using another to decide bins (even with the
same spacing and stuff) produces very different `spec_vec`s.

The problem was I was calculating elements of the linspace using
`i * ((end - start) / (n - 1))`, MATLAB was using 
`(i * (end - start)) / (n - 1)`.

# Install

For mac and linux:

```bash
$ mkdir build
$ cd build
$ cmake ..
$ make
$ ./test_runner
$ ./pmcs
```

For windows (windows terminal; so not cmd, nor powershell):

```powershell
> mkdir build
> cd build
> cmake ..
> msbuild pmcs.sln
> Debug\test_runner
> Debug\pmcs
```

# Library

`peak_sim_measure_L2`
2 4xn matrices (output of `peak_stat`), number of peaks -> similarity of them btwn
0 and 1

`peak_stat`
5 matrices of spectra, and number of peaks -> nx4 matrix of peaks

helpers:

`cos_sim_L2`
2 4-length vectors -> cos of angle between gaussians those vectors represent

`peak_sort`
5 matrices of spectra, number of peaks -> n matrices s.t. ith matrix is 5
pts assoc. with ith largest peak

# Misc

[gnu scientific lib](https://www.gnu.org/software/gsl/doc/html/vectors.html#c.gsl_vector_view)

# Questions:

# Increasing performance

* http://math-atlas.sourceforge.net/atlas_install/
* http://math-atlas.sourceforge.net/faq.html
* https://stackoverflow.com/questions/1303182/how-does-blas-get-such-extreme-performance

* set all `allocd`s to 128 or something, so the expandable memory
  starts larger
* make `vec_std` use a for loop

# Future test
* equivalence partitioning and boundary testing
  https://www.guru99.com/equivalence-partitioning-boundary-value-analysis.html

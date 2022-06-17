# Note on stability

One of the issues with the 1D case is the fact that we are
binning. It isn't a problem in teh way you expect: that there is
something you miss by having important stuff between bins (also
the fact that our algorithm doesn't select the max, but rather
the last number in the bin). The problem is the `linspace()`
function. If you don't have bit perfect doubles coming out of
your `linspace` function you get very different results. For
example, MATLAB uses a different `linspace` than the one you
would expect (multiplying the step size), or the one that you
would next expect (addition of stepsize), or the next (symmetric
subtraction of multiples of stepsize). So if the library used one
linspace and you're using another to decide bins (even with the
same spacing and stuff) produces very different `spec_vec`s.

The problem was I was calculating elements of the linspace using
`i * ((end - start) / (n - 1))`, MATLAB was using 
`(i * (end - start)) / (n - 1)`.

# Install

For mac and linux:

Install cmake and git.

```bash
$ git clone XXXXXXXXXXXX
$ cd XXXX
$ mkdir build
$ cd build
$ cmake ..
$ make
$ ./test_runner
$ ./pmcs
```

For windows:

Download Git, CMake, and Visual Studio (with C++ CMake tools for Windows,
and the latest SDK). Add MSBuild, CMake, and Git to your path. Then run
this in powershell

```powershell
> git clone XXXXXXXXXX
> cd XXXXXXX
> mkdir build
> cd build
> cmake ..
> msbuild pmcs.sln
> Debug\test_runner
> Debug\pmcs
```

# Increasing performance

* remove `vec_copy()` calls in `prob_dot_prod`
* remove `mat_copy()` in `peak_sim_measure_L2` (since the input
  is the output of `peak_stat`?)
* align malloc and realloc
* cache a 9000 `vec_arange` and reuse it, or make one even bigger
  and just make the lenght of the vector stop before the end
* don't use a temp matrix in `bin_stat_1D`, rather, recode the
  standard deviation and mean from scratch on an array of
  vectors, or make the `spec_vec` function take in an answer
  array and fill that out rather than allocating separate one

* mmap file rather than `read_line()` from it
* use gnu wc -l or wc -w method
* store floats in files as hex (or all of these)

## BLAS

* [ATLAS install](http://math-atlas.sourceforge.net/atlas_install/)
* [ATLAS FAQ](http://math-atlas.sourceforge.net/faq.html)
* [STACKOVERFLOW BLAS](https://stackoverflow.com/questions/1303182/how-does-blas-get-such-extreme-performance)
* [BLIS](https://www.cs.utexas.edu/users/flame/pubs/blis1_toms_rev3.pdf)

Possibly try binning and taking the max rather than the most
recent measurement in `spec_vec`.


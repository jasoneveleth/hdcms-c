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

# Future test
* equivalence partitioning and boundary testing
  https://www.guru99.com/equivalence-partitioning-boundary-value-analysis.html

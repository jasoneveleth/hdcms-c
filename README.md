2 big functions

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


### Questions:

* How does integration work for `<f1,f2>/(|f1| |f2|)`
* Why do we add 1e-4 to all std?

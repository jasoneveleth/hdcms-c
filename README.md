<!-- FIXME TODO -->
CHANGE `double` to `size_t` in `vec_argmax`

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

* How does integration work for `<f1,f2>/(|f1| |f2|)`, also how did you
  figure it out?
* Why do we add 1e-4 to all std?
* what happens when:

```
A = {[72 0.68; 97 0.43; 89 0.27; 100 0.1;],
     [83 0.23; 92 0.47; 79 0.61; 110 0.5],
     [0 0; 0 0; 0 0; 0 0],
     [0 0; 0 0; 0 0; 0 0],
     [0 0; 0 0; 0 0; 0 0]}
peak_sort(A, 4)
```
this introduces 2 undefined behaviors. First, there could be multiple
peaks the same distance away, I think this can be solved by the line `i_min
= i_min(1);`. The second behavior is when you set the peak height to 0, it
seems like you don't want to match the same peak multiple times, but that
is what happens now.

* using `n` rather than the lenght of the array (which isn't the max) causes
  you to ignore some elements
* How does data entry work
* How to compile on windows

## Possible matching algorithms

- https://cs.stanford.edu/~amirz/index_files/PAMI_Amir%20Zamir.pdf
- https://www.sciencedirect.com/science/article/pii/S0031320398000867
- https://reader.elsevier.com/reader/sd/pii/S0031320398000867?token=8E1C5F0F262998D51E6F8C5F8350FD3DFF47B7582B67064482E9420ED8F31FF8161F240EA9434F8369FF812C383BCC28&originRegion=us-east-1&originCreation=20220105183901

## Increasing performance

* http://math-atlas.sourceforge.net/atlas_install/
* http://math-atlas.sourceforge.net/faq.html
* https://stackoverflow.com/questions/1303182/how-does-blas-get-such-extreme-performance

## Install for windows

Run this before `setup.ps1`:
```powershell
Set-ExecutionPolicy RemoteSigned
```
And make sure you run it as administrator

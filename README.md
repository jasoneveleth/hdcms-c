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

## Unix (mac and linux)

Install cmake and git.

```bash
$ git clone XXXXXXXXXXXX
$ cd XXXX
$ mkdir build
$ cd build
$ cmake ..
$ make
$ ./test_runner
$ ./prob_mass_spec
```

## Windows

Download Git, CMake, and Visual Studio (with C++ CMake tools for Windows,
and the latest SDK). Add MSBuild, CMake, and Git to your path.
For me they were:

* `C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin`
* `C:\Program File\CMake\bin`
* `C:\Program Files\Git\bin`

Then download [getopt.h from here](https://raw.githubusercontent.com/skandhurkat/Getopt-for-Visual-Studio/master/getopt.h)
, then place it where your includes are, for me this was
`C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\ucrt`. 
But I think you can go into Visual Studio and use the VC folder
too.

Then run this in powershell

```powershell
> git clone XXXXXXXXXX
> cd XXXXXXX
> mkdir build
> cd build
> cmake ..
> msbuild prob_mass_spec.sln
> .\test_runner
> .\prob_mass_spec
```

# Design

My `src/` directory is split up into 3 pieces: main (which has
main.c -- functions associated with CLI), test (which has test.c
-- all the test functions), and util (which has array.c, bin.c,
peak.c). I split them up like this so I could easily craft the
cmake recipe for the two executables; I could use `util/*.c` and
`main/*.c` for `prob_mass_spec`, and `util/*.c` and `test/*.c`
for `test_runner`. This way we don't get different `main()`
symbols when we're linking.

In `util/`, I have my array library `array.c`. In `bin.c` I have
the functions associated with the low resolution data; the
similarity function there is `prob_dot_prod`. In `peak.c` I have
the functions associated with the high resolution data; the
similarity function there is `peak_sim_measure_L2`.

Inside `test/data/` are all the text files with data in them.
Also, there's one c file, `data.c`, which is included in
`test.c`. It has all the arrays that I didn't want cluttering up
my test file.

# Increasing performance

* store floats in files as hex (make a converter)
* don't use a temp matrix in `bin_stat_1D`, rather, recode the
  standard deviation and mean from scratch on an array of
  vectors, or make the `spec_vec` function take in an answer
  array and fill that out rather than allocating separate one

* align malloc and realloc of large arrays -- probably doesn't matter
* cache a 9000 `vec_arange` and reuse it, or make one even bigger
  and just make the lenght of the vector stop before the end
* mmap file / read (gnu wc method) in blocks rather than
  `read_line()` from it
  [1](https://stackoverflow.com/questions/17925051/fast-textfile-reading-in-c)
  [2](https://stackoverflow.com/questions/17465061/how-to-parse-space-separated-floats-in-c-quickly/17479702#17479702)

## BLAS

* [ATLAS install](http://math-atlas.sourceforge.net/atlas_install/)
* [ATLAS FAQ](http://math-atlas.sourceforge.net/faq.html)
* [STACKOVERFLOW BLAS](https://stackoverflow.com/questions/1303182/how-does-blas-get-such-extreme-performance)
* [BLIS](https://www.cs.utexas.edu/users/flame/pubs/blis1_toms_rev3.pdf)

Possibly try binning and taking the max rather than the most
recent measurement in `spec_vec`.


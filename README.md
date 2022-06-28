# High Dimensional Consensus Mass Spec - HDCMS

# TODO

[minmax](https://data.nist.gov/od/id/mds2-2418)

### python bindings

* https://dev.to/erikwhiting88/how-to-use-c-functions-in-python-7do
* http://docs.cython.org/en/latest/src/userguide/external_C_code.html
* https://stavshamir.github.io/python/making-your-c-library-callable-from-python-by-wrapping-it-with-cython/
* https://medium.com/@mliuzzolino/wrapping-c-with-python-in-5-minutes-cdd1124f5c01
* https://intermediate-and-advanced-software-carpentry.readthedocs.io/en/latest/c++-wrapping.html
* https://docs.python.org/3/extending/extending.htmlhttps://opensource.com/article/19/5/how-write-good-c-main-function[how to write a good c main function]

# Install

## Unix (mac and linux)

Install cmake and git.

```bash
$ git clone https://gitlab.nist.gov/gitlab/jje4/hdcms.git
$ cd hdcms
$ mkdir build
$ cd build
$ cmake ..
$ make
$ ./test_runner
$ ./hdcms --1d --list=compound1,compound2 compounds_list
```

## Windows

Download Git, CMake, and Visual Studio (with C++ CMake tools for Windows,
and the latest SDK). Add MSBuild, CMake, and Git to your path.
For me they were:

* `C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin`
* `C:\Program File\CMake\bin`
* `C:\Program Files\Git\bin`
* optional (unix tools): `C:\Program Files\Git\usr\bin`
* optional (cl): `C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.32.31326\bin\Hostx64\x64`

Then download [getopt.h from here](https://raw.githubusercontent.com/skandhurkat/Getopt-for-Visual-Studio/master/getopt.h)
, then place it where your includes are, for me this was
`C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\ucrt`. 
But I think you can go into Visual Studio and use the VC folder
too.

Then run this in powershell

```powershell
> git clone https://gitlab.nist.gov/gitlab/jje4/hdcms.git
> cd hdcms
> mkdir build
> cd build
> cmake ..
> msbuild high_dimensional_consensus_mass_spec.sln
> .\test_runner
> .\hdcms --1d --list=compound1,compound2 compounds_list
```

Notes: Probably irrelevant, but in order to compile something on its own using `cl` (without cmake and everything that is setup for this project), I needed to set up these environmental variables:
```
> $env:INCLUDE = "C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\ucrt;C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.32.31326\include;C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\shared;C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\um;C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\winrt;"
> $env:LIB = "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.22621.0\ucrt\x64;C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.32.31326\lib\x64;C:\Program Files (x86)\Windows Kits\10\Lib\10.0.22621.0\um\x64;"
```
So they might be helpful if things aren't working.

# Design

My `src/` directory has `main.c` (functions associated with CLI),
`test.c` (all the test functions), and `util/` (which has array.c,
bin.c, peak.c). 

In `src/util/`, I have my array library `array.c`. In `bin.c` I have
the functions associated with the low resolution data; the
similarity function there is `prob_dot_prod`. In `peak.c` I have
the functions associated with the high resolution data; the
similarity function there is `peak_sim_measure_L2`.

Inside `data/` are all the text files with data in them.
Also, there's one c file, `data.c`, which is included in
`test.c`. It has all the arrays that I didn't want cluttering up
my test file.

## Parameters

For most of the functions I pass the struct by value. One
advantage to this is that the callee can't edit the fields of
the struct in the caller (although if those fields are pointers,
it can modify what it pointed to). Another advantage is you don't
need to NULL check for proper defensive programming because you
know that you are recieving a struct. The only
disadvantage is if you don't need all the fields of the struct
then the compiler is copying extra things onto the stack, which
could cause a stack overflow if you only have 4KB of stack space
(maximum depth of 10, say 10 matrix structs on the stack for each
call (40 bytes each) ~4000 bytes, but this is a generous
upperbound), but a stack that small is unlikely (musl c has the
smallest stack space I can find of 128KB).

# Increasing performance

* store floats in files as hex (make a converter)
* don't use a temp matrix in `bin_stat_1D`, rather, recode the
  standard deviation and mean from scratch on an array of
  vectors, or make the `spec_vec` function take in an answer
  array and fill that out rather than allocating separate one

* align malloc and realloc of large arrays -- probably doesn't matter
* cache a 9000 `vec_arange` and reuse it, or make one even bigger
  and just make the lenght of the vector stop before the end

Possibly try binning and taking the max rather than the most
recent measurement in `spec_vec`.

only calculate half the table and write to both addresses at the same time

## Flame graphs

### Linux

Setup: 

```bash
$ git clone git@github.com:brendangregg/FlameGraph
$ cd /path/to/bin
$ ln -s /path/to/FlameGraph/stackcollapse-perf.pl
$ ln -s /path/to/FlameGraph/flamegraph.pl
```

Record and visualize:

```
$ # -F 99 so we don't sample at a regular interval, idk why -g. \
  # Also, I has to allow myself to use `perf` without sudo by   \
  # running `sudo visudo` and adding `NOPASSWD: /bin/perf` after\
  # the `ALL= ` for my user's entry. If you can't do this, then \
  # just `chown` the `perf.data` file.                          \
$ perf record -F 99 -g -- ./test_runner
$ perf script | stackcollapse-perf.pl > out.perf-folded
$ flamegraph.pl out.perf-folded > perf.svg
$ # then open up the svg in a browser and click around to zoom in
```

### Windows

Setup: download Windows Performance Recorder and Windows
Performance Analyzer.

Then open Windows Performance Recorder and click Start. Then run
the powershell command `./test_runner` or whatever you are
profiling. Name the file. Open the file in Windows Performance 
Analyzer. Go to Trace > Load Symbols, this may take like 10
minutes. Then go to Computation > CPU Usage (Sampled) > Flame by
Process. Right click on an entry in the flame graph and select
Filter To Flame to zoom in.

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


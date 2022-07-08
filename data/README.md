# Description of the data

## CM1

28x10 measurements with replicate measurements of 28 differnt
compounds. CM1_i_j.txt has mass spectra of compound i measurement
j. i = 1:28, j = 1:10 (matlab notation).

<!-- ## CM2 -->

<!-- 58x10 measurements with replicate measurements of 58 differnt --> 
<!-- compounds. CM2_i_j.txt has mass spectra of compound i -->
<!-- measurement j. i = 1:58, j = 1:10 (matlab notation). -->

<!-- ## CM3 --> 

<!-- 49x10 measurements with replicate measurements of 49 differnt --> 
<!-- compounds. CM3_i_j.txt has mass spectra of compound i -->
<!-- at measurement j. i = 1:49, j = 1:10 (matlab notation). -->

## analytes_normal

Ed has provided 5 replicate measurements for 14 different
compounds of interest. He pulled centroided data (the commonly
used format) and profile mode (the less commonly used format).
The 14 compounds represent 7 pairs of isomers.

Pairs to discriminate:

* 1. Cotinine and 2. Serotonin
* 3. Phenibut and 4. MDA
* 5. MMDPPA and 6. Methylone
* 7. 5-methoxy MET and 8. Norfentanyl
* 9. Cocaine and 10. Scopolamine
* 11. HU-210 and 12. Testosterone Isocaprionate
* 13. Methamphetamine and 14. Phentermine.

The filenames are analytes_normal_i_j_k.txt, using matlab
notation, i = 1:14 (14 compounds), j = 1:5 (replicate
measurements), k = 1:3 (energy levels 30V, 60V, 90V respectively)

## bin_stats_CM1

Files which test the intermediate output of `bin_stat_1D()`, or
test `prob_dot_prod` from the intermediate output.

## spec_vec_CM1

Files which test the intermediate output of `spec_vec()`.

## data.c

A file full of arrays that I didn't want cluttering up test.c

## mat_* no_newline oneline vec_read*

files to test `mat_from_file` and `vec_from_file` edge cases


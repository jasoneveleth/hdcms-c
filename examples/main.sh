#!/bin/sh

../build/hdcms --1d                                                            \
    --list=../data/CM1_10_1.txt,../data/CM1_10_2.txt,../data/CM1_10_3.txt      \
    --list=../data/CM1_8_1.txt,../data/CM1_8_2.txt,../data/CM1_8_3.txt         \
    --list=../data/CM1_9_1.txt,../data/CM1_9_2.txt,../data/CM1_9_3.txt

../build/hdcms --1d ./compound1_low_res.txt --list=../data/CM1_9_1.txt,../data/CM1_9_2.txt,../data/CM1_9_3.txt

../build/hdcms --1d ./compound1_low_res.txt ./compound2_low_res.txt ./compound3_low_res.txt

# ../build/hdcms --2d

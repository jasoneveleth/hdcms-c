import hdcms
from visualize import write_image
import os
import numpy as np

np.set_printoptions(linewidth=200)

# --------------------------------------------------------

cm1 = "../data/CM1_1_1.txt,../data/CM1_1_2.txt,../data/CM1_1_3.txt,../data/CM1_1_4.txt,../data/CM1_1_5.txt,../data/CM1_1_6.txt,../data/CM1_1_7.txt"
cm2 = "../data/CM1_2_1.txt,../data/CM1_2_2.txt,../data/CM1_2_3.txt,../data/CM1_2_4.txt,../data/CM1_2_5.txt,../data/CM1_2_6.txt,../data/CM1_2_7.txt"
cm3 = "../data/CM1_3_1.txt,../data/CM1_3_2.txt,../data/CM1_3_3.txt,../data/CM1_3_4.txt,../data/CM1_3_5.txt,../data/CM1_3_6.txt,../data/CM1_3_7.txt"

cm1_stats = hdcms.filenames_to_stats_1d(cm1)
cm2_stats = hdcms.filenames_to_stats_1d(cm2)
cm3_stats = hdcms.filenames_to_stats_1d(cm3)

# ../build/hdcms --1d compound1_low_res.txt compound2_low_res.txt compound3_low_res.txt
# print(hdcms.compare_all_1d([cm1_stats, cm2_stats, cm3_stats]))

# --------------------------------------------------------
import sys
import os

# extraction functions
xlineno = lambda tb: tb.tb_lineno
xname = lambda tb: tb.tb_frame.f_code.co_name
def xfname(tb):
    abs_path = tb.tb_frame.f_code.co_filename
    rel_path = os.path.relpath(abs_path, os.getcwd())
    argmin = lambda x, y: x if len(x) < len(y) else y
    return argmin(abs_path, rel_path)
def excepthook(exc_type, exc_value, tb):
    print('==================================================================')
    print('Traceback:')
    while tb:
        # lineno is left justified, width of 3
        print(f"{xfname(tb)}:{xlineno(tb):<3} in {xname(tb)}")
        tb = tb.tb_next

    r = "\x1b[31m"; p = "\x1b[35m"; c = "\x1b[0m" # ansi colors
    print(f"{r}{exc_type.__name__}{c}: {p}{exc_value}{c}")
sys.excepthook = excepthook
# this may break on windows

filename_list = ""

file = "./compound1_high_res.txt"
with open(file) as f:
    for line in f:
        filename_list += line + ","

high_res = hdcms.filenames_to_stats_2d(filename_list)

lstats = []
for i in range(1, 14):
    compound = ""
    for j in range(1, 5):
        compound += f"../data/analytes_normal_{i}_{j}_1.txt,"

    stats = hdcms.filenames_to_stats_2d(compound)
    lstats.append(stats)
    write_image(stats, f"analyes_{i}.png")

# print(hdcms.compare_all_2d(lstats))



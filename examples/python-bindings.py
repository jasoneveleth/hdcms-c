import hdcms
import numpy as np
import sys
np.set_printoptions(linewidth=200, threshold=sys.maxsize, precision=2)

write_image = lambda x,y: (x,y)


# Manually write out the files
# ============================
cm1 = "../data/CM1_1_1.txt,../data/CM1_1_2.txt,../data/CM1_1_3.txt,../data/CM1_1_4.txt,../data/CM1_1_5.txt,../data/CM1_1_6.txt,../data/CM1_1_7.txt"
cm2 = "../data/CM1_2_1.txt,../data/CM1_2_2.txt,../data/CM1_2_3.txt,../data/CM1_2_4.txt,../data/CM1_2_5.txt,../data/CM1_2_6.txt,../data/CM1_2_7.txt"
cm3 = "../data/CM1_3_1.txt,../data/CM1_3_2.txt,../data/CM1_3_3.txt,../data/CM1_3_4.txt,../data/CM1_3_5.txt,../data/CM1_3_6.txt,../data/CM1_3_7.txt"
cm4 = "../data/CM1_4_1.txt,../data/CM1_4_2.txt,../data/CM1_4_3.txt,../data/CM1_4_4.txt,../data/CM1_4_5.txt,../data/CM1_4_6.txt,../data/CM1_4_7.txt"
cm5 = "../data/CM1_5_1.txt,../data/CM1_5_2.txt,../data/CM1_5_3.txt,../data/CM1_5_4.txt,../data/CM1_5_5.txt,../data/CM1_5_6.txt,../data/CM1_5_7.txt"
cm6 = "../data/CM1_6_1.txt,../data/CM1_6_2.txt,../data/CM1_6_3.txt,../data/CM1_6_4.txt,../data/CM1_6_5.txt,../data/CM1_6_6.txt,../data/CM1_6_7.txt"
cm7 = "../data/CM1_7_1.txt,../data/CM1_7_2.txt,../data/CM1_7_3.txt,../data/CM1_7_4.txt,../data/CM1_7_5.txt,../data/CM1_7_6.txt,../data/CM1_7_7.txt"
cm8 = "../data/CM1_8_1.txt,../data/CM1_8_2.txt,../data/CM1_8_3.txt,../data/CM1_8_4.txt,../data/CM1_8_5.txt,../data/CM1_8_6.txt,../data/CM1_8_7.txt"
cm9 = "../data/CM1_9_1.txt,../data/CM1_9_2.txt,../data/CM1_9_3.txt,../data/CM1_9_4.txt,../data/CM1_9_5.txt,../data/CM1_9_6.txt,../data/CM1_9_7.txt"

cm1_stats = hdcms.filenames_to_stats_1d(cm1)
cm2_stats = hdcms.filenames_to_stats_1d(cm2)
cm3_stats = hdcms.filenames_to_stats_1d(cm3)
cm4_stats = hdcms.filenames_to_stats_1d(cm4)
cm5_stats = hdcms.filenames_to_stats_1d(cm5)
cm6_stats = hdcms.filenames_to_stats_1d(cm6)
cm7_stats = hdcms.filenames_to_stats_1d(cm7)
cm8_stats = hdcms.filenames_to_stats_1d(cm8)
cm9_stats = hdcms.filenames_to_stats_1d(cm9)

# ../build/hdcms --1d compound1_low_res.txt compound2_low_res.txt compound3_low_res.txt
cmp = hdcms.compare_all_1d([cm1_stats, cm2_stats, cm3_stats, cm4_stats, cm5_stats, cm6_stats, cm7_stats, cm8_stats, cm9_stats])
print(cmp)

cm1_2d = hdcms.filenames_to_stats_2d(cm1)

# Using write_img on both 1d and 2d data
# ===============
write_image(cm1_stats, "img/1d.png")

write_image(cm1_2d, "img/2d.png")

# Using for loops to construct filenames
# ======================================
lstats = []
for i in range(1, 14):
    compound = ""
    for j in range(1, 5):
        compound += f"../data/analytes_normal_{i}_{j}_1.txt,"

    stats = hdcms.filenames_to_stats_2d(compound)
    lstats.append(stats)
    write_image(stats, f"img/analyes_{i}.png")


# Using a config file to get filenames
# ====================================
filename_list = ""

file = "./compound1_high_res.txt"
with open(file) as f:
    for line in f:
        filename_list += line + ","
high_res = hdcms.filenames_to_stats_2d(filename_list)

# Using a numpy array rather than a file
# ======================================
from data import cm1_3_stats, high_res_data
low_res = cm1_3_stats

filename = "img/low_res.png" if len(sys.argv) < 2 else sys.argv[1]
write_image(low_res, filename)

filename = "img/high_res.png" if len(sys.argv) < 3 else sys.argv[2]
write_image(high_res_data, filename)

# Using a regex to get filenames
# ==============================
import re
import os
def regex2filenames(regex, dir="."):
    files = [f for f in os.listdir(dir) if os.path.isfile(os.path.join(dir, f))]

    r = re.compile(regex)
    a = []
    for f in files:
        match = r.match(f)
        if match:
            a.append(match.group())

    full_paths = list(map(lambda f: os.path.join(dir, f), a))
    return ','.join(full_paths)

def regex2stats1d(regex, dir="."):
    filenames = regex2filenames(regex, dir)
    return hdcms.filenames_to_stats_1d(filenames)

cm1_stats = regex2stats1d(r"CM1_1_\d.txt", dir="../data")
cm2_stats = regex2stats1d(r"CM1_2_\d.txt", dir="../data")

print(hdcms.compare_compound_1d(cm1_stats, cm2_stats))


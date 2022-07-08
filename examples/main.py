import numpy as np
import hdcms

c10 = hdcms.filenames_to_stats_1d("../data/CM1_10_1.txt,../data/CM1_10_2.txt,../data/CM1_10_3.txt")
c8 = hdcms.filenames_to_stats_1d("../data/CM1_8_1.txt,../data/CM1_8_2.txt,../data/CM1_8_3.txt")
c9 = hdcms.filenames_to_stats_1d("../data/CM1_9_1.txt,../data/CM1_9_2.txt,../data/CM1_9_3.txt")

print("compound 10 vs compound 8")
print(hdcms.compare_compound_1d(c10, c8), "\n")

print("compound 10 vs compound 8 vs compound 9")
print(hdcms.compare_all_1d([c10,c8,c9]))

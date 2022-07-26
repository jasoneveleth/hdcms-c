import hdcms

cm1 = "../data/CM1_1_1.txt,../data/CM1_1_2.txt,../data/CM1_1_3.txt,../data/CM1_1_4.txt,../data/CM1_1_5.txt,../data/CM1_1_6.txt,../data/CM1_1_7.txt"
cm2 = "../data/CM1_2_1.txt,../data/CM1_2_2.txt,../data/CM1_2_3.txt,../data/CM1_2_4.txt,../data/CM1_2_5.txt,../data/CM1_2_6.txt,../data/CM1_2_7.txt"
cm3 = "../data/CM1_3_1.txt,../data/CM1_3_2.txt,../data/CM1_3_3.txt,../data/CM1_3_4.txt,../data/CM1_3_5.txt,../data/CM1_3_6.txt,../data/CM1_3_7.txt"

cm1_stats = hdcms.filenames_to_stats_1d(cm1)
cm2_stats = hdcms.filenames_to_stats_1d(cm2)
cm3_stats = hdcms.filenames_to_stats_1d(cm3)

print(hdcms.compare_all_1d([cm1_stats, cm2_stats, cm3_stats]))
# ../build/hdcms --1d compound1_low_res.txt compound2_low_res.txt compound3_low_res.txt


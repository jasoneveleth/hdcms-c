import numpy as np
import sys
import hdcms
import os

# these lines make relative paths work nicely if you call this from the
# `examples/` directory, project root, or `python-hdcms-package/` subdirectory
# -- remove these lines if you tinker/want to use your own data unless you want
# to specify all paths as realtive to the project root
# proj_root = os.path.realpath(os.path.join(os.path.dirname(os.path.realpath(__file__)), ".."))
# os.chdir(proj_root)

# c10 = hdcms.filenames_to_stats_1d("data/CM1_10_1.txt,data/CM1_10_2.txt,data/CM1_10_3.txt")
# c8 = hdcms.filenames_to_stats_1d("data/CM1_8_1.txt,data/CM1_8_2.txt,data/CM1_8_3.txt")
# c9 = hdcms.filenames_to_stats_1d("data/CM1_9_1.txt,data/CM1_9_2.txt,data/CM1_9_3.txt")

# print("compound 10 vs compound 8")
# print(hdcms.compare_compound_1d(c10, c8), "\n")

# print("compound 10 vs compound 8 vs compound 9")
# print(hdcms.compare_all_1d([c10,c8,c9]))

np.set_printoptions(threshold=sys.maxsize)
wvu = os.path.realpath("/Users/jason/nist/WVU")
os.chdir(wvu)

def ff_instrument(l):
    mfilenames = ""
    ofilenames = ""
    pfilenames = ""
    for instrument in l:
        for day in ["1", "2", "3", "4", "5"]:
            for time in ["a", "b"]:
                mfilenames += f"ff_me/{instrument}_meta-{day}{time}.txt,"
                ofilenames += f"ff_me/{instrument}_ortho-{day}{time}.txt,"
                pfilenames += f"ff_me/{instrument}_para-{day}{time}.txt,"
    return mfilenames, ofilenames, pfilenames

no_a_meta, no_a_ortho, no_a_para = ff_instrument(["B", "C", "D", "E", "F"])
no_b_meta, no_b_ortho, no_b_para = ff_instrument(["A", "C", "D", "E", "F"])
no_c_meta, no_c_ortho, no_c_para = ff_instrument(["A", "B", "D", "E", "F"])
no_d_meta, no_d_ortho, no_d_para = ff_instrument(["A", "B", "C", "E", "F"])
no_e_meta, no_e_ortho, no_e_para = ff_instrument(["A", "B", "C", "D", "F"])
no_f_meta, no_f_ortho, no_f_para = ff_instrument(["A", "B", "C", "D", "E"])

no_a_para = hdcms.filenames_to_stats_1d(no_a_para)
no_a_meta = hdcms.filenames_to_stats_1d(no_a_meta)
no_a_ortho = hdcms.filenames_to_stats_1d(no_a_ortho)

no_b_para = hdcms.filenames_to_stats_1d(no_b_para)
no_b_meta = hdcms.filenames_to_stats_1d(no_b_meta)
no_b_ortho = hdcms.filenames_to_stats_1d(no_b_ortho)

no_c_meta = hdcms.filenames_to_stats_1d(no_c_meta)
no_c_ortho = hdcms.filenames_to_stats_1d(no_c_ortho)
no_c_para = hdcms.filenames_to_stats_1d(no_c_para)

no_d_meta = hdcms.filenames_to_stats_1d(no_d_meta)
no_d_ortho = hdcms.filenames_to_stats_1d(no_d_ortho)
no_d_para = hdcms.filenames_to_stats_1d(no_d_para)

no_e_meta = hdcms.filenames_to_stats_1d(no_e_meta)
no_e_ortho = hdcms.filenames_to_stats_1d(no_e_ortho)
no_e_para = hdcms.filenames_to_stats_1d(no_e_para)

no_f_meta = hdcms.filenames_to_stats_1d(no_f_meta)
no_f_ortho = hdcms.filenames_to_stats_1d(no_f_ortho)
no_f_para = hdcms.filenames_to_stats_1d(no_f_para)

m = hdcms.compare_all_1d([no_a_meta, no_b_meta, no_c_meta, no_d_meta, no_e_meta, no_f_meta])
o = hdcms.compare_all_1d([no_a_ortho, no_b_ortho, no_c_ortho, no_d_ortho, no_e_ortho, no_f_ortho])
p = hdcms.compare_all_1d([no_a_para, no_b_para, no_c_para, no_d_para, no_e_para, no_f_para])

print("max for meta, max for ortho, max for para")
print("min for meta, min for ortho, min for para")
print(np.max(m))
print(np.max(o))
print(np.max(p))
print(np.min(m))
print(np.min(o))
print(np.min(p))
print("====================")


a = hdcms.compare_all_1d([no_a_meta, no_a_ortho, no_a_para])
b = hdcms.compare_all_1d([no_b_meta, no_b_ortho, no_b_para])
c = hdcms.compare_all_1d([no_c_meta, no_c_ortho, no_c_para])
d = hdcms.compare_all_1d([no_d_meta, no_d_ortho, no_d_para])
e = hdcms.compare_all_1d([no_e_meta, no_e_ortho, no_e_para])
f = hdcms.compare_all_1d([no_f_meta, no_f_ortho, no_f_para])
print("max for a, b, c, d, e, f")
print(np.max(a))
print(np.max(b))
print(np.max(c))
print(np.max(d))
print(np.max(e))
print(np.max(f))


print("min for a, b, c, d, e, f")
print(np.min(a))
print(np.min(b))
print(np.min(c))
print(np.min(d))
print(np.min(e))
print(np.min(f))

# # FF

# meta_fluorofentanyl_files = ""
# ortho_fluorofentanyl_files = ""
# para_fluorofentanyl_files = ""
# for instrument in ["A", "B", "C", "D", "E", "F"]:
#     for day in ["1", "2", "3", "4", "5"]:
#         for time in ["A", "B"]:
#             meta_fluorofentanyl_files += f"FF_me/{instrument}_Meta-{day}{time}.txt"
#             meta_fluorofentanyl_files += ","
#             ortho_fluorofentanyl_files += f"FF_me/{instrument}_Ortho-{day}{time}.txt"
#             ortho_fluorofentanyl_files += ","
#             para_fluorofentanyl_files += f"FF_me/{instrument}_Para-{day}{time}.txt"
#             para_fluorofentanyl_files += ","
# print(meta_fluorofentanyl_files)
# print()
# print()
# print(ortho_fluorofentanyl_files)
# print()
# print()
# print(para_fluorofentanyl_files)

# # remove trailing comma
# meta_fluorofentanyl_files = meta_fluorofentanyl_files[:-1]
# ortho_fluorofentanyl_files = ortho_fluorofentanyl_files[:-1]
# para_fluorofentanyl_files = para_fluorofentanyl_files[:-1]

# meta_fluorofentanyl = hdcms.filenames_to_stats_1d(meta_fluorofentanyl_files)
# ortho_fluorofentanyl = hdcms.filenames_to_stats_1d(ortho_fluorofentanyl_files)
# para_fluorofentanyl = hdcms.filenames_to_stats_1d(para_fluorofentanyl_files)

# ff_compounds = [meta_fluorofentanyl, ortho_fluorofentanyl, para_fluorofentanyl]
# print(hdcms.compare_all_1d(ff_compounds))

# # FMC

# fmc_2_files = ""
# fmc_3_files = ""
# fmc_4_files = ""

# for instrument in ["A", "B", "C", "D", "E", "F"]:
#     for day in ["1", "2", "3", "4", "5"]:
#         for time in ["a", "b"]:
#             fmc_2_files += f"FMC_me/{instrument}_2-{day}{time}.txt"
#             fmc_2_files += ","
#             fmc_3_files += f"FMC_me/{instrument}_3-{day}{time}.txt"
#             fmc_3_files += ","
#             fmc_4_files += f"FMC_me/{instrument}_4-{day}{time}.txt"
#             fmc_4_files += ","

# # remove trailing comma
# fmc_2_files = fmc_2_files[:-1]
# fmc_3_files = fmc_3_files[:-1]
# fmc_4_files = fmc_4_files[:-1]

# fmc_2 = hdcms.filenames_to_stats_1d(fmc_2_files)
# fmc_3 = hdcms.filenames_to_stats_1d(fmc_3_files)
# fmc_4 = hdcms.filenames_to_stats_1d(fmc_4_files)

# fmc = [fmc_2, fmc_3, fmc_4]
# print("FMC:")
# print(hdcms.compare_all_1d(fmc))

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

#filenames = ["skiplist", "tree", "flexlist_ideal", "flexlist_10t", "flexlist_100t", "flexlist_1000",
#             "cbtree_ideal", "cbtree_10t", "cbtree_100t", "cbtree_100", "cbtree_1000", "cbtree_fair_100t"]
#labels = ["Skiplist", "Tree", "Ideal Splaylist", "Splaylist $\\frac{1}{10p}$", "Splaylist $\\frac{1}{100p}$", "Splaylist $\\frac{1}{1000}$",
#          "Ideal CBTree", "CBTree $\\frac{1}{10p}$", "CBTree $\\frac{1}{100p}$", "CBTree $\\frac{1}{100}$", "CBTree $\\frac{1}{1000}$",
#          "CBTree Fair $\\frac{1}{100p}$"]

filenames = ["flexlist_contains_height_x_1",
             "flexlist_contains_height_x_5", "flexlist_contains_height_x_10", "flexlist_contains_height_zipf"]

prefix = "./data_graphics_height_contains/"
#num_of_runs = 5.0


for filename in filenames:
    heights = []
    contains = []
    inp = open(prefix + filename, "r")
    
    print(filename)
    lines = inp.readlines()
    for i in range(1, len(lines)):
        line = lines[i]
        parts = line.split(" ")
        heights.append(int(parts[5][:len(parts[5]) - 1]))
        contains.append(int(parts[8]))
    inp.close()
    plt.figure()
    plt.plot(contains, heights, 'o', markersize=1.5)
    plt.xlabel("number of contains operations to key")
    plt.ylabel("height of key in flexlist")
    plt.savefig(prefix + "graphic_height_contains" + filename[25:] + ".png")
    plt.show()

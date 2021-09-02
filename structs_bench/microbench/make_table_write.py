import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

#filenames = ["skiplist", "tree", "flexlist_ideal", "flexlist_10t", "flexlist_100t", "flexlist_1000",
#             "cbtree_ideal", "cbtree_10t", "cbtree_100t", "cbtree_100", "cbtree_1000", "cbtree_fair_100t"]
#labels = ["Skiplist", "Tree", "Ideal Splaylist", "Splaylist $\\frac{1}{10p}$", "Splaylist $\\frac{1}{100p}$", "Splaylist $\\frac{1}{1000}$",
#          "Ideal CBTree", "CBTree $\\frac{1}{10p}$", "CBTree $\\frac{1}{100p}$", "CBTree $\\frac{1}{100}$", "CBTree $\\frac{1}{1000}$",
#          "CBTree Fair $\\frac{1}{100p}$"]

prefix = "./results_write/" #"./01.05.2020-results/" #"./results_10s_10x_100_100_rebuttal/"
filenames = ["0", "0.02", "0.1", "0.2"]

workloads = ["90/10", "95/5", "99/1", "100/100", "zipf/1"]
threads = [70]
string_threads = [str(thr) for thr in threads]

class Stats:
    def __init__(self):
        self.ops = {filename : 0.0 for filename in filenames}
        self.avgLen = {filename : 0.0 for filename in filenames}
        self.count = {filename : 0 for filename in filenames}

#num_of_runs = 5.0

stats = {name : {thr : Stats() for thr in string_threads} for name in workloads}    

for filename in filenames:
    inp = open(prefix + filename, "r")
#parameters:
#threads: 70
#write prob: 0
#write set: 0.25
#zipf: 0
#alpha: 0
#x: 99
#y: 1
#prefill: 100000
#secs: 10
#upd ops: 7000
#ideal: 0
#results:
#h: 0, ops: 4261142000, sumLengths: 17.61288277

    print(filename)
    lines = inp.readlines()
    pad = 0
    if lines[4].split(" ")[0] == "zipf:":
        pad = 2
    for i in range(0, len(lines), 13 + pad):
        line = lines[i + 1][:-1]
        thrs = line.split(" ")
        line = lines[i + 4][:-1]
        if line.split(" ")[0] == "zipf:":
            zipf = line.split(" ")
            line = lines[i + 5][:-1]
            alpha = line.split(" ")
        line = lines[i + 4 + pad][:-1]
        x = line.split(" ")
        line = lines[i + 5 + pad][:-1]
        y = line.split(" ")
        line = lines[i + 7 + pad][:-1]
        secs = line.split(" ")
        line = lines[i + 8 + pad][:-1]
        upd = line.split(" ")
        results = lines[i + 11 + pad].split(", ")
        ops = float((results[1].split(" "))[1]) / float(secs[1])
        avgLen = float(results[2].split(" ")[1])
        # print(thrs[1] + " " + filename)
        # print(stats[x[1] + "/" + y[1]][thrs[1]].ops[filename])
        if int(thrs[1]) not in threads:
            continue
        if pad == 0 or zipf[1] != "1":
            stats[x[1] + "/" + y[1]][thrs[1]].ops[filename] += ops
            # print(stats[x[1] + "/" + y[1]][thrs[1]].ops[filename])
            stats[x[1] + "/" + y[1]][thrs[1]].avgLen[filename] += avgLen
            stats[x[1] + "/" + y[1]][thrs[1]].count[filename] += 1
        else:
            stats["zipf/" + alpha[1]][thrs[1]].ops[filename] += ops
            stats["zipf/" + alpha[1]][thrs[1]].avgLen[filename] += avgLen
            stats["zipf/" + alpha[1]][thrs[1]].count[filename] += 1
    inp.close()

for workload in workloads:
    print("Workload " + workload)
    y = {fname : [] for fname in filenames}
    # y = {"skiplist" : [], "tree" : [], "cbtree_ideal" : [], "flexlist_ideal" : [], "cbtree_100t" : [], "flexlist_100t" : [], "cbtree_10t" : [], "flexlist_10t" : [], "cbtree_1000" : [], "flexlist_1000" : []}
    no_data = False
    result = ""
    num = "70"
    base = stats[workload][num].ops["0"] / stats[workload][num].count["0"]
    for filename in filenames:                   
        if stats[workload][num].count[filename] == 0:
            no_data = True
            break
        stats[workload][num].ops[filename] /= stats[workload][num].count[filename]
        if filename == "0":
            result += str(stats[workload][num].ops[filename] / (1000000.0)) + " "
        else:
            result += "& %0.2fM (%0.0f) " % (stats[workload][num].ops[filename] / (1000000.0), stats[workload][num].ops[filename] / base * 100)
    print(result)


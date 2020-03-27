import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

filenames = ["skiplist", "tree", "flexlist_ideal", "flexlist_10t", "flexlist_100t", "flexlist_1000",
             "cbtree_ideal", "cbtree_10t", "cbtree_100t", "cbtree_100", "cbtree_1000", "cbtree_fair_100t"]
labels = ["Skiplist", "Tree", "Ideal Splaylist", "Splaylist $\\frac{1}{10p}$", "Splaylist $\\frac{1}{100p}$", "Splaylist $\\frac{1}{1000}$",
          "Ideal CBTree", "CBTree $\\frac{1}{10p}$", "CBTree $\\frac{1}{100p}$", "CBTree $\\frac{1}{100}$", "CBTree $\\frac{1}{1000}$",
          "CBTree Fair $\\frac{1}{100p}$"]

#filenames = ["skiplist",
#             "flexlist_1000t", "flexlist_100t", "flexlist_10t",
#             "cbtree_1000", "cbtree_100", "cbtree_10",
#             "cbtree_fair_1000t", "cbtree_fair_100t", "cbtree_fair_10t"]
#labels = ["Skip-list",
#          "Splay-list $\\frac{1}{1000p}$", "Splay-list $\\frac{1}{100p}$", "Splay-list $\\frac{1}{10p}$",
#          "CBTree $\\frac{1}{1000}$", "CBTree $\\frac{1}{100}$", "CBTree $\\frac{1}{10}$",
#          "CBTree Fair $\\frac{1}{1000p}$", "CBTree Fair $\\frac{1}{100p}$", "CBTree Fair $\\frac{1}{100p}$"]

prefix = "./results/"
filenames = ["flexlist_10t", "cbtree_10", "cbtree_fair_10t"]
labels = ["Splay-list", "CBTree Unfair", "CBTree Fair"]

threads = [1, 2, 4, 8, 10, 20, 30, 40, 50, 60, 70]
#threads = [1, 2, 4, 10, 30, 50, 70]
string_threads = [str(thr) for thr in threads]
workloads = ["90/10", "95/5", "99/1", "zipf/1"]

class Stats:
    def __init__(self):
        self.ops = {filename : 0.0 for filename in filenames}
        self.avgLen = {filename : 0.0 for filename in filenames}

num_of_runs = 5.0



stats = {name : {thr : Stats() for thr in string_threads} for name in workloads}    


for filename in filenames:
    inp = open(prefix + filename, "r")
    # parameters:
    # threads: 1
    # zipf: 0
    # alpha 1
    # x: 99
    # y: 1
    # prefill: 100000
    # secs: 1
    # upd ops: 1
    # ideal: 0
    # results:
    # h: 15, ops: 1783472, sumLengths: 30.99154234
    #
    print(filename)
    lines = inp.readlines()
    for i in range(0, len(lines), 11):
        line = lines[i + 1][:-1]
        thrs = line.split(" ")
        line = lines[i + 2][:-1]
        pad = 0
        if line[0] == "zipf":
            zipf = line.split(" ")
            line = lines[i + 3][:-1]
            alpha = line.split(" ")
            pad = 2
        line = lines[i + 2 + pad][:-1]
        x = line.split(" ")
        line = lines[i + 3 + pad][:-1]
        y = line.split(" ")
        line = lines[i + 5 + pad][:-1]
        secs = line.split(" ")
        line = lines[i + 6 + pad][:-1]
        upd = line.split(" ")
        results = lines[i + 9 + pad].split(", ")
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
        else:
            stats["zipf/" + alpha[1]][thrs[1]].ops[filename] += ops
            stats["zipf/" + alpha[1]][thrs[1]].ops[filename] += avgLen
    inp.close()

markers = ['o', 's', '*', 'x', '+', '^', 'D']
for workload in workloads:
#    print("Workload " + workload)
    y = {fname : [] for fname in filenames}
    # y = {"skiplist" : [], "tree" : [], "cbtree_ideal" : [], "flexlist_ideal" : [], "cbtree_100t" : [], "flexlist_100t" : [], "cbtree_10t" : [], "flexlist_10t" : [], "cbtree_1000" : [], "flexlist_1000" : []}
    for num in string_threads:
        for filename in filenames:
            stats[workload][num].ops[filename] /= num_of_runs
            y[filename].append(stats[workload][num].ops[filename] / (1000000.0))        

    fig, ax = plt.subplots()
    for plot_id, filename in enumerate(filenames):
        yy = y[filename]

#        for i in range(len(threads) - 1, 1, -1):
#            proposal = 1. * (yy[i - 1] - yy[i - 2]) / (threads[i - 1] - threads[i - 2]) * (threads[i] - threads[i - 2]) + yy[i - 2]
#            if proposal > 3. / 2 * yy[i]:
#                yy[i] = (proposal + yy[i]) / 2

        ax.plot(threads, yy, marker=markers[plot_id % len(markers)], label=labels[plot_id])
#        print(filename + " " + str(yy[-1]))
    ax.legend()
    cnm = workload.split("/")
    ax.set(xlabel='Number of processes', ylabel=r'$10^6$ operations per second')
    fig.savefig("./workload" + cnm[0] + "_" + cnm[1] + ".pdf", bbox_inches='tight')
    plt.show()
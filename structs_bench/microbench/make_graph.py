import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

filenames = ["skiplist", "tree", "cbtree_ideal", "flexlist_ideal", "cbtree_100t", "flexlist_100t", "cbtree_10t", "flexlist_10t", "cbtree_1000", "flexlist_1000"]
threads = [1, 2, 4, 8, 10, 20, 30, 40, 50, 60, 70]
string_threads = [str(thr) for thr in threads]
workloads = ["90/10", "95/5", "99/1"]

class Stats:
    ops = {filename : 0.0 for filename in filenames}
    avgLen = {filename : 0.0 for filename in filenames}

num_of_runs = 1.0


stats = {name : {thr : Stats() for thr in string_threads} for name in workloads}    
#loop through all files
filenames = ["skiplist", "tree", "cbtree_ideal", "flexlist_ideal", "cbtree_100t", "flexlist_100t", "cbtree_10t", "flexlist_10t", "cbtree_1000", "flexlist_1000"]
#print (filenames)

for filename in filenames:
    inp = open("./results/" + filename, "r")
    # parameters:
    # threads: 1
    # x: 99
    # y: 1
    # prefill: 100000
    # secs: 1
    # upd ops: 1
    # ideal: 0
    # results:
    # h: 15, ops: 1783472, sumLengths: 30.99154234
    #
    lines = inp.readlines()
    for i in range(0, len(lines), 11):
        line = lines[i + 1][:-1]
        threads = line.split(" ")
        line = lines[i + 2][:-1]
        x = line.split(" ")
        line = lines[i + 3][:-1]
        y = line.split(" ")
        line = lines[i + 5][:-1]
        secs = line.split(" ")
        line = lines[i + 6][:-1]
        upd = line.split(" ")
        results = lines[i + 9].split(", ")
        #print(x, y, secs)
        if secs[1] == "10":
            ops = float((results[1].split(" "))[1]) / float(secs[1])
            avgLen = float(results[2].split(" ")[1])
            stats[x[1] + "/" + y[1]][threads[1]].ops[filename] += ops;
            stats[x[1] + "/" + y[1]][threads[1]].avgLen[filename] += avgLen
    inp.close()

markers = ['o', 's', '*', 'x', '+', '^', 'D']
for workload in workloads:
    y = {fname : [] for fname in filenames}
    # y = {"skiplist" : [], "tree" : [], "cbtree_ideal" : [], "flexlist_ideal" : [], "cbtree_100t" : [], "flexlist_100t" : [], "cbtree_10t" : [], "flexlist_10t" : [], "cbtree_1000" : [], "flexlist_1000" : []}
    for num in string_threads:
        for filename in filenames:
            stats[workload][num].ops[filename] /= num_of_runs
            y[filename].append(stats[workload][num].ops[filename] / (1000000.0))
    fig, ax = plt.subplots()
    for plot_id, filename in enumerate(filenames):
        ax.plot(threads, y[filename], marker=markers[plot_id % len(markers)], label=filename)
    ax.legend()
    cnm = workload.split("/")
    ax.set(xlabel='Number of threads', ylabel=r'$10^6$ operations per second')
    fig.savefig("./workload" + cnm[0] + "_" + cnm[1] + ".png")
    plt.show()
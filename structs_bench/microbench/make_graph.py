import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

class Stats:
    ops_real = 0.0
    ops_ideal = 0.0
    ops_flex = 0.0
    avgLen_real = 0.0
    avgLen_ideal = 0.0
    avgLen_flex = 0.0

stats = {"90/10" : {"1" : Stats(), "2" : Stats(), "4" : Stats(), "8" : Stats(), 
                    "10" : Stats(), "20" : Stats(), "30" : Stats(), "40" : Stats(),
                    "50" : Stats(), "60" : Stats(), "70" : Stats()}, 
         "99/1" : {"1" : Stats(), "2" : Stats(), "4" : Stats(), "8" : Stats(), 
                    "10" : Stats(), "20" : Stats(), "30" : Stats(), "40" : Stats(),
                    "50" : Stats(), "60" : Stats(), "70" : Stats()},
        "95/5" : {"1" : Stats(), "2" : Stats(), "4" : Stats(), "8" : Stats(), 
                    "10" : Stats(), "20" : Stats(), "30" : Stats(), "40" : Stats(),
                    "50" : Stats(), "60" : Stats(), "70" : Stats()}}    
#loop through all files
thread_part = ["threads_" +str(x) + "_cops_" + str(100 * x) for x in [1, 2, 4, 8, 10, 20, 30, 40, 50, 60, 70]]
xys = ["x_99_y_1", "x_95_y_5", "x_90_y_10"]
filenames = ['{}_{}_{}_secs_10'.format(name, xy, threads_cops) for name in ['flex', 'skip'] for xy in xys for threads_cops in thread_part]
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
            if (filename in ['ideal_{}_{}_secs_10'.format(xy, threads_cops) for xy in xys for threads_cops in thread_part]):
                #print(ops)
                stats[x[1] + "/" + y[1]][threads[1]].ops_ideal = ops
                stats[x[1] + "/" + y[1]][threads[1]].avgLen_ideal = avgLen
            if (filename in ['skip_{}_{}_secs_10'.format(xy, threads_cops) for xy in xys for threads_cops in thread_part]):
                stats[x[1] + "/" + y[1]][threads[1]].ops_real = ops
                stats[x[1] + "/" + y[1]][threads[1]].avgLen_real = avgLen
            if (filename in ['flex_{}_{}_secs_10'.format(xy, threads_cops) for xy in xys for threads_cops in thread_part]):
                stats[x[1] + "/" + y[1]][threads[1]].ops_flex = ops
                stats[x[1] + "/" + y[1]][threads[1]].avgLen_flex = avgLen
    inp.close()

for name in ["90/10", "95/5", "99/1"]:
    threads = [1, 2, 4, 8, 10, 20, 30, 40, 50, 60, 70]
    yskip = []
    yideal = []
    yflex = []
    for num in ["1", "2", "4", "8", "10", "20", "30", "40", "50", "60", "70"]:
        print(name, num, str(stats[name][num].ops_real))
        print(name, num, str(stats[name][num].ops_ideal))
        print(name, num, str(stats[name][num].ops_flex))
        yskip.append(stats[name][num].ops_real / (1000000.0))
        yideal.append(stats[name][num].ops_ideal / (1000000.0))
        yflex.append(stats[name][num].ops_flex / (1000000.0))
    fig, ax = plt.subplots()
    ax.plot(threads, yskip, label='skiplist')
    #ax.plot(threads, yideal)
    ax.plot(threads, yflex, label='splaylist')
    ax.legend()
    cnm = name.split("/")
    ax.set(xlabel='Number of threads', ylabel=r'$10^6$ operations per second')
    fig.savefig("workload" + cnm[0] + "_" + cnm[1] + ".png")
    plt.show()
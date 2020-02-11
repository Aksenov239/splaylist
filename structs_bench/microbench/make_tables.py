class Stats:
    ops_real = 0.0
    ops_ideal = 0.0
    ops_cbtree = 0.0
    ops_splaytree = 0.0
    ops_updOpt1 = 0.0
    ops_updOpt2 = 0.0
    ops_updOpt5 = 0.0
    ops_updOpt10 = 0.0
    ops_updOpt30 = 0.0
    ops_updOpt100 = 0.0
    avgLen_real = 0.0
    avgLen_ideal = 0.0
    avgLen_cbtree = 0.0
    avgLen_splaytree = 0.0
    avgLen_updOpt1 = 0.0
    avgLen_updOpt2 = 0.0
    avgLen_updOpt5 = 0.0
    avgLen_updOpt10 = 0.0
    avgLen_updOpt30 = 0.0
    avgLen_updOpt100 = 0.0

stats = {"90/10" : Stats(), "95/5" : Stats(), "99/1" : Stats()}    
#loop through all files
for filename in ["ideal", "skip", "flex"]:
    inp = open("./results_for_table/" + filename, "r")
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
        if threads[1] == "1":
            if secs[1] == "10":
                ops = float((results[1].split(" "))[1]) / float(secs[1])
                avgLen = float(results[2].split(" ")[1])
                if (filename == "ideal"):
                    stats[x[1] + "/" + y[1]].ops_ideal = ops
                    stats[x[1] + "/" + y[1]].avgLen_ideal = avgLen
                if (filename in "skip"):
                    stats[x[1] + "/" + y[1]].ops_real = ops
                    stats[x[1] + "/" + y[1]].avgLen_real = avgLen
                if (filename in "flex"):
                    if upd[2] == "1":
                        stats[x[1] + "/" + y[1]].ops_updOpt1 = ops
                        stats[x[1] + "/" + y[1]].avgLen_updOpt1 = avgLen
                    if upd[2] == "2":
                        stats[x[1] + "/" + y[1]].ops_updOpt2 = ops
                        stats[x[1] + "/" + y[1]].avgLen_updOpt2 = avgLen
                    if upd[2] == "5":
                        stats[x[1] + "/" + y[1]].ops_updOpt5 = ops
                        stats[x[1] + "/" + y[1]].avgLen_updOpt5 = avgLen
                    if upd[2] == "10":
                        stats[x[1] + "/" + y[1]].ops_updOpt10 = ops
                        stats[x[1] + "/" + y[1]].avgLen_updOpt10 = avgLen    
                    if upd[2] == "30":
                        stats[x[1] + "/" + y[1]].ops_updOpt30 = ops
                        stats[x[1] + "/" + y[1]].avgLen_updOpt30 = avgLen
                    if upd[2] == "100":
                        stats[x[1] + "/" + y[1]].ops_updOpt100 = ops
                        stats[x[1] + "/" + y[1]].avgLen_updOpt100 = avgLen            
    inp.close()
out = open("stats_tables.tex", "w")
for key in stats:
    out.write("\\begin{tabular}{|c|c|c|c|c|c|c|c|c|c|c|}\n")
    out.write("$"+ key + "$ & $skiplist$ & $ideal\\_skiplist$ & $cbtree$ & $splaytree$ & $upd\\_opt=1$ & $upd\\_opt=2$ & $upd\\_opt=5$ & $upd\\_opt=10$ & $upd\\_opt=30$ & $upd\\_opt=100$\\\\\\hline\n")
    out.write("$ops/secs$ & {:.2f} & {:.2f} & {:.2f} & {:.2f} & {:.2f} & {:.2f} & {:.2f} & {:.2f} & {:.2f} & {:.2f}\\\\\\hline\n".format(stats[key].ops_real, stats[key].ops_ideal, stats[key].ops_cbtree, stats[key].ops_splaytree, stats[key].ops_updOpt1, stats[key].ops_updOpt2, stats[key].ops_updOpt5, stats[key].ops_updOpt10, stats[key].ops_updOpt30, stats[key].ops_updOpt100))
    out.write("$avgLen$ & {:.2f} & {:.2f} & {:.2f} & {:.2f} & {:.2f} & {:.2f} & {:.2f} & {:.2f} & {:.2f} & {:.2f}\\\\\\hline\n".format(stats[key].avgLen_real, stats[key].avgLen_ideal, stats[key].avgLen_cbtree, stats[key].avgLen_splaytree, stats[key].avgLen_updOpt1, stats[key].avgLen_updOpt2, stats[key].avgLen_updOpt5, stats[key].avgLen_updOpt10, stats[key].avgLen_updOpt30, stats[key].avgLen_updOpt100))
    out.write("\\end{tabular}\n")
    out.write("\\\\\\\\\n")
out.close()
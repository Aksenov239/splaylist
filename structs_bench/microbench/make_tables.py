#prefix = "./results_for_table_2/"
prefix = "./results_zipf_for_table/"
filenames = ["skiplist",
             "flexlist_1", "flexlist_2", "flexlist_5", "flexlist_10", "flexlist_100", "flexlist_1000",
             "cbtree_1", "cbtree_2", "cbtree_5", "cbtree_10", "cbtree_100", "cbtree_1000"]
num_of_runs = 5.0
class Stats:
    def __init__(self):
        self.ops = {filename : 0.0 for filename in filenames}
        self.avgLen = {filename : 0.0 for filename in filenames}
        self.count = {filename: 0 for filename in filenames}

stats = {"90/10" : Stats(), "95/5" : Stats(), "99/1" : Stats(), "zipf/1" : Stats()}    
#loop through all files
for filename in filenames:
    inp = open(prefix + filename, "r")
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
    pad = 0
    if lines[2].split(" ")[0] == "zipf:":
        pad = 2
    for i in range(0, len(lines), 11 + pad):
        line = lines[i + 1][:-1]
        thrs = line.split(" ")
        line = lines[i + 2][:-1]
        if line.split(" ")[0] == "zipf:":
            zipf = line.split(" ")
            line = lines[i + 3][:-1]
            alpha = line.split(" ")
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
        if pad == 0 or zipf[1] != "1":
            stats[x[1] + "/" + y[1]].ops[filename] += ops
            stats[x[1] + "/" + y[1]].avgLen[filename] += avgLen
            stats[x[1] + "/" + y[1]].count[filename] += 1
        else:
            stats["zipf/" + alpha[1]].ops[filename] += ops
            stats["zipf/" + alpha[1]].avgLen[filename] += avgLen
            stats["zipf/" + alpha[1]].count[filename] += 1
    inp.close()

out = open("./stats_tables.tex", "w")

for key in stats:
    for filename in filenames:
        if stats[key].count[filename] != 0:
            stats[key].ops[filename] /= stats[key].count[filename]
            stats[key].avgLen[filename] /= stats[key].count[filename]
    if stats[key].count["skiplist"] == 0:
        continue
    baseline = stats[key].ops["skiplist"]
    s = "\\begin{tabular}{|"
    for i in range(len(filenames) + 1):
        s += "c|"
    s += "}\n"
    out.write(s)
    s = "$" + key + "$";
    for filename in filenames:
        s += " & $" + filename + "$"
    out.write(s + "\\\\\\hline\n")  
    s = "ops/secs"
    t = "length"
    for filename in filenames:
        if filename == "skiplist":  
            s += " & {}".format(baseline)
        else:
            s += " & {:.2f}x".format(1. * stats[key].ops[filename] / baseline)
        t += " & {:.2f}".format(stats[key].avgLen[filename])
    out.write(s + "\\\\\\hline\n")  
    out.write(t + "\\\\\\hline\n")      
    out.write("\\end{tabular}\n")
    out.write("\\\\\\\\\n")
out.close()
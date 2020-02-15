filenames = []
num_of_runs = 5.0
class Stats:
    def __init__(self):
        self.ops = {filename : 0.0 for filename in filenames}
        self.avgLen = {filename : 0.0 for filename in filenames}

stats = {"90/10" : Stats(), "95/5" : Stats(), "99/1" : Stats()}    
loop through all files
for filename in filenames:
    inp = open("./results_for_table_2/" + filename, "r")
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
        ops = float((results[1].split(" "))[1]) / float(secs[1])
        avgLen = float(results[2].split(" ")[1])
        stats[x[1] + "/" + y[1]].ops[filename] += ops;
        stats[x[1] + "/" + y[1]].avgLen[filename] += ops;
    inp.close()
out = open("./stats_tables.tex", "w")



for key in stats:
    for filename in filenames:
        stats[key].ops[filename] /= num_of_runs
        stats[key].avgLen[filename] /= num_of_runs
    s = "\\begin{tabular}{|"
    for i in range(len(filenames) + 1):
        s += "c|"
    s += "}\n"
    out.write(s)
    s = "$" + key + "$";
    for filename in filenames:
        s += " & $" + filename + "$"
    out.write(s + "\\\\\\hline\n")  
    s = "$ops/secs$"
    t = "$avgLen$"
    for filename in filenames:
        s += " & {:.2f}".format(stats[key].ops[filename])
        t += " & {:.2f}".format(stats[key].avgLen[filename])
    out.write(s + "\\\\\\hline\n")  
    out.write(t + "\\\\\\hline\n")      
    out.write("\\end{tabular}\n")
    out.write("\\\\\\\\\n")
out.close()
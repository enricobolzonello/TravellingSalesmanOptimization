import glob
import subprocess
import csv
import pandas as pd 
import os
import sys
import shlex

methods = {
    "heuristic" : ["GREEDY", "GREEDY_ITERATIVE", "2OPT_GREEDY"]
}


# python compare_algs.py {method}
# methods: heuristic
if __name__ == '__main__':
    
    # parse for method in command line
    type = sys.argv[1]
    if type not in methods.keys():
        print("method not recognized")
        sys.exit()

    paths = []
    for name in glob.glob('data/*'):
        paths.append(name)

    time_limit = "1200"   #seconds

    csv_filename = f"{type}.csv"
    
    # if the file does not exist, create it
    if not os.path.exists('results/'+csv_filename):
        df=pd.DataFrame(index=paths,columns=methods[type])
        df.to_csv('results/'+csv_filename,index=True)
    else:
        df=pd.read_csv('results/'+csv_filename,index_col=0)
    

    
    for tsp in paths:
        for m in methods[type]:
            str_exec = f"make/bin/tsp -f {tsp} -q -alg {m} -t {time_limit} -seed 123 --to_file"
            print("Running " + m+ " on dataset " +tsp)
            output = subprocess.run(shlex.split(str_exec), capture_output=True, text=True).stdout
            
            cost = output.split(":")[1].strip()
            print("Cost: " + cost)
            df.loc[tsp,m]=cost
            df.to_csv('results/'+csv_filename, index=True, mode='w+')
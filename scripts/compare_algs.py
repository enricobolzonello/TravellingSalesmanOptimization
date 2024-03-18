import subprocess
import csv
import pandas as pd 
import os
import sys
import shlex
from py_reminder import config
from dotenv import load_dotenv, find_dotenv

load_dotenv(find_dotenv())

METHODS = {
    "heuristic" : ["GREEDY", "GREEDY_ITERATIVE", "2OPT_GREEDY"]
}

TIME_LIMIT = "1200"

# create an .env file to contain email credentials
# the file has the following format
# ACCOUNT_USERNAME=example@mail.com
# ACCOUNT_PASSWORD=passowrd
config(address=os.getenv("ACCOUNT_USERNAME"),
       password=os.getenv("ACCOUNT_PASSWORD"),
       smtp='smtp.gmail.com',
       port=587,  # currently it should be non-SSL port
       default_to=os.getenv("ACCOUNT_USERNAME"))

@monitor(task=f'Travelling Salesman Problem')
def runTSP(df, paths):
    for tsp in paths:
        for m in METHODS[type]:
            str_exec = f"make/bin/tsp -f {tsp} -q -alg {m} -t {TIME_LIMIT} -seed 123 --to_file"
            print("Running " + m+ " on dataset " +tsp)
            output = subprocess.run(shlex.split(str_exec), capture_output=True, text=True).stdout
            
            cost = output.split(":")[1].strip()
            print("Cost: " + cost)
            df.loc[tsp,m]=cost
            df.to_csv('results/'+csv_filename, index=True, mode='w+')


# python compare_algs.py {method}
# methods: heuristic
if __name__ == '__main__':
    
    # parse for method in command line
    type = sys.argv[1]
    if type not in METHODS.keys():
        print("method not recognized")
        sys.exit()

    paths = []
    for root, _, files in os.walk("data/temp"):
        # Append file paths to the list
        for file in files:
            paths.append(os.path.join(root, file))

    csv_filename = f"{type}.csv"
    
    # if the file does not exist, create it
    if not os.path.exists('results/'+csv_filename):
        df=pd.DataFrame(index=paths,columns=METHODS[type])
        df.to_csv('results/'+csv_filename,index=True)
    else:
        df=pd.read_csv('results/'+csv_filename,index_col=0)

    runTSP(df, paths)
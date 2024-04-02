import subprocess
import csv
import pandas as pd 
import os
import sys
import shlex
from py_reminder import config
from dotenv import load_dotenv, find_dotenv

import requests
from logging import Handler, Formatter
import logging
import datetime
import time

load_dotenv(find_dotenv())

METHODS = {
    "heuristic" : ["GREEDY", "GREEDY_ITER", "2OPT_GREEDY"],
    "metaheuristic" : ["TABU_SEARCH", "VNS"],
    "all" : ["CPLEX", "GREEDY_ITER", "2OPT_GREEDY", "TABU_SEARCH", "VNS"]
}

TIME_LIMIT = "1200"
 
# create an .env file to contain telegram token and chat id
# the file has the following format
# TOKEN=token
# ID=id
TELEGRAM_TOKEN = os.getenv('TOKEN')
TELEGRAM_CHAT_ID = os.getenv('ID')

# -------------------------------------------------------------------------------------------------------------
#   TELEGRAM BOT
# -------------------------------------------------------------------------------------------------------------
class RequestsHandler(Handler):
	def emit(self, record):
		log_entry = self.format(record)
		payload = {
			'chat_id': TELEGRAM_CHAT_ID,
			'text': log_entry,
			'parse_mode': 'HTML'
		}
		return requests.post("https://api.telegram.org/bot{token}/sendMessage".format(token=TELEGRAM_TOKEN),
							 data=payload).content
 
class LogstashFormatter(Formatter):
	def __init__(self):
		super(LogstashFormatter, self).__init__()
    
	def format(self, record):
		t = datetime.datetime.utcnow().strftime('%Y-%m-%d %H:%M:%S')
    
		return "<i>{datetime}</i><pre>\n{message}</pre>".format(message=record.msg, datetime=t)

# -------------------------------------------------------------------------------------------------------------
#   END TELEGRAM BOT
# -------------------------------------------------------------------------------------------------------------

def runTSP(paths, csv_filename, logger, start_time):
    data = []

    for i,tsp in enumerate(paths):
        row = [os.path.basename(tsp)]
        for m in METHODS[type]:
            try:
                str_exec = f"make/bin/tsp -f {tsp} -q -alg {m} -t {TIME_LIMIT} -seed 123 --to_file -t 30"
                print("Running " + m+ " on dataset " +tsp)
                output = subprocess.run(shlex.split(str_exec), capture_output=True, text=True).stdout

                cost = output.split(":")[1].strip()
                print("Cost: " + cost)
                row.append(cost)
            except subprocess.CalledProcessError as e:
                logger.error(f"Error executing {m} on dataset {tsp}: {e}")
                continue
            except IndexError as e:
                logger.error(f"Skipping dataset {tsp}")
                continue
    
        if len(row) > 1: # to not write files not done
            with open(csv_filename, 'a', newline='') as csvfile:
                writer = csv.writer(csvfile)
                writer.writerow(row)

        # log every 30 documents done
        if i%30 == 0:
            logger.info(f"Done {i} documents")
    
    logger.warning(f"Program finished in {(time.time() - start_time):.4f} seconds")

# python compare_algs.py {method}
# methods: heuristic
if __name__ == '__main__':

    if len(sys.argv) < 2:
        print("Usage: python script.py <method>")
        sys.exit()
    
    # parse for method in command line
    type = sys.argv[1]
    if type not in METHODS.keys():
        print("method not recognized")
        sys.exit()

    logger = logging.getLogger('Travelling Salesman Problem')
    logger.setLevel(logging.INFO)

    handler = RequestsHandler()
    formatter = LogstashFormatter()
    handler.setFormatter(formatter)
    logger.addHandler(handler)

    start_time = time.time()

    paths = []
    for root, _, files in os.walk("data/test"):
        # Append file paths to the list
        for file in files:
            paths.append(os.path.join(root, file))

    csv_filename = f"results/{type}.csv"
    
    header = [len(METHODS[type])] + METHODS[type]
    with open(csv_filename, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(header)

    runTSP(paths, csv_filename, logger, start_time)
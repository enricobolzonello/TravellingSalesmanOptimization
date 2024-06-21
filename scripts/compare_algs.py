import subprocess
import csv
import pandas as pd 
import os
import sys
import shlex
from py_reminder import config
import tomllib
from alive_progress import alive_bar
import random


import requests
from logging import Handler, Formatter
import logging
import datetime
import time
TIME_LIMIT = "600"
NODES_UPPER = 350
NODES_LOWER = 300
 
# create an .env file to contain telegram token and chat id
# the file has the following format
# TOKEN=token
# ID=id
TELEGRAM_TOKEN = os.environ.get('TOKEN')
TELEGRAM_CHAT_ID = os.environ.get('ID')

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


def runTSP(paths, csv_filename, data, logger, start_time):
    with alive_bar(total=len(paths) * len(data), calibrate=50) as pbar:
        for i,tsp in enumerate(paths):
            row = [os.path.basename(tsp)]
            for m in data:
                try:
                    if 'algorithm' not in m:
                        raise ValueError("Missing 'algorithm' key in dictionary, skipping this iteration")
                    algorithm = m['algorithm']
                    flags = m.get('flags', '')
                    str_exec = f"make/bin/tsp -f {tsp} -q -alg {algorithm} -t {TIME_LIMIT} -seed 123 --to_file {flags}"
                    print("Running " + algorithm + " on dataset " + tsp)

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
                finally:
                    pbar()
        
            if len(row) > 1: # to not write files not done
                with open(csv_filename, 'a', newline='') as csvfile:
                    writer = csv.writer(csvfile)
                    writer.writerow(row)

            # log every 30 documents done
            if i%20 == 0:
                logger.info(f"Done {i} documents")
    
    logger.warning(f"Program finished in {(time.time() - start_time):.4f} seconds")

def runTSPRand(number, csv_filename, data, logger, start_time):
    with alive_bar(total=number * len(data), calibrate=50) as pbar:
        for i in range(number):
            row = [i]
            seed = random.randint(100, 10000)
            nodes = random.randint(NODES_LOWER, NODES_UPPER)
            for m in data:
                try:
                    if 'algorithm' not in m:
                        raise ValueError("Missing 'algorithm' key in dictionary, skipping this iteration")
                    algorithm = m['algorithm']
                    flags = m.get('flags', '')
                    str_exec = f"make/bin/tsp -n {nodes} -seed {seed} -q -alg {algorithm} -t {TIME_LIMIT} --to_file {flags}"
                    print(str_exec)
                    print("Running " + algorithm + " on instance " + str(i))

                    output = subprocess.run(shlex.split(str_exec), capture_output=True, text=True).stdout

                    cost = output.split(":")[1].strip()
                    print("Cost: " + cost)
                    row.append(cost)
                except subprocess.CalledProcessError as e:
                    logger.error(f"Error executing {m} on instance {i}: {e}")
                    continue
                except IndexError as e:
                    logger.error(f"Skipping instance {i}")
                    row.append('error')
                    continue
                finally:
                    pbar()
        
            if len(row) > 1: # to not write files not done
                with open(csv_filename, 'a', newline='') as csvfile:
                    writer = csv.writer(csvfile)
                    writer.writerow(row)

            # log every 30 documents done
            if i%20 == 0:
                logger.info(f"Done {i} documents")
    
    logger.warning(f"Program finished in {(time.time() - start_time):.4f} seconds")

# python compare_algs.py {method}
# methods: heuristic
if __name__ == '__main__':

    if len(sys.argv) < 3:
        print("Usage: python compare_algs.py <config file> <data folder>")
        sys.exit()
    
    # parse for method in command line
    filepath = sys.argv[1]
    if not os.path.isfile(filepath):
        print("toml config file does not exist")
        sys.exit()
    basename = os.path.splitext(os.path.basename(filepath))[0]

    datadir = sys.argv[2]
    if not os.path.isdir(datadir):
        print("data folder does not exist")
        sys.exit()

    with open(filepath, "rb") as f:
        data = tomllib.load(f)
        
        logger = logging.getLogger('Travelling Salesman Problem')
        logger.setLevel(logging.INFO)

        handler = RequestsHandler()
        formatter = LogstashFormatter()
        handler.setFormatter(formatter)
        logger.addHandler(handler)

        start_time = time.time()

        paths = []
        for root, _, files in os.walk(datadir):
            # Append file paths to the list
            for file in files:
                paths.append(os.path.join(root, file))
        
        csv_filename = f"results/{basename}.csv"

        header = [len(data.keys())] + list(data.keys())

        with open(csv_filename, 'w', newline='') as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow(header)

        data = list(data.values())
        #runTSP(paths, csv_filename, data, logger, start_time)
        runTSPRand(20, csv_filename, data, logger, start_time)
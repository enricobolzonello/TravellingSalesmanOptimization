# Traveling Salesman Optimization
![GitHub Repo stars](https://img.shields.io/github/stars/enricobolzonello/TravellingSalesmanOptimization?style=for-the-badge&logo=github)
![GitHub Issues or Pull Requests](https://img.shields.io/github/issues/enricobolzonello/TravellingSalesmanOptimization?style=for-the-badge&logo=github)



Repository containing the project developed during the Operations Research 2 course in the academic year 2023/24.

## 💻 Requirements 
Install the following libraries to build the C program:
*   **gnu make**, v.3.81
*   **gnuplot**, v6.0
*   **IBM ILOG CPLEX**
*   **cmocka**, v1.1.7 (optional)

For the profiling, install requirements with ```pip install scripts/requirements.txt```. 

## 🛠️ Usage
Once the repo is cloned, go to the folder and run ```make```. Once it has finished, the executable will be located in ```make/bin```

To run profiling run:
```
python scripts/compare_algs.py {method}
```
where ```method``` is heuristic,... This will create a .csv file in ```/results``` with times for each algorithm and each dataset.

Once done, run the profiling with:
```
python scripts/perfprof.py results/{filename}.csv results/{outputfile}.pdf
```

## 🛡️ License

![GitHub License](https://img.shields.io/github/license/enricobolzonello/TravellingSalesmanOptimization?style=for-the-badge)


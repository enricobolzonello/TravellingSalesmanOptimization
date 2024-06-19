import pandas as pd
import sys

def add_ten_to_numeric_values(input_csv, output_csv, added_time):
    # Read the CSV file into a DataFrame
    df = pd.read_csv(input_csv)
    
    # Add 10 to each numeric value in the DataFrame
    # Function to add 10 and round the result
    def add_ten_and_round(x):
        if isinstance(x, (int, float)):
            return round(x + added_time, 2)
        return x
    df = df.map(add_ten_and_round)
    
    # Save the modified DataFrame to a new CSV file
    df.to_csv(output_csv, index=False)

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python scale_times.py <result file>.csv")
        sys.exit()

    filepath = sys.argv[1]
    time = int(sys.argv[2])

    output_csv = "results/test.csv"

    add_ten_to_numeric_values(filepath, output_csv, time)
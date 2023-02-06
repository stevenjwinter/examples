""" Simple example of reading a CSV file and getting some statistics from it """
import os
import pandas as pd


# GLOBAL VARIABLE NAME SPACE
CSV_FILE = 'python/workstation/data/booklist.csv'
PRICE_COLUMN = 'price'
PQT_FILE = 'python/workstation/data/booklist.pqt'

def check_for_file():
    """_summary_ Print cwd and check for existence of CSV (global name) file
    """
    print(os.getcwd())
    file_exists = os.path.exists(CSV_FILE)
    print('File ', CSV_FILE, 'Exists: ', file_exists)

def print_stats(interesting_dataframe):
    """_summary_ Print some interesting things about this dataframe

    Args:
      interesting_dataframe (_type_): _description_ The dataframe to use for stats
    """
    mean_price = interesting_dataframe[PRICE_COLUMN].mean()
    print('Mean price:', mean_price)
    sum_prices = interesting_dataframe[PRICE_COLUMN].sum()
    print('The sum of all the prices', sum_prices)
    num_rows = interesting_dataframe.shape[0]
    print('Number of rows:', num_rows)

# MAIN SCRIPT
check_for_file()

try:
    CsvFileDataFrame = pd.read_csv(CSV_FILE)
    print(CsvFileDataFrame.head())
    print('done reading file\n------')
    print_stats(CsvFileDataFrame)
    CsvFileDataFrame.to_parquet(PQT_FILE) # Super inflates required libraries
    print('done\n----')
except IOError as io_error:
    print(io_error)
 
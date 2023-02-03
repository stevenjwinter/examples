
import pandas as pd
import os


CSV_FILE = 'python/workstation/data/booklist.csv'

print(os.getcwd())
FileExists = os.path.exists(CSV_FILE)
print('File ', CSV_FILE, 'Exists: ', FileExists)

try:
    CsvFileDataFrame = pd.read_csv(CSV_FILE)
    print(CsvFileDataFrame.head())
    print('read file')
except Exception as err:
  print('Error')
  print(err)


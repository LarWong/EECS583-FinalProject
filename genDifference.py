import pandas as pd

# Function to check if a number is prime
def is_prime(n):
    if n < 2:
        return False
    for i in range(2, int(n**0.5) + 1):
        if n % i == 0:
            return False
    return True

# Read CSV file into a DataFrame
df = pd.read_csv('data64.csv')
df = df.sort_values(by='File')

# Filter the DataFrame to exclude rows where 'File' is a prime number
#df = df[~df['File'].apply(is_prime)]

# Calculate the percent differences for each corresponding pair of values
percent_diff = ((df['D1_misses_1'] - df['D1_misses_2']) / df['D1_misses_1']) * 100

# Calculate the average of the percent differences
average_percent_diff = percent_diff.mean()

print(f'Average percent difference between D1_misses_1 and D1_misses_2: {average_percent_diff:.2f}%')

import csv
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Function to check if a number is prime
def is_prime(n):
    if n < 2:
        return False
    for i in range(2, int(n**0.5) + 1):
        if n % i == 0:
            return False
    return True

def cherry_pick(n):
    if n % 5 == 0:
        return False
    return True

# Read CSV file into a list of dictionaries
data = []
with open('results.csv', 'r') as csvfile:
    reader = csv.DictReader(csvfile)
    for row in reader:
        data.append(row)

# Convert 'File', 'D1_misses_1', and 'D1_misses_2' columns to numeric type
for row in data:
    row['File'] = pd.to_numeric(row['File'], errors='coerce')
    row['D1_misses_1'] = pd.to_numeric(row['D1_misses_1'], errors='coerce')
    row['D1_misses_2'] = pd.to_numeric(row['D1_misses_2'], errors='coerce')
    row['D_refs_1'] = pd.to_numeric(row['D_refs_1'], errors='coerce')
    row['D_refs_2'] = pd.to_numeric(row['D_refs_2'], errors='coerce')

# Create a DataFrame from the list of dictionaries
df = pd.DataFrame(data)

# Sort the DataFrame by the 'File' column
df = df.sort_values(by='File')

# Filter the DataFrame to exclude rows where 'File' is a prime number
df = df[~df['File'].apply(is_prime)]

# Calculate new columns: 'D1_misses_1 / 2' and 'D1_misses_2 / 4'
df['D1_misses_1_normalized'] = df['D1_misses_1'] / df['D_refs_1']
df['D1_misses_2_normalized'] = df['D1_misses_2'] / df['D_refs_2']

# Plot the normalized data
plt.plot(df['File'].to_numpy(), df['D1_misses_1_normalized'].to_numpy(), label='D1 miss rate')
plt.plot(df['File'].to_numpy(), df['D1_misses_2_normalized'].to_numpy(), label='D1 miss rate')

# Add labels and a legend
plt.xlabel('File')
plt.ylabel('D1 Miss Rate')
plt.legend()

# Save the plot to a PNG file
plt.savefig('miss_rate.png')

# Optionally, you can also display the plot if needed
# plt.show()

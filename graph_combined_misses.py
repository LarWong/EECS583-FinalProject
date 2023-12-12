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

# Read CSV file into a list of dictionaries
data = []
with open('combined64.csv', 'r') as csvfile:
    reader = csv.DictReader(csvfile)
    for row in reader:
        data.append(row)

# Convert 'File', 'D1_misses_1', and 'D1_misses_2' columns to numeric type
for row in data:
    row['File'] = pd.to_numeric(row['File'], errors='coerce')
    row['D_misses'] = pd.to_numeric(row['D_misses'], errors='coerce')
    row['D_misses_square'] = pd.to_numeric(row['D_misses_square'], errors='coerce')
    row['D_misses_original'] = pd.to_numeric(row['D_misses_original'], errors='coerce')

# Create a DataFrame from the list of dictionaries
df = pd.DataFrame(data)

# Sort the DataFrame by the 'File' column
df = df.sort_values(by='File')

# Filter the DataFrame to exclude rows where 'File' is a prime number
#df = df[~df['File'].apply(is_prime)]

# Determine even multiples based on the y-axis values
min_value = min(df['D_misses'].min(), df['D_misses_square'].min(), df['D_misses_original'].min())
max_value = max(df['D_misses'].max(), df['D_misses_square'].max(), df['D_misses_original'].max())

# Calculate even multiples within the range
even_multiples = np.arange(min_value, max_value, step=2)

# Filter the DataFrame to include only rows where 'D1_misses_1' or 'D1_misses_2' match the even multiples
df_downsampled = df[
        df['D_misses'].isin(even_multiples) | 
        df['D_misses_square'].isin(even_multiples) |
        df['D_misses_original'].isin(even_multiples)
    ]

# Plot the downsampled data
plt.plot(df_downsampled['File'].to_numpy(), df_downsampled['D_misses'].to_numpy(), label='Untiled Misses', color='#87CEFA')
plt.plot(df_downsampled['File'].to_numpy(), df_downsampled['D_misses_square'].to_numpy(), label='Sqrt Misses', color='orange')
plt.plot(df_downsampled['File'].to_numpy(), df_downsampled['D_misses_original'].to_numpy(), label='Lam et al. Misses')


# Add labels and a legend
plt.xlabel('Matrix Size')
plt.ylabel('L1 Data Cache Misses')
plt.title('Untiled vs Tiled Misses 64KB')
plt.legend()

# Save the plot to a PNG file
plt.savefig('graphs/combined64_misses.png')

# Optionally, you can also display the plot if needed
# plt.show()

import csv
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

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

# Create a DataFrame from the list of dictionaries
df = pd.DataFrame(data)

# Sort the DataFrame by the 'File' column
df = df.sort_values(by='File')

# Determine even multiples based on the y-axis values
min_value = min(df['D1_misses_1'].min(), df['D1_misses_2'].min())
max_value = max(df['D1_misses_1'].max(), df['D1_misses_2'].max())

# Calculate even multiples within the range
even_multiples = np.arange(min_value, max_value, step=2)

# Filter the DataFrame to include only rows where 'D1_misses_1' or 'D1_misses_2' match the even multiples
df_downsampled = df[df['D1_misses_1'].isin(even_multiples) | df['D1_misses_2'].isin(even_multiples)]

# Plot the downsampled data
plt.plot(df_downsampled['File'].to_numpy(), df_downsampled['D1_misses_1'].to_numpy(), label='D1 misses 1')
plt.plot(df_downsampled['File'].to_numpy(), df_downsampled['D1_misses_2'].to_numpy(), label='D1 misses 2')

# Add labels and a legend
plt.xlabel('File')
plt.ylabel('D1 misses')
plt.legend()

# Save the plot to a PNG file
plt.savefig('miss_rate.png')

# Optionally, you can also display the plot if needed
# plt.show()

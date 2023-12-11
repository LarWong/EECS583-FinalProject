import matplotlib.pyplot as plt

file_path1 = '/home/stilex/EECS583-FinalProject/tests/D1_misses_blocked1.txt'
file_path2 = '/home/stilex/EECS583-FinalProject/tests/D1_misses_unblocked1.txt'

# Read data from the first text file
with open(file_path1, 'r') as file:
    data1 = [int(line.strip()) for line in file]

# Read data from the second text file
with open(file_path2, 'r') as file:
    data2 = [int(line.strip()) for line in file]

# Generate x values (assuming sequential integers starting from 1)
x_values = list(range(1, len(data1) + 1))

# Plot the first curve
plt.plot(x_values, data1, label='Curve 1')

# Plot the second curve
plt.plot(x_values, data2, label='Curve 2')

# Add labels and title
plt.xlabel('Index')
plt.ylabel('Data Value')
plt.title('Two Curves on One Plot')

# Show legend
plt.legend()
plt.savefig('output_plot.png')
# Show the plot
plt.show()
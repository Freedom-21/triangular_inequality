#!/usr/bin/env python3

import matplotlib
matplotlib.use('TkAgg') 

import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.patches as patches

# File paths for the dataset
problematic_loc_file = 'banks_loc.csv'  # Replace with your file path

# Load the dataset
problematic_loc_data = pd.read_csv(problematic_loc_file)
problematic_loc_data.columns = ['ObjectID', 'X', 'Y']

# Define grid size for spatial partitioning
grid_size = 1.0  # Partition space into 1x1 units
grid_partition = problematic_loc_data.copy()

# Assign grid indices to each point
grid_partition['GridX'] = (grid_partition['X'] // grid_size).astype(int)
grid_partition['GridY'] = (grid_partition['Y'] // grid_size).astype(int)

# Group by grid and count points in each grid cell
grid_counts = grid_partition.groupby(['GridX', 'GridY']).size().reset_index(name='Count')

# Identify grid cells with a high number of points (potential overlaps)
high_density_threshold = 10  # Adjust as needed
high_density_cells = grid_counts[grid_counts['Count'] > high_density_threshold]

# Create a scatter plot for the dataset
plt.figure(figsize=(12, 8))
plt.scatter(problematic_loc_data['X'], problematic_loc_data['Y'], 
            alpha=0.5, s=10, label='Objects')

# Highlight high-density regions
ax = plt.gca()
for _, row in high_density_cells.iterrows():
    grid_x = row['GridX']
    grid_y = row['GridY']
    count = row['Count']
    
    # Draw a rectangle around the high-density grid cell
    rect = patches.Rectangle(
        (grid_x * grid_size, grid_y * grid_size),
        grid_size,
        grid_size,
        linewidth=1.5,
        edgecolor='red',
        facecolor='none'
    )
    ax.add_patch(rect)
    
    # Annotate the count
    plt.text(
        grid_x * grid_size + grid_size / 2,
        grid_y * grid_size + grid_size / 2,
        str(count),
        color='red',
        fontsize=9,
        ha='center',
        va='center'
    )

# Customize the plot
plt.title("Visualization of Object Locations with High-Density Regions")
plt.xlabel("X Coordinate")
plt.ylabel("Y Coordinate")
plt.legend()
plt.grid(True)
plt.tight_layout()

# Save the plot as an image (optional)
plt.savefig('object_locations.png', dpi=300)

# Display the plot
plt.show()


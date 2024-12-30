#!/usr/bin/env python3

import pandas as pd
import matplotlib.pyplot as plt
import matplotlib

matplotlib.use("TkAgg")

# File paths for the problematic dataset
problematic_id_file = 'uk_id.csv'
problematic_loc_file = 'uk_loc.csv'

# Load the problematic location dataset
problematic_loc_data = pd.read_csv(problematic_loc_file)
problematic_loc_data.columns = ['ObjectID', 'X', 'Y']

# Define grid size for spatial partitioning
grid_size = 1.0  # Partition space into 1x1 units
grid_partition = problematic_loc_data.copy()

# Assign grid indices to each point
grid_partition['GridX'] = (grid_partition['X'] // grid_size).astype(int)
grid_partition['GridY'] = (grid_partition['Y'] // grid_size).astype(int)

# Group by grid and count points in each grid cell
grid_counts = grid_partition.groupby(['GridX', 'GridY']).size()

# Identify grid cells with a high number of points (potential overlaps)
high_density_threshold = 10  # Arbitrary threshold for "many objects"
high_density_cells = grid_counts[grid_counts > high_density_threshold]

# Plot the dataset points to visualize clusters
plt.figure(figsize=(10, 8))
plt.scatter(problematic_loc_data['X'], problematic_loc_data['Y'], s=1, alpha=0.5, label="Objects")

# Highlight high-density regions
for (grid_x, grid_y), count in high_density_cells.items():
    plt.text(grid_x, grid_y, str(count), fontsize=8, color="red")

plt.title("Visualization of Object Locations and High-Density Clusters")
plt.xlabel("X Coordinate")
plt.ylabel("Y Coordinate")
plt.legend()
plt.grid(alpha=0.3)
plt.show()


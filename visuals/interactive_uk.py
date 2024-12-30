#!/usr/bin/env python3

import pandas as pd
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import seaborn as sns 
import numpy as np
from matplotlib import cm

# File paths
loc_file = 'syn_loc3.csv'  
id_file = 'syn_doc3.csv'    

loc_data = pd.read_csv(loc_file, header=None, names=['objectId', 'X', 'Y'])
id_data = pd.read_csv(id_file, header=None, names=['objectId', 'featureId'])

merged_data = pd.merge(loc_data, id_data, on='objectId')

unique_features = merged_data['featureId'].unique()
unique_features.sort() 

# Number of unique features
num_features = len(unique_features)

# Choose a colormap
cmap = cm.get_cmap('tab10') if num_features <= 10 else cm.get_cmap('tab20')

# Generate a color palette
colors = cmap(np.linspace(0, 1, num_features))

# Create a mapping from featureId to color
feature_color_mapping = {feature: colors[i] for i, feature in enumerate(unique_features)}

merged_data['color'] = merged_data['featureId'].map(feature_color_mapping)

# Set Seaborn style for aesthetics (optional)
sns.set(style="whitegrid")

# Define grid size for spatial partitioning
grid_size = 1.0 

# Assign grid indices
merged_data['GridX'] = (merged_data['X'] // grid_size).astype(int)
merged_data['GridY'] = (merged_data['Y'] // grid_size).astype(int)

# Group by grid and count points in each grid cell
grid_counts = merged_data.groupby(['GridX', 'GridY']).size().reset_index(name='Count')

# Identify high-density cells
high_density_threshold = 10 
high_density_cells = grid_counts[grid_counts['Count'] > high_density_threshold]

# Create the plot
plt.figure(figsize=(12, 8))

# Scatter plot with colors based on featureId
scatter = plt.scatter(
    merged_data['X'],
    merged_data['Y'],
    c=merged_data['color'],
    alpha=0.6,
    s=50,  
    label='Objects'
)

# Create a legend for featureIds
handles = []
labels = []
for feature, color in feature_color_mapping.items():
    handles.append(plt.Line2D([0], [0], marker='o', color='w', label=f'Feature {feature}',
                              markerfacecolor=color, markersize=10))
    labels.append(f'Feature {feature}')
plt.legend(handles=handles, title='Feature IDs', bbox_to_anchor=(1.05, 1), loc='upper left')

# Highlight high-density regions
# ax = plt.gca()
# for _, row in high_density_cells.iterrows():
#     grid_x = row['GridX']
#     grid_y = row['GridY']
#     count = row['Count']
    
#     # Draw a rectangle around the high-density grid cell
#     rect = patches.Rectangle(
#         (grid_x * grid_size, grid_y * grid_size),
#         grid_size,
#         grid_size,
#         linewidth=2,
#         edgecolor='black',
#         facecolor='none'
#     )
#     ax.add_patch(rect)
    
#     # Annotate the count
#     plt.text(
#         grid_x * grid_size + grid_size / 2,
#         grid_y * grid_size + grid_size / 2,
#         str(count),
#         color='black',
#         fontsize=12,
#         ha='center',
#         va='center',
#         weight='bold'
#     )

# Customize the plot
plt.title("Interactive Visualization of Object Locations by Feature")
plt.xlabel("X Coordinate")
plt.ylabel("Y Coordinate")
plt.grid(True)
plt.tight_layout()

# Save the plot as an image (optional)
#plt.savefig('object_locations_colored.png', dpi=300, bbox_inches='tight')

# Display the plot
plt.show()


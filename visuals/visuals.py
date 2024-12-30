#!/usr/bin/env python3

import pandas as pd
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import seaborn as sns 
import numpy as np

# Revert to older API
import matplotlib.cm as cm  

# For hover labels
# Make sure you have installed mplcursors: `pip install mplcursors`
import mplcursors

loc_file = 'blm_loc.csv'
id_file = 'blm_id.csv'

loc_data = pd.read_csv(loc_file, header=None, names=['objectId', 'X', 'Y'])
id_data = pd.read_csv(id_file, header=None, names=['objectId', 'featureId'])

merged_data = pd.merge(loc_data, id_data, on='objectId')

unique_features = merged_data['featureId'].unique()
unique_features.sort()

# Number of unique features
num_features = len(unique_features)
print(f"\nTotal number of unique features: {num_features}")

# Count how many objects each feature has
feature_counts = merged_data.groupby('featureId').size()

print("Number of objects per feature:")
for feature, count in feature_counts.items():
    print(f"  Feature {feature}: {count} objects")

# Use the older cm API
if num_features <= 10:
    cmap = cm.get_cmap('tab10')
else:
    cmap = cm.get_cmap('tab20')

# Generate a color palette
colors = cmap(np.linspace(0, 1, num_features))

# Create a mapping from featureId to color
feature_color_mapping = {feature: colors[i] for i, feature in enumerate(unique_features)}
merged_data['color'] = merged_data['featureId'].map(feature_color_mapping)

# Convert X, Y to arrays for indexing
Xvals = merged_data['X'].to_numpy()
Yvals = merged_data['Y'].to_numpy()

sns.set(style="whitegrid")

grid_size = 1.0
merged_data['GridX'] = (merged_data['X'] // grid_size).astype(int)
merged_data['GridY'] = (merged_data['Y'] // grid_size).astype(int)

grid_counts = merged_data.groupby(['GridX', 'GridY']).size().reset_index(name='Count')
high_density_threshold = 10 
high_density_cells = grid_counts[grid_counts['Count'] > high_density_threshold]

plt.figure(figsize=(12, 8))

scatter = plt.scatter(
    Xvals,
    Yvals,
    c=merged_data['color'],
    alpha=0.6,
    s=50,  
    label='Objects'
)

handles = []
labels = []
for feature, color in feature_color_mapping.items():
    handles.append(
        plt.Line2D([0], [0], marker='o',
                   color='w',
                   label=f'Feature {feature}',
                   markerfacecolor=color,
                   markersize=10)
    )
    labels.append(f'Feature {feature}')

plt.legend(handles=handles, title='Feature IDs', bbox_to_anchor=(1.05, 1), loc='upper left')
plt.title("Interactive Visualization of Object Locations by Feature")
plt.xlabel("X Coordinate")
plt.ylabel("Y Coordinate")
plt.grid(True)

# Leave a bit of space for legend on the right
plt.tight_layout(rect=[0, 0, 0.8, 1])

cursor = mplcursors.cursor(scatter, hover=True)

@cursor.connect("add")
def on_add(sel):
    # 'sel.index' is the index of the selected point
    idx = sel.index
    object_id = merged_data.iloc[idx]['objectId']
    x_val = Xvals[idx]
    y_val = Yvals[idx]
    sel.annotation.set_text(
        f"Object ID: {object_id}\nX: {x_val:.2f}, Y: {y_val:.2f}"
    )

plt.show()

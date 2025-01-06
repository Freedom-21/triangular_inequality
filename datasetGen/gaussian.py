#!/usr/bin/env python3
import numpy as np
import pandas as pd

def generate_gaussian_mixture(
    cluster_params,
    objects_per_cluster,
    output_prefix='synthetic_gaussian'
):
    """
    Generates data from multiple 2D Gaussian clusters. 
    Each cluster can have multiple possible labels.

    :param cluster_params: A list of dicts. Example:
         [
           {'mean': (10, 10), 'std': 2, 'labels': [1, 2]},
           {'mean': (50, 40), 'std': 5, 'labels': [3]},
           ...
         ]
       'mean': the (x, y) center
       'std':  standard deviation (isotropic)
       'labels': list of possible featureIds in that cluster
    :param objects_per_cluster: How many points to generate for each cluster
    :param output_prefix: base name for output files
    """

    rows_loc = []
    rows_id = []
    object_id = 1

    for cluster_dict in cluster_params:
        cx, cy = cluster_dict['mean']
        std = cluster_dict['std']
        label_choices = cluster_dict['labels']

        # Generate points from normal distribution (shape: (objects_per_cluster, 2))
        points = np.random.normal(loc=[cx, cy], scale=std, size=(objects_per_cluster, 2))

        # Randomly assign among cluster's labels
        assigned_labels = np.random.choice(label_choices, size=objects_per_cluster)

        # Build rows
        for i in range(objects_per_cluster):
            x_val = points[i, 0]
            y_val = points[i, 1]
            feature_id = assigned_labels[i]

            rows_loc.append([object_id, x_val, y_val])
            rows_id.append([object_id, feature_id])
            object_id += 1

    # Convert to DataFrames
    loc_df = pd.DataFrame(rows_loc, columns=['objectId', 'X', 'Y'])
    id_df  = pd.DataFrame(rows_id,  columns=['objectId', 'featureId'])

    # Determine total count k
    k = len(loc_df)

    # Write outputs
    loc_file = f"{output_prefix}_{k}_loc.csv"
    id_file  = f"{output_prefix}_{k}_id.csv"

    loc_df.to_csv(loc_file, index=False)
    id_df.to_csv(id_file, index=False)

    print(f"Gaussian mixture dataset created: {loc_file}, {id_file} (total objects={k})")


if __name__ == "__main__":
    # Example usage:
    # cluster_params: define where each cluster is & what labels it can have
    clusters = [
        {'mean': (10, 10), 'std': 2,  'labels': [1, 2, 3]},
        {'mean': (50, 40), 'std': 5,  'labels': [4]},
        {'mean': (70, 80), 'std': 10, 'labels': [5, 6, 7, 8]},
    ]
    objects_per_cluster = 3000  # total ~ 9000
    generate_gaussian_mixture(clusters, objects_per_cluster, output_prefix='synthetic_gaussian')

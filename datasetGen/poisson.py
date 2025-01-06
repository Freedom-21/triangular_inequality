#!/usr/bin/env python3
import numpy as np
import pandas as pd

def generate_poisson_2d(
    x_max=100.0,
    y_max=100.0,
    intensity=4.0,
    num_features=105,
    output_prefix='synthetic_poisson'
):
    """
    Generates 2D homogeneous Poisson process in [0, x_max] x [0, y_max].
    Number of points N ~ Poisson(lambda * Area), where lambda=intensity.
    Then each point gets a random label from [1..num_features].
    
    Writes:
      synthetic_poisson_{k}_loc.csv
      synthetic_poisson_{k}_id.csv
    where k is the actual # of points drawn.
    """

    area = x_max * y_max
    expected_points = intensity * area

    # 1. Number of points from Poisson
    N = np.random.poisson(expected_points)

    # 2. Uniformly generate the point coordinates
    X = np.random.uniform(low=0, high=x_max, size=N)
    Y = np.random.uniform(low=0, high=y_max, size=N)

    # 3. Randomly assign labels
    labels = np.random.randint(1, num_features + 1, size=N)

    # 4. Prepare data
    object_ids = np.arange(1, N + 1)
    
    loc_df = pd.DataFrame({'objectId': object_ids, 'X': X, 'Y': Y})
    id_df  = pd.DataFrame({'objectId': object_ids, 'featureId': labels})

    # 5. Write to CSV
    loc_file = f"{output_prefix}_{N}_loc.csv"
    id_file  = f"{output_prefix}_{N}_id.csv"

    loc_df.to_csv(loc_file, index=False)
    id_df.to_csv(id_file, index=False)

    print(f"Poisson dataset created: {loc_file}, {id_file} (total objects={N}, expected ~{int(expected_points)})")


if __name__ == "__main__":
    generate_poisson_2d()

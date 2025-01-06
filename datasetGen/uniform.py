#!/usr/bin/env python3
import numpy as np
import pandas as pd

def generate_uniform(
    num_objects=100_000,
    num_features=100,
    x_max=100.0,
    y_max=100.0
):
    """
    Generates synthetic data with uniformly distributed points, then
    randomly assigns each point a featureId from [1..num_features].
    Writes two files:
      synthetic_{k}_loc.csv   : columns [objectId, X, Y]
      synthetic_{k}_id.csv    : columns [objectId, featureId]
    where k = num_objects.
    """

    # 1. Generate uniform coordinates
    X = np.random.uniform(0, x_max, size=num_objects)
    Y = np.random.uniform(0, y_max, size=num_objects)

    # 2. Assign features uniformly
    feature_ids = np.random.randint(1, num_features + 1, size=num_objects)

    # 3. Prepare data for .csv
    object_ids = np.arange(1, num_objects + 1)

    # 4. Write synthetic_{k}_loc.csv
    loc_df = pd.DataFrame({
        'objectId': object_ids,
        'X': X,
        'Y': Y
    })
    loc_file = f"synthetic_{num_objects}_loc.csv"
    loc_df.to_csv(loc_file, index=False)

    # 5. Write synthetic_{k}_id.csv
    id_df = pd.DataFrame({
        'objectId': object_ids,
        'featureId': feature_ids
    })
    id_file = f"synthetic_{num_objects}_id.csv"
    id_df.to_csv(id_file, index=False)

    print(f"Uniform dataset created: {loc_file}, {id_file} (total objects={num_objects})")


if __name__ == "__main__":
    generate_uniform()

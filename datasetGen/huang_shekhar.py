#!/usr/bin/env python3
import numpy as np
import pandas as pd
import random

def generate_synthetic_coloc(
    N_coLoc=20,         # Number of co-location sets
    lambda1=5,          # Mean of Poisson for label-set size
    lambda2=5,          # Mean of Poisson for #instances per co-location pattern
    m_overlap=1,        # Number of overlaps to create a "maximal co-location"
    m_clump=5,          # #objects per label in each instance
    D=10**6,            # Size of the bounding region [0,D] x [0,D]
    d=10,               # Size of each "cell" into which objects are placed
    r_noisy_label=0.5,  # Fraction for noisy labels vs. real labels
    r_noisy_num=0.5,    # Fraction for noisy instances vs. real instances
    seed=42             # For reproducibility
):
    """
    Mimics the synthetic data generation approach from:
      - Huang et al. (TKDE 2004),
      - Other references [13], [22].

    Steps:
      1) Label set generation (Poisson(lambda1)).
      2) Instance construction (Poisson(lambda2) per set; place inside d x d cells).
      3) Noise injection (some fraction of noisy labels & instances).
    
    Returns:
      loc_df (objectId, X, Y),
      id_df (objectId, featureId)
    """

    # F = 50
    rng = np.random.default_rng(seed)

    # ------------------
    # 1) Label Set Generation
    # ------------------
    # Real label sets
    real_label_sets = []
    for _ in range(N_coLoc):
        # Number of labels in this set ~ Poisson(lambda1)
        set_size = rng.poisson(lambda1)
        if set_size < 1:
            set_size = 1  # ensure at least 1 label
        # Generate label IDs arbitrarily from e.g. 1..10000 (or big range)
        label_ids = rng.choice(range(1, 200), size=set_size, replace=False)
        
        # Create "maximal" co-location patterns by augmenting w/ m_overlap labels
        # If you strictly want 1 overlap: just add 1 label. 
        # If you want multiple overlap, do m_overlap times.
        # Here we do the simpler approach: add exactly m_overlap new labels.
        new_labels = rng.choice(range(1, 200), size=m_overlap, replace=False)
        max_pattern = np.unique(np.concatenate([label_ids, new_labels]))
        
        real_label_sets.append(max_pattern)

    # Count how many real labels in total
    all_real_labels = np.concatenate(real_label_sets)
    n1 = len(all_real_labels)  # needed for noise injection

    # ------------------
    # 2) Instance Construction
    # ------------------
    # For each "maximal co-location pattern," we create a certain # of instances
    # ~ Poisson(lambda2). Then each instance => place m_clump objects per label
    # in a random (d x d) cell inside [0,D] x [0,D].
    objects_loc = []
    objects_id  = []
    object_counter = 1

    num_real_instances = 0  # to keep track, needed for noise injection

    for pattern in real_label_sets:
        # how many instances does this pattern get?
        n_instances = rng.poisson(lambda2)
        if n_instances < 1:
            n_instances = 1
        num_real_instances += n_instances
        
        for _inst in range(n_instances):
            # pick a random cell
            cell_origin_x = rng.uniform(0, D - d)
            cell_origin_y = rng.uniform(0, D - d)
            # place objects
            for label in pattern:
                # we place m_clump objects for this label inside the cell
                # so all m_clump objects are near each other => controlling "distance"
                x_vals = rng.uniform(cell_origin_x, cell_origin_x + d, m_clump)
                y_vals = rng.uniform(cell_origin_y, cell_origin_y + d, m_clump)
                for i in range(m_clump):
                    objects_loc.append([object_counter, x_vals[i], y_vals[i]])
                    objects_id.append([object_counter, label])
                    object_counter += 1

    # ------------------
    # 3) Noise Injection
    # ------------------
    # Step 3a) Generate some # of noisy labels
    # r_noisy_label * n1 = # of new "fake" label IDs
    n_noisy_labels = int(r_noisy_label * n1)
    # pick them from the same large range but ensure we don't reuse existing IDs
    used_labels = set(all_real_labels)
    noisy_labels = []
    while len(noisy_labels) < n_noisy_labels:
        candidate = rng.integers(1, 500)
        if candidate not in used_labels:
            noisy_labels.append(candidate)
            used_labels.add(candidate)
    noisy_labels = np.array(noisy_labels)

    # Step 3b) Noisy instances
    # r_noisy_num * n2 => # of noisy instances
    # n2 is the total # of real instances we had
    n2 = num_real_instances
    num_noisy_instances = int(r_noisy_num * n2)

    # We'll create "noisy patterns" from the noisy labels just like the real ones
    # E.g., each noisy pattern might have random size from [1..some range].
    # Then we place them in random cells as well.
    for _noise_inst in range(num_noisy_instances):
        # pick a random subset of the noisy labels
        # e.g. 1..(some small number). We'll do Poisson(lambda1) again
        noisy_set_size = rng.poisson(lambda1)
        if noisy_set_size < 1:
            noisy_set_size = 1
        chosen_noisy_labels = rng.choice(noisy_labels, size=noisy_set_size, replace=False)

        # pick a random cell
        cell_origin_x = rng.uniform(0, D - d)
        cell_origin_y = rng.uniform(0, D - d)

        # place the objects
        for label in chosen_noisy_labels:
            x_vals = rng.uniform(cell_origin_x, cell_origin_x + d, m_clump)
            y_vals = rng.uniform(cell_origin_y, cell_origin_y + d, m_clump)
            for i in range(m_clump):
                objects_loc.append([object_counter, x_vals[i], y_vals[i]])
                objects_id.append([object_counter, label])
                object_counter += 1

    # 4) Convert to DataFrames & return
    loc_df = pd.DataFrame(objects_loc, columns=['objectId', 'X', 'Y'])
    id_df  = pd.DataFrame(objects_id,  columns=['objectId', 'featureId'])
    return loc_df, id_df


if __name__ == "__main__":

    # Default params from your snippet:
    # N_coLoc=20, lambda1=5, D=1e6, d=10, r_noisy_label=0.5, r_noisy_num=0.5
    # The paper also used ~94,028 objects and 462 labels under some config,
    # so you might tweak m_clump, lambda2, etc., to get that scale.

    loc_df, id_df = generate_synthetic_coloc(
        N_coLoc=20,
        lambda1=5,
        lambda2=50,
        m_overlap=10,
        m_clump=1,
        D=10**6,
        d=10,
        r_noisy_label=0.5,
        r_noisy_num=0.5,
        seed=420004
    )

    # Example: write two CSV files
    total_objects = len(loc_df)
    loc_file = f"synthetic_{total_objects}_loc.csv"
    id_file  = f"synthetic_{total_objects}_id.csv"

    loc_df.to_csv(loc_file, index=False, header=False)
    id_df.to_csv(id_file, index=False, header=False)
    print(f"Wrote {loc_file} and {id_file} with {total_objects} objects total.")

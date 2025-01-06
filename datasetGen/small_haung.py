#!/usr/bin/env python3
import numpy as np

def small_example(N_coLoc=2, F=5, lambda1=2, lambda2=2, m_overlap=1, m_clump=2, D=100, d=10, seed=42):
    rng = np.random.default_rng(seed)

    # Step 1: Label set generation
    label_sets = []
    for _ in range(N_coLoc):
        # Poisson(lambda1)
        set_size = rng.poisson(lambda1)
        if set_size < 1:
            set_size = 1
        # pick from 1..F
        base_labels = rng.choice(range(1, F+1), size=set_size, replace=False)
        # If we do the "maximal" step:
        new_label = rng.choice(range(1, F+1), size=m_overlap, replace=False)
        augmented = np.unique(np.concatenate([base_labels, new_label]))
        label_sets.append(augmented)

    # Step 2: Instance Construction
    object_counter = 1
    objects_loc = []
    objects_id = []

    for pattern in label_sets:
        # how many instances?
        n_inst = rng.poisson(lambda2)
        if n_inst < 1:
            n_inst = 1
        for _ in range(n_inst):
            cell_x = rng.uniform(0, D - d)
            cell_y = rng.uniform(0, D - d)
            for label in pattern:
                # place m_clump objects
                x_vals = rng.uniform(cell_x, cell_x + d, m_clump)
                y_vals = rng.uniform(cell_y, cell_y + d, m_clump)
                for i in range(m_clump):
                    objects_loc.append((object_counter, x_vals[i], y_vals[i]))
                    objects_id.append((object_counter, label))
                    object_counter += 1

    print("Label sets: ", label_sets)
    print("Total objects: ", len(objects_loc))
    print("Sample of object loc: ", objects_loc[:10])
    print("Sample of object id: ", objects_id[:10])

small_example()

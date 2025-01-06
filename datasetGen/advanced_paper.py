#!/usr/bin/env python3

import numpy as np
import random

def rand_f(min_v, max_v):
    """Generate a random float in [min_v, max_v]."""
    return random.uniform(min_v, max_v)

def rand_i(min_v, max_v):
    """Generate a random integer in [min_v, max_v]."""
    return random.randint(min_v, max_v)

def generate_synthetic_data2(
    # Parameters
    Ncoloc=20,         # number of co-location "core" patterns
    lambda1=5,         # average (Poisson) # of features in each "core" pattern
    lambda2=10,        # average (Poisson) table size (i.e. #instances)
    seed1=123,         # random seed for the pattern size distribution
    seed2=456,         # random seed for the table size distribution
    D=1_000_000,       # bounding region [0..D] x [0..D]
    d=10,              # size of each local neighborhood
    r_noise_fea=0.5,   # ratio of additional (new) noise features
    r_local=0.0,       # fraction of "local noise" objects
    r_global=0.0,      # fraction of "global noise" objects
    m_overlap=1,       # for each core pattern, we create m_overlap "maximal" patterns
    m_clump=1,         # # of objects per feature (fixed or maximum if random)
    m_clump_type=0,    # 0 => all features have m_clump objects; 1 => random # in [1..m_clump]
    loc_file='synthetic_loc.csv',
    id_file='synythetic_id.csv'
):
    """
    Mimics the synthetic data generation approach from:
      - Huang et al. (TKDE 2004),

    Steps:
      1) Label set generation (Poisson(lambda1)).
      2) Instance construction (Poisson(lambda2) per set; place inside d x d cells).
      3) Noise injection (some fraction of noisy labels & instances).

    Writes two files:
      1) loc_file: "objectID,x,y"
      2) id_file: "objectID,featureID"

    Steps (matching the paper):
      1) For i in [0..Ncoloc):
         - patternSize[i] ~ Poisson(lambda1) => base pattern has 'patternSize[i]' features
         - For a in [0..m_overlap):
             + tableSize ~ Poisson(lambda2)
             + extrafeaID = feaID + patternSize[i] + a (the "one more" feature)
             + Build an array m_clump_value[] of length (patternSize[i] + 1).
               If m_clump_type=0 => each is m_clump
               If m_clump_type=1 => each is a random int in [1..m_clump]
             + For j in [0..tableSize):
                 # pick random cell corner (rectX, rectY) in [0..D-d]
                 # for each base feature in [feaID..feaID + patternSize[i] - 1],
                 # place m_clump_value[k] objects
                 # also place m_clump_value[last] objects for extrafeaID
         - feaID += patternSize[i] + m_overlap
      2) local noise => r_local * num_instance
         - pick random feature in [1..feaID], random (x,y) in [0..D]
      3) global noise => expand feaID = feaID * (1 + r_noise_fea)
         - pick random feature in [oldFeaID+1..feaID], random (x,y) in [0..D]
    """

    # 1) Setup Poisson distributions
    rng1 = np.random.default_rng(seed1)  # for pattern sizes
    rng2 = np.random.default_rng(seed2)  # for table sizes

    # If we want random m_clump with the same seed2, we can do:
    rng_clump = np.random.default_rng(seed2)

    # open output files
    loc_fp = open(loc_file, 'w')
    id_fp = open(id_file, 'w')

    objectID = 1
    feaID = 1
    num_instance = 0  # count how many objects placed, for noise calc

    # 2) For each of the Ncoloc patterns
    pattern_sizes = rng1.poisson(lambda1, size=Ncoloc)
    for i in range(Ncoloc):
        base_size = pattern_sizes[i] if pattern_sizes[i] > 0 else 1

        # For a in [0..m_overlap): we create "maximal" patterns of size base_size + 1
        for a in range(m_overlap):
            table_size = rng2.poisson(lambda2)
            if table_size < 1:
                table_size = 1

            # extrafeaID is the "one more" feature
            extra_fea = feaID + base_size + a

            # allocate m_clump_value array for patternSize[i] + 1 features

            clump_values = []
            for k in range(base_size + 1):
                if m_clump_type == 0:
                    clump_values.append(m_clump)
                else:
                    val = rand_i(1, m_clump)  # in [1..m_clump]
                    clump_values.append(val)

            # Place table_size lumps for that "maximal" colocation
            # i.e. each row => pick a rect corner => place objects
            for _ in range(table_size):
                rectX = rand_f(0, D - d)
                rectY = rand_f(0, D - d)

                # for k in [0..(base_size-1)] => feature feaID + k
                for k in range(base_size):
                    n_objs = clump_values[k]
                    feature_k = feaID + k
                    # place n_objs in [rectX..rectX+d], [rectY..rectY+d]
                    for _b in range(n_objs):
                        x = rand_f(rectX, rectX + d)
                        y = rand_f(rectY, rectY + d)
                        loc_fp.write(f"{objectID},{x},{y}\n")
                        id_fp.write(f"{objectID},{feature_k}\n")
                        objectID += 1

                # place objects for the extra feature
                n_extra = clump_values[base_size]
                for _b in range(n_extra):
                    x = rand_f(rectX, rectX + d)
                    y = rand_f(rectY, rectY + d)
                    loc_fp.write(f"{objectID},{x},{y}\n")
                    id_fp.write(f"{objectID},{extra_fea}\n")
                    objectID += 1

            # add #objects for this pattern => table_size * (base_size+1) * average clump
            num_instance += table_size * (base_size + 1) * m_clump

        # after create m_overlap patterns from the same "core", we jump feaID
        feaID += (base_size + m_overlap)

    # 3) Local noise: r_local * num_instance
    local_noise_count = int(r_local * num_instance)
    for _ln in range(local_noise_count):
        # pick from existing features [1..feaID-1]
        feat = rand_i(1, feaID - 1)
        x = rand_f(0, D)
        y = rand_f(0, D)
        loc_fp.write(f"{objectID},{x},{y}\n")
        id_fp.write(f"{objectID},{feat}\n")
        objectID += 1

    # 4) Global noise: we expand feaID = int(feaID*(1 + r_noise_fea))
    originalFeaID = feaID
    feaID = int(feaID * (1.0 + r_noise_fea))

    global_noise_count = int(r_global * num_instance)
    for _gn in range(global_noise_count):
        # pick from [originalFeaID..feaID]
        feat = rand_i(originalFeaID, feaID)
        x = rand_f(0, D)
        y = rand_f(0, D)
        loc_fp.write(f"{objectID},{x},{y}\n")
        id_fp.write(f"{objectID},{feat}\n")
        objectID += 1

    loc_fp.close()
    id_fp.close()

    final_obj = objectID - 1
    print(f"total number of obj: {final_obj}")
    print(f"total number of feature: {feaID}")
    print(f"*fea[1..{originalFeaID - 1}] are original, [ {originalFeaID}..{feaID} ] are noise.")

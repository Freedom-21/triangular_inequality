#!/usr/bin/env python3

import numpy as np
import random
import sys

def rand_f(min_v, max_v):
    """Generate a random float in [min_v, max_v]."""
    return random.uniform(min_v, max_v)

def rand_i(min_v, max_v):
    """Generate a random integer in [min_v, max_v]."""
    return random.randint(min_v, max_v)

def generate_synthetic_data(
    Ncoloc=20,         # number of co-location patterns
    lambda1=5,         # average #features per pattern
    lambda2=10,        # average table size (instances)
    seed1=123,         # seed for patternSize
    seed2=456,         # seed for tableSize
    D1=1_000_000,      # bounding region for X
    D2=1_000_000,      # bounding region for Y
    d=10,              # size of the local neighborhood
    r_noise_fea=0.5,   # ratio of new noise features
    r_local=0.0,       # fraction of local noise objects
    r_global=0.0,      # fraction of global noise objects
    loc_file='synthetic_loc.csv',
    doc_file='synthetic_doc.csv'
):
    """
    Python version of gen_syn_data() from the C++ code.
    Outputs 2 CSV files:
      loc_file: "objectID,x,y"
      doc_file: "objectID,featureID"
    """
    # 1) Poisson distributions
    #   The C++ code uses std::poisson_distribution with two separate random engines (seed1, seed2).
    rng1 = np.random.default_rng(seed1)
    rng2 = np.random.default_rng(seed2)

    # object & feature counters
    objectID = 1
    feaID = 1
    num_instance = 0  # track how many objects total

    # open output files
    loc_fp = open(loc_file, 'w')
    doc_fp = open(doc_file, 'w')

    # 2) Generate Ncoloc patterns
    patternSizes = rng1.poisson(lam=lambda1, size=Ncoloc)  # draw Ncoloc from Poisson(lambda1)

    for i in range(Ncoloc):
        patternSize = patternSizes[i]  # how many features in this pattern?
        if patternSize < 1:
            patternSize = 1

        # next we draw from the second Poisson to see how many times to replicate
        tableSize = rng2.poisson(lam=lambda2)
        if tableSize < 1:
            tableSize = 1

        # increment num_instance by how many objects we'll place (the c++ logic does patternSize * tableSize)
        # but let's keep consistent with the code's definition:
        num_instance += tableSize * patternSize

        # features are [feaID, feaID+1, ..., feaID+patternSize-1]
        # place tableSize lumps for each feature
        for row in range(tableSize):
            rectX = rand_f(0, D1 - d)
            rectY = rand_f(0, D2 - d)
            # place one object per feature
            for k in range(patternSize):
                locX = rand_f(rectX, rectX + d)
                locY = rand_f(rectY, rectY + d)
                # write to CSV
                loc_fp.write(f"{objectID},{locX},{locY}\n")
                doc_fp.write(f"{objectID},{(feaID + k)}\n")
                objectID += 1

        # done with this pattern
        feaID += patternSize

    # 3) Local noise: r_local * num_instance objects
    local_noise_count = int(r_local * num_instance)
    for _ in range(local_noise_count):
        # pick from existing features
        random_feature = rand_i(1, feaID - 1)
        locX = rand_f(0, D1)
        locY = rand_f(0, D2)
        loc_fp.write(f"{objectID},{locX},{locY}\n")
        doc_fp.write(f"{objectID},{random_feature}\n")
        objectID += 1

    # 4) Global noise: r_global * num_instance objects
    #   We add new features. c++ code:
    #   originalFeaID = feaID
    #   feaID = int(feaID*(1.0 + r_noise_fea))
    originalFeaID = feaID
    feaID = int(feaID * (1.0 + r_noise_fea))  # new range [originalFeaID..feaID]

    global_noise_count = int(r_global * num_instance)
    for _ in range(global_noise_count):
        random_feature = rand_i(originalFeaID, feaID)
        locX = rand_f(0, D1)
        locY = rand_f(0, D2)
        loc_fp.write(f"{objectID},{locX},{locY}\n")
        doc_fp.write(f"{objectID},{random_feature}\n")
        objectID += 1

    loc_fp.close()
    doc_fp.close()

    final_obj = objectID - 1
    print(f"total number of obj: {final_obj}")
    print(f"total number of feature: {feaID}")
    print(f"*fea[1,{originalFeaID - 1}] are original, [{originalFeaID},{feaID}] are noise")


if __name__ == "__main__":
    # Example usage:
    generate_synthetic_data(
        Ncoloc=20,
        lambda1=5,
        lambda2=80,
        seed1=123,
        seed2=4563,
        D1=1_000_000,
        D2=1_000_000,
        d=10,
        r_noise_fea=0.5,
        r_local=0.3,
        r_global=0.5,
        loc_file='syn_loc3.csv',
        doc_file='syn_id3.csv'
    )

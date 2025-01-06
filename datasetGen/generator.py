#!/usr/bin/env python3

from advanced_paper import generate_synthetic_data2


if __name__ == "__main__":
    generate_synthetic_data2(
        Ncoloc=25,         # # co-locations
        lambda1=5,        # average # features in each base pattern
        lambda2=100,        # average # table instances per pattern        
        seed1=13131,         # random seed for pattern size
        seed2=13131,         # random seed for table size
        D=10**6,         # bounding region
        d=50,            # each lump is 10x10
        r_noise_fea=0.5,  # 50% more features as noise
        r_local=0.2,      # local noise is 20% of total objects
        r_global=0.3,     # global noise is 30% of total objects
        m_overlap=10,      # each base pattern => 2 "maximal" patterns
        m_clump=4,        # each feature has 3 objects if m_clump_type=0
        m_clump_type=0,   # 0 => fixed #objects per feature, 1 => random in [1..m_clump]
        loc_file='mc4_synthetic_loc.csv',
        id_file='mc4_synthetic_id.csv'
    )
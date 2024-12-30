#!/usr/bin/env python3
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

# Example data with multiple datasets in the same DataFrame
results_varied = [
    # D1, dist=10, vary PI
    {'Dataset': 'D1', 'Algorithm': 'Joinless', 'PI': 0.50, 'dist': 10, 'Time': 460.86, 'Memory': 1495.53},
    {'Dataset': 'D1', 'Algorithm': 'Improved', 'PI': 0.50, 'dist': 10, 'Time': 302.16, 'Memory': 528.59},
    {'Dataset': 'D1', 'Algorithm': 'Joinless', 'PI': 0.60, 'dist': 10, 'Time': 370.12, 'Memory': 1200.00},
    {'Dataset': 'D1', 'Algorithm': 'Improved', 'PI': 0.60, 'dist': 10, 'Time': 250.40, 'Memory': 500.00},
    {'Dataset': 'D1', 'Algorithm': 'Joinless', 'PI': 0.70, 'dist': 10, 'Time': 362.91, 'Memory': 601.94},
    {'Dataset': 'D1', 'Algorithm': 'Improved', 'PI': 0.70, 'dist': 10, 'Time': 182.55, 'Memory': 188.33},

    # D2, dist=10, vary PI
    {'Dataset': 'D2', 'Algorithm': 'Joinless', 'PI': 0.50, 'dist': 10, 'Time': 300.00, 'Memory': 600.00},
    {'Dataset': 'D2', 'Algorithm': 'Improved', 'PI': 0.50, 'dist': 10, 'Time': 220.00, 'Memory': 400.00},
    {'Dataset': 'D2', 'Algorithm': 'Joinless', 'PI': 0.60, 'dist': 10, 'Time': 250.00, 'Memory': 550.00},
    {'Dataset': 'D2', 'Algorithm': 'Improved', 'PI': 0.60, 'dist': 10, 'Time': 200.00, 'Memory': 350.00},
    {'Dataset': 'D2', 'Algorithm': 'Joinless', 'PI': 0.70, 'dist': 10, 'Time': 362.91, 'Memory': 601.94},
    {'Dataset': 'D2', 'Algorithm': 'Improved', 'PI': 0.70, 'dist': 10, 'Time': 182.55, 'Memory': 188.33},
]

df = pd.DataFrame(results_varied)

# Create facet subplots by 'Dataset', line-plot Time vs. PI, separated by Algorithm
g = sns.FacetGrid(df, col='Dataset', hue='Algorithm', height=4, aspect=1.2)
g.map(sns.lineplot, 'PI', 'Time', marker='o').add_legend()
plt.suptitle('Time vs. PI for Multiple Datasets', y=1.05)
plt.show()

# Similarly, you could do Memory vs. PI facet
g2 = sns.FacetGrid(df, col='Dataset', hue='Algorithm', height=4, aspect=1.2)
g2.map(sns.lineplot, 'PI', 'Memory', marker='o').add_legend()
plt.suptitle('Memory vs. PI for Multiple Datasets', y=1.05)
plt.show()

#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Suppose we fix dist=10 for dataset D1, but vary PI
results_varied = [
    {'Dataset': 'D1', 'Algorithm': 'Joinless', 'PI': 0.50, 'dist': 10, 'Time': 460.86, 'Memory': 1495.53},
    {'Dataset': 'D1', 'Algorithm': 'Improved', 'PI': 0.50, 'dist': 10, 'Time': 302.16, 'Memory': 528.59},
    
    {'Dataset': 'D1', 'Algorithm': 'Joinless', 'PI': 0.60, 'dist': 10, 'Time': 370.12, 'Memory': 1200.00},
    {'Dataset': 'D1', 'Algorithm': 'Improved', 'PI': 0.60, 'dist': 10, 'Time': 250.40, 'Memory': 500.00},
    
    {'Dataset': 'D1', 'Algorithm': 'Joinless', 'PI': 0.70, 'dist': 10, 'Time': 362.91, 'Memory': 601.94},
    {'Dataset': 'D1', 'Algorithm': 'Improved', 'PI': 0.70, 'dist': 10, 'Time': 182.55, 'Memory': 188.33},
    # Add more as you like...
]

df_varied = pd.DataFrame(results_varied)

# Filter for D1, dist=10
df_d1_dist10 = df_varied[(df_varied['Dataset'] == 'D1') & (df_varied['dist'] == 10)]

# TIME vs. PI
plt.figure(figsize=(6, 4))
sns.lineplot(data=df_d1_dist10, x='PI', y='Time', hue='Algorithm', marker='o')
plt.title('Time vs. PI for D1 (dist=10)')
plt.ylabel('Time (seconds)')
plt.xlabel('Participation Index (PI)')
plt.grid(True)
plt.tight_layout()
plt.show()

# MEMORY vs. PI
plt.figure(figsize=(6, 4))
sns.lineplot(data=df_d1_dist10, x='PI', y='Memory', hue='Algorithm', marker='o')
plt.title('Memory vs. PI for D1 (dist=10)')
plt.ylabel('Memory (MB)')
plt.xlabel('Participation Index (PI)')
plt.grid(True)
plt.tight_layout()
plt.show()

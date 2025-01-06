#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

results_varied = [
    {'Dataset': 'D2', 'Algorithm': 'Joinless', 'PI': 0.50, 'dist': 5,  'Time': 95.3, 'Memory': 1845.97},
    {'Dataset': 'D2', 'Algorithm': 'Improved', 'PI': 0.50, 'dist': 5,  'Time': 76.54, 'Memory': 748.32},

    {'Dataset': 'D2', 'Algorithm': 'Joinless', 'PI': 0.50, 'dist': 8,  'Time': 176.87, 'Memory': 1905.11},
    {'Dataset': 'D2', 'Algorithm': 'Improved', 'PI': 0.50, 'dist': 8,  'Time': 112.47, 'Memory': 748.32},
    
    {'Dataset': 'D2', 'Algorithm': 'Joinless', 'PI': 0.50, 'dist': 10, 'Time': 310.76, 'Memory': 2019.17},
    {'Dataset': 'D2', 'Algorithm': 'Improved', 'PI': 0.50, 'dist': 10, 'Time': 187.95, 'Memory':748.32},
    
    {'Dataset': 'D2', 'Algorithm': 'Joinless', 'PI': 0.50, 'dist': 15, 'Time': 309, 'Memory': 2042.83},
    {'Dataset': 'D2', 'Algorithm': 'Improved', 'PI': 0.50, 'dist': 15, 'Time': 149.55, 'Memory': 748.32},
]

df_varied = pd.DataFrame(results_varied)

# Filter for D2, PI=0.50
df_d2_pi50 = df_varied[(df_varied['Dataset'] == 'D2') & (df_varied['PI'] == 0.50)]

# TIME vs. distance
plt.figure(figsize=(6, 4))
sns.lineplot(data=df_d2_pi50, x='dist', y='Time', hue='Algorithm', marker='o')
plt.title('Time vs. Distance for synthetic_5 Dataset with (Obj: 112k, Feature: 565 (PI=0.50)')
plt.ylabel('Time (seconds)')
plt.xlabel('Distance Threshold')
plt.grid(True)
plt.tight_layout()
plt.show()

# MEMORY vs. distance
plt.figure(figsize=(6, 4))
sns.lineplot(data=df_d2_pi50, x='dist', y='Memory', hue='Algorithm', marker='o')
plt.title('Memory vs. Distance synthetic_5 Dataset with (Obj: 112k, Feature: 565 (PI=0.50)')
plt.ylabel('Memory (MB)')
plt.xlabel('Distance Threshold')
plt.grid(True)
plt.tight_layout()
plt.show()

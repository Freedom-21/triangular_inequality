#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Suppose we fix dist=10 for dataset D1, but vary PI
results_varied = [
    {'Dataset': 'D1', 'Algorithm': 'Joinless', 'PI': 0.20, 'dist': 10,  'Time': 311.18, 'Memory': 2019.35},
    {'Dataset': 'D1', 'Algorithm': 'Improved', 'PI': 0.20, 'dist': 10,  'Time': 184.47, 'Memory': 748.32},

    {'Dataset': 'D1', 'Algorithm': 'Joinless', 'PI': 0.50, 'dist': 10,  'Time': 310.76, 'Memory': 2019.17},
    {'Dataset': 'D1', 'Algorithm': 'Improved', 'PI': 0.50, 'dist': 10,  'Time': 187.95, 'Memory': 748.32},
    
    {'Dataset': 'D1', 'Algorithm': 'Joinless', 'PI': 0.70, 'dist': 10, 'Time': 284.05, 'Memory': 1993.82},
    {'Dataset': 'D1', 'Algorithm': 'Improved', 'PI': 0.70, 'dist': 10, 'Time': 160.99, 'Memory': 748.32},
    
    {'Dataset': 'D1', 'Algorithm': 'Joinless', 'PI': 0.90, 'dist': 10, 'Time': 103.4, 'Memory': 1856.75},
    {'Dataset': 'D1', 'Algorithm': 'Improved', 'PI': 0.90, 'dist': 10, 'Time': 8.55, 'Memory': 748.35},

]

df_varied = pd.DataFrame(results_varied)

# Filter for D1, dist=10
df_d1_dist10 = df_varied[(df_varied['Dataset'] == 'D1') & (df_varied['dist'] == 10)]

# TIME vs. PI
plt.figure(figsize=(6, 4))
sns.lineplot(data=df_d1_dist10, x='PI', y='Time', hue='Algorithm', marker='o')
plt.title('Time vs. PI for synthetic_5 Dataset with (Obj: 112k, Feature: 565) (dist=10)')
plt.ylabel('Time (seconds)')
plt.xlabel('Participation Index (PI)')
plt.grid(True)
plt.tight_layout()
plt.show()

# MEMORY vs. PI
plt.figure(figsize=(6, 4))
sns.lineplot(data=df_d1_dist10, x='PI', y='Memory', hue='Algorithm', marker='o')
plt.title('Memory vs. PI for synthetic_5 Dataset with (Obj: 112k, Feature: 565) (dist=10)')
plt.ylabel('Memory (MB)')
plt.xlabel('Participation Index (PI)')
plt.grid(True)
plt.tight_layout()
plt.show()

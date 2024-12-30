#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

results_varied = [
    {'Dataset': 'D2', 'Algorithm': 'Joinless', 'PI': 0.50, 'dist': 5,  'Time': 17.6, 'Memory': 344.00},
    {'Dataset': 'D2', 'Algorithm': 'Improved', 'PI': 0.50, 'dist': 5,  'Time': 11.75, 'Memory': 174.63},

    {'Dataset': 'D2', 'Algorithm': 'Joinless', 'PI': 0.50, 'dist': 10,  'Time': 393.00, 'Memory': 638.00},
    {'Dataset': 'D2', 'Algorithm': 'Improved', 'PI': 0.50, 'dist': 10,  'Time': 236.9, 'Memory': 194.63},
    
    {'Dataset': 'D2', 'Algorithm': 'Joinless', 'PI': 0.50, 'dist': 15, 'Time': 380.00, 'Memory': 693.00},
    {'Dataset': 'D2', 'Algorithm': 'Improved', 'PI': 0.50, 'dist': 15, 'Time': 134.00, 'Memory': 190.00},
    
    {'Dataset': 'D2', 'Algorithm': 'Joinless', 'PI': 0.50, 'dist': 20, 'Time': 380.00, 'Memory': 693.00},
    {'Dataset': 'D2', 'Algorithm': 'Improved', 'PI': 0.50, 'dist': 20, 'Time': 61.00, 'Memory': 177.00},
]

df_varied = pd.DataFrame(results_varied)

# Filter for D2, PI=0.50
df_d2_pi50 = df_varied[(df_varied['Dataset'] == 'D2') & (df_varied['PI'] == 0.50)]

# TIME vs. distance
plt.figure(figsize=(6, 4))
sns.lineplot(data=df_d2_pi50, x='dist', y='Time', hue='Algorithm', marker='o')
plt.title('Time vs. Distance for D2 (PI=0.50)')
plt.ylabel('Time (seconds)')
plt.xlabel('Distance Threshold')
plt.grid(True)
plt.tight_layout()
plt.show()

# MEMORY vs. distance
plt.figure(figsize=(6, 4))
sns.lineplot(data=df_d2_pi50, x='dist', y='Memory', hue='Algorithm', marker='o')
plt.title('Memory vs. Distance for D2 (PI=0.50)')
plt.ylabel('Memory (MB)')
plt.xlabel('Distance Threshold')
plt.grid(True)
plt.tight_layout()
plt.show()

#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Sample data (replace with real measurements)
results = [
    {
        'Dataset': 'D1 (94k objs, 462 feats, PI=0.50, dist=10)',
        'Algorithm': 'Joinless',
        'Time': 460.864166,
        'Memory': 1495.53
    },
    {
        'Dataset': 'D1 (94k objs, 462 feats, PI=0.50, dist=10)',
        'Algorithm': 'Improved',
        'Time': 302.167114,
        'Memory': 528.59
    },
    {
        'Dataset': 'D2 (46k objs, 312 feats, PI=0.70, dist=10)',
        'Algorithm': 'Joinless',
        'Time': 362.912720,
        'Memory': 601.94
    },
    {
        'Dataset': 'D2 (46k objs, 312 feats, PI=0.70, dist=10)',
        'Algorithm': 'Improved',
        'Time': 182.555405,
        'Memory': 188.33
    },
    # Add more datasets as needed...
]

df = pd.DataFrame(results)

# Bar chart for Time
plt.figure(figsize=(8, 6))
sns.barplot(data=df, x='Dataset', y='Time', hue='Algorithm')
plt.title('Comparison of Joinless vs. Improved Algorithm - Time')
plt.ylabel('Time (seconds)')
plt.xticks(rotation=15, ha='right')
plt.tight_layout()
plt.show()

# Bar chart for Memory
plt.figure(figsize=(8, 6))
sns.barplot(data=df, x='Dataset', y='Memory', hue='Algorithm')
plt.title('Comparison of Joinless vs. Improved Algorithm - Memory Usage')
plt.ylabel('Memory (MB)')
plt.xticks(rotation=15, ha='right')
plt.tight_layout()
plt.show()

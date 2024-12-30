#!/usr/bin/env python3
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

# Suppose you create a list of dicts with your results:
results = [
    {
        'Dataset': 'D1 (94k objs, 462 feats, p=0.5)', 
        'Algorithm': 'Joinless', 
        'Time': 460.864166, 
        'Memory': 1495.53
    },
    {
        'Dataset': 'D1 (94k objs, 462 feats, p=0.5)', 
        'Algorithm': 'Improved', 
        'Time': 302.167114, 
        'Memory': 528.59
    },
    {
        'Dataset': 'D2 (46k objs, 312 feats, p=0.7)', 
        'Algorithm': 'Joinless', 
        'Time': 362.912720, 
        'Memory': 601.94
    },
    {
        'Dataset': 'D2 (46k objs, 312 feats, p=0.7)', 
        'Algorithm': 'Improved', 
        'Time': 182.555405, 
        'Memory': 188.33
    }
    # ... add more as you test more datasets ...
]

df = pd.DataFrame(results)

# Option A: Bar chart comparing time
plt.figure(figsize=(10, 6))
sns.barplot(data=df, x='Dataset', y='Time', hue='Algorithm')
plt.title('Comparison of Joinless vs. Improved Algorithm (Time)')
plt.ylabel('Time (seconds)')
plt.xticks(rotation=15)
plt.tight_layout()
plt.show()

# Option B: Bar chart comparing memory usage
plt.figure(figsize=(10, 6))
sns.barplot(data=df, x='Dataset', y='Memory', hue='Algorithm')
plt.title('Comparison of Joinless vs. Improved Algorithm (Memory)')
plt.ylabel('Memory Usage (MB)')
plt.xticks(rotation=15)
plt.tight_layout()
plt.show()

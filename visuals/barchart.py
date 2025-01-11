#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

results = [
    # {
    #     'Dataset': 'D1 (146k objects, 585 features, PI=0.70, dist=50)',
    #     'Algorithm': 'Joinless',
    #     'Time': 1802.11,
    #     'Memory': 3214.78
    # },
    # {
    #     'Dataset': 'D1 (146k objects, 585 features, PI=0.70, dist=50)',
    #     'Algorithm': 'Improved',
    #     'Time': 786.88,
    #     'Memory': 1042
    # },
    # {
    #     'Dataset': 'D2 (Objects: 189k, Features: 576, PI=0.70, dist=50)',
    #     'Algorithm': 'Joinless',
    #     'Time': 914.67,
    #     'Memory': 3518.03
    # },
    # {
    #     'Dataset': 'D2 (Objects: 189k, Features: 576, PI=0.70, dist=50)',
    #     'Algorithm': 'Improved',
    #     'Time': 414.27,
    #     'Memory': 1283.53
    # },
    {
        'Dataset': 'D1 (Objects: 248k, Features: 546, PI=0.70, dist=50)',
        'Algorithm': 'Joinless',
        'Time': 878.58,
        'Memory': 4194.08
    },
    {
        'Dataset': 'D1 (Objects: 248k, Features: 546, PI=0.70, dist=50)',
        'Algorithm': 'Improved',
        'Time': 359.28,
        'Memory': 1594.46
    },
    {
        'Dataset': 'D2 (Objects: 332k, Features: 594, PI=0.70, dist=50)',
        'Algorithm': 'Joinless',
        'Time': 3467.67,
        'Memory': 4194.08
    },
    {
        'Dataset': 'D2 (Objects: 332k, Features: 594, PI=0.70, dist=50)',
        'Algorithm': 'Improved',
        'Time': 1017.60,
        'Memory': 2335.94
    },
    {
        'Dataset': 'D3 (Objects: 420k, Features: 601, PI=0.70, dist=50)',
        'Algorithm': 'Joinless',
        'Time': 7461.208,
        'Memory': 8791.6
    },
    {
        'Dataset': 'D3 (Objects: 420k, Features: 601, PI=0.70, dist=50)',
        'Algorithm': 'Improved',
        'Time':2460.54,
        'Memory': 3041.69
    },]

df = pd.DataFrame(results)

# Bar chart for Time
plt.figure(figsize=(8, 6))
sns.barplot(data=df, x='Dataset', y='Time', hue='Algorithm')
plt.title('Comparison - Time')
plt.ylabel('Time (seconds)')
plt.xticks(rotation=15, ha='right')
plt.tight_layout()
plt.show()

# Bar chart for Memory
plt.figure(figsize=(8, 6))
sns.barplot(data=df, x='Dataset', y='Memory', hue='Algorithm')
plt.title('Comparison - Memory Usage')
plt.ylabel('Memory (MB)')
plt.xticks(rotation=15, ha='right')
plt.tight_layout()
plt.show()

import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv('results/statistics.csv')
print(df.head())

plt.figure(figsize=(12, 8))

df = df.drop(columns=[col for col in df.columns if 'Unnamed' in col])
for column in df.columns:
    if column != 'size':
        plt.plot(df['size'], df[column], marker='o',
                 label=column, linewidth=2, markersize=4)
print(df)
plt.title('Результаты экспериментов')
plt.xlabel('Размер матрицы n')
plt.ylabel('Время перемножения, с.')
plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.show()

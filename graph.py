import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from scipy import stats
import os

# Настройка русских шрифтов
plt.rcParams['font.family'] = 'DejaVu Sans'

# Создаем папку для графиков, если её нет
output_dir = 'results/graphs'  # Исправлен разделитель на /
os.makedirs(output_dir, exist_ok=True)
print(f"Графики будут сохранены в папку: {output_dir}")

# Загрузка данных
df = pd.read_csv('results/statistics.csv')

# Определяем название столбца с процессами
process_col = 'proccess_number' if 'proccess_number' in df.columns else 'process_count'
if process_col not in df.columns:
    process_col = df.columns[1]

print(f"Используем столбец: {process_col}")
print(f"Первые 5 строк данных:")
print(df.head())

# Группировка и вычисление статистик для 5 повторений
df_stats = df.groupby(['matrix_size', process_col])['execution_time'].agg(
    mean_time='mean',
    std_time='std',
    min_time='min',
    max_time='max',
    count='count'
).reset_index()

# Переименовываем столбец для удобства
df_stats.rename(columns={process_col: 'process_count'}, inplace=True)

print(
    f"\nУникальные размеры матриц: {sorted(df_stats['matrix_size'].unique())}")
print(
    f"Уникальное число процессов: {sorted(df_stats['process_count'].unique())}")
print(f"\nПроверка количества повторений:")
print(df_stats[['matrix_size', 'process_count', 'count']].head(10))

# Получаем уникальные значения числа процессов
processes = sorted(df_stats['process_count'].unique())

# Настройка стиля графиков
plt.style.use('seaborn-v0_8-darkgrid')
colors = plt.cm.viridis(np.linspace(0, 1, len(processes)))

# ==================== ГРАФИК 1: Время выполнения (линейный масштаб) ====================
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(16, 6))

for i, proc in enumerate(processes):
    data = df_stats[df_stats['process_count'] == proc]
    ax1.plot(data['matrix_size'], data['mean_time'],
             marker='o', label=f'{proc} процесс(а/ов)',
             color=colors[i], linewidth=2, markersize=8)

    ax1.fill_between(data['matrix_size'],
                     data['mean_time'] - data['std_time'],
                     data['mean_time'] + data['std_time'],
                     alpha=0.2, color=colors[i])

ax1.set_xlabel('Размер матрицы (N x N)', fontsize=12, fontweight='bold')
ax1.set_ylabel('Время выполнения (секунды)', fontsize=12, fontweight='bold')
ax1.set_title('Зависимость времени выполнения от размера матрицы\n(линейный масштаб, ±1 стандартное отклонение)',
              fontsize=14, fontweight='bold')
ax1.legend(loc='upper left', fontsize=10)
ax1.grid(True, alpha=0.3)

# ==================== ГРАФИК 2: Логарифмический масштаб ====================
for i, proc in enumerate(processes):
    data = df_stats[df_stats['process_count'] == proc]
    ax2.loglog(data['matrix_size'], data['mean_time'],
               marker='s', label=f'{proc} процесс(а/ов)',
               color=colors[i], linewidth=2, markersize=8)

ax2.set_xlabel('Размер матрицы (N x N)', fontsize=12, fontweight='bold')
ax2.set_ylabel('Время выполнения (секунды)', fontsize=12, fontweight='bold')
ax2.set_title('Зависимость времени выполнения от размера матрицы\n(логарифмический масштаб, для анализа сложности)',
              fontsize=14, fontweight='bold')
ax2.legend(loc='upper left', fontsize=10)
ax2.grid(True, alpha=0.3, which='both')

plt.tight_layout()
plt.savefig(os.path.join(output_dir, '1_execution_time.png'),
            dpi=300, bbox_inches='tight')
plt.show()
print(f"Сохранен: {os.path.join(output_dir, '1_execution_time.png')}")

# ==================== ГРАФИК 3: Сравнение среднего времени (столбчатая диаграмма) ====================
fig, ax3 = plt.subplots(figsize=(14, 8))

# Выбираем каждый 2-й размер для читаемости
all_sizes = sorted(df_stats['matrix_size'].unique())
selected_sizes = all_sizes[::2]

# Создаем словарь для быстрого доступа к данным по каждому размеру
data_by_size = {}
for proc in processes:
    data = df_stats[df_stats['process_count'] == proc]
    data_by_size[proc] = {row['matrix_size']                          : row for _, row in data.iterrows()}

bar_width = 0.2
x_positions = np.arange(len(selected_sizes))

for i, proc in enumerate(processes):
    means = []
    stds = []
    for size in selected_sizes:
        if size in data_by_size[proc]:
            means.append(data_by_size[proc][size]['mean_time'])
            stds.append(data_by_size[proc][size]['std_time'])
        else:
            means.append(0)
            stds.append(0)

    offset = (i - len(processes)/2) * bar_width + bar_width/2

    bars = ax3.bar(x_positions + offset, means,
                   width=bar_width, label=f'{proc} процесс(а/ов)',
                   color=colors[i], alpha=0.7, edgecolor='black')

    # Добавляем error bars только для ненулевых значений
    for j, (mean, std) in enumerate(zip(means, stds)):
        if mean > 0:
            ax3.errorbar(x_positions[j] + offset, mean, yerr=std,
                         fmt='none', color='black', capsize=3, capthick=1)

ax3.set_xlabel('Размер матрицы (N x N)', fontsize=12, fontweight='bold')
ax3.set_ylabel('Время выполнения (секунды)', fontsize=12, fontweight='bold')
ax3.set_title('Сравнение среднего времени выполнения\n(±1 стандартное отклонение)',
              fontsize=14, fontweight='bold')
ax3.set_xticks(x_positions)
ax3.set_xticklabels(selected_sizes, rotation=45)
ax3.legend(loc='upper left', fontsize=10)
ax3.grid(True, alpha=0.3, axis='y')

plt.tight_layout()
plt.savefig(os.path.join(output_dir, '2_bar_comparison.png'),
            dpi=300, bbox_inches='tight')
plt.show()
print(f"Сохранен: {os.path.join(output_dir, '2_bar_comparison.png')}")

# ==================== ГРАФИК 4: Ускорение (Speedup) ====================
fig, ax4 = plt.subplots(figsize=(12, 8))

# Берем время для 1 процесса как базовое
baseline = df_stats[df_stats['process_count']
                    == 1][['matrix_size', 'mean_time']]
baseline.columns = ['matrix_size', 'baseline_time']

# Объединяем данные
df_speedup = df_stats.merge(baseline, on='matrix_size')

# Вычисляем ускорение
df_speedup['speedup'] = df_speedup['baseline_time'] / df_speedup['mean_time']
df_speedup['speedup_std'] = df_speedup['speedup'] * \
    (df_speedup['std_time'] / df_speedup['mean_time'])

# Отображаем ускорение для разных чисел процессов
for i, proc in enumerate(processes):
    if proc == 1:
        continue
    data = df_speedup[df_speedup['process_count'] == proc]
    ax4.plot(data['matrix_size'], data['speedup'],
             marker='D', label=f'{proc} процессов (реальное ускорение)',
             color=colors[i], linewidth=2, markersize=8)

    ax4.fill_between(data['matrix_size'],
                     data['speedup'] - data['speedup_std'],
                     data['speedup'] + data['speedup_std'],
                     alpha=0.2, color=colors[i])

    ax4.plot(data['matrix_size'], [proc] * len(data['matrix_size']),
             linestyle='--', alpha=0.6, color=colors[i], linewidth=2,
             label=f'{proc} процессов (идеальное ускорение)')

ax4.set_xlabel('Размер матрицы (N x N)', fontsize=12, fontweight='bold')
ax4.set_ylabel('Ускорение', fontsize=12, fontweight='bold')
ax4.set_title('Ускорение параллельных вычислений\n(выше - лучше, пунктирные линии - идеальное ускорение)',
              fontsize=14, fontweight='bold')
ax4.legend(loc='upper left', fontsize=10, ncol=2)
ax4.grid(True, alpha=0.3)
ax4.set_xscale('log')
ax4.set_xlim([min(df_stats['matrix_size']), max(df_stats['matrix_size'])])
ax4.set_ylim([0, max(processes) + 1])

plt.tight_layout()
plt.savefig(os.path.join(output_dir, '3_speedup.png'),
            dpi=300, bbox_inches='tight')
plt.show()
print(f"Сохранен: {os.path.join(output_dir, '3_speedup.png')}")

# ==================== ГРАФИК 5: Эффективность параллелизации ====================
fig, ax5 = plt.subplots(figsize=(12, 8))

for i, proc in enumerate(processes):
    if proc == 1:
        continue
    data = df_speedup[df_speedup['process_count'] == proc]
    efficiency = data['speedup'] / proc
    efficiency_std = data['speedup_std'] / proc

    ax5.plot(data['matrix_size'], efficiency,
             marker='o', label=f'{proc} процесса(ов)',
             color=colors[i], linewidth=2, markersize=8)

    ax5.fill_between(data['matrix_size'],
                     efficiency - efficiency_std,
                     efficiency + efficiency_std,
                     alpha=0.2, color=colors[i])

ax5.set_xlabel('Размер матрицы (N x N)', fontsize=12, fontweight='bold')
ax5.set_ylabel('Эффективность параллелизации', fontsize=12, fontweight='bold')
ax5.set_title('Эффективность параллельных вычислений\n(чем ближе к 1.0, тем лучше)',
              fontsize=14, fontweight='bold')
ax5.legend(loc='best', fontsize=10)
ax5.grid(True, alpha=0.3)
ax5.set_xscale('log')
ax5.set_ylim([0, 1.1])
ax5.axhline(y=1.0, linestyle=':', alpha=0.5,
            color='black', label='Идеальная эффективность')
ax5.set_xlim([min(df_stats['matrix_size']), max(df_stats['matrix_size'])])

plt.tight_layout()
plt.savefig(os.path.join(output_dir, '4_efficiency.png'),
            dpi=300, bbox_inches='tight')
plt.show()
print(f"Сохранен: {os.path.join(output_dir, '4_efficiency.png')}")


# ==================== СТАТИСТИЧЕСКИЙ АНАЛИЗ ====================
print("\n" + "="*80)
print("СТАТИСТИЧЕСКИЙ АНАЛИЗ ЭКСПЕРИМЕНТОВ")
print("="*80)

for proc in processes:
    print(f"\n{'='*40}")
    print(f"ЧИСЛО ПРОЦЕССОВ: {proc}")
    print(f"{'='*40}")

    data = df_stats[df_stats['process_count'] == proc]

    print(f"\n{'Размер':<10} {'Среднее время':<15} {'Стд.откл.':<12} {'Минимум':<12} {'Максимум':<12} {'Коэф.вар.(%)':<12}")
    print("-"*75)

    for _, row in data.iterrows():
        cv = (row['std_time'] / row['mean_time']) * \
            100 if row['mean_time'] > 0 else 0
        print(f"{row['matrix_size']:<10} {row['mean_time']:<15.4f} {row['std_time']:<12.6f} "
              f"{row['min_time']:<12.4f} {row['max_time']:<12.4f} {cv:<12.2f}")

# Анализ вычислительной сложности
print("\n" + "="*80)
print("АНАЛИЗ ВЫЧИСЛИТЕЛЬНОЙ СЛОЖНОСТИ")
print("="*80)

for proc in processes:
    data = df_stats[df_stats['process_count'] == proc].copy()
    data['n_cubed'] = data['matrix_size'] ** 3
    data['n_squared'] = data['matrix_size'] ** 2

    # Регрессия для O(n³)
    slope_cubic, intercept_cubic, r_cubic, p_cubic, se_cubic = stats.linregress(
        data['n_cubed'], data['mean_time'])

    # Регрессия для O(n²)
    slope_quad, intercept_quad, r_quad, p_quad, se_quad = stats.linregress(
        data['n_squared'], data['mean_time'])

    print(f"\nПроцессов: {proc}")
    print(
        f"  Модель O(n³):  время = {slope_cubic:.3e} * n³ + {intercept_cubic:.3e}")
    print(f"                R² = {r_cubic**2:.4f}")
    print(
        f"  Модель O(n²):  время = {slope_quad:.3e} * n² + {intercept_quad:.3e}")
    print(f"                R² = {r_quad**2:.4f}")

    # Определяем лучшую модель
    if r_cubic**2 > r_quad**2:
        print(f"  ✓ Лучше описывается моделью O(n³)")
    else:
        print(f"  ✓ Лучше описывается моделью O(n²)")

# Расчет среднего ускорения для больших матриц
print("\n" + "="*80)
print("АНАЛИЗ УСКОРЕНИЯ ДЛЯ БОЛЬШИХ МАТРИЦ")
print("="*80)

large_sizes = df_stats[df_stats['matrix_size'] >= 1500]['matrix_size'].unique()
for proc in processes:
    if proc == 1:
        continue
    data_large = df_speedup[(df_speedup['process_count'] == proc) &
                            (df_speedup['matrix_size'].isin(large_sizes))]
    if len(data_large) > 0:
        avg_speedup = data_large['speedup'].mean()
        print(f"\nДля {proc} процессов на матрицах ≥1500:")
        print(f"  Среднее ускорение: {avg_speedup:.2f}x")
        print(f"  Эффективность: {avg_speedup/proc*100:.1f}%")

# Сохранение статистики в CSV
df_stats.to_csv(os.path.join(
    output_dir, 'statistics_summary.csv'), index=False)
df_speedup.to_csv(os.path.join(output_dir, 'speedup_summary.csv'), index=False)
print(f"\n\nСтатистика сохранена в файлы:")
print(f"  - {os.path.join(output_dir, 'statistics_summary.csv')}")
print(f"  - {os.path.join(output_dir, 'speedup_summary.csv')}")
print(f"\nВсе графики сохранены в папку: {output_dir}")

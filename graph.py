import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from scipy import stats
import os
import json

# ==================== ЗАГРУЗКА КОНФИГУРАЦИИ ====================
with open('graph_config.json', 'r', encoding='utf-8') as f:
    config = json.load(f)

# ==================== НАСТРОЙКА ====================
# Настройка русских шрифтов
plt.rcParams['font.family'] = config['plot_settings']['font_family']

# Создаем папку для графиков
output_dir = config['paths']['output_dir']
os.makedirs(output_dir, exist_ok=True)
print(f"Графики будут сохранены в папку: {output_dir}")

# Загрузка данных
input_path = config['paths']['input_data']
df = pd.read_csv(input_path)

# Определяем название столбца с процессами
process_col = config['processes']['column_name']
if process_col not in df.columns:
    process_col = config['processes']['fallback_column']
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
plt.style.use(config['plot_settings']['style'])
colors = plt.cm.viridis(np.linspace(0, 1, len(processes)))

# ==================== ГРАФИК 1: Время выполнения (линейный масштаб) ====================
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=tuple(
    config['plot_settings']['figure_sizes']['main']))

for i, proc in enumerate(processes):
    data = df_stats[df_stats['process_count'] == proc]
    ax1.plot(data['matrix_size'], data['mean_time'],
             marker='o', label=f'{proc} процесс(а/ов)',
             color=colors[i], linewidth=2, markersize=8)

    # Добавляем полосы ошибок (стандартное отклонение)
    ax1.fill_between(data['matrix_size'],
                     data['mean_time'] - data['std_time'],
                     data['mean_time'] + data['std_time'],
                     alpha=config['plot_settings']['error_alpha'], color=colors[i])

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
plt.savefig(os.path.join(output_dir, config['paths']['output_files']['execution_time']),
            dpi=config['plot_settings']['dpi'], bbox_inches='tight')
plt.show()
print(
    f"Сохранен: {os.path.join(output_dir, config['paths']['output_files']['execution_time'])}")

# ==================== ГРАФИК 3: Сравнение среднего времени (столбчатая диаграмма) ====================
fig, ax3 = plt.subplots(figsize=tuple(
    config['plot_settings']['figure_sizes']['bar']))

# Выбираем каждый N-й размер для читаемости
selected_sizes = df_stats['matrix_size'].unique(
)[::config['plot_settings']['selected_sizes_step']]

bar_width = config['plot_settings']['bar_width']
x_positions = np.arange(len(selected_sizes))

for i, proc in enumerate(processes):
    data = df_stats[df_stats['process_count'] == proc]
    data_selected = data[data['matrix_size'].isin(selected_sizes)]

    offset = (i - len(processes)/2) * bar_width + bar_width/2

    bars = ax3.bar(x_positions + offset, data_selected['mean_time'],
                   width=bar_width, label=f'{proc} процесс(а/ов)',
                   color=colors[i], alpha=0.7, edgecolor='black')

    # Добавляем error bars
    ax3.errorbar(x_positions + offset, data_selected['mean_time'],
                 yerr=data_selected['std_time'],
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
plt.savefig(os.path.join(output_dir, config['paths']['output_files']['bar_comparison']),
            dpi=config['plot_settings']['dpi'], bbox_inches='tight')
plt.show()
print(
    f"Сохранен: {os.path.join(output_dir, config['paths']['output_files']['bar_comparison'])}")

# ==================== ГРАФИК 4: Ускорение (Speedup) ====================
fig, ax4 = plt.subplots(figsize=tuple(
    config['plot_settings']['figure_sizes']['speedup']))

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

    # Добавляем полосу ошибок
    ax4.fill_between(data['matrix_size'],
                     data['speedup'] - data['speedup_std'],
                     data['speedup'] + data['speedup_std'],
                     alpha=config['plot_settings']['error_alpha'], color=colors[i])

    # Добавляем линию идеального ускорения
    if config['speedup']['ideal_lines']:
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
ax4.set_ylim([0, max(processes) + config['speedup']['ylim_max_offset']])

plt.tight_layout()
plt.savefig(os.path.join(output_dir, config['paths']['output_files']['speedup']),
            dpi=config['plot_settings']['dpi'], bbox_inches='tight')
plt.show()
print(
    f"Сохранен: {os.path.join(output_dir, config['paths']['output_files']['speedup'])}")

# ==================== ГРАФИК 5: Эффективность параллелизации ====================
fig, ax5 = plt.subplots(figsize=tuple(
    config['plot_settings']['figure_sizes']['speedup']))

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
                     alpha=config['plot_settings']['error_alpha'], color=colors[i])

ax5.set_xlabel('Размер матрицы (N x N)', fontsize=12, fontweight='bold')
ax5.set_ylabel('Эффективность параллелизации', fontsize=12, fontweight='bold')
ax5.set_title('Эффективность параллельных вычислений\n(чем ближе к 1.0, тем лучше)',
              fontsize=14, fontweight='bold')
ax5.legend(loc='best', fontsize=10)
ax5.grid(True, alpha=0.3)
ax5.set_xscale('log')
ax5.set_ylim([0, config['efficiency']['ylim_max']])
ax5.axhline(y=1.0, linestyle=':', alpha=0.5,
            color='black', label='Идеальная эффективность')
ax5.set_xlim([min(df_stats['matrix_size']), max(df_stats['matrix_size'])])

plt.tight_layout()
plt.savefig(os.path.join(output_dir, config['paths']['output_files']['efficiency']),
            dpi=config['plot_settings']['dpi'], bbox_inches='tight')
plt.show()
print(
    f"Сохранен: {os.path.join(output_dir, config['paths']['output_files']['efficiency'])}")

# ==================== ГРАФИК 6: Разброс измерений ====================
fig, ax6 = plt.subplots(figsize=tuple(
    config['plot_settings']['figure_sizes']['scatter']))

markers = config['markers']
for i, proc in enumerate(processes):
    data_raw = df[df[process_col] == proc]
    jitter = (i - len(processes)/2) * 5
    ax6.scatter(data_raw['matrix_size'] + jitter, data_raw['execution_time'],
                label=f'{proc} процесс(а/ов)', alpha=0.6, s=30,
                marker=markers[i % len(markers)], color=colors[i])

    data_stats = df_stats[df_stats['process_count'] == proc]
    ax6.plot(data_stats['matrix_size'], data_stats['mean_time'],
             color=colors[i], linewidth=2, alpha=0.8)

ax6.set_xlabel('Размер матрицы (N x N)', fontsize=12, fontweight='bold')
ax6.set_ylabel('Время выполнения (секунды)', fontsize=12, fontweight='bold')
ax6.set_title('Все измерения со средними линиями\n(каждая точка - один эксперимент)',
              fontsize=14, fontweight='bold')
ax6.legend(loc='upper left', fontsize=10)
ax6.grid(True, alpha=0.3)
ax6.set_yscale('log')
ax6.set_xscale('log')

plt.tight_layout()
plt.savefig(os.path.join(output_dir, config['paths']['output_files']['all_measurements']),
            dpi=config['plot_settings']['dpi'], bbox_inches='tight')
plt.show()
print(
    f"Сохранен: {os.path.join(output_dir, config['paths']['output_files']['all_measurements'])}")

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

    slope_cubic, intercept_cubic, r_cubic, p_cubic, se_cubic = stats.linregress(
        data['n_cubed'], data['mean_time'])
    slope_quad, intercept_quad, r_quad, p_quad, se_quad = stats.linregress(
        data['n_squared'], data['mean_time'])

    print(f"\nПроцессов: {proc}")
    print(
        f"  Модель O(n³):  время = {slope_cubic:.3e} * n³ + {intercept_cubic:.3e}")
    print(f"                R² = {r_cubic**2:.4f}")
    print(
        f"  Модель O(n²):  время = {slope_quad:.3e} * n² + {intercept_quad:.3e}")
    print(f"                R² = {r_quad**2:.4f}")

    if r_cubic**2 > r_quad**2:
        print(f"  ✓ Лучше описывается моделью O(n³)")
    else:
        print(f"  ✓ Лучше описывается моделью O(n²)")

# Расчет среднего ускорения для больших матриц
print("\n" + "="*80)
print("АНАЛИЗ УСКОРЕНИЯ ДЛЯ БОЛЬШИХ МАТРИЦ")
print("="*80)

large_threshold = config['plot_settings']['large_sizes_threshold']
large_sizes = df_stats[df_stats['matrix_size']
                       >= large_threshold]['matrix_size'].unique()
for proc in processes:
    if proc == 1:
        continue
    data_large = df_speedup[(df_speedup['process_count'] == proc) &
                            (df_speedup['matrix_size'].isin(large_sizes))]
    if len(data_large) > 0:
        avg_speedup = data_large['speedup'].mean()
        print(f"\nДля {proc} процессов на матрицах ≥{large_threshold}:")
        print(f"  Среднее ускорение: {avg_speedup:.2f}x")
        print(f"  Эффективность: {avg_speedup/proc*100:.1f}%")

# Сохранение статистики в CSV
df_stats.to_csv(os.path.join(
    output_dir, config['paths']['output_files']['statistics_summary']), index=False)
df_speedup.to_csv(os.path.join(
    output_dir, config['paths']['output_files']['speedup_summary']), index=False)
print(f"\n\nСтатистика сохранена в файлы:")
print(
    f"  - {os.path.join(output_dir, config['paths']['output_files']['statistics_summary'])}")
print(
    f"  - {os.path.join(output_dir, config['paths']['output_files']['speedup_summary'])}")
print(f"\nВсе графики сохранены в папку: {output_dir}")

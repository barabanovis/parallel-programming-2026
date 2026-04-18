@echo off
setlocal enabledelayedexpansion

echo MPI Matrix Multiplication Benchmark
echo ===================================
echo.

set EXE_PATH=out\build\x64-Debug\lab3_parallel.exe

if not exist "%EXE_PATH%" (
    echo Error: %EXE_PATH% not found!
    pause
    exit /b 1
)

REM Создаем папку results в текущей директории
if not exist "results" mkdir results

set RESULTS_FILE=%CD%\results\statistics.csv
echo matrix_size,proccess_number,execution_time > "%RESULTS_FILE%"

echo CSV file initialized: %RESULTS_FILE%
echo.

REM Параметры
set PROCESSES=1 2 4 8
set REPEATS=5

set COUNT=0
set TOTAL=400

echo Starting tests...
echo.

REM Простые циклы без сложных вычислений
for %%p in (1 2 4 8) do (
    echo ========================================
    echo Testing with %%p processes
    echo ========================================
    
    for /l %%s in (100,100,2000) do (
        echo   Matrix size: %%s x %%s
        
        for /l %%r in (1,1,5) do (
            set /a COUNT+=1
            echo     [!COUNT!/400] Run %%r of 5...
            
            mpiexec -n %%p ./"%EXE_PATH%" %%s
        )
        echo.
    )
)

echo.
echo ============================================
echo BENCHMARK COMPLETED
echo ============================================
echo Total tests executed: %COUNT%
echo Results saved to: %RESULTS_FILE%
echo ============================================
pause
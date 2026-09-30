#!/bin/bash
# Исследование: несколько размеров матриц x несколько чисел потоков x размер блока.
# Каждый запуск дописывает строку в results.csv (N,threads,block_size,time_sec)

echo "N,threads,block_size,time_sec" > results.csv

echo "Компиляция..."
g++ -O2 -fopenmp main.cpp -o matrix_mult

for N in 200 400 800; do
    echo "Генерация матриц $N x $N..."
    python3 generate.py $N
    for T in 1 2 4; do
        echo "== N=$N, потоков=$T, блок=32 =="
        ./matrix_mult $T 32
    done
    # дополнительно проверяем влияние размера блока при фиксированном числе потоков
    for BS in 16 32 64; do
        echo "== N=$N, потоков=4, блок=$BS =="
        ./matrix_mult 4 $BS
    done
done

echo ""
echo "Готово! Все результаты в results.csv"

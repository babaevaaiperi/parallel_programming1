// main.cpp
// Блочное (tiled) умножение квадратных матриц с распараллеливанием по блокам (OpenMP)
//
// Сборка:
//   g++ -O2 -fopenmp main.cpp -o matrix_mult
//
// Запуск:
//   ./matrix_mult <потоки> <размер_блока>
//   пример: ./matrix_mult 4 32   (4 потока, блок 32x32)

#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <cstdlib>
#include <algorithm>

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;
using namespace chrono;

int main(int argc, char* argv[]) {
    cout << "========== БЛОЧНОЕ УМНОЖЕНИЕ МАТРИЦ (OpenMP) ==========" << endl;

    // Аргументы: число потоков и размер блока
    int threads = (argc > 1) ? atoi(argv[1]) : 0;
    int blockSize = (argc > 2) ? atoi(argv[2]) : 32; // размер блока по умолчанию

#ifdef _OPENMP
    if (threads > 0) omp_set_num_threads(threads);
#endif

    // Открываем файлы с матрицами
    ifstream fileA("matrix_a.txt");
    ifstream fileB("matrix_b.txt");

    if (!fileA.is_open() || !fileB.is_open()) {
        cout << "Ошибка: не удалось открыть файлы matrix_a.txt / matrix_b.txt!" << endl;
        return 1;
    }

    int n;
    fileA >> n;
    fileB >> n;

    cout << "Размер матриц: " << n << "x" << n << endl;
    cout << "Размер блока:  " << blockSize << "x" << blockSize << endl;

    vector<vector<double>> A(n, vector<double>(n));
    vector<vector<double>> B(n, vector<double>(n));
    vector<vector<double>> C(n, vector<double>(n, 0.0));

    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            fileA >> A[i][j];

    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            fileB >> B[i][j];

    fileA.close();
    fileB.close();

    // Число блоков по каждой стороне (последний блок может быть неполным)
    int numBlocks = (n + blockSize - 1) / blockSize;

    auto start = high_resolution_clock::now();

    cout << "Выполняется блочное умножение..." << endl;

    // Распараллеливание: каждый поток обрабатывает свой набор блоков (bi, bj)
    #pragma omp parallel for collapse(2) schedule(dynamic)
    for (int bi = 0; bi < numBlocks; bi++) {
        for (int bj = 0; bj < numBlocks; bj++) {

            int iStart = bi * blockSize;
            int iEnd   = min(iStart + blockSize, n);
            int jStart = bj * blockSize;
            int jEnd   = min(jStart + blockSize, n);

            // Проходим по всем блокам bk, чтобы накопить C[bi][bj] += A[bi][bk] * B[bk][bj]
            for (int bk = 0; bk < numBlocks; bk++) {
                int kStart = bk * blockSize;
                int kEnd   = min(kStart + blockSize, n);

                // Умножение внутри одного блока - обычные три цикла,
                // но данные блока маленькие и помещаются в кэш
                for (int i = iStart; i < iEnd; i++) {
                    for (int k = kStart; k < kEnd; k++) {
                        double a = A[i][k];
                        for (int j = jStart; j < jEnd; j++) {
                            C[i][j] += a * B[k][j];
                        }
                    }
                }
            }
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    double time_sec = duration.count() / 1000.0;

    int actualThreads = 1;
#ifdef _OPENMP
    actualThreads = (threads > 0) ? threads : omp_get_max_threads();
#endif

    cout << "Число потоков: " << actualThreads << endl;
    cout << "Время выполнения: " << time_sec << " секунд" << endl;

    // Сохраняем результат с высокой точностью (важно для верификации!)
    ofstream resultFile("result_cpp.txt");
    resultFile << fixed << setprecision(10);
    resultFile << n << endl;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            resultFile << C[i][j] << " ";
        }
        resultFile << endl;
    }
    resultFile.close();

    cout << "Результат сохранён в result_cpp.txt" << endl;

    // Объём задачи
    long long memory = 3LL * n * n * sizeof(double);
    long long operations = 2LL * n * n * n;

    cout << "\n========== МЕТРИКИ ==========" << endl;
    cout << "Объём памяти: " << memory / 1024 << " KB" << endl;
    cout << "Количество операций: " << operations << endl;
    if (time_sec > 0)
        cout << "Производительность: " << (operations / 1e9) / time_sec << " GFLOPS" << endl;

    // Дописываем строку в лог для построения графиков
    ofstream log("results.csv", ios::app);
    log << n << "," << actualThreads << "," << blockSize << "," << time_sec << endl;

    cout << "\nЗапуск верификации через Python..." << endl;
    system("python3 verify.py");

    return 0;
}

import random
import sys

def generate_matrix(n, filename):
    with open(filename, 'w') as f:
        f.write(f"{n}\n")
        for _ in range(n):
            row = [str(round(random.uniform(0, 10), 3)) for _ in range(n)]
            f.write(" ".join(row) + "\n")

n = int(sys.argv[1]) if len(sys.argv) > 1 else 100

print(f"Генерация матриц {n}x{n}...")
generate_matrix(n, "matrix_a.txt")
generate_matrix(n, "matrix_b.txt")
print("Готово!")

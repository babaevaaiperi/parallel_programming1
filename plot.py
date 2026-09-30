import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("results.csv")

# 1) Время от N при блоке=32, разные потоки
plt.figure()
sub = df[df["block_size"] == 32]
for t, group in sub.groupby("threads"):
    g = group.groupby("N")["time_sec"].mean().reset_index()
    plt.plot(g["N"], g["time_sec"], marker="o", label=f"{t} поток(ов)")
plt.xlabel("Размер матрицы N")
plt.ylabel("Время выполнения, с")
plt.title("Время выполнения от объёма задачи (блок=32)")
plt.legend()
plt.grid(True)
plt.savefig("graph_time.png", dpi=150)

# 2) Ускорение от числа потоков при блоке=32, максимальный N
max_n = df["N"].max()
sub2 = df[(df["N"] == max_n) & (df["block_size"] == 32)].groupby("threads")["time_sec"].mean().reset_index()
t1 = sub2.loc[sub2["threads"] == 1, "time_sec"].values[0]
sub2["speedup"] = t1 / sub2["time_sec"]

plt.figure()
plt.plot(sub2["threads"], sub2["speedup"], marker="o", label="Ускорение")
plt.plot(sub2["threads"], sub2["threads"], "--", label="Идеальное ускорение")
plt.xlabel("Число потоков")
plt.ylabel("Ускорение (speedup)")
plt.title(f"Ускорение при N={max_n}, блок=32")
plt.legend()
plt.grid(True)
plt.savefig("graph_speedup.png", dpi=150)

# 3) Влияние размера блока при потоках=4, максимальный N
sub3 = df[(df["N"] == max_n) & (df["threads"] == 4)].groupby("block_size")["time_sec"].mean().reset_index()

plt.figure()
plt.plot(sub3["block_size"], sub3["time_sec"], marker="o")
plt.xlabel("Размер блока")
plt.ylabel("Время выполнения, с")
plt.title(f"Влияние размера блока при N={max_n}, потоков=4")
plt.grid(True)
plt.savefig("graph_blocksize.png", dpi=150)

print("Графики сохранены: graph_time.png, graph_speedup.png, graph_blocksize.png")

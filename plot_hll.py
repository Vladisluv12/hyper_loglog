import csv
import math
from collections import defaultdict

import matplotlib.pyplot as plt


def load_graph1(path):
    by_stream = defaultdict(list)
    with open(path, newline="", encoding="ascii") as f:
        reader = csv.DictReader(f)
        for row in reader:
            sid = int(row["stream_id"])
            by_stream[sid].append(
                {
                    "step": int(row["step_index"]),
                    "items": int(row["items"]),
                    "exact": int(row["exact"]),
                    "estimate": float(row["estimate"]),
                }
            )
    for sid in by_stream:
        by_stream[sid].sort(key=lambda r: r["step"])
    return by_stream


def load_graph2(path):
    rows = []
    with open(path, newline="", encoding="ascii") as f:
        reader = csv.DictReader(f)
        for row in reader:
            rows.append(
                {
                    "step": int(row["step_index"]),
                    "items": int(row["items"]),
                    "mean": float(row["mean_estimate"]),
                    "std": float(row["stddev_estimate"]),
                }
            )
    rows.sort(key=lambda r: r["step"])
    return rows


P = 10
M = 2 ** P
RSE_LOW = 1.04 / math.sqrt(M)
RSE_HIGH = 1.3 / math.sqrt(M)


def plot_graph1(by_stream):
    plt.figure(figsize=(9, 5))
    steps = None
    mean_exact = []
    mean_items = []
    for sid, rows in sorted(by_stream.items()):
        x = [r["items"] for r in rows]
        y_exact = [r["exact"] for r in rows]
        y_est = [r["estimate"] for r in rows]
        plt.plot(x, y_exact, linewidth=2, label=f"exact (stream {sid})")
        plt.plot(x, y_est, linestyle="--", label=f"estimate (stream {sid})")
        if steps is None:
            steps = [r["step"] for r in rows]
            mean_exact = [0.0 for _ in steps]
            mean_items = [0 for _ in steps]
        for i, r in enumerate(rows):
            mean_exact[i] += r["exact"]
            mean_items[i] = r["items"]
    if steps is not None:
        streams = len(by_stream)
        mean_exact = [v / streams for v in mean_exact]
        low_104 = [v * (1.0 - RSE_LOW) for v in mean_exact]
        high_104 = [v * (1.0 + RSE_LOW) for v in mean_exact]
        low_130 = [v * (1.0 - RSE_HIGH) for v in mean_exact]
        high_130 = [v * (1.0 + RSE_HIGH) for v in mean_exact]
        plt.fill_between(mean_items, low_104, high_104, alpha=0.12, label="theory ±1.04/√(2^p)")
        plt.fill_between(mean_items, low_130, high_130, alpha=0.08, label="theory ±1.3/√(2^p)")
    plt.xlabel("processed items")
    plt.ylabel("unique count")
    plt.title("HyperLogLog estimate vs exact")
    plt.grid(True, alpha=0.3)
    plt.legend()
    plt.tight_layout()
    plt.savefig("graph1.png", dpi=150)


def plot_graph2(rows):
    plt.figure(figsize=(9, 5))
    x = [r["items"] for r in rows]
    mean = [r["mean"] for r in rows]
    std = [r["std"] for r in rows]
    upper = [m + s for m, s in zip(mean, std)]
    lower = [m - s for m, s in zip(mean, std)]

    plt.plot(x, mean, linewidth=2, label="E(Nt)")
    plt.fill_between(x, lower, upper, alpha=0.2, label="E(Nt) ± σ")
    plt.xlabel("processed items")
    plt.ylabel("estimated unique count")
    plt.title("HyperLogLog estimate statistics")
    plt.grid(True, alpha=0.3)
    plt.legend()
    plt.tight_layout()
    plt.savefig("graph2.png", dpi=150)

graph1 = load_graph1("graph1_data.csv")
graph2 = load_graph2("graph2_stats.csv")

plot_graph1(graph1)
plot_graph2(graph2)
print("Saved graph1.png and graph2.png")
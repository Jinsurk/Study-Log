"""Recompute corrections from saved raw readings; preserve the original CSV."""
import csv
import pathlib
import statistics
import sys

path = pathlib.Path(sys.argv[1])
with path.open(encoding="utf-8-sig", newline="") as handle:
    rows = list(csv.DictReader(handle))
if len(rows) != 1000 or [int(r["sample"]) for r in rows] != list(range(1, 1001)):
    raise ValueError("Expected 1000 sequential samples")

biases = {"gx": -0.00721, "gy": -0.50085, "gz": 0.07413}
stats = []
mismatches = []
for axis, bias in biases.items():
    raw_key = axis + "_deg_s"
    corrected_key = axis + "_corrected_deg_s"
    raw = [float(r[raw_key]) for r in rows]
    corrected = [v - bias for v in raw]
    for row, value in zip(rows, corrected):
        if abs(float(row[corrected_key]) - value) > 1e-5:
            mismatches.append((row["sample"], axis))
        row[corrected_key] = f"{value:.6f}"
    for key, values in [(raw_key, raw), (corrected_key, corrected)]:
        stats.append({"axis": key, "samples": len(values),
                      "mean_deg_s": f"{statistics.mean(values):.8f}",
                      "sample_std_deg_s": f"{statistics.stdev(values):.8f}"})

output = path.with_name(path.stem + ".recomputed.csv")
summary = path.with_name(path.stem + ".recomputed.summary.csv")
for destination, records in [(output, rows), (summary, stats)]:
    with destination.open("w", encoding="utf-8-sig", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=list(records[0]))
        writer.writeheader()
        writer.writerows(records)
print("Saved correction mismatches (sample, axis):", mismatches)
for row in stats:
    print(row)
print("Recomputed CSV:", output)
print("Summary:", summary)

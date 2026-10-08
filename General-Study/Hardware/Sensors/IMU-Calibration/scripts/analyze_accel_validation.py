"""Apply fixed calibration on PC to an independent static capture."""
import csv
import json
import math
from pathlib import Path
import statistics
import sys

source = Path(sys.argv[1])
root = Path(__file__).resolve().parents[1] / 'results'
calibration = json.loads((root / "accel_calibration.json").read_text(encoding="utf-8"))
parameters = calibration["parameters"]
with source.open(encoding="utf-8-sig", newline="") as handle:
    rows = list(csv.DictReader(handle))
if len(rows) != 1000 or [int(r["sample"]) for r in rows] != list(range(1, 1001)):
    raise ValueError("Expected 1000 sequential samples")
times = [int(r["t_us"]) for r in rows]
if not all(b > a for a, b in zip(times, times[1:])):
    raise ValueError("Non-increasing timestamps")

raw_norms, corrected_norms = [], []
for row in rows:
    raw = [float(row[axis]) for axis in ("ax_g", "ay_g", "az_g")]
    corrected = []
    for axis, value in zip(("ax_g", "ay_g", "az_g"), raw):
        p = parameters[axis]
        result = (value - p["offset_g"]) * p["gain"]
        corrected.append(result)
        row[axis.replace("_g", "_corrected_g")] = f"{result:.9f}"
    raw_norms.append(math.sqrt(sum(v*v for v in raw)))
    corrected_norms.append(math.sqrt(sum(v*v for v in corrected)))
    row["raw_norm_g"] = f"{raw_norms[-1]:.9f}"
    row["corrected_norm_g"] = f"{corrected_norms[-1]:.9f}"

stats = []
for stage, values in (("raw", raw_norms), ("corrected", corrected_norms)):
    stats.append({"stage": stage, "samples": len(values),
                  "mean_norm_g": f"{statistics.mean(values):.9f}",
                  "std_norm_g": f"{statistics.stdev(values):.9f}",
                  "mean_abs_error_g": f"{statistics.mean(abs(v-1) for v in values):.9f}",
                  "rmse_from_1g": f"{math.sqrt(statistics.mean((v-1)**2 for v in values)):.9f}"})
for destination, records in ((source.with_name(source.stem + ".validated.csv"), rows),
                             (source.with_name(source.stem + ".validation_summary.csv"), stats)):
    with destination.open("w", encoding="utf-8-sig", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=list(records[0]))
        writer.writeheader()
        writer.writerows(records)
    print("Saved:", destination)
for stat in stats:
    print(stat)
# Retain the exact calibration used for this analysis.
source.with_name(source.stem + ".calibration_used.json").write_text(
    json.dumps(calibration, indent=2), encoding="utf-8")

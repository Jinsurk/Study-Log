"""Axis-aligned six-face offset/scale estimate, in g. Standard library only."""
import csv
import json
import math
from pathlib import Path
import statistics

root = Path(__file__).resolve().parents[1]
data = root / 'data'
results = root / 'results'
mapping = {"pos1": ("az_g", 1), "pos2": ("az_g", -1),
           "pos3": ("ax_g", 1), "pos4": ("ax_g", -1),
           "pos5": ("ay_g", 1), "pos6": ("ay_g", -1)}
faces = {}
for position, (axis, sign) in mapping.items():
    files = sorted(f for f in data.glob(f"accel_{position}_*.csv")
                   if ".summary." not in f.name and ".partial." not in f.name)
    if not files:
        raise ValueError(f"Missing {position}")
    source = files[-1]
    with source.open(encoding="utf-8-sig", newline="") as handle:
        rows = list(csv.DictReader(handle))
    if len(rows) != 1000 or [int(r["sample"]) for r in rows] != list(range(1, 1001)):
        raise ValueError(f"Invalid sample sequence: {source.name}")
    times = [int(r["t_us"]) for r in rows]
    if not all(b > a for a, b in zip(times, times[1:])):
        raise ValueError(f"Invalid timestamps: {source.name}")
    means = {key: statistics.mean(float(r[key]) for r in rows)
             for key in ("ax_g", "ay_g", "az_g")}
    stds = {key: statistics.stdev(float(r[key]) for r in rows) for key in means}
    # Infer polarity from the measured gravity, not the position filename.
    sign = 1 if means[axis] > 0 else -1
    if abs(means[axis]) < 0.8 or max(means, key=lambda key: abs(means[key])) != axis:
        raise ValueError(f"Wrong face: {source.name}")
    faces[position] = {"file": source.name, "samples": len(rows),
                       "axis": axis, "sign": sign, "mean_g": means, "std_g": stds}

parameters = {}
for axis in ("ax_g", "ay_g", "az_g"):
    positives = [f["mean_g"][axis] for f in faces.values() if f["axis"] == axis and f["sign"] == 1]
    negatives = [f["mean_g"][axis] for f in faces.values() if f["axis"] == axis and f["sign"] == -1]
    if len(positives) != 1 or len(negatives) != 1:
        raise ValueError(f"Need one positive and one negative face for {axis}")
    positive, negative = positives[0], negatives[0]
    offset = (positive + negative) / 2
    scale = (positive - negative) / 2
    parameters[axis] = {"offset_g": offset, "raw_scale": scale, "gain": 1 / scale}

for face in faces.values():
    corrected = {axis: (value - parameters[axis]["offset_g"]) * parameters[axis]["gain"]
                 for axis, value in face["mean_g"].items()}
    face["corrected_mean_g"] = corrected
    face["corrected_mean_norm_g"] = math.sqrt(sum(v*v for v in corrected.values()))

report = {"method": "axis-aligned six-face offset/scale; ideal +/-1 g assumed",
          "limitations": "Mounting tilt and cross-axis errors are not fitted. Training-face results are not independent validation.",
          "parameters": parameters, "faces": faces}
(results / "accel_calibration.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
lines = ["// Six-face first-pass estimate; input and output in g.",
         "// corrected = (raw - offset) * gain; validate independently.",
         "#pragma once"]
for axis, p in parameters.items():
    name = axis[:2].upper()
    lines.extend([f"constexpr float {name}_OFFSET_G = {p['offset_g']:.9f}f;",
                  f"constexpr float {name}_GAIN = {p['gain']:.9f}f;"])
(results / "accel_calibration.h").write_text("\n".join(lines) + "\n", encoding="utf-8")
print(json.dumps(parameters, indent=2))
for position, face in faces.items():
    print(position, face["samples"], face["mean_g"], "corrected norm", face["corrected_mean_norm_g"])

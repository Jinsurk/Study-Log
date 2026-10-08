"""Static gravity ellipsoid calibration; does not identify absolute rotation."""
import csv
import json
from pathlib import Path
import numpy as np

ROOT = Path(__file__).resolve().parents[1]
DATA = ROOT / 'data'
RESULTS = ROOT / 'results'
AXES = ('ax_g', 'ay_g', 'az_g')

def read(path):
    with path.open(encoding='utf-8-sig', newline='') as handle:
        rows = list(csv.DictReader(handle))
    if len(rows) != 1000 or [int(r['sample']) for r in rows] != list(range(1, 1001)):
        raise ValueError(f'Invalid sample sequence: {path.name}')
    ts = np.array([int(r['t_us']) for r in rows])
    values = np.array([[float(r[a]) for a in AXES] for r in rows])
    if not np.isfinite(values).all() or not (np.diff(ts) > 0).all():
        raise ValueError(f'Invalid values or timestamps: {path.name}')
    return values

def fit(points):
    x, y, z = points.T
    design = np.column_stack((x*x, y*y, z*z, 2*x*y, 2*x*z, 2*y*z, x, y, z))
    q, _, rank, singular = np.linalg.lstsq(design, np.ones(len(points)), rcond=None)
    if rank != 9:
        raise ValueError('Insufficient orientation diversity')
    Q = np.array([[q[0], q[3], q[4]], [q[3], q[1], q[5]], [q[4], q[5], q[2]]])
    offset = -0.5 * np.linalg.solve(Q, q[6:9])
    M = Q / (1 + offset @ Q @ offset)
    if np.min(np.linalg.eigvalsh(M)) <= 0:
        raise ValueError('Fitted surface is not an ellipsoid')
    C = np.linalg.cholesky(M).T
    return offset, C, float(singular[0]/singular[-1])

def metrics(values, offset, C):
    norms = np.linalg.norm((values-offset) @ C.T, axis=1)
    return {'mean_norm_g': float(norms.mean()),
            'std_norm_g': float(norms.std(ddof=1)),
            'rmse_from_1g': float(np.sqrt(np.mean((norms-1)**2)))}

captures, means, manifest = [], [], []
selection_path = RESULTS / 'multi_pose_selection.json'
selection = json.loads(selection_path.read_text(encoding='utf-8'))['selected'] if selection_path.exists() else {}
for i in range(1, 21):
    label = f'pose{i:02d}'
    files = sorted(f for f in DATA.glob(f'accel_{label}_*.csv') if f.name.count('.') == 1)
    if not files:
        raise ValueError(f'Missing {label}')
    path = files[-1]
    # Explicit user selection takes precedence over recency.
    if i == 15:
        path = DATA / 'accel_pose15_20261008_123042_927.csv'
    if label in selection:
        path = DATA / selection[label]
    values = read(path)
    captures.append(values)
    means.append(values.mean(axis=0))
    manifest.append({'pose': label, 'file': path.name, 'samples': len(values),
                     'mean_g': values.mean(axis=0).tolist(),
                     'std_g': values.std(axis=0, ddof=1).tolist()})

means = np.array(means)
offset, C, condition = fit(means)
loo = []
for i in range(20):
    b, matrix, _ = fit(np.delete(means, i, axis=0))
    loo.append(float(np.linalg.norm(matrix @ (means[i]-b))-1))
old = json.loads((RESULTS/'accel_calibration.json').read_text(encoding='utf-8'))
old_b = np.array([old['parameters'][a]['offset_g'] for a in AXES])
old_C = np.diag([old['parameters'][a]['gain'] for a in AXES])
comparison = []
for item, values in zip(manifest, captures):
    for method, b, matrix in [('raw',np.zeros(3),np.eye(3)), ('six_face',old_b,old_C), ('multi',offset,C)]:
        comparison.append({'file':item['file'], 'dataset':'training', 'method':method, **metrics(values,b,matrix)})
for path in sorted(DATA.glob('accel_tilt*.csv')):
    if path.name.count('.') != 1:
        continue
    values = read(path)
    for method, b, matrix in [('raw',np.zeros(3),np.eye(3)), ('six_face',old_b,old_C), ('multi',offset,C)]:
        comparison.append({'file':path.name, 'dataset':'earlier_holdout', 'method':method, **metrics(values,b,matrix)})

report = {'method':'equal-weight pose-mean algebraic ellipsoid; upper-triangular Cholesky correction',
          'formula':'corrected_g = C @ (raw_g - offset_g)',
          'limitations':'Static norm does not identify absolute rotation. Earlier holdout is from the previous day; new independent validation required. Not gyro or LiDAR extrinsic calibration.',
          'offset_g':offset.tolist(), 'C':C.tolist(), 'design_condition':condition,
          'training_mean_norm_rmse_g':float(np.sqrt(np.mean((np.linalg.norm((means-offset)@C.T,axis=1)-1)**2))),
          'leave_one_pose_out_rmse_g':float(np.sqrt(np.mean(np.array(loo)**2))),
          'leave_one_pose_out_errors_g':loo, 'manifest':manifest}
(RESULTS/'accel_multi_calibration.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
with (RESULTS/'accel_multi_comparison.csv').open('w',encoding='utf-8-sig',newline='') as handle:
    writer=csv.DictWriter(handle,fieldnames=list(comparison[0])); writer.writeheader(); writer.writerows(comparison)
header=['#pragma once','// Input/output in g; norm calibration, absolute rotation not identified.',
        'constexpr float ACCEL_MULTI_OFFSET[3] = {'+', '.join(f'{v:.10f}f' for v in offset)+'};',
        'constexpr float ACCEL_MULTI_C[3][3] = {']
header += ['    {'+', '.join(f'{v:.10f}f' for v in row)+'},' for row in C]
header += ['};']
(RESULTS/'accel_multi_calibration.h').write_text('\n'.join(header)+'\n',encoding='utf-8')
print(json.dumps({k:report[k] for k in ['offset_g','C','design_condition','training_mean_norm_rmse_g','leave_one_pose_out_rmse_g']},indent=2))
print('MAX POSE STD:',max(max(item['std_g']) for item in manifest))
for row in comparison:
    if row['dataset']=='earlier_holdout':
        print(row['file'],row['method'],round(row['rmse_from_1g'],7))

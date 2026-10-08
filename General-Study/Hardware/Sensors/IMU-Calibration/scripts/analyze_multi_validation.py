import csv
import json
from pathlib import Path
import sys
import numpy as np

root=Path(__file__).resolve().parents[1] / 'results'
source=Path(sys.argv[1])
with source.open(encoding='utf-8-sig',newline='') as handle:
    rows=list(csv.DictReader(handle))
if len(rows)!=1000 or [int(r['sample']) for r in rows]!=list(range(1,1001)):
    raise ValueError('Expected 1000 sequential samples')
axes=('ax_g','ay_g','az_g')
raw=np.array([[float(r[a]) for a in axes] for r in rows])
if not np.isfinite(raw).all(): raise ValueError('Nonfinite data')
multi=json.loads((root/'accel_multi_calibration.json').read_text())
six=json.loads((root/'accel_calibration.json').read_text())
models={'raw':(np.zeros(3),np.eye(3)),
        'six_face':(np.array([six['parameters'][a]['offset_g'] for a in axes]),
                    np.diag([six['parameters'][a]['gain'] for a in axes])),
        'multi':(np.array(multi['offset_g']),np.array(multi['C']))}
stats=[]
for method,(b,C) in models.items():
    corrected=(raw-b)@C.T
    norms=np.linalg.norm(corrected,axis=1)
    stats.append({'method':method,'samples':1000,'mean_norm_g':float(norms.mean()),
                  'std_norm_g':float(norms.std(ddof=1)),
                  'rmse_from_1g':float(np.sqrt(np.mean((norms-1)**2)))})
    for i,row in enumerate(rows):
        row[method+'_norm_g']=f'{norms[i]:.9f}'
        if method=='multi':
            for a,v in zip(axes,corrected[i]): row[a.replace('_g','_multi_g')]=f'{v:.9f}'
for suffix,records in [('.multi_validated.csv',rows),('.multi_summary.csv',stats)]:
    target=source.with_name(source.stem+suffix)
    with target.open('w',encoding='utf-8-sig',newline='') as handle:
        writer=csv.DictWriter(handle,fieldnames=list(records[0])); writer.writeheader(); writer.writerows(records)
    print('Saved:',target)
source.with_name(source.stem+'.multi_models_used.json').write_text(
    json.dumps({'six_face':six,'multi':multi},indent=2),encoding='utf-8')
for stat in stats: print(stat)

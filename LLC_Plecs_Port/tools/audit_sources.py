"""Record source drift without changing firmware or replacing the frozen model."""
import hashlib,json
from pathlib import Path
from datetime import datetime
from plecs_text import blocks,field
from validate_models import params
ROOT=Path(__file__).resolve().parents[1]
WORK=ROOT.parent
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def components(p):
    t=p.read_text(encoding='utf-8')
    return {field(t[a:b],'Name'):(field(t[a:b],'Type'),params(t[a:b]))
            for a,b,d,k in blocks(t) if d==2 and k=='Component'}
def run():
    old=json.loads((WORK/'docs/LLC移植分析_源码基线_SHA256.json').read_text(encoding='utf-8'))
    rows=[]
    for item in old['files']:
        p=Path(item['path']);now=digest(p) if p.is_file() else None
        rows.append(dict(item,current_sha256=now,status='missing' if now is None else
                         ('match' if now==item['sha256'].lower() else 'changed')))
    frozen=ROOT/'model/baseline/LLC1600_PWM - DLL.plecs'
    candidates=[WORK/'LLC1600_PWM - DLL.plecs',
                WORK/'Visual studio projects/pi_controller/x64/Debug/LLC1600_PWM - DLL.plecs']
    models=[]
    for p in candidates:
        if not p.is_file():continue
        a,b=components(frozen),components(p)
        models.append({'path':str(p),'sha256':digest(p),
                       'added':sorted(b.keys()-a.keys()),'removed':sorted(a.keys()-b.keys()),
                       'changed_common_components':{n:{'frozen':a[n],'current':b[n]}
                                                    for n in sorted(a.keys()&b.keys()) if a[n]!=b[n]}})
    report={'checked_at':datetime.now().astimezone().isoformat(),'sources':rows,
            'frozen_model':{'path':str(frozen),'sha256':digest(frozen)},'current_models':models,
            'note':'Historical source snapshots were hashes only; changed-file history cannot be reconstructed. Generated models continue to use the frozen model.'}
    (ROOT/'results/source_audit.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
    print(json.dumps({'sources':[(Path(x['path']).name,x['status']) for x in rows],
                      'current_models':models},ensure_ascii=False,indent=2))
if __name__=='__main__':run()

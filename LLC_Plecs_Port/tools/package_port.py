"""Package source, verified Debug DLLs, models and selected evidence only."""
import hashlib,json,zipfile
from pathlib import Path
from datetime import datetime
ROOT=Path(__file__).resolve().parents[1]
def selected(p):
    r=p.relative_to(ROOT);parts=r.parts
    if '__pycache__' in parts:return False
    if len(parts)==1:return p.suffix in ('.md','.ps1','.sln','.vcxproj')
    if parts[0] in ('include','core','supervisor','adapters','vendor','tools','tests','config'):
        return p.suffix in ('.h','.c','.py','.cmd','.md')
    if parts[0]=='model':return p.suffix in ('.plecs','.c','.h')
    if parts[0]=='bin':return p.suffix in ('.dll','.exe')
    if parts[0]=='worklog':return p.suffix=='.md'
    if parts[0]=='results':
        return len(parts)==2 and p.suffix in ('.json','.csv','.png','.md') and not p.name.startswith('plecs_debug') and p.name!='delivery_manifest.json'
    return False
def run():
    evidence=json.loads((ROOT/'results/verification.json').read_text(encoding='utf-8'))
    assert evidence['all_passed'],'Run verify_port.py first'
    for n,h in evidence['binary_sha256'].items():
        assert hashlib.sha256((ROOT/'bin/x64'/n).read_bytes()).hexdigest()==h,'DLL changed since verification'
    startup=json.loads((ROOT/'results/startup_summary.json').read_text())
    assert all(startup[n]['last_time']==2 for n in ['Combined','Split'])
    files=sorted(p for p in ROOT.rglob('*') if p.is_file() and selected(p))
    manifest={'packaged_at':datetime.now().astimezone().isoformat(),'configuration':'x64 Debug / PWM development',
              'files':[{'path':p.relative_to(ROOT).as_posix(),'bytes':p.stat().st_size,
                        'sha256':hashlib.sha256(p.read_bytes()).hexdigest()} for p in files]}
    mp=ROOT/'results/delivery_manifest.json';mp.write_text(json.dumps(manifest,indent=2),encoding='utf-8')
    target=ROOT.parent/'LLC_Plecs_Port_PWM_Dev.zip'
    with zipfile.ZipFile(target,'w',compression=zipfile.ZIP_DEFLATED) as z:
        for p in files+[mp]:z.write(p,'LLC_Plecs_Port/'+p.relative_to(ROOT).as_posix())
    with zipfile.ZipFile(target) as z:assert z.testzip() is None
    print(f'Package: {target}\nFiles: {len(files)+1}; bytes: {target.stat().st_size}')
if __name__=='__main__':run()

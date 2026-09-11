"""Run existing native/DLL/model checks and save fresh, machine-readable evidence.
Build first with build.ps1. This does not launch long electrical simulations.
"""
import hashlib,json,subprocess,sys
from pathlib import Path
from datetime import datetime
ROOT=Path(__file__).resolve().parents[1]
def run():
    commands=[[str(ROOT/f'bin/x64/{name}.exe')] for name in ['test_timer','test_gate','test_core','test_slow']]
    commands += [[sys.executable,'-X','utf8',str(ROOT/p)] for p in
                 ['tests/test_dll.py','tests/test_multirate.py','tests/test_run_plecs.py','tools/validate_models.py']]
    commands += [['cmd.exe','/d','/c',str(ROOT/'tools/check_cscripts.cmd')]]
    checks=[]
    for command in commands:
        r=subprocess.run(command,cwd=ROOT,capture_output=True)
        output=(r.stdout+r.stderr).decode('utf-8',errors='replace')
        checks.append({'command':command,'exit_code':r.returncode,'output':output})
        print(('PASS' if r.returncode==0 else 'FAIL')+': '+Path(command[-1]).name,flush=True)
        if r.returncode:break
    bins={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted((ROOT/'bin/x64').glob('*.dll'))}
    report={'checked_at':datetime.now().astimezone().isoformat(),'binary_sha256':bins,
            'all_passed':len(checks)==len(commands) and all(c['exit_code']==0 for c in checks),
            'checks':checks,'scope':'Native tests, loaded DLL replay and structural/C-Script compilation only. Electrical simulation evidence is separate.'}
    (ROOT/'results/verification.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
    if not report['all_passed']:raise SystemExit(1)
if __name__=='__main__':run()

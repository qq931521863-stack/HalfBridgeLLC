"""Compare loaded Combined/Fast/Slow DLLs on the SAME timing contract."""
import csv
from pathlib import Path
from test_dll import Dll
ROOT=Path(__file__).resolve().parents[1]
P=[1,25e-6,1e-3,680e6,1]
def make_f(u,v):
    return u[:3]+[u[10],u[21],u[20],u[22]]+v[:3]+v[3:10]+[u[19]]+v[10:14]
def make_s(u,previous):
    s=u[:20]
    s[3:10]=[previous[i] for i in [21,22,13,15,16,24,23]]
    return s
def run():
    combo,fast,slow=[Dll(ROOT/f'bin/x64/LLC_{n}.dll',P) for n in ['Combined','Fast','Slow']]
    for x in [combo,fast,slow]:assert x.start() is None
    u=[0,32,0]+[0]*7+[1,230,12,1,1,1,0,54,4,0,230,0,0]
    assert len(u)==23
    previous=fast.step(0,make_f(u,list(slow.outputs)))
    v=slow.step(0,make_s(u,previous))
    assert combo.step(0,u)==previous+v
    rows=[]
    for tick in range(1,80001):
        t=tick*25e-6
        # Replay precharge, then relay settling, followed by CC and fault events.
        u[0]=0 if t<1.15 else (35 if t<1.20 else 32)
        u[2]=0 if t<1.8 else 2
        u[21]=0 if t<1.9 else 4194304
        if t>=1.95:u[15]=0
        old=previous
        previous=fast.step(t,make_f(u,v))
        if tick%40==0:v=slow.step(t,make_s(u,old))
        y=combo.step(t,u)
        assert y==previous+v,(tick,[(i,a,b) for i,(a,b) in enumerate(zip(y,previous+v)) if a!=b])
        if tick%40==0:rows.append([t]+y)
    for x in [combo,fast,slow]:x.close()
    with (ROOT/'results/multirate_replay.csv').open('w',newline='') as f:
        w=csv.writer(f);w.writerow(['time']+[f'fast_{i}' for i in range(28)]+[f'slow_{i}' for i in range(17)]);w.writerows(rows)
    print('multirate: PASS (80000 actual DLL ticks / 2 seconds; all 45 outputs exactly equal)')
if __name__=='__main__':run()

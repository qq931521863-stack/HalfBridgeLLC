"""Validate/plot actual PLECS gate outputs (not regenerated ideal curves)."""
import json,csv
from pathlib import Path
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
ROOT=Path(__file__).resolve().parents[1]
fig,axes=plt.subplots(2,1,figsize=(11,6),layout='constrained')
records=[]
for ax,name,first in zip(axes,['03_Gate_PWM','03_Gate_PFM_Windows'],[[1700,2550,5950,6800],[120,1240,1480,2600]]):
    r=json.loads((ROOT/f'results/plecs_{name}.json').read_text())
    ts=r['Time'];v=r['Values'];edges=[[],[]]
    for j in range(2):
        for i in range(1,len(ts)):
            if v[j][i]!=v[j][i-1]:
                edges[j].append((ts[i],v[j][i]));records.append([name,'TC'+str(j+1),ts[i],v[j][i]])
        assert len(edges[j])>=2
        assert abs(edges[j][0][0]-first[j*2]/680e6)<1e-13
        assert abs(edges[j][1][0]-first[j*2+1]/680e6)<1e-13
    assert all(not(a and b) for a,b in zip(v[0],v[1]))
    assert max(v[2])==0 and max(v[3])==0
    for j,color in enumerate(['#137b80','#e08d2a']):ax.step([t*1e6 for t in ts], [a+j*1.5 for a in v[j]],where='post',label=['TC1 / lower primary','TC2 / upper primary'][j],color=color)
    ax.set_xlim(0,30);ax.set_ylim(-.15,2.85);ax.set_yticks([0,1,1.5,2.5],['0','1','0','1']);ax.grid(alpha=.2)
    ax.set_title('PWM: 80 kHz, duty parameter 0.1' if name.endswith('PWM') else 'PFM window test: 250 kHz, 240-count inter-switch gap = 352.941 ns')
    ax.set_xlabel('PLECS simulation time (us)');ax.legend(loc='upper right')
with (ROOT/'results/plecs_gate_edges.csv').open('w',newline='') as f:
    w=csv.writer(f);w.writerow(['model','gate','time_s','value']);w.writerows(records)
fig.savefig(ROOT/'results/plecs_gate_waveforms.png',dpi=160)
print('PLECS edge checks PASS: first on/off edges match counts within 0.1 ps; no primary overlap; SR off')

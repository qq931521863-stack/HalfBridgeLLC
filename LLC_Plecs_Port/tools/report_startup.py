"""Summarize actual PLECS startup traces; sampled at 1 ms, not gate-edge data."""
import json,math,csv
from pathlib import Path
from run_plecs import validate_result
ROOT=Path(__file__).resolve().parents[1]
RESULTS=ROOT/'results'
def transitions(t,y):return [[t[i],v] for i,v in enumerate(y) if i==0 or v!=y[i-1]]
def run():
    loaded={};summary={}
    for name in ['Combined','Split']:
        r=json.loads((RESULTS/f'plecs_LLC1600_{name}_DLL_Port.json').read_text())
        validate_result(r,2.0)
        t,y=r['Time'],r['Values'];loaded[name]=(t,y)
        assert len(y)==58 and all(math.isfinite(v) for row in y for v in row)
        assert y[15][-1]==5 and y[13][-1]==1,(name,'not in PWM CC with relay closed')
        assert max(y[23])==0 and max(y[27])==0,(name,'fault/diagnostic during startup')
        assert y[17][-1]>1 and y[22][-1]>1,(name,'no positive charging-current response')
        summary[name]={'last_time':t[-1],'mode_events_sampled':transitions(t,y[15]),
                       'precharge_events_sampled':transitions(t,y[16]),
                       'relay_events_sampled':transitions(t,y[13]),
                       'final_Iref_A':y[17][-1],'final_Ibat_FIR_A':y[22][-1],
                       'final_Vbat_FIR_V':y[21][-1],'final_frequency_Hz':y[19][-1],
                       'final_duty_parameter':y[20][-1],
                       'max_fast_fault':max(y[23]),'max_diagnostics':max(y[27])}
    a,b=loaded['Combined'],loaded['Split'];assert a[0]==b[0]
    differences={i:max(abs(x-y) for x,y in zip(a[1][i],b[1][i])) for i in range(28)}
    # Numerical electrical states can differ with solver ordering. Discrete
    # operating events must agree on the observation grid.
    for i in [12,13,14,15,16,23,24,25,26,27]:assert differences[i]==0,(i,differences[i])
    summary['max_fast_output_difference']=differences
    summary['observation']='1 ms output grid; transition timestamps have up to 1 ms observation quantization. This is startup, not steady-state/1600 W acceptance.'
    (RESULTS/'startup_summary.json').write_text(json.dumps(summary,indent=2),encoding='utf-8')
    with (RESULTS/'startup_comparison.csv').open('w',newline='') as f:
        w=csv.writer(f);w.writerow(['time_s']+[f'{n}_{k}' for n in loaded for k in ['mode','preOK','relay','Iref_A','Ibat_FIR_A','Vbat_FIR_V','duty']])
        for j,t in enumerate(a[0]):w.writerow([t]+[loaded[n][1][i][j] for n in loaded for i in [15,16,13,17,22,21,20]])
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    fig,axes=plt.subplots(3,1,figsize=(11,8),sharex=True,layout='constrained')
    for n,(t,y) in loaded.items():
        style='-' if n=='Combined' else '--'
        axes[0].step(t,y[15],where='post',linestyle=style,label=n)
        axes[1].plot(t,y[17],style,label=n+' Iref')
        axes[1].plot(t,y[22],style,alpha=.7,label=n+' Ibat FIR')
        axes[2].step(t,y[13],where='post',linestyle=style,label=n+' relay')
    axes[0].set_yticks([0,2,5],['Idle','BMS precharge','PWM CC'])
    axes[1].set_ylabel('Current (A)');axes[2].set_ylabel('Relay command')
    axes[2].set_xlabel('PLECS simulation time (s)');axes[2].set_xlim(0,2)
    for ax in axes:ax.grid(alpha=.2);ax.legend(loc='upper left',fontsize=8)
    fig.suptitle('Actual PLECS startup: 380 V bus / 32 V battery / 4 A request\n1 ms observations; SR switches disabled; not steady-state acceptance')
    fig.savefig(RESULTS/'plecs_startup_comparison.png',dpi=150)
    print(json.dumps(summary,indent=2))
if __name__=='__main__':run()

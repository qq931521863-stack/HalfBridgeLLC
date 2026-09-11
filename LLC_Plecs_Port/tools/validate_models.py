"""Structural checks plus extraction of actual C-Script code for host compilation.
Does NOT substitute for loading/simulating the models in PLECS.
"""
from pathlib import Path
import re,json,ctypes,hashlib
from plecs_text import blocks,field
ROOT=Path(__file__).resolve().parents[1]
def params(block):return {field(block[a:b],'Variable'):field(block[a:b],'Value') for a,b,_,_ in blocks(block,'Parameter')}
def vec(v):return [float(x) for x in re.findall(r'[-+]?\d+(?:\.\d*)?(?:e[-+]?\d+)?',v,re.I)]
def endpoints(block):return [(json.loads(n),int(p)) for n,p in re.findall(r'(?:Src|Dst)Component\s+("(?:[^"\\]|\\.)*")\s+(?:Src|Dst)Terminal\s+(\d+)',block)]
def validate(path):
    text=path.read_text(encoding='utf-8');bs=blocks(text)
    # PLECS silently ignores declarations/wiring out of schematic order.
    stages={'Component':0,'Connection':1,'Annotation':2}
    order=[stages[k] for a,b,d,k in bs if d==2 and k in stages]
    assert order==sorted(order),(path.name,'components/connections/annotations out of order')
    comps={field(text[a:b],'Name'):(field(text[a:b],'Type'),params(text[a:b])) for a,b,d,k in bs if d==2 and k=='Component'}
    assert len(comps)==sum(d==2 and k=='Component' for a,b,d,k in bs),'duplicate root names'
    ports=sorted(int(p['Index']) for kind,p in comps.values() if kind=='Output')
    registered=sorted(int(field(text[a:b],'Index')) for a,b,d,k in bs
                      if d==1 and k=='Terminal' and field(text[a:b],'Type')=='Output')
    assert ports==registered,(path.name,'unregistered top-level outputs')
    used={}; nets=[]
    for a,b,d,k in bs:
        if k!='Connection' or d!=2:continue
        ep=endpoints(text[a:b]);assert len(ep)>=2
        for name,port in ep:
            assert name in comps,(path.name,'missing component',name)
            assert (name,port) not in used,(path.name,'terminal belongs to duplicate nets',name,port)
            used[name,port]=field(text[a:b],'Type')
        nets.append(ep)
    for name,(kind,p) in comps.items():
        if name in ('K_RELAY','K_DIS'):
            assert kind=='Switch',(path.name,name,'requires single-pole signal-controlled switch')
            assert all((name,k) in used for k in (1,2,3)),(path.name,name,'unconnected switch terminal')
        if kind=='Dll':
            assert (path.parent/p['Filename']).is_file(),p['Filename']
            assert len(vec(p['Parameters']))==(2 if name=='LLC_Probe' else 5)
            assert (name,1) in used and (name,2) in used
        if kind=='CScript':
            ni=len(vec(p['NumInputs']));no=len(vec(p['NumOutputs']))
            for port in range(1,ni+1):assert (name,port) in used,(path.name,name,'unconnected input',port)
            extract(path,name,p,ni,no)
    print(f'STRUCTURE PASS: {path.name}: {len(comps)} components, {len(nets)} nets')
def extract(path,name,p,ni,no):
    target=ROOT/'build/cscript_checks';target.mkdir(parents=True,exist_ok=True)
    decl=p['Declarations']
    decl=re.sub(r'#include "([^"]+)"',lambda m:'#include "'+(path.parent/m.group(1)).resolve().as_posix()+'"',decl)
    harness='''#include <stddef.h>
#include <math.h>
static double inputs[4][64],outputs[4][64],disc[64],CurrentTime,NextSampleHit;
#define InputSignal(i,j) inputs[i][j]
#define OutputSignal(i,j) outputs[i][j]
#define DiscState(i) disc[i]
#define IsSampleHit(i) (1)
#define SetErrorMessage(s) ((void)(s))
'''
    harness+=decl+'\nvoid start(void){'+p['StartFcn']+'}\nvoid output(void){'+p['OutputFcn']+'}\nvoid update(void){'+p['UpdateFcn']+'}\n'
    filename=path.stem+'_'+name
    (target/(filename+'.c')).write_text(harness,encoding='utf-8')
def run():
    for path in sorted((ROOT/'model').glob('*.plecs')):validate(path)
    for path in sorted((ROOT/'model/benches').glob('*.plecs')):validate(path)
if __name__=='__main__':run()

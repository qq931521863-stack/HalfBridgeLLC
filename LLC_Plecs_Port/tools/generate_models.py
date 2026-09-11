from pathlib import Path
import json,hashlib
from plecs_text import *
ROOT=Path(__file__).resolve().parents[1]
MODEL=ROOT/'model'
BASE=MODEL/'baseline/LLC1600_PWM - DLL.plecs'
source=BASE.read_text(encoding='utf-8')
header=source[:next(a for a,b,d,k in blocks(source,'Schematic') if d==1)]
def model(name,body,span='.001',init=''):
    h=set_field(header,'Name',name)
    h=set_field(h,'TimeSpan',span)
    h=set_field(h,'InitializationCommands',init)
    return h+'Schematic {\nLocation [50,50;1400,850]\nZoomFactor 1\nSliderPosition [0,0]\n'+body+'\n}\n}\n'
def const(name,xy,value): return component('Constant',name,xy,{'Value':value,'DataType':10})
def output(name,xy,index,width): return component('Output',name,xy,{'Index':index,'Width':width})
def dll(name,xy,filename,ts,params):
    return component('Dll',name,xy,{'Filename':'../bin/x64/'+filename,'SampleTime':ts,'OutputDelay':0,'Parameters':params})
def cscript(name,xy,ni,no,ts,decl,start,run,update='',nd=0):
    params={'NumInputs':ni,'NumOutputs':no,'NumContStates':0,'NumDiscStates':nd,'NumZCSignals':0,'DirectFeedthrough':1,'Ts':ts,'Parameters':'','LangStandard':2,'GnuExtensions':1,'RuntimeCheck':2,'Declarations':decl,'StartFcn':start,'OutputFcn':run,'UpdateFcn':update,'DerivativeFcn':'','TerminateFcn':'','StoreCustomStateFcn':'','RestoreCustomStateFcn':''}
    return component('CScript',name,xy,params)
def gate(name,xy,relative='../gate'):
    return cscript(name,xy,15,4,'[25e-6,0;-2,0]',
        f'#include "{relative}/llc_gate_events.c"\n#include <float.h>\nstatic LlcGate committed,candidate;',
        'llcGateReset(&committed); llcGateReset(&candidate); DiscState(0)=0; NextSampleHit=DBL_MAX;',
        'double u[15],y[4],next; int k;\nfor(k=0;k<15;k++) u[k]=InputSignal(0,k);\nif(!llcGateEvaluate(&committed,CurrentTime,IsSampleHit(0),u,&candidate,y,&next)) SetErrorMessage("Invalid LLC timer frame");\nfor(k=0;k<4;k++) OutputSignal(0,k)=y[k];\nNextSampleHit=isfinite(next)?next:DBL_MAX;',
        'committed=candidate; DiscState(0)+=1;',1)
def write(path,text):
    if path.parent.name=='benches':text=text.replace('"../bin/x64/','"../../bin/x64/')
    # PLECS reads component declarations before wiring; declarations appended
    # after the first Connection are not registered as schematic components.
    declarations=[(a,b,text[a:b]) for a,b,d,_ in blocks(text,'Component') if d==2]
    for a,b,_ in reversed(declarations):text=text[:a]+text[b:]
    connections=[a for a,b,d,_ in blocks(text,'Connection') if d==2]
    if connections:
        a=connections[0]
    else:a=next(b-1 for a,b,d,_ in blocks(text,'Schematic') if d==1)
    text=text[:a]+'\n'.join(t for a,b,t in declarations)+'\n'+text[a:]
    wiring=[(a,b,text[a:b]) for a,b,d,_ in blocks(text,'Connection') if d==2]
    for a,b,_ in reversed(wiring):text=text[:a]+text[b:]
    a=max(b for a,b,d,_ in blocks(text,'Component') if d==2)
    text=text[:a]+'\n'+'\n'.join(t for a,b,t in wiring)+'\n'+text[a:]
    # Top-level ports also require a model-level Terminal registration in PLECS.
    for a,b,d,_ in reversed(blocks(text,'Terminal')):
        if d==1:text=text[:a]+text[b:]
    ports=[]
    for a,b,d,_ in blocks(text,'Component'):
        if d==2 and field(text[a:b],'Type')=='Output':
            for x,y,_,_ in blocks(text[a:b],'Parameter'):
                p=text[a:b][x:y]
                if field(p,'Variable')=='Index':ports.append(int(field(p,'Value')))
    a=next(a for a,b,d,_ in blocks(text,'Schematic') if d==1)
    text=text[:a]+''.join(f'Terminal {{\nType Output\nIndex "{i}"\n}}\n' for i in sorted(ports))+text[a:]
    blocks(text) # fail immediately on malformed braces/strings
    path.parent.mkdir(parents=True,exist_ok=True);path.write_text(text,encoding='utf-8')
def connect(text,src,st,dst,dt,typ='Signal'):
    """Add a branch to the existing net rather than duplicating a source terminal."""
    def endpoints(t):
        return [(json.loads(n),int(i)) for n,i in re.findall(r'(?:Src|Dst)Component\s+("(?:[^"\\]|\\.)*")\s+(?:Src|Dst)Terminal\s+(\d+)',t)]
    for a,b,d,_ in blocks(text,'Connection'):
        if d!=2 or field(text[a:b],'Type')!=typ:continue
        ep=endpoints(text[a:b])
        if (src,st) in ep or (typ=='Wire' and (dst,dt) in ep):
            if (src,st) in ep and (dst,dt) in ep:return text
            new=(dst,dt) if (src,st) in ep else (src,st)
            block=text[a:b]
            # A simple connection's destination becomes a branch before fanout.
            if not blocks(block,'Branch'):
                m=re.search(r'\s*DstComponent\s+"(?:[^"\\]|\\.)*"\s+DstTerminal\s+\d+',block)
                if m:block=block[:m.start()]+'\nBranch {\n'+m.group()+'\n}\n'+block[m.end():]
            block=block[:-1]+f'\nBranch {{\nDstComponent {json.dumps(new[0])}\nDstTerminal {new[1]}\n}}\n}}'
            return text[:a]+block+text[b:]
    return insert(text,wire(src,st,dst,dt,typ))
def insert(text,body):
    a,b,_,_=next(x for x in blocks(text,'Schematic') if x[2]==1)
    return text[:b-1]+body+'\n'+text[b-1:]
def plant():
    t=set_field(source,'Name','LLC1600_PWM_DLL_Port')
    t=set_field(t,'TimeSpan','.05')
    # Remove only the old control/reference blocks and their signal nets.
    remove={'DT','LEAD7','RE1','Constant2','Vref','Sum','PI voltage\ncontroller','Scope1','Mux12'}
    cuts=[]
    for a,b,d,kind in blocks(t):
        if d==2 and kind=='Component' and field(t[a:b],'Name') in remove:cuts.append((a,b,''))
        if d==2 and kind=='Connection':
            n=[json.loads(x) for x in re.findall(r'(?:Src|Dst)Component\s+("(?:[^"\\]|\\.)*")',t[a:b])]
            if any(x in remove for x in n):cuts.append((a,b,''))
    for a,b,x in sorted(cuts,reverse=True):t=t[:a]+x+t[b:]
    # Restore Vm25 scope net removed together with the PI feedback branch.
    t=connect(t,'Vm25',3,'Scope5',5)
    # Break ONLY the C15->R16 branch. Preserve R24/C16 filter and battery model.
    old='DstComponent  "R16"\n            DstTerminal   1'
    if t.count(old)!=1:raise ValueError('R16 branch baseline changed')
    t=t.replace(old,'DstComponent  "R_RELAY_ON"\n            DstTerminal   1')
    body=component('Resistor','R_RELAY_ON',(1800,570),{'R':'1e-3'})
    body+=component('Switch','K_RELAY',(1840,570),{'s_init':0,'SwitchModel':1,'f_grid':0})
    body+=component('Resistor','R_DIS',(2100,590),{'R':100})
    body+=component('Switch','K_DIS',(2100,700),{'s_init':0,'SwitchModel':1,'f_grid':0})
    body+=component('Voltmeter','V_BAT',(1990,740))
    # Default CC replay command. seq=1 is consumed once; use startup bench for precharge.
    body+=const('Fast_commands_3_to_21',(300,1060),'[1 0 230 0 1 5 0 4 54 1 1 3 1 0 0 1 1 0 0]')
    body+=component('SignalMux','Fast_input_mux',(560,1020),{'Width':'[1 1 1 19]'})
    body+=dll('LLC_Fast',(740,1020),'LLC_Fast.dll','25e-6','[1,25e-6,1e-3,680e6,1]')
    body+=cscript('Gate_frame_adapter',(990,1020),'[28 19]',15,'25e-6','', '',
        'int k; for(k=0;k<12;k++) OutputSignal(0,k)=InputSignal(0,k);\nOutputSignal(0,12)=InputSignal(0,26);\nOutputSignal(0,13)=(InputSignal(0,0)>0 && InputSignal(0,27)==0);\nOutputSignal(0,14)=InputSignal(1,14);')
    body+=gate('LLC_Gate',(1210,1020),'gate')
    body+=component('SignalDemux','Primary_and_SR',(1410,1020),{'Width':'[1 1 1 1]'})
    body+=component('SignalSelector','Relay_cmd',(950,1120),{'InputWidth':28,'OutputIndices':'14'})
    body+=component('SignalSelector','Discharge_cmd',(1140,1120),{'InputWidth':28,'OutputIndices':'15'})
    body+=const('SR_off_pending_polarity',(1430,1150),'0')
    body+=output('Fast_trace_28',(1620,1030),1,28)+output('Gate_trace_4',(1620,1090),2,4)
    body+=component('SignalMux','Additional_measurements',(1860,1200),{'Width':'[1 1 1 1 1 1 1 1 1]'})
    body+=output('Measurements_and_SR_requests',(2050,1200),3,9)
    t=insert(t,body)
    for s,sp,d,dp in [('R_RELAY_ON',2,'K_RELAY',1),('K_RELAY',2,'R16',1),('K_RELAY',2,'V_BAT',1),('V_BAT',2,'Am7',2),('C15',1,'R_DIS',1),('R_DIS',2,'K_DIS',1),('K_DIS',2,'Am7',2)]:t=connect(t,s,sp,d,dp,'Wire')
    for s,sp,d,dp in [('Vm15',3,'Fast_input_mux',2),('V_BAT',3,'Fast_input_mux',3),('Am7',3,'Fast_input_mux',4),('Fast_commands_3_to_21',1,'Fast_input_mux',5),('Fast_commands_3_to_21',1,'Gate_frame_adapter',2),('Fast_input_mux',1,'LLC_Fast',1),('LLC_Fast',2,'Fast_trace_28',1),('LLC_Fast',2,'Gate_frame_adapter',1),('Gate_frame_adapter',3,'LLC_Gate',1),('LLC_Gate',2,'Primary_and_SR',1),('LLC_Gate',2,'Gate_trace_4',1),('LLC_Gate',2,'Scope5',1),('Primary_and_SR',2,'FETD21',3),('Primary_and_SR',3,'FETD22',3),('LLC_Fast',2,'Relay_cmd',1),('LLC_Fast',2,'Discharge_cmd',1),('Relay_cmd',2,'K_RELAY',3),('Discharge_cmd',2,'K_DIS',3),('SR_off_pending_polarity',1,'FETD23',3),('SR_off_pending_polarity',1,'FETD24',3)]:t=connect(t,s,sp,d,dp)
    for port,name in enumerate(['Am6','Am8','Am9','Vm20','Vm22','Vm16','Am15'],2):t=connect(t,name,3,'Additional_measurements',port)
    t=connect(t,'Primary_and_SR',4,'Additional_measurements',9)
    t=connect(t,'Primary_and_SR',5,'Additional_measurements',10)
    t=connect(t,'Additional_measurements',1,'Measurements_and_SR_requests',1)
    write(MODEL/'LLC1600_PWM_DLL_Port.plecs',t)
    startup=edit_named(t,'Fast_commands_3_to_21',lambda b:set_param(b,'Value','[1 0 230 0 1 2 151 4 54 1 1 3 0 0 0 3 0 0 0]'),2)
    startup=set_field(startup,'Name','05_Startup_BMS')
    startup=set_field(startup,'TimeSpan','1')
    startup=edit_named(startup,'LLC_Gate',lambda b:set_param(b,'Declarations',field(next(b[x:y] for x,y,_,_ in blocks(b,'Parameter') if field(b[x:y],'Variable')=='Declarations'),'Value').replace('"gate/','"../gate/')),2)
    write(MODEL/'benches/05_Startup_BMS.plecs',startup)
def benches():
    body=const('Stimulus',(100,150),'[11 22 33 '+ '0 '*19+']')
    body+=dll('LLC_Probe',(300,150),'LLC_Probe.dll','25e-6','[1,25e-6]')
    body+=output('Probe_28',(500,150),1,28)+wire('Stimulus',1,'LLC_Probe',1)+wire('LLC_Probe',2,'Probe_28',1)
    write(MODEL/'benches/01_DLL_IO.plecs',model('01_DLL_IO',body))
    # Frequency/window-only PFM bench does not claim closed-loop PFM availability.
    for name,u in [('03_Gate_PWM','[8500 1700 2550 5950 6800 0 0 0 0 1 1 0 1 1 0]'),('03_Gate_PFM_Windows','[2720 120 1240 1480 2600 0 0 0 0 1 1 0 1 1 0]')]:
        body=const('Timer_frame',(100,150),u)+gate('LLC_Gate',(350,150))+output('TC1_TC2_TD1_TD2',(600,150),1,4)
        body+=wire('Timer_frame',1,'LLC_Gate',1)+wire('LLC_Gate',2,'TC1_TC2_TD1_TD2',1)
        write(MODEL/f'benches/{name}.plecs',model(name,body,'.0001'))
    body=const('Fast_22_inputs',(100,150),'[32 32 0 1 0 230 0 1 5 0 4 54 1 1 3 1 0 0 0 0 0 0]')
    body+=dll('LLC_Fast',(350,150),'LLC_Fast.dll','25e-6','[1,25e-6,1e-3,680e6,1]')+output('Fast_28',(600,150),1,28)
    body+=wire('Fast_22_inputs',1,'LLC_Fast',1)+wire('LLC_Fast',2,'Fast_28',1)
    write(MODEL/'benches/04_PWM_CC_Replay.plecs',model('04_PWM_CC_Replay',body,'.01'))
def system_models():
    t=(MODEL/'LLC1600_PWM_DLL_Port.plecs').read_text(encoding='utf-8')
    t=set_field(t,'Name','LLC1600_Combined_DLL_Port');t=set_field(t,'TimeSpan','2')
    t=edit_named(t,'Fast_commands_3_to_21',lambda b:set_param(b,'Value','[0 0 0 0 0 0 0 1 230 12 1 1 1 0 54 4 0 230 0 0]'),2)
    t=edit_named(t,'Fast_input_mux',lambda b:set_param(b,'Width','[1 1 1 20]'),2)
    t=edit_named(t,'LLC_Fast',lambda b:set_param(b,'Filename','../bin/x64/LLC_Combined.dll'),2)
    t=t.replace('"LLC_Fast"','"LLC_Combined"').replace('"Fast_commands_3_to_21"','"Scenario_3_to_22"').replace('"Fast_input_mux"','"Scenario_mux"')
    t=edit_named(t,'Fast_trace_28',lambda b:set_param(b,'Width','45'),2)
    t=t.replace('"Fast_trace_28"','"Combined_trace_45"')
    for name in ['Relay_cmd','Discharge_cmd']:t=edit_named(t,name,lambda b:set_param(b,'InputWidth','45'),2)
    t=edit_named(t,'Gate_frame_adapter',lambda b:set_param(set_param(b,'NumInputs','[45 20]'),'OutputFcn',
        'int k; for(k=0;k<12;k++) OutputSignal(0,k)=InputSignal(0,k);\nOutputSignal(0,12)=InputSignal(0,26);\nOutputSignal(0,13)=(InputSignal(0,44)!=0 && InputSignal(0,0)>0 && (((unsigned int)InputSignal(0,27)&7)==0));\nOutputSignal(0,14)=InputSignal(1,16);'),2)
    # Remove the direct scenario-to-controller edge before inserting fault latch.
    for a,b,d,_ in reversed(blocks(t,'Connection')):
        if d==2 and field(t[a:b],'SrcComponent')=='Scenario_mux':t=t[:a]+t[b:]
    body=cscript('Hardware_fault_latch',(600,1210),23,23,'[0,-1]',
        '#include <math.h>\n#include <stdint.h>',
        'DiscState(0)=0;DiscState(1)=0;',
        'int k; uint32_t fault=(uint32_t)DiscState(0); double raw=InputSignal(0,21);\nif(InputSignal(0,19)!=0 && DiscState(1)==0) fault=0;\nif(!isfinite(raw)||raw<0||raw>4294967295.0||floor(raw)!=raw) { SetErrorMessage("Invalid hardware fault bits"); fault=0x80000000u; } else fault|=(uint32_t)raw;\nfor(k=0;k<23;k++) OutputSignal(0,k)=InputSignal(0,k); OutputSignal(0,21)=(double)fault;',
        'double raw=InputSignal(0,21); uint32_t fault=(uint32_t)DiscState(0);\nif(InputSignal(0,19)!=0 && DiscState(1)==0) fault=0;\nif(isfinite(raw)&&raw>=0&&raw<=4294967295.0&&floor(raw)==raw) fault|=(uint32_t)raw; else fault|=0x80000000u;\nDiscState(0)=(double)fault; DiscState(1)=(InputSignal(0,19)!=0);',2)
    body+=cscript('Immediate_gate_inhibit',(1330,1190),'[4 23]',4,'[0,-1]','','',
        'int k; double allow=(InputSignal(1,21)==0); for(k=0;k<4;k++) OutputSignal(0,k)=InputSignal(0,k)*allow;')
    t=insert(t,body)
    # Route all external gate destinations through the immediate hardware inhibit.
    for a,b,d,_ in reversed(blocks(t,'Connection')):
        if d==2 and field(t[a:b],'SrcComponent')=='LLC_Gate':
            block=set_field(t[a:b],'SrcComponent','Immediate_gate_inhibit')
            block=re.sub(r'(SrcTerminal\s+)2',r'\g<1>3',block,count=1)
            t=t[:a]+block+t[b:]
    for s,sp,d,dp in [('Scenario_mux',1,'Hardware_fault_latch',1),('Hardware_fault_latch',2,'LLC_Combined',1),('Hardware_fault_latch',2,'Immediate_gate_inhibit',2),('LLC_Gate',2,'Immediate_gate_inhibit',1)]:t=connect(t,s,sp,d,dp)
    write(MODEL/'LLC1600_Combined_DLL_Port.plecs',t)
    split=set_field(t,'Name','LLC1600_Split_DLL_Port')
    split=edit_named(split,'LLC_Combined',lambda b:set_param(b,'Filename','../bin/x64/LLC_Fast.dll'),2)
    split=split.replace('"LLC_Combined"','"LLC_Fast"').replace('"Combined_trace_45"','"Fast_trace_28"')
    split=edit_named(split,'Fast_trace_28',lambda b:set_param(b,'Width','28'),2)
    for name in ['Relay_cmd','Discharge_cmd']:split=edit_named(split,name,lambda b:set_param(b,'InputWidth','28'),2)
    # Hardware latch fanout: replace its former Fast destination with Fast assembler.
    split=split.replace('DstComponent "LLC_Fast"\n DstTerminal 1','DstComponent "Fast_assemble"\n DstTerminal 1')
    # Existing connection formatting is not necessarily one space; operate structurally.
    for a,b,d,_ in reversed(blocks(split,'Connection')):
        if d==2 and field(split[a:b],'SrcComponent')=='Hardware_fault_latch':
            block=re.sub(r'(DstComponent\s+)"LLC_Fast"',r'\g<1>"Fast_assemble"',split[a:b])
            split=split[:a]+block+split[b:]
    body=cscript('Fast_assemble',(690,1280),'[23 17]',22,'25e-6','','',
        'int k; for(k=0;k<3;k++) OutputSignal(0,k)=InputSignal(0,k);\nOutputSignal(0,3)=InputSignal(0,10);OutputSignal(0,4)=InputSignal(0,21);OutputSignal(0,5)=InputSignal(0,20);OutputSignal(0,6)=InputSignal(0,22);\nfor(k=0;k<10;k++)OutputSignal(0,7+k)=InputSignal(1,k);\nOutputSignal(0,17)=InputSignal(0,19);for(k=0;k<4;k++)OutputSignal(0,18+k)=InputSignal(1,10+k);')
    body+=cscript('Slow_assemble',(1000,1370),'[23 28]',20,'1e-3','','',
        'int k; int ix[7]={21,22,13,15,16,24,23};for(k=0;k<20;k++)OutputSignal(0,k)=InputSignal(0,k);for(k=0;k<7;k++)OutputSignal(0,3+k)=InputSignal(1,ix[k]);')
    delay=cscript('Fast_snapshot_z1',(820,1430),28,28,'25e-6','',
        'int k;for(k=0;k<28;k++)DiscState(k)=0;',
        'int k;for(k=0;k<28;k++)OutputSignal(0,k)=DiscState(k);',
        'int k;for(k=0;k<28;k++)DiscState(k)=InputSignal(0,k);',28)
    body+=set_param(delay,'DirectFeedthrough',0)
    body+=set_param(dll('LLC_Slow',(1220,1370),'LLC_Slow.dll','1e-3','[1,25e-6,1e-3,680e6,1]'),'OutputDelay','1e-9')
    body+=output('Slow_trace_17',(1620,1370),4,17)
    split=insert(split,body)
    split=edit_named(split,'Gate_frame_adapter',lambda b:set_param(set_param(b,'NumInputs','[28 20 17]'),'OutputFcn',
        'int k;for(k=0;k<12;k++)OutputSignal(0,k)=InputSignal(0,k);OutputSignal(0,12)=InputSignal(0,26);OutputSignal(0,13)=(InputSignal(2,16)!=0&&InputSignal(0,0)>0&&(((unsigned int)InputSignal(0,27)&7)==0));OutputSignal(0,14)=InputSignal(1,16);'),2)
    # Gate adapter now has three input ports; its output is terminal4.
    for a,b,d,_ in reversed(blocks(split,'Connection')):
        if d==2 and field(split[a:b],'SrcComponent')=='Gate_frame_adapter':
            block=re.sub(r'(SrcTerminal\s+)3',r'\g<1>4',split[a:b],count=1);split=split[:a]+block+split[b:]
    for s,sp,d,dp in [('Hardware_fault_latch',2,'Slow_assemble',1),('Fast_assemble',3,'LLC_Fast',1),('LLC_Slow',2,'Fast_assemble',2),('LLC_Slow',2,'Gate_frame_adapter',3),('LLC_Slow',2,'Slow_trace_17',1),('LLC_Fast',2,'Fast_snapshot_z1',1),('Fast_snapshot_z1',2,'Slow_assemble',2),('Slow_assemble',3,'LLC_Slow',1)]:split=connect(split,s,sp,d,dp)
    write(MODEL/'LLC1600_Split_DLL_Port.plecs',split)
if __name__=='__main__':
    benches()
    plant()
    system_models()
    (ROOT/'results/model_baseline.json').write_text(json.dumps({'source':str(BASE),'sha256':hashlib.sha256(BASE.read_bytes()).hexdigest()},indent=2),encoding='utf-8')
    print('Generated probe, gate PWM/PFM-window, PWM replay, connected plant and BMS startup models')


"""Exercise the actual x64 DLL ABI, restart and independent instances."""
import ctypes as C
from pathlib import Path
class Sizes(C.Structure):
    _pack_=4
    _fields_=[(x,C.c_int) for x in ['numInputs','numOutputs','numStates','numParameters']]
class State(C.Structure):
    _pack_=4
    _fields_=[(x,C.POINTER(C.c_double)) for x in ['inputs','outputs','states','parameters']]+[('time',C.c_double),('errorMessage',C.c_char_p),('userData',C.c_void_p)]
class Dll:
    def __init__(self,path,parameters):
        self.lib=C.CDLL(str(path)); self.sizes=Sizes()
        for name,typ in [('plecsSetSizes',Sizes),('plecsStart',State),('plecsOutput',State),('plecsTerminate',State)]:
            f=getattr(self.lib,name); f.argtypes=[C.POINTER(typ)]; f.restype=None
        self.lib.plecsSetSizes(C.byref(self.sizes))
        s=self.sizes
        assert len(parameters)==s.numParameters
        self.inputs=(C.c_double*s.numInputs)(); self.outputs=(C.c_double*s.numOutputs)()
        self.states=(C.c_double*max(1,s.numStates))(); self.params=(C.c_double*s.numParameters)(*parameters)
        self.state=State(self.inputs,self.outputs,self.states,self.params,0,None,None)
    def start(self):
        self.lib.plecsStart(C.byref(self.state))
        return self.state.errorMessage
    def step(self,t,values):
        self.inputs[:]=values; self.state.time=t
        self.lib.plecsOutput(C.byref(self.state))
        return list(self.outputs)
    def close(self): self.lib.plecsTerminate(C.byref(self.state))
def probe():
    path=Path(__file__).resolve().parents[1]/'bin/x64/LLC_Probe.dll'
    a,b=Dll(path,[1,25e-6]),Dll(path,[1,25e-6])
    assert (a.sizes.numInputs,a.sizes.numOutputs,a.sizes.numStates)==(22,28,1)
    assert a.start() is None and b.start() is None
    for k in range(101):
        y=a.step(k*25e-6,[11,22,33]+[0]*19)
        assert y[19:22]==[11,22,33] and y[26]==k
        assert y[9:12]==[0,0,0]
    assert b.step(0,[44,55,66]+[0]*19)[26]==0
    a.close(); assert a.start() is None
    assert a.step(0,[0]*22)[26]==0
    a.close(); b.close()
    bad=Dll(path,[2,25e-6]); assert bad.start(); bad.close()
    print('DLL probe: PASS (exports, pack4 ABI, 101 callbacks, two instances, restart, bad ABI)')
def fast():
    path=Path(__file__).resolve().parents[1]/'bin/x64/LLC_Fast.dll'
    params=[1,25e-6,1e-3,680e6,1]
    a,b=Dll(path,params),Dll(path,params)
    assert a.start() is None and b.start() is None
    u=[32,32,0,1,0,230,0,1,5,1,4,54,1,1,3,1,0,0,1,1,0,0]
    assert a.step(0,u)==[0]*28
    for k in range(1,1001):
        y=a.step(k*25e-6,u)
        assert y[26]==k and y[27]==0 and y[15]==5
        assert y==a.step(k*25e-6,u)
    first=b.step(25e-6,u)
    assert first[26]==1 and first[17]<y[17] and first[25]==1
    u[1]=62;y=a.step(.025025,u)
    assert y[9:12]==[0,0,0] and y[23]==128 and y[13]==0
    a.close(); assert a.start() is None
    # Restart with a fresh time base as supplied by PLECS.
    a.state.time=0;a.close();assert a.start() is None
    u[1]=40;u[8]=7;y=a.step(25e-6,u)
    assert y[27]==1 and y[9:14]==[0,0,0,0,0]
    a.close();b.close()
    bad=Dll(path,[1,25e-6,1e-3,680e6,0]); assert bad.start();bad.close()
    print('DLL fast: PASS (t0, 1000 ticks, duplicate callbacks, instances, fault, PFM/profile rejection)')
if __name__=='__main__': probe();fast()

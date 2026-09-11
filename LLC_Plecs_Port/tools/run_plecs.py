"""Run generated models via the installed PLECS documented XML-RPC interface.

Enable RPC in PLECS Preferences (this workspace uses port 1180). No original user model is
closed or modified. Results are from PLECS itself, not the native replay tests.
"""
import xmlrpc.client,socket,json,csv,argparse
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def validate_result(result,expected_end):
    times=result['Time'];values=result['Values']
    assert times and values,'Simulation returned no registered top-level output data'
    assert times[-1]>=expected_end-1e-10,(
        f'Incomplete simulation: returned {times[-1]:.9g}s, requested {expected_end:.9g}s')
    # Installed 4.9 returns one row per signal.
    assert all(len(row)==len(times) for row in values),'Unknown RPC output layout'
    return times,list(map(list,zip(*values)))

def run():
    p=argparse.ArgumentParser();p.add_argument('models',nargs='*',default=['benches/01_DLL_IO.plecs','benches/03_Gate_PWM.plecs','benches/03_Gate_PFM_Windows.plecs','benches/04_PWM_CC_Replay.plecs']);p.add_argument('--port',type=int,default=1180);p.add_argument('--duration',type=float);p.add_argument('--output-step',type=float);p.add_argument('--max-step',type=float);args=p.parse_args()
    # Fast connection check; leave longer simulation timeout to PLECS SolverOpts.
    try:
        with socket.create_connection(('127.0.0.1',args.port),2):pass
    except OSError as e:
        raise SystemExit(f'PLECS RPC unavailable. Enable Preferences > RPC, port {args.port}. '+str(e))
    proxy=xmlrpc.client.ServerProxy(f'http://localhost:{args.port}')
    for name in args.models:
        path=(ROOT/'model'/name).resolve()
        if not path.is_relative_to((ROOT/'model').resolve()):raise ValueError('Generated model directory only')
        try:proxy.plecs.close(path.stem)
        except xmlrpc.client.Fault:pass
        proxy.plecs.load(path.as_posix())
        opts={'SolverOpts':{}}
        if args.duration:opts['SolverOpts']['TimeSpan']=args.duration
        if args.max_step:opts['SolverOpts']['MaxStep']=args.max_step
        if args.output_step:
            assert args.duration,'Specify --duration with --output-step'
            count=int(round(args.duration/args.output_step))
            opts['SolverOpts']['OutputTimes']=[i*args.output_step for i in range(count+1)]
            opts['SolverOpts']['OutputTimesOption']='specified'
        result=proxy.plecs.simulate(path.stem,opts)
        target=ROOT/'results'/f'plecs_{path.stem}.json'
        target.write_text(json.dumps(result),encoding='utf-8')
        target.with_suffix('.options.json').write_text(json.dumps(opts,indent=2),encoding='utf-8')
        from plecs_text import field
        expected_end=args.duration or float(field(path.read_text(encoding='utf-8'),'TimeSpan'))
        times,values=validate_result(result,expected_end)
        if path.stem=='01_DLL_IO':
            assert all(y[19:22]==[11,22,33] for y in values)
            assert all(y[9:12]==[0,0,0] for y in values)
        if path.stem.startswith('03_Gate'):
            assert all(y[0]*y[1]==0 and y[2]*y[3]==0 for y in values)
            assert any(y[0]==1 for y in values) and any(y[1]==1 for y in values)
        print(f'PLECS PASS: {path.stem}, {len(times)} output points -> {target.name}')
if __name__=='__main__':run()

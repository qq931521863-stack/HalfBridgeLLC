# Patch the copied Duty plant into LLC1600_Split_Duty.plecs (ABI-Duty-1).
from pathlib import Path

SRC = Path(r"d:\Work\1600W\代码分析\Visual studio projects\pi_controller\x64\Debug\LLC1600_PWM - DLL.plecs")
DST = Path(r"d:\Work\1600W\代码分析\Visual studio projects\llc_loop\model\LLC1600_Split_Duty.plecs")

EXTRA_COMPONENTS = r'''
        Component {
          Type          Dll
          Name          "SlowDLL"
          Show          on
          Position      [350, 360]
          Direction     right
          Flipped       off
          LabelPosition south
          Parameter {
            Variable      "Filename"
            Value         "llc_slow"
            Show          off
            Evaluate      off
          }
          Parameter {
            Variable      "SampleTime"
            Value         "5e-3"
            Show          off
          }
          Parameter {
            Variable      "OutputDelay"
            Value         "1e-9"
            Show          off
          }
          Parameter {
            Variable      "Parameters"
            Value         "[1, 5e-3]"
            Show          off
          }
        }
        Component {
          Type          SignalMux
          Name          "SlowMux"
          Show          off
          Position      [275, 360]
          Direction     right
          Flipped       off
          LabelPosition south
          Parameter {
            Variable      "Width"
            Value         "10"
            Show          off
          }
        }
        Component {
          Type          SignalDemux
          Name          "SlowDemux"
          Show          off
          Position      [430, 360]
          Direction     right
          Flipped       on
          LabelPosition south
          Parameter {
            Variable      "Width"
            Value         "8"
            Show          off
          }
        }
        Component {
          Type          CScript
          Name          "Fast_snapshot_z1"
          Show          on
          Position      [430, 430]
          Direction     right
          Flipped       off
          LabelPosition south
          Parameter {
            Variable      "DialogGeometry"
            Value         ""
            Show          off
          }
          Parameter {
            Variable      "NumInputs"
            Value         "[1 1]"
            Show          off
          }
          Parameter {
            Variable      "NumOutputs"
            Value         "[1 1]"
            Show          off
          }
          Parameter {
            Variable      "NumContStates"
            Value         "0"
            Show          off
          }
          Parameter {
            Variable      "NumDiscStates"
            Value         "2"
            Show          off
          }
          Parameter {
            Variable      "NumZCSignals"
            Value         "0"
            Show          off
          }
          Parameter {
            Variable      "DirectFeedthrough"
            Value         "0"
            Show          off
          }
          Parameter {
            Variable      "Ts"
            Value         "25e-6"
            Show          off
          }
          Parameter {
            Variable      "TerminalBasedSampleTimes"
            Value         "1"
            Show          off
          }
          Parameter {
            Variable      "Parameters"
            Value         ""
            Show          off
          }
          Parameter {
            Variable      "LangStandard"
            Value         "2"
            Show          off
          }
          Parameter {
            Variable      "GnuExtensions"
            Value         "1"
            Show          off
          }
          Parameter {
            Variable      "RuntimeCheck"
            Value         "2"
            Show          off
          }
          Parameter {
            Variable      "HighlightLevel"
            Value         "0"
            Show          off
          }
          Parameter {
            Variable      "Declarations"
            Value         ""
            Show          off
          }
          Parameter {
            Variable      "StartFcn"
            Value         "DiscState(0)=0;DiscState(1)=0;"
            Show          off
          }
          Parameter {
            Variable      "OutputFcn"
            Value         "OutputSignal(0,0)=DiscState(0);OutputSignal(1,0)=DiscState(1);"
            Show          off
          }
          Parameter {
            Variable      "UpdateFcn"
            Value         "DiscState(0)=InputSignal(0,0);DiscState(1)=InputSignal(1,0);"
            Show          off
          }
          Parameter {
            Variable      "DerivativeFcn"
            Value         ""
            Show          off
          }
          Parameter {
            Variable      "TerminateFcn"
            Value         ""
            Show          off
          }
          Parameter {
            Variable      "StoreCustomStateFcn"
            Value         ""
            Show          off
          }
          Parameter {
            Variable      "RestoreCustomStateFcn"
            Value         ""
            Show          off
          }
        }
        Component {
          Type          Constant
          Name          "Handshake"
          Show          on
          Position      [150, 360]
          Direction     right
          Flipped       off
          LabelPosition south
          Frame         [-10, -10; 10, 10]
          Parameter {
            Variable      "Value"
            Value         "1"
            Show          off
          }
          Parameter {
            Variable      "DataType"
            Value         "10"
            Show          off
          }
        }
        Component {
          Type          Constant
          Name          "CvReq"
          Show          on
          Position      [150, 385]
          Direction     right
          Flipped       off
          LabelPosition south
          Frame         [-10, -10; 10, 10]
          Parameter {
            Variable      "Value"
            Value         "1"
            Show          off
          }
          Parameter {
            Variable      "DataType"
            Value         "10"
            Show          off
          }
        }
        Component {
          Type          Constant
          Name          "Vreq"
          Show          on
          Position      [150, 410]
          Direction     right
          Flipped       off
          LabelPosition south
          Frame         [-10, -10; 10, 10]
          Parameter {
            Variable      "Value"
            Value         "54"
            Show          off
          }
          Parameter {
            Variable      "DataType"
            Value         "10"
            Show          off
          }
        }
        Component {
          Type          Constant
          Name          "Ireq"
          Show          on
          Position      [150, 435]
          Direction     right
          Flipped       off
          LabelPosition south
          Frame         [-10, -10; 10, 10]
          Parameter {
            Variable      "Value"
            Value         "4"
            Show          off
          }
          Parameter {
            Variable      "DataType"
            Value         "10"
            Show          off
          }
        }
        Component {
          Type          Constant
          Name          "PfcOk"
          Show          on
          Position      [150, 270]
          Direction     right
          Flipped       off
          LabelPosition south
          Frame         [-10, -10; 10, 10]
          Parameter {
            Variable      "Value"
            Value         "1"
            Show          off
          }
          Parameter {
            Variable      "DataType"
            Value         "10"
            Show          off
          }
        }
        Component {
          Type          Output
          Name          "Freq"
          Show          on
          Position      [515, 285]
          Direction     right
          Flipped       off
          LabelPosition south
          Parameter {
            Variable      "Index"
            Value         "9"
            Show          on
          }
          Parameter {
            Variable      "Width"
            Value         "-1"
            Show          off
          }
        }
'''

EXTRA_CONNECTIONS = r'''
        Connection {
          Type          Signal
          SrcComponent  "SlowMux"
          SrcTerminal   1
          DstComponent  "SlowDLL"
          DstTerminal   1
        }
        Connection {
          Type          Signal
          SrcComponent  "SlowDLL"
          SrcTerminal   2
          DstComponent  "SlowDemux"
          DstTerminal   1
        }
        Connection {
          Type          Signal
          SrcComponent  "Vout"
          SrcTerminal   1
          DstComponent  "SlowMux"
          DstTerminal   2
        }
        Connection {
          Type          Signal
          SrcComponent  "Vbat"
          SrcTerminal   1
          DstComponent  "SlowMux"
          DstTerminal   3
        }
        Connection {
          Type          Signal
          SrcComponent  "Iout"
          SrcTerminal   1
          DstComponent  "SlowMux"
          DstTerminal   4
        }
        Connection {
          Type          Signal
          SrcComponent  "Fast_snapshot_z1"
          SrcTerminal   3
          DstComponent  "SlowMux"
          DstTerminal   5
        }
        Connection {
          Type          Signal
          SrcComponent  "State"
          SrcTerminal   1
          DstComponent  "SlowMux"
          DstTerminal   7
        }
        Connection {
          Type          Signal
          SrcComponent  "Handshake"
          SrcTerminal   1
          DstComponent  "SlowMux"
          DstTerminal   8
        }
        Connection {
          Type          Signal
          SrcComponent  "CvReq"
          SrcTerminal   1
          DstComponent  "SlowMux"
          DstTerminal   9
        }
        Connection {
          Type          Signal
          SrcComponent  "Vreq"
          SrcTerminal   1
          DstComponent  "SlowMux"
          DstTerminal   10
        }
        Connection {
          Type          Signal
          SrcComponent  "Ireq"
          SrcTerminal   1
          DstComponent  "SlowMux"
          DstTerminal   11
        }
        Connection {
          Type          Signal
          SrcComponent  "SlowDemux"
          SrcTerminal   2
          DstComponent  "Mux"
          DstTerminal   5
        }
        Connection {
          Type          Signal
          SrcComponent  "SlowDemux"
          SrcTerminal   3
          DstComponent  "Mux"
          DstTerminal   6
        }
        Connection {
          Type          Signal
          SrcComponent  "SlowDemux"
          SrcTerminal   4
          DstComponent  "Mux"
          DstTerminal   7
        }
        Connection {
          Type          Signal
          SrcComponent  "SlowDemux"
          SrcTerminal   5
          DstComponent  "Mux"
          DstTerminal   8
        }
        Connection {
          Type          Signal
          SrcComponent  "SlowDemux"
          SrcTerminal   6
          DstComponent  "Mux"
          DstTerminal   9
        }
        Connection {
          Type          Signal
          SrcComponent  "SlowDemux"
          SrcTerminal   7
          DstComponent  "Mux"
          DstTerminal   10
        }
        Connection {
          Type          Signal
          SrcComponent  "PfcOk"
          SrcTerminal   1
          DstComponent  "Mux"
          DstTerminal   11
        }
        Connection {
          Type          Signal
          SrcComponent  "Demux"
          SrcTerminal   6
          DstComponent  "Freq"
          DstTerminal   1
        }
        Connection {
          Type          Signal
          SrcComponent  "Demux"
          SrcTerminal   7
          DstComponent  "Fast_snapshot_z1"
          DstTerminal   1
        }
        Connection {
          Type          Signal
          SrcComponent  "Demux"
          SrcTerminal   8
          DstComponent  "Fast_snapshot_z1"
          DstTerminal   2
        }
'''

CARRIER = r'''
        Component {
          Type          CScript
          Name          "Carrier"
          Show          on
          Position      [230, 360]
          Direction     right
          Flipped       off
          LabelPosition south
          Parameter {
            Variable      "DialogGeometry"
            Value         ""
            Show          off
          }
          Parameter {
            Variable      "NumInputs"
            Value         "1"
            Show          off
          }
          Parameter {
            Variable      "NumOutputs"
            Value         "1"
            Show          off
          }
          Parameter {
            Variable      "NumContStates"
            Value         "1"
            Show          off
          }
          Parameter {
            Variable      "NumDiscStates"
            Value         "0"
            Show          off
          }
          Parameter {
            Variable      "NumZCSignals"
            Value         "0"
            Show          off
          }
          Parameter {
            Variable      "DirectFeedthrough"
            Value         "1"
            Show          off
          }
          Parameter {
            Variable      "Ts"
            Value         "-1"
            Show          off
          }
          Parameter {
            Variable      "TerminalBasedSampleTimes"
            Value         "1"
            Show          off
          }
          Parameter {
            Variable      "Parameters"
            Value         ""
            Show          off
          }
          Parameter {
            Variable      "LangStandard"
            Value         "2"
            Show          off
          }
          Parameter {
            Variable      "GnuExtensions"
            Value         "1"
            Show          off
          }
          Parameter {
            Variable      "RuntimeCheck"
            Value         "2"
            Show          off
          }
          Parameter {
            Variable      "HighlightLevel"
            Value         "0"
            Show          off
          }
          Parameter {
            Variable      "Declarations"
            Value         ""
            Show          off
          }
          Parameter {
            Variable      "StartFcn"
            Value         "ContState(0)=0;"
            Show          off
          }
          Parameter {
            Variable      "OutputFcn"
            Value         "double p=ContState(0);while(p>=1)p-=1;while(p<0)p+=1;OutputSignal(0,0)=(p<0.5)?(2*p):(2-2*p);"
            Show          off
          }
          Parameter {
            Variable      "UpdateFcn"
            Value         ""
            Show          off
          }
          Parameter {
            Variable      "DerivativeFcn"
            Value         "double f=InputSignal(0,0);if(!(f==f)||f<1e3)f=8e4;if(f>3e5)f=3e5;ContDerivative(0)=f;"
            Show          off
          }
          Parameter {
            Variable      "TerminateFcn"
            Value         ""
            Show          off
          }
          Parameter {
            Variable      "StoreCustomStateFcn"
            Value         ""
            Show          off
          }
          Parameter {
            Variable      "RestoreCustomStateFcn"
            Value         ""
            Show          off
          }
        }
        Component {
          Type          Input
          Name          "Freq"
          Show          on
          Position      [150, 360]
          Direction     right
          Flipped       off
          LabelPosition south
          Parameter {
            Variable      "Index"
            Value         "6"
            Show          on
          }
          Parameter {
            Variable      "Width"
            Value         "-1"
            Show          off
          }
        }
'''

text = SRC.read_text(encoding="utf-8")
text = text.replace('Name          "LLC1600_PWM - DLL"', 'Name          "LLC1600_Split_Duty"')
text = text.replace('Value         "pi_controller"', 'Value         "llc_fast"')
text = text.replace(
    '''          Parameter {
            Variable      "Parameters"
            Value         "[]"
            Show          off
          }
        }
        Component {
          Type          SignalMux
          Name          "Mux"''',
    '''          Parameter {
            Variable      "Parameters"
            Value         "[1, 25e-6]"
            Show          off
          }
        }
        Component {
          Type          SignalMux
          Name          "Mux"''')
text = text.replace(
    '''          Name          "Mux"
          Show          off
          Position      [275, 185]
          Direction     right
          Flipped       off
          LabelPosition south
          Parameter {
            Variable      "Width"
            Value         "4"''',
    '''          Name          "Mux"
          Show          off
          Position      [275, 185]
          Direction     right
          Flipped       off
          LabelPosition south
          Parameter {
            Variable      "Width"
            Value         "10"''')
text = text.replace(
    '''          Name          "Demux"
          Show          off
          Position      [430, 185]
          Direction     right
          Flipped       on
          LabelPosition south
          Parameter {
            Variable      "Width"
            Value         "4"''',
    '''          Name          "Demux"
          Show          off
          Position      [430, 185]
          Direction     right
          Flipped       on
          LabelPosition south
          Parameter {
            Variable      "Width"
            Value         "8"''')

# ABI order: V_RLY, V_BAT, I_BAT
text = text.replace(
    '''          SrcComponent  "Iout"
          SrcTerminal   1
          DstComponent  "Mux"
          DstTerminal   3''',
    '''          SrcComponent  "Vbat"
          SrcTerminal   1
          DstComponent  "Mux"
          DstTerminal   3''')
text = text.replace(
    '''          SrcComponent  "Vbat"
          SrcTerminal   1
          Points        [245, 230; 245, 190]
          DstComponent  "Mux"
          DstTerminal   4''',
    '''          SrcComponent  "Iout"
          SrcTerminal   1
          Points        [245, 180; 245, 190]
          DstComponent  "Mux"
          DstTerminal   4''')
# State no longer feeds Fast mux; Slow uses it as charge_enable.
text = text.replace(
    '''        Connection {
          Type          Signal
          SrcComponent  "State"
          SrcTerminal   1
          Points        [250, 275; 250, 200]
          DstComponent  "Mux"
          DstTerminal   5
        }''',
    '''        Connection {
          Type          Signal
          SrcComponent  "Fast_snapshot_z1"
          SrcTerminal   4
          DstComponent  "SlowMux"
          DstTerminal   6
        }''')

marker = '''        Connection {
          Type          Signal
          SrcComponent  "Mux"
          SrcTerminal   1
          DstComponent  "DLL"
          DstTerminal   1
        }'''
if marker not in text:
    raise SystemExit("mux connection marker not found")
text = text.replace(marker, EXTRA_COMPONENTS + "\n" + marker + "\n" + EXTRA_CONNECTIONS, 1)

# Subsystem extra Freq output terminal
text = text.replace(
    '''      Terminal {
        Type          Output
        Position      [79, 35]
        Direction     right
      }
      Schematic {''',
    '''      Terminal {
        Type          Output
        Position      [79, 35]
        Direction     right
      }
      Terminal {
        Type          Output
        Position      [79, 50]
        Direction     right
      }
      Schematic {''')

# PWM carrier: replace triangle generator feed
text = text.replace(
    '''          SrcComponent  "LEAD7"
          SrcTerminal   1
          Points        [290, 315; 290, 260]
          DstComponent  "RE1"
          DstTerminal   2''',
    '''          SrcComponent  "Carrier"
          SrcTerminal   2
          Points        [290, 315; 290, 260]
          DstComponent  "RE1"
          DstTerminal   2''')
pwm_marker = '''        Component {
          Type          TriangleGenerator
          Name          "LEAD7"'''
if pwm_marker not in text:
    raise SystemExit("triangle generator not found")
text = text.replace(pwm_marker, CARRIER + "\n" + pwm_marker, 1)

freq_conn = r'''
        Connection {
          Type          Signal
          SrcComponent  "Freq"
          SrcTerminal   1
          DstComponent  "Carrier"
          DstTerminal   1
        }
'''
text = text.replace(
    '''        Connection {
          Type          Signal
          SrcComponent  "Duty"
          SrcTerminal   1
          DstComponent  "RE1"
          DstTerminal   1
        }''',
    freq_conn + '''        Connection {
          Type          Signal
          SrcComponent  "Duty"
          SrcTerminal   1
          DstComponent  "RE1"
          DstTerminal   1
        }''')

# Top-level Freq from Subsystem to PWM control
text = text.replace(
    '''    Connection {
      Type          Signal
      SrcComponent  "From"
      SrcTerminal   1
      DstComponent  "PWM control"
      DstTerminal   1
    }''',
    '''    Component {
      Type          Goto
      Name          "GotoFreq"
      Show          off
      Position      [770, 1180]
      Direction     right
      Flipped       off
      LabelPosition south
      Parameter {
        Variable      "Tag"
        Value         "FREQ"
        Show          off
      }
      Parameter {
        Variable      "Visibility"
        Value         "1"
        Show          off
      }
      Parameter {
        Variable      "NoMatchingCounterpartAction"
        Value         "2"
        Show          off
      }
    }
    Component {
      Type          From
      Name          "FromFreq"
      Show          off
      Position      [920, 1180]
      Direction     right
      Flipped       off
      LabelPosition south
      Parameter {
        Variable      "Tag"
        Value         "FREQ"
        Show          off
      }
      Parameter {
        Variable      "Visibility"
        Value         "1"
        Show          off
      }
      Parameter {
        Variable      "NoMatchingCounterpartAction"
        Value         "1"
        Show          off
      }
    }
    Connection {
      Type          Signal
      SrcComponent  "From"
      SrcTerminal   1
      DstComponent  "PWM control"
      DstTerminal   1
    }
    Connection {
      Type          Signal
      SrcComponent  "Subsystem"
      SrcTerminal   9
      DstComponent  "GotoFreq"
      DstTerminal   1
    }
    Connection {
      Type          Signal
      SrcComponent  "FromFreq"
      SrcTerminal   1
      DstComponent  "PWM control"
      DstTerminal   6
    }''')

# PWM control extra terminal
text = text.replace(
    '''      Terminal {
        Type          Output
        Position      [74, 10]
        Direction     right
      }
      Schematic {
        Location      [0, 24; 1536, 871]
        ZoomFactor    1.30957''',
    '''      Terminal {
        Type          Output
        Position      [74, 10]
        Direction     right
      }
      Terminal {
        Type          Input
        Position      [-70, 35]
        Direction     left
      }
      Schematic {
        Location      [0, 24; 1536, 871]
        ZoomFactor    1.30957''')

DST.parent.mkdir(parents=True, exist_ok=True)
DST.write_text(text, encoding="utf-8")
print("wrote", DST)
print("llc_fast" in text, "SlowDLL" in text, "Carrier" in text, "[1, 25e-6]" in text)

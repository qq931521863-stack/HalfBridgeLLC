"""Minimal lossless balanced-block access for the PLECS text format.

Quoted strings may span concatenated lines and contain braces. Positions refer
to original text; mutations are applied from the end to retain all other data.
"""
import re,json
def blocks(text,kind=None):
    stack=[]; quoted=False; escaped=False; result=[]
    for i,c in enumerate(text):
        if quoted:
            if escaped: escaped=False
            elif c=='\\': escaped=True
            elif c=='"': quoted=False
        elif c=='"': quoted=True
        elif c=='{':
            end=i
            while end and text[end-1].isspace():end-=1
            start=end
            while start and (text[start-1].isalnum() or text[start-1]=='_'):start-=1
            stack.append((text[start:end],start,i,len(stack)))
        elif c=='}':
            if not stack: raise ValueError('Unbalanced PLECS file')
            typ,start,op,depth=stack.pop()
            if kind is None or kind==typ: result.append((start,i+1,depth,typ))
    if stack or quoted: raise ValueError('Unclosed PLECS block/string')
    return sorted(result)
def field(text,key):
    m=re.search(r'(?m)^\s*'+re.escape(key)+r'\s+("(?:[^"\\]|\\.)*"(?:\s*\n"(?:[^"\\]|\\.)*")*|[^\r\n]+)',text)
    if not m: return None
    value=m.group(1)
    if value.startswith('"'): return ''.join(json.loads(x) for x in re.findall(r'"(?:[^"\\]|\\.)*"',value))
    return value.strip()
def set_field(text,key,value):
    p=r'(?m)^(\s*'+re.escape(key)+r'\s+)("(?:[^"\\]|\\.)*"(?:\s*\n"(?:[^"\\]|\\.)*")*|[^\r\n]+)'
    text,n=re.subn(p,lambda m:m.group(1)+json.dumps(str(value),ensure_ascii=False),text,count=1)
    if n!=1: raise ValueError('Missing field '+key)
    return text
def set_param(text,key,value):
    for a,b,_,_ in blocks(text,'Parameter'):
        if field(text[a:b],'Variable')==key:
            return text[:a]+set_field(text[a:b],'Value',value)+text[b:]
    raise ValueError('Missing parameter '+key)
def named(text,name,depth=None):
    matches=[(a,b) for a,b,d,_ in blocks(text,'Component') if (depth is None or d==depth) and field(text[a:b],'Name')==name]
    if len(matches)!=1: raise ValueError(f'{name}: {len(matches)} matches')
    return matches[0]
def edit_named(text,name,fn,depth=None):
    a,b=named(text,name,depth); return text[:a]+fn(text[a:b])+text[b:]
def component(typ,name,xy,params=None,extra=''):
    s=f'Component {{\n Type {typ}\n Name {json.dumps(name)}\n Show on\n Position [{xy[0]}, {xy[1]}]\n Direction right\n Flipped off\n LabelPosition south\n'
    for k,v in (params or {}).items(): s+=' Parameter {\n Variable '+json.dumps(k)+'\n Value '+json.dumps(str(v),ensure_ascii=False)+'\n Show off\n }\n'
    return s+extra+'\n}\n'
def wire(src,st,dst,dt,typ='Signal'):
    return f'Connection {{\n Type {typ}\n SrcComponent "{src}"\n SrcTerminal {st}\n DstComponent "{dst}"\n DstTerminal {dt}\n}}\n'

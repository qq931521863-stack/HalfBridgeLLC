"""Extract installed vendor documentation, without touching settings/license files."""
import sqlite3, zlib, re, sys
from pathlib import Path
db=sqlite3.connect('file:D:/Plexim/PLECS 4.9 (64 bit)/onlinehelp/plecshelp.qch?mode=ro',uri=True)
for name, data in db.execute('SELECT Name,Data FROM FileNameTable JOIN FileDataTable ON FileNameTable.FileId=FileDataTable.Id'):
    try: s=zlib.decompress(data[4:]).decode('utf-8')
    except Exception: continue
    if any(re.search(term,s,re.I) for term in sys.argv[1:]):
        plain=re.sub('<[^>]+>',' ',s)
        plain=re.sub(r'\s+',' ',plain)
        print(name, plain[:24000], '\n')

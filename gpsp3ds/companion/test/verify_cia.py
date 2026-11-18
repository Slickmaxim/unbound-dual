#!/usr/bin/env python3
"""Check CIA hashes, unique IDs, SMDH labels and compiled launch routing."""
import hashlib,struct,sys
from pathlib import Path
SIG={0x10000:0x240,0x10001:0x140,0x10002:0x80,0x10003:0x240,0x10004:0x140,0x10005:0x80}
def align(x):return (x+63)&~63
def sha(x):return hashlib.sha256(x).digest()
def verify(path,title_id,label,shortcut):
 d=Path(path).read_bytes();hdr,_,_,cert,tick,tmd,meta,content=struct.unpack_from('<IHHIIIIQ',d)
 ot=align(align(hdr)+cert);om=align(ot+tick);oc=align(om+tmd);omet=align(oc+content)
 tm=om+SIG[struct.unpack_from('>I',d,om)[0]];tk=ot+SIG[struct.unpack_from('>I',d,ot)[0]]
 info=tm+0xC4;chunks=info+0x900;count=struct.unpack_from('>H',d,info+2)[0]
 assert count==1
 assert struct.unpack_from('>H',d,tm+0x9C)[0]==13, "CIA version must be 0.0.13"
 size=struct.unpack_from('>Q',d,chunks+8)[0];ncch=d[oc:oc+size]
 assert sha(ncch)==d[chunks+16:chunks+48]
 assert sha(d[chunks:chunks+count*48])==d[info+4:info+36]
 assert sha(d[info:info+0x900])==d[tm+0xA4:tm+0xC4]
 assert ncch[0x100:0x104]==b'NCCH' and ncch[0x18F]&4
 ids=[struct.unpack_from('>Q',d,tk+0x9C)[0],struct.unpack_from('>Q',d,tm+0x4C)[0]]
 ids += [struct.unpack_from('<Q',ncch,x)[0] for x in (0x108,0x118,0x400)]
 assert all(i==title_id for i in ids),ids
 assert sha(ncch[0x200:0x600])==ncch[0x160:0x180]
 eo,es,eh=[x*512 for x in struct.unpack_from('<III',ncch,0x1A0)];exefs=ncch[eo:eo+es]
 assert sha(exefs[:eh])==ncch[0x1C0:0x1E0]
 files={}
 for i in range(10):
  name,off,n=struct.unpack_from('<8sII',exefs,i*16);name=name.rstrip(b'\0')
  if not name:continue
  b=exefs[512+off:512+off+n];assert sha(b)==exefs[512-(i+1)*32:512-i*32];files[name]=b
 smdh=files[b'icon'];assert smdh[:4]==b'SMDH'
 assert smdh[8:0x88].decode('utf-16le').rstrip('\0')==label
 if meta>=0x400+0x36C0:assert d[omet+0x400:omet+0x400+0x36C0]==smdh
 code=files[b'.code'];assert b'https://github.com/Slickmaxim/unbound-dual/releases/latest' in code;assert b'companion-launch.txt' in code and b'@companion' in code
 if shortcut:assert struct.pack('<Q',0x000400000BAC8200) in code
 else:assert b'companion-gpsp3d.cfg' in code and b'companion_rom' in code
 assert files[b'banner'][:4]==b'CBMD'
 print(f'{Path(path).name}: SHA-256/TMD/ExeFS, title {title_id:016X}, label and launch routing PASS')
root=Path(sys.argv[1])
verify(root/'unbound-dual.cia',0x000400000BAC8200,'Unbound Dual',False)

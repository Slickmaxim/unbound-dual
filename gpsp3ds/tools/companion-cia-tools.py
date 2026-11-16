#!/usr/bin/env python3
"""Fetch pinned official Linux CIA tools into an ignored local directory."""
import hashlib,io,json,os,urllib.request,zipfile
from pathlib import Path
root=Path(__file__).resolve().parents[1]/'.cia-tools'
root.mkdir(exist_ok=True)
for name,repo,tag in [('makerom','3DSGuy/Project_CTR','makerom-v0.18.4'),('bannertool','Epicpkmn11/bannertool','v1.2.2')]:
 dest=root/name
 if dest.exists() and dest.read_bytes()[:5]==b"\x7fELF\x02":continue
 req=urllib.request.Request(f'https://api.github.com/repos/{repo}/releases/tags/{tag}',headers={'User-Agent':'gpsp-companion-build'})
 with urllib.request.urlopen(req,timeout=30) as f:release=json.load(f)
 assets=[a for a in release['assets'] if a['name'].endswith('.zip')]
 linux=[a for a in assets if ('linux' in a['name'].lower() or 'ubuntu' in a['name'].lower()) and ('64' in a['name'])]
 asset=(linux or assets)[0]
 with urllib.request.urlopen(asset['browser_download_url'],timeout=60) as f:data=f.read()
 with zipfile.ZipFile(io.BytesIO(data)) as z:
  candidates=[p for p in z.namelist() if p.split('/')[-1]==name and ('linux' in p.lower() or len(p.split('/'))==1)]
  if not candidates:raise RuntimeError(f'No Linux {name} in {asset["name"]}')
  binaries=[z.read(p) for p in candidates]
  binary=next((b for b in binaries if b[:5]==b"\x7fELF\x02"),None)
  if binary is None:raise RuntimeError(f'No 64-bit Linux {name}: {candidates}')
  dest.write_bytes(binary);dest.chmod(0o755)
 (root/f'{name}.json').write_text(json.dumps({'url':asset['browser_download_url'],'tag':tag,'archive_sha256':hashlib.sha256(data).hexdigest()},indent=2)+'\n')
 print(f'Fetched {name} {tag}')

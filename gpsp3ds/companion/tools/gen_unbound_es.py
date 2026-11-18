#!/usr/bin/env python3
"""Regenerate ES v1.08 display tables from the user's ROM, never download a game.
GPL-3.0; based on PokeDaisy's Gen 3/CFRU table formats (see vendor/NOTICE).
"""
import argparse,hashlib,json,struct
from pathlib import Path
SHA='d5b312465c0a95bcb6428bd61cc170994a2a65fc'
B=0x08000000
CH={0:' ',0x2d:'&',0x2e:'+',0x35:'=',0x36:';',0x51:'¿',0x52:'¡',0x5a:'Í',0x5b:'%',0x5c:'(',0x5d:')',0x6f:'í',0x77:'',0xab:'!',0xac:'?',0xad:'.',0xae:'-',0xaf:'·',0xb0:'…',0xb1:'“',0xb2:'”',0xb3:'‘',0xb4:"'",0xb5:'♂',0xb6:'♀',0xb7:'¥',0xb8:',',0xb9:'×',0xba:'/',0xf0:':'}
CH.update({1+i:c for i,c in enumerate('ÀÁÂÇÈÉÊËÌ ÎÏÒÓÔŒÙÚÛÑßàá çèéêëì îïòóôœùúûñºª') if c!=' '})
CH.update({0xbb+i:chr(65+i) for i in range(26)})
CH.update({0xd5+i:chr(97+i) for i in range(26)})
CH.update({0xa1+i:str(i) for i in range(10)})
CH.update({0xf1+i:c for i,c in enumerate('ÄÖÜäöü')})
def generate(rom):
 if hashlib.sha1(rom).hexdigest()!=SHA:raise ValueError('Requires exact Unbound ES v1.08 SHA-1 '+SHA)
 def u16(a):return struct.unpack_from('<H',rom,a-B)[0]
 def u32(a):return struct.unpack_from('<I',rom,a-B)[0]
 def text(a,n):
  if not B<=a<B+len(rom) or a+n>B+len(rom):raise ValueError('Text pointer out of ROM')
  out=[]
  for c in rom[a-B:a-B+n]:
   if c==255:break
   if c not in CH:raise ValueError('Unknown text byte %02x at %08x'%(c,a))
   out.append(CH[c])
  return ''.join(out).strip()
 species={i:text(0x0964c778+i*11,11) for i in range(1,1314)}
 items={}
 for i in range(1,767):
  a=0x095fbc90+i*44;ptr=u32(a);name=text(ptr,32) if B<=ptr<B+len(rom) else text(a,14)
  iid=u16(a+14)
  if name and not name.startswith('????') and iid not in items:items[iid]=name
 moves={i:(text(0x08a40a10+i*13,13),rom[0xa769af+i*12+2],rom[0xa769af+i*12+1]) for i in range(1,923)}
 secs={0x58+i:text(u32(0x083f1cac+i*4),48) for i in range(109)}
 natures=[text(u32(0x08463e60+i*4),24) for i in range(25)]
 # Active ES chart includes Fairy and the translation/mod's extra type slot.
 # The FE separator marks Foresight immunities, not the end of the table.
 chart=[];at=0x009619C0
 while rom[at]!=0xFF:
  atk,defender,mult=rom[at:at+3];at+=3
  if atk==0xFE:continue
  assert atk<24 and defender<24 and mult in (0,5,10,20)
  chart.append((atk,defender,mult*10))
 assert len(chart)==131 and (16,23,0) in chart and (23,16,200) in chart
 types=[text(B+at+4+i*7,7) for i in range(24)]
 assert types[0]=='Normal' and types[23]=='Fairy'

 assert species[496]=='Gible' and moves[33][0]=='Placaje' and moves[45][0]=='Gruñido'
 assert items[13]=='Poción' and items[729]=='Dragonitita' and items[764]=='Crabominablita'
 assert secs[0x59]=='Pueblo Bellin' and natures[0]=='Fuerte'
 q=lambda s:json.dumps(s,ensure_ascii=False)
 lines=['// Generated from the user-provided Unbound ES v1.08 ROM by tools/gen_unbound_es.py.', '// SHA-1: '+SHA+'; display text/metadata only, no graphics or ROM binary.', '#include "pd_tables.h"']
 for name,values,count in [('es_species',species,1314),('es_items',items,767),('es_mapsecs',secs,197)]:
  lines += [f'static const char* const {name}[{count}] = {{']+[f' [{i}] = {q(s)},' for i,s in values.items()]+['};']
 lines+=['static const struct pd_move_info es_moves[923] = {']+[f' [{i}] = {{ {q(n)}, {t}, {p} }},' for i,(n,t,p) in moves.items()]+['};']
 lines+=['static const uint8_t es_gender[1314] = {']+[f' {rom[0x163bc90+i*28+16]},' for i in range(1314)]+['};']
 lines+=['static const uint8_t es_types[1314][2] = {']+[f' {{ {rom[0x163bc90+i*28+6]}, {rom[0x163bc90+i*28+7]} }},' for i in range(1314)]+['};']
 lines+=['const char* const pd_unbound_es_natures[25] = {']+[' '+q(s)+',' for s in natures]+['};']
 lines+=['static const struct pd_type_pair es_chart[] = {']+[f' {{ {a}, {b}, {v} }},' for a,b,v in chart]+['};']
 lines+=['static const char* const es_type_names[24] = {']+[' '+q(s)+',' for s in types]+['};']
 lines+=['const struct pd_text pd_text_unbound_es = {','.species=es_species,.speciesCount=1314,.items=es_items,.itemsCount=767,','.moves=es_moves,.movesCount=923,.speciesTypes=es_types,.speciesTypesCount=1314,',
 '.typeNames=es_type_names,.typeNamesCount=24,.typeChart=es_chart,.typeChartCount=131,','.genderRatios=es_gender,.genderRatiosCount=1314,.mapsecs=es_mapsecs,.mapsecsCount=197,.mapsecsRom=true,','};']
 return '\n'.join(lines)+'\n'
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('rom',type=Path);p.add_argument('-o',type=Path,default=Path(__file__).resolve().parents[1]/'vendor/pd_unbound_es_gen.c');a=p.parse_args();a.o.write_text(generate(a.rom.read_bytes()));print(a.o)

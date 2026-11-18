#!/usr/bin/env python3
"""Regenerate supported Spanish display tables from the user's ROM, never download a game.
GPL-3.0; based on PokeDaisy's Gen 3/CFRU table formats (see vendor/NOTICE).
"""
import argparse,hashlib,json,struct
from pathlib import Path
# Exact identities only. Addresses were checked against live code references,
# not just the legacy tables which the Z-A patch leaves behind.
PROFILES={
 'd5b312465c0a95bcb6428bd61cc170994a2a65fc': ('es', 'ES v1.08', 0x0964C778, 1314, 0x095FBC90, 767, 0x0963BC90),
 '117449bd7822fad0db293a4f7e364787e4f8543a': ('es_sinza', 'ES v1.08 sinza', 0x0966A98C, 1294, 0x08876200, 729, 0x099E0C9C),
 '1104054a3061b35b85b04757fbca5a3686cca60c': ('es_2111', 'ES 2.1.1.1', 0x0966A98C, 1294, 0x08876200, 729, 0x099E0C9C),
}
B=0x08000000
CH={0:' ',0x2d:'&',0x2e:'+',0x35:'=',0x36:';',0x51:'¿',0x52:'¡',0x5a:'Í',0x5b:'%',0x5c:'(',0x5d:')',0x6f:'í',0x77:'',0xab:'!',0xac:'?',0xad:'.',0xae:'-',0xaf:'·',0xb0:'…',0xb1:'“',0xb2:'”',0xb3:'‘',0xb4:"'",0xb5:'♂',0xb6:'♀',0xb7:'¥',0xb8:',',0xb9:'×',0xba:'/',0xf0:':'}
CH.update({1+i:c for i,c in enumerate('ÀÁÂÇÈÉÊËÌ ÎÏÒÓÔŒÙÚÛÑßàá çèéêëì îïòóôœùúûñºª') if c!=' '})
CH.update({0xbb+i:chr(65+i) for i in range(26)})
CH.update({0xd5+i:chr(97+i) for i in range(26)})
CH.update({0xa1+i:str(i) for i in range(10)})
CH.update({0xf1+i:c for i,c in enumerate('ÄÖÜäöü')})
def generate(rom):
 sha=hashlib.sha1(rom).hexdigest()
 if sha not in PROFILES:raise ValueError('Unknown Spanish ROM SHA-1 '+sha)
 tag,label,species_addr,species_count,item_addr,item_count,stats_addr=PROFILES[sha]
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
 species={i:text(species_addr+i*11,11) for i in range(1,species_count)}
 items={}
 for i in range(1,item_count):
  a=item_addr+i*44;ptr=u32(a);name=text(ptr,32) if B<=ptr<B+len(rom) else text(a,14)
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
 assert items[13]=='Poción' and items[728]=='Roca Hisui'
 if tag=='es':assert items[729]=='Dragonitita' and items[764]=='Crabominablita'
 assert secs[0x59]==('Ciudad Bellin' if tag=='es_2111' else 'Pueblo Bellin') and natures[0]=='Fuerte'
 q=lambda s:json.dumps(s,ensure_ascii=False)
 lines=['// Generated from user-provided Unbound '+label+' by tools/gen_unbound_es.py.', '// SHA-1: '+sha+'; display text/metadata only, no graphics or ROM binary.', '#include "pd_tables.h"']
 for name,values,count in [('es_species',species,species_count),('es_items',items,item_count),('es_mapsecs',secs,197)]:
  lines += [f'static const char* const {name}[{count}] = {{']+[f' [{i}] = {q(s)},' for i,s in values.items()]+['};']
 lines+=['static const struct pd_move_info es_moves[923] = {']+[f' [{i}] = {{ {q(n)}, {t}, {p} }},' for i,(n,t,p) in moves.items()]+['};']
 lines+=[f'static const uint8_t es_gender[{species_count}] = {{']+[f' {rom[stats_addr-B+i*28+16]},' for i in range(species_count)]+['};']
 lines+=[f'static const uint8_t es_types[{species_count}][2] = {{']+[f' {{ {rom[stats_addr-B+i*28+6]}, {rom[stats_addr-B+i*28+7]} }},' for i in range(species_count)]+['};']
 lines+=['static const char* const es_natures[25] = {']+[' '+q(s)+',' for s in natures]+['};']
 lines+=['static const struct pd_type_pair es_chart[] = {']+[f' {{ {a}, {b}, {v} }},' for a,b,v in chart]+['};']
 lines+=['static const char* const es_type_names[24] = {']+[' '+q(s)+',' for s in types]+['};']
 lines += [f'const struct pd_text pd_text_unbound_{tag} = {{',
 f'.species=es_species,.speciesCount={species_count},.items=es_items,.itemsCount={item_count},',
 '.moves=es_moves,.movesCount=923,.speciesTypes=es_types,',
 f'.speciesTypesCount={species_count},.genderRatios=es_gender,.genderRatiosCount={species_count},',
 '.typeNames=es_type_names,.typeNamesCount=24,.typeChart=es_chart,.typeChartCount=131,',
 '.mapsecs=es_mapsecs,.mapsecsCount=197,.mapsecsRom=true,.natures=es_natures,','};']
 return '\n'.join(lines)+'\n'
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('rom',type=Path);p.add_argument('-o',type=Path)
 a=p.parse_args();rom=a.rom.read_bytes();result=generate(rom)
 tag=PROFILES[hashlib.sha1(rom).hexdigest()][0]
 out=a.o or Path(__file__).resolve().parents[1]/f'vendor/pd_unbound_{tag}_gen.c'
 out.write_text(result);print(out)

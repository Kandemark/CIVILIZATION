#!/usr/bin/env python3
from pathlib import Path
import re,sys
cm=Path('CMakeLists.txt').read_text()
listed=set(re.findall(r"\bsrc/[\w/.-]+\.c\b", cm))
allsrc=set(str(p).replace('\\','/') for p in Path('src').rglob('*.c'))
# intentionally excluded legacy/duplicate module
excluded={
    'src/core/world.c',
    'src/core/diplomacy/international_orgs.c',
}
missing=sorted(allsrc-listed-excluded)
extra=sorted(listed-allsrc)
if missing or extra:
    if missing:
        print('Missing from CMakeLists.txt:')
        for m in missing: print('  -',m)
    if extra:
        print('Nonexistent entries in CMakeLists.txt:')
        for e in extra: print('  -',e)
    sys.exit(1)
print('Source coverage check passed.')
print('Excluded legacy modules:')
for e in sorted(excluded): print('  -',e)

from pathlib import Path
import struct, zlib
root=Path(__file__).resolve().parent
src=bytearray((root/'out/ex015-a.qbc').read_bytes())

def fix_crc(b):
    struct.pack_into('<I',b,20,zlib.crc32(b[24:]) & 0xffffffff)
    return b

def instr_start(b):
    nc=struct.unpack_from('<I',b,8)[0]
    at=24
    for _ in range(nc):
        ln=struct.unpack_from('<I',b,at+4)[0]
        at += 8 + ln
    return at

b=bytearray(src); b[0:4]=b'BAD!'; (root/'out/bad-magic.qbc').write_bytes(b)
b=bytearray(src); b[5]=9; (root/'out/bad-version.qbc').write_bytes(b)
b=bytearray(src); b[-1]^=1; (root/'out/bad-crc.qbc').write_bytes(b)

b=bytearray(src[:-1]); fix_crc(b); (root/'out/truncated.qbc').write_bytes(b)

at=instr_start(src)
b=bytearray(src); struct.pack_into('<H',b,at,0xffff); fix_crc(b); (root/'out/unknown-op.qbc').write_bytes(b)

# Find the first CONST in a program that actually has a constant pool.
b=bytearray((root/'out/const-base.qbc').read_bytes())
at=instr_start(b); ni=struct.unpack_from('<I',b,12)[0]
found=False
for _ in range(ni):
    op,nops,flags=struct.unpack_from('<HBB',b,at)
    if op==1 and nops==2:
        struct.pack_into('<I',b,at+8,0xffffffff)
        found=True
        break
    at += 4 + 4*nops
if not found:
    raise SystemExit('CONST instruction not found')
fix_crc(b); (root/'out/bad-const.qbc').write_bytes(b)

#!/usr/bin/env python3
"""Rebuild data/binary/fwd_gc_logo.bin and fwd_gc_bg.bin (TPL, RGB5A3) from the loader's
data/images/gc_icon_bg.png and gc_banner_bg.png. GameCube shortcut channels put them into
the channel banner (icon logo and banner background). The TPL header and texture filters
are those of HBLogo.tpl in data/binary/custom_banner.bnr."""
import hashlib
import os
import struct
import sys
import zlib

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..")


def png_rgba(data):
    o, idat, w, h = 8, b"", 0, 0
    while o < len(data):
        n, kind = struct.unpack_from(">I4s", data, o)
        body = data[o + 8:o + 8 + n]
        if kind == b"IHDR":
            w, h, depth, ctype, _c, _f, inter = struct.unpack(">IIBBBBB", body)
            assert depth == 8 and ctype == 6 and inter == 0, "8-bit RGBA, not interlaced"
        elif kind == b"IDAT":
            idat += body
        o += 12 + n
    raw, stride, out, prev = zlib.decompress(idat), w * 4, bytearray(), bytearray(w * 4)
    for y in range(h):
        f = raw[y * (stride + 1)]
        line = bytearray(raw[y * (stride + 1) + 1:(y + 1) * (stride + 1)])
        for i in range(stride):
            a, b, c = (line[i - 4] if i >= 4 else 0), prev[i], (prev[i - 4] if i >= 4 else 0)
            if f == 1:
                line[i] = (line[i] + a) & 0xFF
            elif f == 2:
                line[i] = (line[i] + b) & 0xFF
            elif f == 3:
                line[i] = (line[i] + (a + b) // 2) & 0xFF
            elif f == 4:
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                line[i] = (line[i] + (a if pa <= pb and pa <= pc else b if pb <= pc else c)) & 0xFF
        out += line
        prev = line
    return w, h, bytes(out)


def rgb5a3(w, h, rgba):
    out = bytearray()
    for ty in range(0, h, 4):
        for tx in range(0, w, 4):
            for y in range(ty, ty + 4):
                for x in range(tx, tx + 4):
                    r, g, b, a = rgba[(y * w + x) * 4:(y * w + x) * 4 + 4]
                    v = (0x8000 | (r >> 3) << 10 | (g >> 3) << 5 | b >> 3) if a >= 0xE0 else \
                        ((a >> 5) << 12 | (r >> 4) << 8 | (g >> 4) << 4 | b >> 4)
                    out += struct.pack(">H", v)
    return bytes(out)


def hblogo_tpl_header():
    """First 0x40 bytes of arc/timg/HBLogo.tpl inside custom_banner.bnr (meta/icon.bin)."""
    bnr = open(os.path.join(ROOT, "data/binary/custom_banner.bnr"), "rb").read()[0x40:]
    arc = bnr[0x600:]

    def u8(a):
        root = struct.unpack_from(">I", a, 4)[0]
        total = struct.unpack_from(">I", a, root + 8)[0]
        strings, files, stack = root + total * 12, {}, []
        for i in range(1, total):
            while stack and i >= stack[-1][1]:
                stack.pop()
            tn, off, size = struct.unpack_from(">3I", a, root + i * 12)
            name = a[strings + (tn & 0xFFFFFF):a.index(b"\0", strings + (tn & 0xFFFFFF))].decode()
            if tn >> 24 == 1:
                stack.append((name, size))
            else:
                files["/".join([s[0] for s in stack] + [name])] = a[off:off + size]
        return files

    icon = u8(arc)["meta/icon.bin"][0x20:]  # IMD5 header
    assert icon[:4] == b"LZ77" and icon[4] == 0x10
    data, size, out, i = icon[4:], int.from_bytes(icon[5:8], "little"), bytearray(), 4
    while len(out) < size:
        flags = data[i]
        i += 1
        for bit in range(8):
            if len(out) >= size:
                break
            if flags & (0x80 >> bit):
                n, disp = (data[i] >> 4) + 3, ((data[i] & 0xF) << 8 | data[i + 1]) + 1
                i += 2
                for _ in range(n):
                    out.append(out[-disp])
            else:
                out.append(data[i])
                i += 1
    return u8(bytes(out))["arc/timg/HBLogo.tpl"][:0x40]


def main():
    hdr = bytearray(hblogo_tpl_header())
    for png, name in (("gc_icon_bg.png", "fwd_gc_logo.bin"), ("gc_banner_bg.png", "fwd_gc_bg.bin")):
        w, h, px = png_rgba(open(os.path.join(ROOT, "data/images", png), "rb").read())
        t = bytearray(hdr)
        img = struct.unpack_from(">I", t, 0x0C)[0]
        struct.pack_into(">HHII", t, img, h, w, 5, 0x40)
        struct.pack_into(">I", t, 0x10, 0)
        data = bytes(t) + rgb5a3(w, h, px)
        path = os.path.join(ROOT, "data/binary", name)
        if "--check" in sys.argv:
            same = open(path, "rb").read() == data
            print(f"{name}: {'up to date' if same else 'DIFFERS'}")
        else:
            open(path, "wb").write(data)
            print(f"{name}: {w}x{h}, sha1 {hashlib.sha1(data).hexdigest()[:12]}")


if __name__ == "__main__":
    main()

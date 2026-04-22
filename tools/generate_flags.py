#!/usr/bin/env python3
"""
Download national flags and generate an index for Dominion.

Downloads PNG flags from flagcdn.com for all countries in the nations
index. Falls back to a simple colored rectangle for countries without
flag data.

Output:
    data/flags/index.bin   — mapping from country_id to filename
    data/flags/*.png       — individual flag textures

Usage:
    python3 generate_flags.py [--nations data/nations.bin]
                              [--flags-dir data/flags]
"""

import argparse
import os
import struct
import sys
import urllib.request
import urllib.error

MAGIC = 0x47414C46  # "FLAG" little-endian
VERSION = 1

FLAGCDN_BASE = "https://flagcdn.com/w160"


def load_nation_iso_codes(nations_path):
    """Read nations.bin and return list of (id, iso_a2) tuples."""
    if not os.path.exists(nations_path):
        print(f"Nations file not found: {nations_path}")
        print("Run generate_nations.py first.")
        return []

    with open(nations_path, "rb") as f:
        magic, version, count = struct.unpack("<III", f.read(12))
        if magic != 0x4E414E54:
            print(f"Bad magic in nations file: 0x{magic:08X}")
            return []

        nations = []
        for i in range(count):
            rec_start = f.tell()

            # id
            nid = struct.unpack("<I", f.read(4))[0]

            # iso_a3
            iso_a3_len = struct.unpack("<B", f.read(1))[0]
            iso_a3 = f.read(iso_a3_len).decode("utf-8")

            # iso_a2
            iso_a2_len = struct.unpack("<B", f.read(1))[0]
            iso_a2 = f.read(iso_a2_len).decode("utf-8")

            # name
            name_len = struct.unpack("<H", f.read(2))[0]
            name = f.read(name_len).decode("utf-8")

            # Skip rest of this record (color_rgb + continent + region +
            # subregion + population + gdp + coords + area)
            # Each is pack("<I")=4, then pack("<B")+len for 3 strings,
            # then 2xI + 4xf = 32 bytes for remaining fields
            # We'll just seek relative to pos since the format is variable length

            # Skip: color_rgb (4)
            f.read(4)

            # Skip: continent
            cont_len = struct.unpack("<B", f.read(1))[0]
            f.read(cont_len)

            # Skip: region
            reg_len = struct.unpack("<B", f.read(1))[0]
            f.read(reg_len)

            # Skip: subregion
            sub_len = struct.unpack("<B", f.read(1))[0]
            f.read(sub_len)

            # Skip: population (4), gdp (4), capital_lon (4), capital_lat (4),
            # centroid_lon (4), centroid_lat (4), area (4) = 28 bytes
            f.read(28)

            nations.append((nid, iso_a2.lower(), name))

        return nations


def download_flag(iso_a2, flags_dir):
    """Download flag PNG from flagcdn.com. Returns (filename, width, height)."""
    filename = f"{iso_a2}.png"
    filepath = os.path.join(flags_dir, filename)

    if os.path.exists(filepath):
        # Already have it — get dimensions from file
        try:
            with open(filepath, "rb") as f:
                # Quick PNG dimension extraction from IHDR
                f.read(16)  # skip PNG signature
                w = struct.unpack(">I", f.read(4))[0]
                h = struct.unpack(">I", f.read(4))[0]
                return filename, w, h
        except Exception:
            return filename, 160, 96  # default guess

    url = f"{FLAGCDN_BASE}/{iso_a2}.png"
    try:
        req = urllib.request.Request(url, headers={"User-Agent": "Dominion/1.0"})
        with urllib.request.urlopen(req, timeout=10) as resp:
            data = resp.read()
            with open(filepath, "wb") as f:
                f.write(data)

        # Get actual dimensions
        try:
            import struct as st
            w = st.unpack(">I", data[16:20])[0]
            h = st.unpack(">I", data[20:24])[0]
            return filename, w, h
        except Exception:
            return filename, 160, 96
    except Exception as e:
        print(f"  Failed to download flag for {iso_a2}: {e}")
        return None, 0, 0


def write_index_bin(filepath, flags):
    """Write flags/index.bin."""
    os.makedirs(os.path.dirname(filepath) or ".", exist_ok=True)

    with open(filepath, "wb") as f:
        f.write(struct.pack("<I", MAGIC))
        f.write(struct.pack("<I", VERSION))
        f.write(struct.pack("<I", len(flags)))

        for country_id, filename, w, h in flags:
            encoded = filename.encode("utf-8")
            f.write(struct.pack("<I", country_id))
            f.write(struct.pack("<B", len(encoded)))
            f.write(encoded)
            f.write(struct.pack("<HH", w, h))

    size = os.path.getsize(filepath)
    print(f"Written {filepath} ({size} bytes, {len(flags)} flags)")


def main():
    parser = argparse.ArgumentParser(
        description="Download national flags for Dominion"
    )
    parser.add_argument("--nations", type=str, default="data/nations.bin")
    parser.add_argument("--flags-dir", type=str, default="data/flags")
    args = parser.parse_args()

    nations = load_nation_iso_codes(args.nations)
    if not nations:
        print("No nations found. Run generate_nations.py first.", file=sys.stderr)
        sys.exit(1)

    print(f"Loading flags for {len(nations)} nations...")
    os.makedirs(args.flags_dir, exist_ok=True)

    flags = []
    downloaded = 0
    failed = 0

    for i, (nid, iso_a2, name) in enumerate(nations):
        if not iso_a2:
            failed += 1
            continue

        filename, w, h = download_flag(iso_a2, args.flags_dir)
        if filename:
            flags.append((nid, filename, w, h))
            downloaded += 1
            if downloaded % 50 == 0:
                print(f"  Downloaded {downloaded} flags...")
        else:
            failed += 1

    print(f"Downloaded {downloaded} flags, {failed} failed")

    index_path = os.path.join(args.flags_dir, "index.bin")
    write_index_bin(index_path, flags)

    size_mb = sum(
        os.path.getsize(os.path.join(args.flags_dir, fn))
        for _, fn, _, _ in flags
        if os.path.exists(os.path.join(args.flags_dir, fn))
    ) / (1024 * 1024)
    print(f"Total flag size: {size_mb:.1f} MB")
    print("Done.")


if __name__ == "__main__":
    main()

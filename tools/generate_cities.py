#!/usr/bin/env python3
"""
Generate populated places data for Dominion from Natural Earth.

Downloads Natural Earth 1:10m populated places GeoJSON and extracts
cities with population > 100K. Each city is mapped to tile coordinates
on the 2048x1024 equirectangular grid.

Output: data/cities.bin

Usage:
    python3 generate_cities.py [--width 2048] [--height 1024]
                               [--min-pop 100000] [--output data/cities.bin]
"""

import argparse
import json
import math
import os
import struct
import sys
import urllib.request

MAGIC = 0x54494349  # "ICIT" little-endian
VERSION = 1

POPULATED_PLACES_URL = (
    "https://raw.githubusercontent.com/nvkelso/"
    "natural-earth-vector/master/geojson/ne_10m_populated_places.geojson"
)

# City tier thresholds
TIER_MEGA = 10000000    # >10M
TIER_LARGE = 1000000    # 1M-10M
TIER_MEDIUM = 100000    # 100K-1M
# TIER_SMALL below 100K


def download_places(cache_path="/tmp/ne_10m_populated_places.geojson"):
    if os.path.exists(cache_path):
        size_mb = os.path.getsize(cache_path) / 1024 / 1024
        print(f"Using cached {cache_path} ({size_mb:.1f} MB)")
        return cache_path
    print("Downloading Natural Earth 1:10m populated places...")
    urllib.request.urlretrieve(POPULATED_PLACES_URL, cache_path)
    size_mb = os.path.getsize(cache_path) / 1024 / 1024
    print(f"  Downloaded {size_mb:.1f} MB")
    return cache_path


def lonlat_to_tile(lon, lat, width, height):
    x = int((lon + 180.0) / 360.0 * width) % width
    y = int((90.0 - lat) / 180.0 * height)
    return x, max(0, min(height - 1, y))


def assign_tier(pop):
    if pop >= TIER_MEGA:
        return 0  # mega
    elif pop >= TIER_LARGE:
        return 1  # large
    elif pop >= TIER_MEDIUM:
        return 2  # medium
    else:
        return 3  # small


def extract_cities(geojson_path, min_pop, width, height):
    with open(geojson_path) as f:
        data = json.load(f)

    cities = []
    seen_tiles = {}  # (tx, ty) -> index in cities list

    for feat in data.get("features", []):
        props = feat.get("properties", {})
        geom = feat.get("geometry")
        if not props or not geom:
            continue

        # Population
        pop = 0
        raw_pop = props.get("POP_MAX") or props.get("POP_MIN") or props.get("POP_OTHER")
        if raw_pop is not None:
            try:
                pop = int(float(raw_pop))
            except (ValueError, TypeError):
                continue

        if pop < min_pop:
            continue

        name = (props.get("NAME") or props.get("NAMEASCII") or "").strip()
        if not name:
            continue

        coords = geom.get("coordinates", [])
        if len(coords) < 2:
            continue

        lon, lat = float(coords[0]), float(coords[1])
        tx, ty = lonlat_to_tile(lon, lat, width, height)

        # Capital detection
        capital_flag = 0
        featclass = props.get("FEATURECLA", "")
        if featclass == "Admin-0 capital" or featclass == "Admin-0 region capital":
            capital_flag |= 1  # national capital
        adm0cap = props.get("ADM0CAP", 0)
        if adm0cap and float(adm0cap) > 0:
            capital_flag |= 1

        iso_a2 = (props.get("ISO_A2") or "").strip()
        tier = assign_tier(pop)

        city = {
            "name": name,
            "iso_a2": iso_a2,
            "tile_x": tx,
            "tile_y": ty,
            "population": pop,
            "capital_flag": capital_flag,
            "tier": tier,
        }

        # Deduplicate tiles: keep largest city per tile
        key = (tx, ty)
        if key in seen_tiles:
            existing_idx = seen_tiles[key]
            if pop > cities[existing_idx]["population"]:
                cities[existing_idx] = city
        else:
            seen_tiles[key] = len(cities)
            cities.append(city)

    cities.sort(key=lambda c: c["population"], reverse=True)
    return cities


def write_cities_bin(filepath, cities, width, height):
    os.makedirs(os.path.dirname(filepath) or ".", exist_ok=True)

    with open(filepath, "wb") as f:
        f.write(struct.pack("<I", MAGIC))
        f.write(struct.pack("<I", VERSION))
        f.write(struct.pack("<I", width))
        f.write(struct.pack("<I", height))
        f.write(struct.pack("<I", len(cities)))

        for i, c in enumerate(cities):
            name_enc = c["name"].encode("utf-8")
            iso_enc = c["iso_a2"].encode("utf-8")[:2]

            f.write(struct.pack("<H", len(name_enc)))
            f.write(name_enc)
            f.write(struct.pack("<B", len(iso_enc)))
            f.write(iso_enc)
            f.write(struct.pack("<HH", c["tile_x"], c["tile_y"]))
            f.write(struct.pack("<I", c["population"]))
            f.write(struct.pack("<B", c["capital_flag"]))
            f.write(struct.pack("<B", c["tier"]))

    size_kb = os.path.getsize(filepath) / 1024
    capitals = sum(1 for c in cities if c["capital_flag"] > 0)
    print(f"Written {filepath} ({size_kb:.1f} KB, {len(cities)} cities, "
          f"{capitals} capitals)")


def main():
    parser = argparse.ArgumentParser(
        description="Generate populated places data for Dominion"
    )
    parser.add_argument("--width", type=int, default=2048)
    parser.add_argument("--height", type=int, default=1024)
    parser.add_argument("--min-pop", type=int, default=100000)
    parser.add_argument("--output", type=str, default="data/cities.bin")
    args = parser.parse_args()

    try:
        geojson_path = download_places()
        cities = extract_cities(geojson_path, args.min_pop, args.width, args.height)
        print(f"Extracted {len(cities)} cities (pop >= {args.min_pop:,})")
        write_cities_bin(args.output, cities, args.width, args.height)
        print("Done.")
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main()

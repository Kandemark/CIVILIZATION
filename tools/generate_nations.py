#!/usr/bin/env python3
"""
Generate master nation index for Dominion from Natural Earth Admin 0 data.

Downloads Natural Earth 1:10m Admin 0 (countries) GeoJSON and extracts
metadata for ~195 sovereign states. Assigns unique distinct colors and
writes a binary index that the game loads via civ_nations_data_load().

Output: data/nations.bin

Usage:
    python3 generate_nations.py [--output data/nations.bin]

Requirements:
    None beyond stdlib.
"""

import argparse
import json
import os
import struct
import sys
import urllib.request

MAGIC = 0x4E414E54  # "TNAN" little-endian
VERSION = 1

ADMIN0_URL = (
    "https://raw.githubusercontent.com/nvkelso/"
    "natural-earth-vector/master/geojson/ne_10m_admin_0_countries.geojson"
)

# Entities to exclude (not independent nations)
EXCLUDE_SOVEREIGN = {
    "Antarctica",
}

# ISO codes for entities that ARE independent but Natural Earth may mark
# as dependencies or have weird ADMIN values
FORCE_INCLUDE_ISO = {
    "TWN",  # Taiwan
    "PSE",  # Palestine
    "XKO",  # Kosovo
    "ESH",  # Western Sahara
}


def download_admin0(cache_path="/tmp/ne_10m_admin_0_countries.geojson"):
    """Download Natural Earth Admin 0 GeoJSON if not cached."""
    if os.path.exists(cache_path):
        size_mb = os.path.getsize(cache_path) / 1024 / 1024
        print(f"Using cached {cache_path} ({size_mb:.1f} MB)")
        return cache_path
    print("Downloading Natural Earth 1:10m Admin 0 countries...")
    urllib.request.urlretrieve(ADMIN0_URL, cache_path)
    size_mb = os.path.getsize(cache_path) / 1024 / 1024
    print(f"  Downloaded {size_mb:.1f} MB")
    return cache_path


def hsl_to_rgb(h, s, l):
    """Convert HSL (all 0-1) to RGB (0-255 tuple)."""
    if s == 0:
        v = int(l * 255)
        return (v, v, v)

    def hue_to_rgb(p, q, t):
        if t < 0:
            t += 1
        if t > 1:
            t -= 1
        if t < 1 / 6:
            return p + (q - p) * 6 * t
        if t < 1 / 2:
            return q
        if t < 2 / 3:
            return p + (q - p) * (2 / 3 - t) * 6
        return p

    q = l * (1 + s) if l < 0.5 else l + s - l * s
    p = 2 * l - q
    r = hue_to_rgb(p, q, h + 1 / 3)
    g = hue_to_rgb(p, q, h)
    b = hue_to_rgb(p, q, h - 1 / 3)
    return (int(r * 255), int(g * 255), int(b * 255))


def assign_colors(count):
    """Assign visually distinct colors using golden-ratio hue spacing."""
    golden_ratio = 0.618033988749895
    colors = []
    hue = 0.0
    for i in range(count):
        hue = (hue + golden_ratio) % 1.0
        saturation = 0.65 + (i % 3) * 0.10
        lightness = 0.45 + (i % 4) * 0.08
        r, g, b = hsl_to_rgb(hue, saturation, lightness)
        colors.append((r << 16) | (g << 8) | b)
    return colors


def extract_countries(geojson_path):
    """Parse the Admin 0 GeoJSON and return a list of country dicts."""
    with open(geojson_path) as f:
        data = json.load(f)

    countries = []
    seen = set()

    for feat in data.get("features", []):
        props = feat.get("properties", {})
        if not props:
            continue

        sovereign = (props.get("ADMIN") or props.get("SOVEREIGNT") or "").strip()
        name = (props.get("NAME") or props.get("NAME_LONG") or "").strip()
        iso_a3 = (props.get("ISO_A3") or props.get("ADM0_A3") or "").strip()
        iso_a2 = (props.get("ISO_A2") or "").strip()

        # Filter out non-countries
        if sovereign in EXCLUDE_SOVEREIGN:
            continue

        # Skip features that are clearly not independent nations
        # Natural Earth includes dependent territories as Admin 0 features
        # with ADMIN = parent country. We check ISO_A3 existence as primary
        # sovereignty signal, but also force-include certain entities.
        if not iso_a3:
            continue

        # Deduplicate by ISO_A3
        if iso_a3 in seen:
            continue
        seen.add(iso_a3)

        if not name:
            name = iso_a3

        # Extract numerical fields safely
        pop_est = 0
        raw_pop = props.get("POP_EST")
        if raw_pop is not None:
            try:
                pop_est = int(float(raw_pop))
            except (ValueError, TypeError):
                pass

        gdp_est = 0
        raw_gdp = props.get("GDP_MD_EST")
        if raw_gdp is not None:
            try:
                gdp_est = int(float(raw_gdp))
            except (ValueError, TypeError):
                pass

        # Label point as capital approximation
        label_x = props.get("LABEL_X")
        label_y = props.get("LABEL_Y")
        capital_lon = float(label_x) if label_x is not None else 0.0
        capital_lat = float(label_y) if label_y is not None else 0.0

        continent = (props.get("CONTINENT") or "").strip()
        region = (props.get("REGION_UN") or "").strip()
        subregion = (props.get("SUBREGION") or "").strip()

        # Area
        area_sqkm = 0.0
        raw_area = props.get("AREA") or props.get("SQKM")
        if raw_area is not None:
            try:
                area_sqkm = float(raw_area)
            except (ValueError, TypeError):
                pass

        countries.append({
            "iso_a3": iso_a3,
            "iso_a2": iso_a2,
            "name": name,
            "continent": continent,
            "region": region,
            "subregion": subregion,
            "population": pop_est,
            "gdp": gdp_est,
            "capital_lon": capital_lon,
            "capital_lat": capital_lat,
            "area": area_sqkm,
        })

    # Sort by name for stable ordering
    countries.sort(key=lambda c: c["name"])
    return countries


def write_nations_bin(filepath, countries):
    """Write the nations.bin binary file."""
    os.makedirs(os.path.dirname(filepath) or ".", exist_ok=True)

    with open(filepath, "wb") as f:
        # Header
        f.write(struct.pack("<III", MAGIC, VERSION, len(countries)))

        # Generate colors
        colors = assign_colors(len(countries))

        for i, c in enumerate(countries):
            color_rgb = colors[i]

            iso_a3 = c["iso_a3"].encode("utf-8")[:3]
            iso_a2 = c["iso_a2"].encode("utf-8")[:2]
            name = c["name"].encode("utf-8")
            continent = c["continent"].encode("utf-8")
            region = c["region"].encode("utf-8")
            subregion = c["subregion"].encode("utf-8")

            # Per-country record
            f.write(struct.pack("<I", i))  # id
            f.write(struct.pack("<B", len(iso_a3)))
            f.write(iso_a3)
            f.write(struct.pack("<B", len(iso_a2)))
            f.write(iso_a2)
            f.write(struct.pack("<H", len(name)))
            f.write(name)
            f.write(struct.pack("<I", color_rgb))
            f.write(struct.pack("<B", len(continent)))
            f.write(continent)
            f.write(struct.pack("<B", len(region)))
            f.write(region)
            f.write(struct.pack("<B", len(subregion)))
            f.write(subregion)
            f.write(struct.pack("<I", c["population"]))
            f.write(struct.pack("<I", c["gdp"]))
            f.write(struct.pack("<f", c["capital_lon"]))
            f.write(struct.pack("<f", c["capital_lat"]))
            f.write(struct.pack("<f", 0.0))  # centroid_lon placeholder
            f.write(struct.pack("<f", 0.0))  # centroid_lat placeholder
            f.write(struct.pack("<f", c["area"]))

    size_kb = os.path.getsize(filepath) / 1024
    print(f"Written {filepath} ({size_kb:.1f} KB, {len(countries)} countries)")


def main():
    parser = argparse.ArgumentParser(
        description="Generate master nation index for Dominion"
    )
    parser.add_argument(
        "--output", type=str, default="data/nations.bin"
    )
    args = parser.parse_args()

    try:
        geojson_path = download_admin0()
        countries = extract_countries(geojson_path)
        print(f"Extracted {len(countries)} countries from Natural Earth")
        write_nations_bin(args.output, countries)
        print("Done.")
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()

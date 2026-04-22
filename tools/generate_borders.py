#!/usr/bin/env python3
"""
Generate per-tile country borders for Dominion from Natural Earth Admin 0 data.

Rasterizes Natural Earth country polygons to the 2048x1024 equirectangular
grid and outputs a binary file mapping every tile to a country ID (-1 = water
or no country). The format matches what civ_political_borders_load() expects.

Output: data/earth_borders.bin

Usage:
    python3 generate_borders.py [--width 2048] [--height 1024] [--output data/earth_borders.bin]

Note: Run generate_nations.py first to cache the Admin 0 GeoJSON at
/tmp/ne_10m_admin_0_countries.geojson
"""

import argparse
import json
import os
import struct
import sys
import urllib.request

ADMIN0_URL = (
    "https://raw.githubusercontent.com/nvkelso/"
    "natural-earth-vector/master/geojson/ne_10m_admin_0_countries.geojson"
)

EXCLUDE_SOVEREIGN = {"Antarctica"}


def download_admin0(cache_path="/tmp/ne_10m_admin_0_countries.geojson"):
    if os.path.exists(cache_path):
        size_mb = os.path.getsize(cache_path) / 1024 / 1024
        print(f"Using cached {cache_path} ({size_mb:.1f} MB)")
        return cache_path
    print("Downloading Natural Earth 1:10m Admin 0 countries...")
    urllib.request.urlretrieve(ADMIN0_URL, cache_path)
    size_mb = os.path.getsize(cache_path) / 1024 / 1024
    print(f"  Downloaded {size_mb:.1f} MB")
    return cache_path


def lonlat_to_tile(lon, lat, width, height):
    """Convert lon/lat to tile coordinates."""
    x = int((lon + 180.0) / 360.0 * width) % width
    y = int((90.0 - lat) / 180.0 * height)
    return x, max(0, min(height - 1, y))


def fill_polygon_scanline(mask, ring_coords, country_idx, width, height):
    """Even-odd rule polygon fill assigning country_idx to mask tiles."""
    if len(ring_coords) < 3:
        return

    px, py = [], []
    for lon, lat in ring_coords:
        x, y = lonlat_to_tile(lon, lat, width, height)
        px.append(x)
        py.append(y)

    min_y = max(0, min(py))
    max_y = min(height - 1, max(py))

    for y in range(min_y, max_y + 1):
        intersections = []
        n = len(px)
        for i in range(n):
            j = (i + 1) % n
            yi, yj = py[i], py[j]
            if (yi <= y < yj) or (yj <= y < yi):
                xi, xj = px[i], px[j]
                if yj != yi:
                    x_int = xi + (y - yi) * (xj - xi) / (yj - yi)
                    intersections.append(x_int)
                elif yi == y:
                    intersections.append(float(xi))

        intersections.sort()
        for k in range(0, len(intersections) - 1, 2):
            xs = max(0, int(intersections[k]))
            xe = min(width - 1, int(intersections[k + 1]))
            if xs <= xe:
                # Handle horizontal wrap: if span is > half width,
                # it crosses the 180/-180 meridian
                if xe - xs > width // 2:
                    mask[y, xs:] = country_idx
                    mask[y, : xe + 1] = country_idx
                else:
                    mask[y, xs : xe + 1] = country_idx


def hsl_to_rgb(h, s, l):
    """Convert HSL (0-1) to 0xRRGGBB integer."""
    if s == 0:
        v = int(l * 255)
        return (v << 16) | (v << 8) | v

    def hue_to_rgb(p, q, t):
        if t < 0: t += 1
        if t > 1: t -= 1
        if t < 1/6: return p + (q - p) * 6 * t
        if t < 1/2: return q
        if t < 2/3: return p + (q - p) * (2/3 - t) * 6
        return p

    q = l * (1 + s) if l < 0.5 else l + s - l * s
    p = 2 * l - q
    r = int(hue_to_rgb(p, q, h + 1/3) * 255)
    g = int(hue_to_rgb(p, q, h) * 255)
    b = int(hue_to_rgb(p, q, h - 1/3) * 255)
    return (r << 16) | (g << 8) | b


def assign_colors(count):
    """Assign visually distinct colors using golden-ratio hue spacing."""
    golden_ratio = 0.618033988749895
    colors = []
    hue = 0.0
    for i in range(count):
        hue = (hue + golden_ratio) % 1.0
        saturation = 0.65 + (i % 3) * 0.10
        lightness = 0.45 + (i % 4) * 0.08
        colors.append(hsl_to_rgb(hue, saturation, lightness))
    return colors


def extract_countries_from_geojson(geojson_path):
    """Extract country polygons with metadata from Admin 0 GeoJSON."""
    with open(geojson_path) as f:
        data = json.load(f)

    countries = []
    seen_iso = set()

    for feat in data.get("features", []):
        props = feat.get("properties", {})
        geom = feat.get("geometry")
        if not props or not geom:
            continue

        sovereign = (props.get("ADMIN") or props.get("SOVEREIGNT") or "").strip()
        if sovereign in EXCLUDE_SOVEREIGN:
            continue

        iso_a3 = (props.get("ISO_A3") or props.get("ADM0_A3") or "").strip()
        if not iso_a3 or iso_a3 in seen_iso:
            continue
        seen_iso.add(iso_a3)

        name = (props.get("NAME") or props.get("NAME_LONG") or iso_a3).strip()
        area = 0.0
        raw_area = props.get("AREA") or props.get("SQKM")
        if raw_area is not None:
            try:
                area = float(raw_area)
            except (ValueError, TypeError):
                pass

        coords = geom.get("coordinates", [])
        gtype = geom.get("type", "")

        # Flatten to list of polygon rings
        rings = []
        if gtype == "Polygon":
            if coords and len(coords) > 0:
                rings.append(coords[0])  # outer ring
        elif gtype == "MultiPolygon":
            for poly in coords:
                if poly and len(poly) > 0:
                    rings.append(poly[0])  # outer ring of each sub-polygon

        if rings:
            countries.append({
                "iso_a3": iso_a3,
                "name": name,
                "area": area,
                "rings": rings,
            })

    # Sort by area descending so larger countries rasterize first,
    # smaller countries overwrite (more precise borders for small nations)
    countries.sort(key=lambda c: c["area"], reverse=True)
    return countries


def rasterize_countries(countries, width, height):
    """
    Rasterize all countries to an int16 tile grid.

    Returns (tile_grid, country_list) where tile_grid is a 1D int16 array
    of length width*height (-1 = no country) and country_list is the list
    of (name, color) tuples for the output file.
    """
    import numpy as np
    mask = np.full((height, width), -1, dtype=np.int16)

    colors = assign_colors(len(countries))
    country_list = []

    for idx, c in enumerate(countries):
        if idx % 50 == 0:
            print(f"  Rasterizing country {idx+1}/{len(countries)}...")

        for ring in c["rings"]:
            fill_polygon_scanline(mask, ring, idx, width, height)

        country_list.append((c["name"], colors[idx]))

    return mask, country_list


def check_land_water_consistency(tile_grid, earth_map_path, width, height):
    """
    Verify land tiles in the earth map have country assignments.
    Prints statistics about coverage.
    """
    if not os.path.exists(earth_map_path):
        print("  (earth map not found, skipping land/water consistency check)")
        return

    with open(earth_map_path, "rb") as f:
        f.read(48)  # skip header (44 bytes + 4 reserved)
        landmask = f.read(width * height)

    import numpy as np
    land = np.frombuffer(landmask, dtype=np.uint8).reshape((height, width))

    land_tiles = int(np.count_nonzero(land))
    land_with_country = int(np.count_nonzero((land > 0) & (tile_grid >= 0)))
    coverage = land_with_country / land_tiles * 100 if land_tiles > 0 else 0

    print(f"  Land tiles: {land_tiles}")
    print(f"  Land tiles with country: {land_with_country} ({coverage:.1f}%)")

    if coverage < 90:
        print("  WARNING: Low country coverage on land. Coastline may be mismatched.")


def write_borders_bin(filepath, tile_grid, country_list, width, height):
    """Write earth_borders.bin in the format expected by political_borders.c."""
    os.makedirs(os.path.dirname(filepath) or ".", exist_ok=True)

    with open(filepath, "wb") as f:
        # Country count
        f.write(struct.pack("<I", len(country_list)))

        # Per-country: name length, name, color
        for name, color in country_list:
            encoded = name.encode("utf-8")
            f.write(struct.pack("<B", len(encoded)))
            f.write(encoded)
            f.write(struct.pack("<I", color))

        # Tile grid: width*height int16 values
        for y in range(height):
            for x in range(width):
                f.write(struct.pack("<h", int(tile_grid[y, x])))

    size_mb = os.path.getsize(filepath) / (1024 * 1024)
    print(f"Written {filepath} ({size_mb:.1f} MB, {len(country_list)} countries)")


def main():
    parser = argparse.ArgumentParser(
        description="Generate political borders for Dominion"
    )
    parser.add_argument("--width", type=int, default=2048)
    parser.add_argument("--height", type=int, default=1024)
    parser.add_argument("--output", type=str, default="data/earth_borders.bin")
    parser.add_argument("--earth-map", type=str, default="data/earth_2048x1024.earth")
    args = parser.parse_args()

    try:
        import numpy as np
    except ImportError:
        print("numpy required. Install: pip install numpy", file=sys.stderr)
        sys.exit(1)

    try:
        geojson_path = download_admin0()
        countries = extract_countries_from_geojson(geojson_path)
        print(f"Extracted {len(countries)} countries with polygon data")

        print(f"Rasterizing to {args.width}x{args.height}...")
        tile_grid, country_list = rasterize_countries(
            countries, args.width, args.height
        )

        check_land_water_consistency(
            tile_grid, args.earth_map, args.width, args.height
        )

        write_borders_bin(args.output, tile_grid, country_list, args.width, args.height)
        print("Done.")
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main()

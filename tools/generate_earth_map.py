#!/usr/bin/env python3
"""
Generate Earth land-water mask for Dominion.

Primary: Downloads Natural Earth 1:10m land polygon data and rasterizes
it to an equirectangular grid with pixel-accurate coastlines (~32% land).

Fallback: Built-in simplified continent outlines (~21% land) if Natural
Earth download fails or numpy is unavailable.

Outputs a .earth binary file the game loads directly via civ_earth_map_load().

Usage:
    python3 generate_earth_map.py [--width 2048] [--height 1024] \\
                                  [--output data/earth_2048x1024.earth]
                                  [--fallback]

Requirements:
    pip install numpy (required for Natural Earth rasterization)
"""

import argparse
import json
import os
import struct
import sys
import urllib.request

MAGIC = 0x48545241
VERSION = 1
NATURAL_EARTH_URL = (
    "https://raw.githubusercontent.com/nvkelso/"
    "natural-earth-vector/master/geojson/ne_10m_land.geojson"
)


def download_natural_earth(cache_path="/tmp/ne_10m_land.geojson"):
    """Download Natural Earth 1:10m land GeoJSON if not cached."""
    if os.path.exists(cache_path):
        print(f"Using cached {cache_path} ({os.path.getsize(cache_path)/1024/1024:.1f} MB)")
        return cache_path
    print(f"Downloading Natural Earth 1:10m land data...")
    urllib.request.urlretrieve(NATURAL_EARTH_URL, cache_path)
    print(f"  Downloaded {os.path.getsize(cache_path)/1024/1024:.1f} MB")
    return cache_path


def lonlat_to_pixel(lon, lat, width, height):
    x = int((lon + 180.0) / 360.0 * width) % width
    y = int((90.0 - lat) / 180.0 * height)
    return x, max(0, min(height - 1, y))


def fill_scanline(mask, ring_coords, value, width, height):
    """Even-odd rule polygon fill on a 2D integer grid."""
    import numpy as np
    if len(ring_coords) < 3:
        return
    px, py = [], []
    for lon, lat in ring_coords:
        x, y = lonlat_to_pixel(lon, lat, width, height)
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
            xs = max(0, int(np.floor(intersections[k])))
            xe = min(width - 1, int(np.ceil(intersections[k + 1])))
            if xs <= xe:
                if xe - xs > width // 2:
                    mask[y, xs:] = value
                    mask[y, : xe + 1] = value
                else:
                    mask[y, xs : xe + 1] = value


def rasterize_natural_earth(geojson_path, width, height):
    """Rasterize Natural Earth land polygons to a land-water mask."""
    import numpy as np

    with open(geojson_path) as f:
        data = json.load(f)

    mask = np.zeros((height, width), dtype=np.uint8)
    features = data.get("features", [])

    for feat in features:
        geom = feat.get("geometry")
        if not geom:
            continue
        gtype = geom.get("type", "")
        coords = geom.get("coordinates", [])

        if gtype == "Polygon":
            if coords and len(coords) > 0:
                fill_scanline(mask, coords[0], 1, width, height)
                for hole in coords[1:]:
                    fill_scanline(mask, hole, 0, width, height)
        elif gtype == "MultiPolygon":
            for poly in coords:
                if poly and len(poly) > 0:
                    fill_scanline(mask, poly[0], 1, width, height)
                    for hole in poly[1:]:
                        fill_scanline(mask, hole, 0, width, height)

    # Arctic Ocean: top rows are water
    mask[0, :] = 0
    mask[1, :] = 0

    return mask


def coastline_polygons():
    """Fallback: simplified continent outlines at ~2-3 degree resolution."""
    return {
        "africa": [
            (-17, 15), (-15, 28), (-5, 36), (10, 37), (12, 33),
            (30, 31), (33, 30), (40, 22), (43, 10), (51, 10),
            (50, 2), (42, -2), (40, -10), (35, -20), (33, -26),
            (28, -33), (18, -35), (15, -30), (12, -17), (10, -5),
            (8, 4), (5, 5), (-5, 5), (-8, 4), (-17, 15),
        ],
        "europe": [
            (-10, 36), (-10, 43), (-5, 44), (0, 47), (5, 46),
            (8, 44), (13, 45), (16, 48), (20, 52), (23, 55),
            (30, 60), (28, 63), (20, 65), (10, 63), (7, 58),
            (10, 55), (5, 53), (-5, 55), (-10, 58), (-20, 63),
            (-25, 65), (-25, 70), (-10, 70), (5, 70), (15, 70),
            (30, 70), (35, 68), (40, 68), (60, 70), (70, 72),
            (80, 72), (100, 73), (120, 70), (140, 65), (160, 62),
            (180, 65), (180, 68), (170, 68), (160, 70), (140, 72),
            (120, 72), (100, 74), (80, 72), (70, 72), (60, 70),
            (40, 68), (30, 68), (10, 70), (-5, 70), (-25, 70),
            (-25, 65), (-10, 58), (-5, 55), (5, 53), (7, 56),
            (12, 55), (12, 50), (8, 44), (5, 43), (-5, 40), (-10, 36),
        ],
        "asia": [
            (30, 60), (40, 60), (50, 55), (60, 50), (70, 42),
            (73, 38), (80, 30), (77, 22), (80, 15), (90, 22),
            (95, 20), (95, 15), (100, 8), (105, 5), (108, 2),
            (105, -2), (105, -5), (110, -7), (120, -5), (125, -3),
            (130, -2), (135, -3), (140, -2), (145, -3), (150, -5),
            (153, -3), (155, -5), (160, -8), (150, -10), (145, -8),
            (140, -7), (135, -6), (125, -8), (120, -8), (115, -6),
            (105, -7), (100, -8), (95, -6), (90, -5), (80, -10),
            (75, -15), (70, -20), (65, -25), (60, -25), (55, -25),
            (50, -20), (45, -15), (40, -10), (35, -5), (30, 0),
            (28, 5), (25, 10), (22, 15), (20, 15), (18, 12),
            (15, 10), (12, 8), (8, 6), (15, 5), (20, 5),
            (25, 8), (28, 10), (30, 12), (35, 12), (40, 15),
            (45, 20), (50, 25), (53, 30), (55, 30), (60, 30),
            (65, 30), (70, 35), (73, 38), (80, 40), (90, 45),
            (100, 50), (110, 50), (120, 55), (130, 60), (140, 65),
            (160, 60), (180, 65), (180, 68), (140, 72), (100, 74),
            (80, 72), (60, 70), (40, 68), (30, 60),
        ],
        "north_america": [
            (-170, 65), (-160, 60), (-150, 60), (-140, 60), (-130, 55),
            (-125, 50), (-120, 48), (-115, 45), (-110, 40), (-105, 35),
            (-100, 30), (-95, 28), (-90, 30), (-88, 28), (-85, 30),
            (-82, 25), (-80, 25), (-75, 35), (-70, 42), (-65, 45),
            (-60, 47), (-55, 50), (-60, 55), (-65, 60), (-70, 65),
            (-75, 70), (-80, 72), (-90, 75), (-100, 78), (-110, 75),
            (-120, 75), (-130, 70), (-140, 68), (-150, 65), (-160, 63),
            (-170, 65),
        ],
        "south_america": [
            (-80, 8), (-75, 10), (-72, 12), (-70, 12), (-65, 5),
            (-60, 2), (-55, 0), (-50, -2), (-45, -5), (-40, -8),
            (-38, -15), (-40, -20), (-42, -23), (-45, -27), (-48, -30),
            (-50, -32), (-52, -35), (-55, -35), (-58, -33), (-60, -35),
            (-63, -40), (-65, -45), (-68, -50), (-70, -53), (-72, -53),
            (-74, -50), (-75, -45), (-73, -40), (-72, -35), (-70, -30),
            (-68, -25), (-65, -20), (-62, -15), (-60, -10), (-58, -5),
            (-55, -2), (-52, 0), (-50, 2), (-48, 0), (-50, -2),
            (-52, -5), (-55, -8), (-57, -13), (-58, -18), (-56, -22),
            (-53, -25), (-50, -25), (-47, -22), (-44, -18), (-42, -13),
            (-40, -8), (-38, -5), (-35, -3), (-33, -2), (-30, 0),
            (-28, -3), (-27, -5), (-30, -8), (-35, -10), (-40, -13),
            (-45, -15), (-50, -18), (-55, -20), (-60, -22), (-65, -22),
            (-68, -18), (-70, -12), (-72, -8), (-73, -5), (-75, 0),
            (-78, 2), (-80, 5), (-80, 8),
        ],
        "australia": [
            (115, -20), (120, -18), (125, -15), (130, -15), (135, -15),
            (140, -12), (145, -10), (148, -8), (150, -10), (152, -12),
            (153, -15), (152, -20), (150, -25), (148, -30), (145, -35),
            (140, -38), (135, -35), (130, -32), (125, -30), (120, -28),
            (115, -25), (113, -25), (115, -22), (115, -20),
        ],
        "greenland": [
            (-55, 60), (-45, 60), (-40, 63), (-35, 65), (-30, 68),
            (-25, 70), (-20, 72), (-18, 75), (-22, 77), (-30, 80),
            (-40, 83), (-50, 82), (-55, 80), (-55, 75), (-50, 70),
            (-48, 65), (-55, 60),
        ],
        "antarctica": [
            (-180, -70), (-150, -72), (-120, -70), (-90, -70), (-60, -72),
            (-30, -70), (0, -70), (30, -70), (60, -72), (90, -70),
            (120, -70), (150, -72), (180, -70), (180, -80), (-180, -80),
            (-180, -70),
        ],
        "japan": [
            (130, 30), (135, 32), (140, 35), (145, 40), (145, 35),
            (143, 32), (140, 30), (135, 28), (130, 30),
        ],
        "indonesia_png": [
            (95, -5), (100, -3), (105, -3), (110, -5), (115, -8),
            (120, -8), (125, -8), (130, -5), (135, -5), (140, -5),
            (145, -5), (150, -8), (155, -8), (150, -10), (145, -10),
            (140, -10), (135, -10), (130, -10), (125, -10), (120, -10),
            (115, -10), (110, -8), (105, -8), (100, -8), (95, -8),
            (95, -3),
        ],
        "uk_ireland": [
            (-10, 50), (-6, 49), (-3, 50), (0, 52), (2, 51),
            (0, 50), (-3, 49), (-6, 50), (-10, 50),
        ],
        "madagascar": [
            (43, -12), (48, -15), (50, -18), (50, -22), (48, -25),
            (45, -25), (43, -22), (43, -18), (43, -12),
        ],
        "new_zealand": [
            (166, -35), (170, -37), (173, -38), (175, -40), (178, -42),
            (178, -44), (175, -46), (172, -44), (170, -42), (168, -38),
            (166, -35),
        ],
    }


def polygon_to_mask(polygon, width, height):
    """Rasterize simplified polygon to a boolean mask."""
    import numpy as np
    mask = np.zeros((height, width), dtype=np.uint8)
    if len(polygon) < 3:
        return mask
    px, py = [], []
    for lon, lat in polygon:
        x = int((lon + 180.0) / 360.0 * width) % width
        y = int((90.0 - lat) / 180.0 * height)
        py.append(max(0, min(height - 1, y)))
        px.append(max(0, min(width - 1, x)))
    min_y, max_y = max(0, min(py)), min(height - 1, max(py))
    for y in range(min_y, max_y + 1):
        intersections = []
        n = len(px)
        for i in range(n):
            j = (i + 1) % n
            if (py[i] <= y < py[j]) or (py[j] <= y < py[i]):
                if py[j] != py[i]:
                    x_int = px[i] + (y - py[i]) * (px[j] - px[i]) / (py[j] - py[i])
                    intersections.append(x_int)
                elif py[i] == y:
                    intersections.append(float(px[i]))
        intersections.sort()
        for k in range(0, len(intersections) - 1, 2):
            xs = max(0, int(np.floor(intersections[k])))
            xe = min(width - 1, int(np.ceil(intersections[k + 1])))
            if xs <= xe:
                mask[y, xs : xe + 1] = 1
    return mask


def generate_fallback_mask(width, height):
    """Generate simplified Earth landmask from built-in polygon outlines."""
    import numpy as np
    print(f"Rasterizing simplified Earth outlines to {width}x{height}...")
    landmask = np.zeros((height, width), dtype=np.uint8)
    polys = coastline_polygons()
    for name, poly in polys.items():
        print(f"  Rasterizing {name}...")
        mask = polygon_to_mask(poly, width, height)
        landmask = np.bitwise_or(landmask, mask)
    pct = np.count_nonzero(landmask) / landmask.size * 100
    print(f"Land coverage: {pct:.1f}% ({np.count_nonzero(landmask)} tiles)")
    return landmask


def write_earth_file(filepath, landmask, sea_level=0.35, seed=0xC1A17A5):
    """Write the .earth binary file."""
    height, width = landmask.shape
    os.makedirs(os.path.dirname(filepath) or ".", exist_ok=True)
    with open(filepath, "wb") as f:
        header = struct.pack(
            "<IIii fI 8I",
            MAGIC, VERSION, width, height, sea_level, seed, *([0] * 8),
        )
        f.write(header)
        f.write(landmask.tobytes())
    size_mb = os.path.getsize(filepath) / (1024 * 1024)
    print(f"Written {filepath} ({size_mb:.1f} MB)")


def main():
    parser = argparse.ArgumentParser(description="Generate Earth map for Dominion")
    parser.add_argument("--width", type=int, default=2048)
    parser.add_argument("--height", type=int, default=1024)
    parser.add_argument("--output", type=str, default="data/earth_2048x1024.earth")
    parser.add_argument("--sea-level", type=float, default=0.35)
    parser.add_argument("--fallback", action="store_true",
                        help="Use built-in simplified outlines instead of Natural Earth")
    args = parser.parse_args()

    if args.fallback:
        landmask = generate_fallback_mask(args.width, args.height)
    else:
        try:
            import numpy as np
            geojson_path = download_natural_earth()
            print(f"Rasterizing to {args.width}x{args.height}...")
            landmask = rasterize_natural_earth(
                geojson_path, args.width, args.height
            )
            land = np.count_nonzero(landmask)
            print(f"Land coverage: {land / landmask.size * 100:.1f}% ({land} tiles)")
        except ImportError:
            print("numpy not available — falling back to simplified outlines")
            landmask = generate_fallback_mask(args.width, args.height)
        except Exception as e:
            print(f"Natural Earth download/rasterize failed: {e}")
            print("Falling back to simplified outlines...")
            landmask = generate_fallback_mask(args.width, args.height)

    write_earth_file(args.output, landmask, args.sea_level)
    print("Done.")


if __name__ == "__main__":
    main()

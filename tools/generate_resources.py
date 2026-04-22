#!/usr/bin/env python3
"""
Generate real-world resource distribution for Dominion.

Contains hardcoded major global deposits for 20 resource types
(oil, gas, coal, metals, minerals, etc.) mapped to the 2048x1024
equirectangular grid.

Each deposit is defined by lat/lon center, radius in km, estimated
total quantity, and a quality score (0-100). Deposits are rasterized
to tile circles with radial falloff.

Output: data/resources.bin (sparse per-resource-type format)

Usage:
    python3 generate_resources.py [--width 2048] [--height 1024]
                                  [--output data/resources.bin]

Note: All data is hardcoded from publicly available geological surveys
(USGS, BP Statistical Review, etc.).
"""

import argparse
import math
import os
import struct
import sys

MAGIC = 0x52534F52  # "ROSR" little-endian
VERSION = 1

# Resource type enum (must match C enum in resource_map.h)
RESOURCE_TYPES = {
    "oil": 0,
    "natural_gas": 1,
    "coal": 2,
    "iron": 3,
    "copper": 4,
    "bauxite": 5,
    "gold": 6,
    "silver": 7,
    "uranium": 8,
    "diamonds": 9,
    "phosphates": 10,
    "rare_earth": 11,
    "lithium": 12,
    "cobalt": 13,
    "tin": 14,
    "zinc": 15,
    "lead": 16,
    "nickel": 17,
    "chromite": 18,
    "manganese": 19,
}

# Major global deposits.
# Format: (name, lat, lon, radius_km, resource_type, quantity, quality)
# quantity: estimated tonnes (or barrels for oil, cubic meters for gas)
# quality: 0-100 score

DEPOSITS = [
    # ── OIL (conventional reserves + major fields) ─────────────────────
    ("Ghawar Field", 25.4, 49.6, 120, "oil", 75000000000, 88),
    ("Burgan Field", 29.1, 47.9, 60, "oil", 35000000000, 85),
    ("Safaniya Field", 28.3, 48.8, 50, "oil", 30000000000, 82),
    ("Rumaila Field", 30.2, 47.4, 50, "oil", 17000000000, 80),
    ("West Qurna", 30.5, 47.3, 30, "oil", 15000000000, 78),
    ("Kirkuk Field", 35.5, 44.2, 30, "oil", 10000000000, 75),
    ("Ahvaz Field", 31.3, 48.7, 40, "oil", 20000000000, 80),
    ("Gachsaran Field", 30.2, 50.8, 40, "oil", 15000000000, 78),
    ("Marun Field", 31.2, 49.3, 30, "oil", 12000000000, 76),
    ("Cantarell Field", 19.5, -92.0, 40, "oil", 8000000000, 72),
    ("Prudhoe Bay", 70.3, -148.5, 60, "oil", 12000000000, 70),
    ("Samotlor Field", 61.0, 76.0, 40, "oil", 15000000000, 72),
    ("Daqing Field", 46.5, 125.0, 40, "oil", 8000000000, 68),
    ("Tengiz Field", 46.2, 53.3, 30, "oil", 9000000000, 75),
    ("Venezuela Orinoco Belt", 8.0, -63.5, 150, "oil", 300000000000, 62),
    ("Canadian Oil Sands", 57.0, -111.5, 200, "oil", 170000000000, 55),
    ("Libyan Murzuq Basin", 26.0, 16.0, 120, "oil", 25000000000, 78),
    ("Nigerian Niger Delta", 5.0, 6.5, 100, "oil", 37000000000, 82),
    ("Angolan Offshore", -8.0, 12.5, 80, "oil", 12000000000, 76),
    ("North Sea Brent", 58.0, 1.5, 100, "oil", 8000000000, 72),
    ("US Permian Basin", 32.0, -103.0, 100, "oil", 20000000000, 74),
    ("Gulf of Mexico", 27.0, -91.0, 120, "oil", 10000000000, 70),
    ("Brazilian Pre-Salt", -22.0, -39.5, 120, "oil", 15000000000, 76),
    ("Kashagan Field", 45.0, 52.0, 30, "oil", 13000000000, 74),

    # ── NATURAL GAS ────────────────────────────────────────────────────
    ("South Pars / North Dome", 26.5, 52.0, 80, "natural_gas", 50000000000000, 92),
    ("Urengoy Field", 66.5, 78.0, 60, "natural_gas", 10000000000000, 85),
    ("Yamburg Field", 67.5, 75.0, 40, "natural_gas", 8000000000000, 82),
    ("Shtokman Field", 73.0, 43.0, 50, "natural_gas", 4000000000000, 78),
    ("Groningen Field", 53.2, 6.8, 25, "natural_gas", 2800000000000, 80),
    ("Marine Marcellus Shale", 41.0, -77.0, 120, "natural_gas", 3500000000000, 70),
    ("Qatari North Field Ext.", 26.0, 51.5, 40, "natural_gas", 25000000000000, 90),
    ("Turkmen Galkynysh", 37.0, 61.0, 30, "natural_gas", 5000000000000, 82),
    ("Algerian Hassi R'Mel", 32.5, 3.5, 30, "natural_gas", 3500000000000, 80),
    ("Australian NW Shelf", -19.0, 116.0, 100, "natural_gas", 4000000000000, 76),
    ("Bovanenkovo Field", 70.3, 68.0, 40, "natural_gas", 5000000000000, 78),
    ("Mozambique Rovuma Basin", -11.5, 40.5, 60, "natural_gas", 3500000000000, 82),

    # ── COAL ───────────────────────────────────────────────────────────
    ("Appalachian Basin", 38.0, -82.0, 120, "coal", 200000000000, 78),
    ("Powder River Basin", 44.0, -106.0, 100, "coal", 150000000000, 74),
    ("Illinois Basin", 38.5, -88.0, 80, "coal", 100000000000, 72),
    ("Kuznetsk Basin", 54.0, 86.0, 80, "coal", 180000000000, 76),
    ("Shanxi Province", 37.5, 112.0, 100, "coal", 200000000000, 75),
    ("Inner Mongolia Fields", 43.0, 115.0, 120, "coal", 150000000000, 72),
    ("Australian Bowen Basin", -23.0, 148.0, 80, "coal", 80000000000, 82),
    ("Witbank Coalfield", -26.0, 29.5, 60, "coal", 50000000000, 78),
    ("Indian Jharia Coalfield", 23.8, 86.5, 40, "coal", 40000000000, 74),
    ("Donets Basin", 48.0, 38.0, 50, "coal", 50000000000, 70),
    ("German Rhineland Lignite", 51.0, 6.5, 30, "coal", 40000000000, 72),
    ("Polish Silesia", 50.2, 19.0, 30, "coal", 20000000000, 74),
    ("Indonesian Kalimantan", 0.0, 115.0, 100, "coal", 40000000000, 68),

    # ── IRON ORE ───────────────────────────────────────────────────────
    ("Pilbara Region", -22.0, 118.0, 150, "iron", 50000000000, 85),
    ("Carajas Mine", -6.0, -50.5, 60, "iron", 25000000000, 82),
    ("Minas Gerais Quadrilatero", -20.0, -44.0, 80, "iron", 15000000000, 78),
    ("Kursk Magnetic Anomaly", 51.5, 37.0, 100, "iron", 30000000000, 72),
    ("Krivoy Rog Basin", 48.0, 33.5, 40, "iron", 10000000000, 74),
    ("Chinese Anshan-Benxi", 41.0, 123.0, 60, "iron", 12000000000, 65),
    ("Swedish Kiruna", 68.0, 20.0, 30, "iron", 3000000000, 90),
    ("Indian Odisha Belt", 21.5, 85.0, 60, "iron", 8000000000, 72),
    ("South African Sishen", -28.0, 23.5, 40, "iron", 5000000000, 78),
    ("Canadian Labrador Trough", 54.0, -67.0, 80, "iron", 10000000000, 70),
    ("Mauritanian Zouerate", 22.5, -13.0, 40, "iron", 3000000000, 76),
    ("Ukrainian Poltava", 49.5, 34.0, 30, "iron", 4000000000, 70),

    # ── COPPER ─────────────────────────────────────────────────────────
    ("Chilean Escondida", -24.3, -69.0, 30, "copper", 100000000, 92),
    ("Chuquicamata", -22.3, -69.0, 25, "copper", 80000000, 88),
    ("Collahuasi", -21.0, -68.5, 20, "copper", 60000000, 85),
    ("Grasberg", -4.0, 137.0, 20, "copper", 50000000, 80),
    ("Oyu Tolgoi", 43.0, 107.0, 25, "copper", 40000000, 76),
    ("Morenci", 33.0, -109.5, 20, "copper", 30000000, 72),
    ("Kennecott Bingham", 40.5, -112.2, 15, "copper", 25000000, 74),
    ("Zambian Copperbelt", -13.0, 28.5, 60, "copper", 40000000, 70),
    ("Congolese Katanga", -10.5, 26.0, 60, "copper", 50000000, 75),
    ("Peruvian Cerro Verde", -16.5, -71.5, 20, "copper", 20000000, 76),
    ("Antamina", -9.5, -77.0, 15, "copper", 15000000, 78),
    ("Russian Norilsk", 69.3, 88.0, 30, "copper", 30000000, 68),
    ("Polish Legnica-Glogow", 51.5, 16.0, 25, "copper", 20000000, 72),
    ("Kazakh Zhezkazgan", 47.8, 67.7, 25, "copper", 15000000, 70),

    # ── BAUXITE (Aluminum ore) ─────────────────────────────────────────
    ("Guinea Boke Region", 10.5, -14.0, 100, "bauxite", 7400000000, 82),
    ("Australian Weipa", -12.5, 142.0, 60, "bauxite", 3000000000, 78),
    ("Darling Range", -33.0, 116.5, 50, "bauxite", 2000000000, 74),
    ("Brazilian Trombetas", -1.5, -56.5, 50, "bauxite", 2000000000, 72),
    ("Jamaican Central Plateau", 18.2, -77.3, 30, "bauxite", 600000000, 76),
    ("Indian Eastern Ghats", 19.0, 83.0, 50, "bauxite", 1500000000, 68),
    ("Vietnam Central Highlands", 14.0, 108.0, 40, "bauxite", 2000000000, 70),
    ("Chinese Guangxi", 23.0, 107.5, 40, "bauxite", 800000000, 66),
    ("Suriname Bakhuis", 4.5, -56.5, 30, "bauxite", 400000000, 72),
    ("Russian North Urals", 60.0, 60.0, 40, "bauxite", 500000000, 68),
    ("Saudi Arabian Az Zabirah", 28.5, 43.0, 30, "bauxite", 200000000, 70),
    ("Guyana Linden", 6.0, -58.3, 25, "bauxite", 300000000, 72),

    # ── GOLD ───────────────────────────────────────────────────────────
    ("Witwatersrand Basin", -26.5, 28.0, 80, "gold", 50000, 90),
    ("Muruntau", 41.5, 64.0, 20, "gold", 4000, 85),
    ("Grasberg", -4.0, 137.0, 15, "gold", 3000, 82),
    ("Carlin Trend", 41.0, -116.0, 60, "gold", 8000, 78),
    ("Kalgoorlie Super Pit", -30.8, 121.5, 30, "gold", 2000, 82),
    ("Yanacocha", -7.0, -78.5, 20, "gold", 3000, 76),
    ("Pueblo Viejo", 19.0, -70.0, 15, "gold", 2000, 74),
    ("Kumtor", 41.8, 78.0, 15, "gold", 1500, 78),
    ("Lihir Island", -3.0, 152.5, 15, "gold", 2000, 72),
    ("Olimpiada", 60.0, 93.0, 15, "gold", 1500, 76),
    ("Ghanaian Ashanti Belt", 6.0, -2.0, 40, "gold", 3000, 74),
    ("Malian Sadiola", 13.5, -10.0, 25, "gold", 1000, 72),
    ("Canadian Red Lake", 51.0, -94.0, 20, "gold", 1000, 80),
    ("Brazilian Serra Pelada", -6.0, -50.0, 15, "gold", 500, 68),
    ("Indonesian Batu Hijau", -8.5, 117.0, 15, "gold", 1200, 72),

    # ── SILVER ─────────────────────────────────────────────────────────
    ("Cerro Rico de Potosi", -19.6, -65.8, 20, "silver", 60000, 88),
    ("Fresnillo District", 23.0, -103.0, 25, "silver", 30000, 80),
    ("Cannington", -21.5, 140.5, 15, "silver", 25000, 78),
    ("Peruvian Arcata", -15.0, -72.0, 20, "silver", 15000, 76),
    ("Bolivian San Cristobal", -21.0, -67.5, 15, "silver", 12000, 74),
    ("Mexican Penasquito", 24.5, -101.5, 15, "silver", 10000, 76),
    ("Australian Mount Isa", -20.5, 139.5, 20, "silver", 10000, 72),
    ("Polish Silesian Deposits", 50.5, 19.0, 25, "silver", 8000, 68),
    ("Russian Dukat", 62.5, 150.0, 15, "silver", 8000, 74),

    # ── URANIUM ────────────────────────────────────────────────────────
    ("Kazakh Chu-Sarysu Basin", 44.0, 67.0, 80, "uranium", 800000, 82),
    ("Australian Olympic Dam", -30.5, 137.0, 25, "uranium", 400000, 78),
    ("Canadian Athabasca Basin", 58.0, -108.0, 80, "uranium", 500000, 85),
    ("Niger Arlit", 18.5, 7.5, 50, "uranium", 200000, 72),
    ("Namibian Rossing", -22.5, 15.0, 25, "uranium", 100000, 74),
    ("Russian Streltsovsk", 50.5, 117.0, 25, "uranium", 150000, 70),
    ("South African Karoo", -32.0, 25.0, 50, "uranium", 100000, 68),
    ("Chinese Yili Basin", 43.5, 82.0, 40, "uranium", 80000, 66),
    ("Uzbek Uchkuduk", 42.0, 63.5, 30, "uranium", 120000, 72),

    # ── DIAMONDS ───────────────────────────────────────────────────────
    ("Russian Mir Pipe", 62.5, 114.0, 15, "diamonds", 200, 90),
    ("Botswana Orapa", -21.5, 25.5, 20, "diamonds", 300, 88),
    ("Jwaneng", -25.0, 25.0, 15, "diamonds", 250, 90),
    ("Canadian Diavik", 64.5, -110.0, 15, "diamonds", 100, 85),
    ("Ekati", 64.7, -110.5, 15, "diamonds", 80, 84),
    ("South African Cullinan", -25.8, 28.5, 15, "diamonds", 150, 82),
    ("Angolan Catoca", -10.0, 19.5, 20, "diamonds", 200, 78),
    ("Congolese Mbuji Mayi", -6.0, 23.5, 25, "diamonds", 300, 76),
    ("Sierra Leone Koidu", 8.5, -11.0, 20, "diamonds", 50, 74),
    ("Australian Argyle", -16.5, 128.0, 15, "diamonds", 80, 86),

    # ── PHOSPHATES ─────────────────────────────────────────────────────
    ("Morocco/Western Sahara Basin", 27.0, -10.0, 150, "phosphates", 50000000000, 88),
    ("Florida Bone Valley", 27.5, -82.0, 50, "phosphates", 5000000000, 76),
    ("Jordanian Al-Hasa", 31.5, 36.5, 30, "phosphates", 2000000000, 78),
    ("Chinese Yunnan", 24.5, 103.0, 50, "phosphates", 3000000000, 72),
    ("Russian Kola Peninsula", 67.5, 34.0, 40, "phosphates", 2000000000, 74),
    ("Tunisian Gafsa", 34.5, 8.5, 25, "phosphates", 1000000000, 76),
    ("South African Phalaborwa", -24.0, 31.0, 20, "phosphates", 500000000, 74),
    ("Brazilian Catalao", -18.0, -48.0, 25, "phosphates", 500000000, 70),
    ("Togolese Hahotoe", 6.5, 1.5, 15, "phosphates", 200000000, 72),
    ("Senegalese Taiba", 14.5, -17.0, 20, "phosphates", 300000000, 74),

    # ── RARE EARTH ELEMENTS ────────────────────────────────────────────
    ("Bayan Obo", 41.8, 110.0, 30, "rare_earth", 48000000, 82),
    ("Mountain Pass", 35.5, -115.5, 15, "rare_earth", 1800000, 76),
    ("Mount Weld", -28.8, 122.5, 15, "rare_earth", 2000000, 78),
    ("Brazilian Araxa", -19.5, -47.0, 20, "rare_earth", 800000, 72),
    ("Indian Orissa Sands", 19.5, 85.5, 30, "rare_earth", 3000000, 70),
    ("Vietnamese Dong Pao", 22.5, 103.5, 20, "rare_earth", 2000000, 68),
    ("Greenland Kvanefjeld", 61.0, -46.0, 15, "rare_earth", 4000000, 74),
    ("Myanmar Kachin State", 25.5, 97.0, 25, "rare_earth", 1500000, 66),
    ("Burundi Gakara", -3.5, 29.5, 15, "rare_earth", 500000, 70),
    ("Tanzanian Ngualla", -7.5, 35.0, 15, "rare_earth", 800000, 72),

    # ── LITHIUM ────────────────────────────────────────────────────────
    ("Atacama Salt Flat", -23.5, -68.3, 40, "lithium", 9000000, 92),
    ("Uyuni Salt Flat", -20.0, -67.5, 60, "lithium", 10000000, 85),
    ("Australian Greenbushes", -34.0, 116.0, 20, "lithium", 4000000, 88),
    ("Chinese Zabuye Salt Lake", 31.5, 84.0, 20, "lithium", 2000000, 75),
    ("Hombre Muerto", -25.5, -66.5, 20, "lithium", 2000000, 82),
    ("Olaroz", -23.8, -66.5, 15, "lithium", 1500000, 80),
    ("Portuguese Barroso", 41.5, -7.5, 10, "lithium", 500000, 74),
    ("Zimbabwe Bikita", -20.0, 31.5, 15, "lithium", 800000, 70),
    ("Canadian Quebec Lithium", 48.5, -77.0, 20, "lithium", 1000000, 72),
    ("Mali Goulamina", 12.5, -7.0, 15, "lithium", 500000, 74),
    ("Serbian Jadar", 44.5, 19.5, 10, "lithium", 800000, 76),
    ("Afghan Helmand Basin", 31.0, 64.0, 25, "lithium", 2000000, 65),

    # ── COBALT ─────────────────────────────────────────────────────────
    ("Congolese Katanga Copperbelt", -10.5, 26.0, 80, "cobalt", 3500000, 78),
    ("Moroccan Bou Azzer", 31.0, -7.0, 15, "cobalt", 500000, 72),
    ("Australian Murrin Murrin", -28.5, 121.5, 15, "cobalt", 200000, 70),
    ("Canadian Cobalt District", 47.5, -79.5, 25, "cobalt", 300000, 74),
    ("Cuban Moa Bay", 20.5, -75.0, 20, "cobalt", 400000, 68),
    ("Papua New Guinea Ramu", -5.5, 146.0, 20, "cobalt", 200000, 66),
    ("Russian Norilsk", 69.3, 88.0, 30, "cobalt", 500000, 70),
    ("Chinese Qinghai", 36.0, 98.0, 30, "cobalt", 300000, 64),
    ("Indonesian Sulawesi", -2.0, 121.5, 30, "cobalt", 500000, 66),
    ("Philippine Palawan", 10.0, 118.5, 25, "cobalt", 300000, 68),

    # ── TIN ────────────────────────────────────────────────────────────
    ("Chinese Dachang", 25.0, 108.0, 25, "tin", 2000000, 78),
    ("Gejiu District", 23.5, 103.0, 25, "tin", 1500000, 76),
    ("Indonesian Bangka Island", -2.0, 106.0, 30, "tin", 1500000, 72),
    ("Peruvian San Rafael", -14.5, -70.0, 15, "tin", 500000, 74),
    ("Bolivian Huanuni", -18.0, -66.5, 15, "tin", 400000, 72),
    ("Brazilian Pitinga", -1.0, -60.0, 20, "tin", 600000, 70),
    ("Malaysian Perak", 4.5, 101.0, 20, "tin", 300000, 68),
    ("Myanmar Wa State", 22.0, 98.5, 20, "tin", 400000, 66),
    ("Congolese Maniema", -3.0, 27.0, 25, "tin", 300000, 64),
    ("Rwandan Tin Belt", -2.0, 29.5, 15, "tin", 200000, 66),

    # ── ZINC ───────────────────────────────────────────────────────────
    ("Australian Mount Isa", -20.5, 139.5, 25, "zinc", 15000000, 82),
    ("Broken Hill", -32.0, 141.5, 20, "zinc", 10000000, 80),
    ("McArthur River", -16.5, 136.0, 20, "zinc", 8000000, 78),
    ("Red Dog", 68.0, -163.0, 20, "zinc", 8000000, 76),
    ("Peruvian Antamina", -9.5, -77.0, 15, "zinc", 5000000, 74),
    ("Chinese Jinding", 26.5, 99.5, 20, "zinc", 5000000, 72),
    ("Irish Navan", 53.5, -7.0, 15, "zinc", 3000000, 74),
    ("Kazakh Ridder-Sokol", 50.3, 83.5, 20, "zinc", 4000000, 70),
    ("Indian Rampura Agucha", 25.5, 74.5, 15, "zinc", 3000000, 72),
    ("Mexican Penasquito", 24.5, -101.5, 15, "zinc", 2000000, 68),

    # ── LEAD ───────────────────────────────────────────────────────────
    ("Australian Mount Isa", -20.5, 139.5, 25, "lead", 10000000, 80),
    ("Broken Hill", -32.0, 141.5, 20, "lead", 8000000, 78),
    ("Missouri Lead Belt", 37.5, -90.5, 40, "lead", 8000000, 76),
    ("Peruvian Cerro de Pasco", -10.5, -76.0, 15, "lead", 3000000, 72),
    ("Chinese Fankou", 25.0, 113.5, 15, "lead", 3000000, 70),
    ("Kazakh Tekeli", 44.5, 79.0, 15, "lead", 2000000, 68),
    ("Swedish Laisvall", 65.5, 17.0, 15, "lead", 2000000, 72),
    ("Russian Dalnegorsk", 44.5, 135.5, 15, "lead", 2500000, 68),
    ("Mexican Naica", 28.0, -105.5, 10, "lead", 1500000, 70),
    ("Indian Zawar", 24.5, 74.0, 15, "lead", 2000000, 68),

    # ── NICKEL ─────────────────────────────────────────────────────────
    ("Russian Norilsk", 69.3, 88.0, 50, "nickel", 10000000, 82),
    ("Canadian Sudbury Basin", 46.5, -81.0, 40, "nickel", 6000000, 78),
    ("Indonesian Sulawesi", -2.0, 121.5, 40, "nickel", 5000000, 72),
    ("Australian Kambalda", -31.0, 121.5, 25, "nickel", 3000000, 76),
    ("Philippine Surigao", 9.5, 125.5, 25, "nickel", 3000000, 68),
    ("Cuban Nicaro", 21.0, -76.0, 20, "nickel", 2000000, 70),
    ("Brazilian Barro Alto", -15.5, -49.0, 20, "nickel", 2000000, 68),
    ("New Caledonian Goro", -22.0, 167.0, 20, "nickel", 3000000, 74),
    ("Guatemalan El Estor", 15.5, -89.5, 15, "nickel", 1000000, 66),
    ("Tanzanian Kabanga", -3.0, 31.0, 15, "nickel", 1000000, 70),

    # ── CHROMITE ───────────────────────────────────────────────────────
    ("South African Bushveld Complex", -25.5, 28.0, 120, "chromite", 5000000000, 85),
    ("Zimbabwean Great Dyke", -20.0, 30.0, 80, "chromite", 1000000000, 78),
    ("Kazakh Donskoy", 50.3, 59.0, 40, "chromite", 500000000, 76),
    ("Indian Sukinda Valley", 21.0, 85.5, 30, "chromite", 300000000, 74),
    ("Turkish Elazig-Guleman", 38.5, 39.5, 30, "chromite", 200000000, 72),
    ("Finnish Kemi", 65.5, 24.5, 15, "chromite", 100000000, 74),
    ("Albanian Bulqiza", 41.5, 20.5, 15, "chromite", 80000000, 70),
    ("Philippine Zambales", 15.5, 120.0, 25, "chromite", 150000000, 68),
    ("Brazilian Campo Formoso", -10.5, -40.5, 20, "chromite", 80000000, 66),
    ("Omani Semail", 23.0, 57.5, 25, "chromite", 150000000, 72),

    # ── MANGANESE ──────────────────────────────────────────────────────
    ("South African Kalahari Field", -27.5, 23.0, 80, "manganese", 5000000000, 84),
    ("Australian Groote Eylandt", -14.0, 136.5, 25, "manganese", 800000000, 80),
    ("Gabonese Moanda", -1.5, 13.5, 25, "manganese", 500000000, 78),
    ("Ghanaian Nsuta", 5.5, -2.0, 15, "manganese", 200000000, 74),
    ("Ukrainian Nikopol", 47.5, 34.5, 30, "manganese", 300000000, 72),
    ("Georgian Chiatura", 42.3, 43.3, 15, "manganese", 200000000, 74),
    ("Indian Balaghat", 21.5, 80.0, 20, "manganese", 200000000, 70),
    ("Brazilian Azul", -6.5, -50.5, 20, "manganese", 300000000, 68),
    ("Chinese Guangxi", 23.0, 107.5, 30, "manganese", 200000000, 64),
    ("Mexican Molango", 20.5, -98.5, 20, "manganese", 150000000, 68),
    ("Ivorian Bondoukou", 8.0, -3.0, 15, "manganese", 100000000, 70),
    ("Kazakh Ushkatyn", 48.0, 68.0, 20, "manganese", 150000000, 66),
]


def lonlat_to_tile(lon, lat, width, height):
    x = int((lon + 180.0) / 360.0 * width) % width
    y = int((90.0 - lat) / 180.0 * height)
    return x, max(0, min(height - 1, y))


def km_to_tiles(km, width, height):
    """Approximate km to tile count at the equator."""
    earth_circumference = 40075.0
    km_per_tile = earth_circumference / width
    return max(1, int(km / km_per_tile))


def rasterize_deposit_circle(accumulator, resource_idx, cx, cy, radius_tiles,
                              quantity, quality, width, height):
    """Fill a circle of tiles with weighted quantity, summing into accumulator."""
    for dy in range(-radius_tiles, radius_tiles + 1):
        ty = cy + dy
        if ty < 0 or ty >= height:
            continue
        for dx in range(-radius_tiles, radius_tiles + 1):
            tx = (cx + dx) % width
            dist = math.sqrt(dx * dx + dy * dy)
            if dist <= radius_tiles:
                falloff = 1.0 - (dist / (radius_tiles + 1))
                qty = int(quantity * falloff)
                qual = int(quality * (0.5 + 0.5 * falloff))
                if qty > 0:
                    key = (resource_idx, ty, tx)
                    prev = accumulator.get(key)
                    if prev:
                        prev[0] = min(65535, prev[0] + qty)
                        prev[1] = max(prev[1], qual)
                    else:
                        accumulator[key] = [min(65535, qty), qual]


def generate_resource_map(deposits, width, height):
    """Generate sparse resource map from deposit definitions."""
    accumulator = {}
    total = len(deposits)

    for i, dep in enumerate(deposits):
        name, lat, lon, radius_km, rtype, quantity, quality = dep
        if i % 25 == 0:
            print(f"  Processing deposit {i+1}/{total}...")

        cx, cy = lonlat_to_tile(lon, lat, width, height)
        radius_tiles = km_to_tiles(radius_km, width, height)
        resource_idx = RESOURCE_TYPES[rtype]

        rasterize_deposit_circle(
            accumulator, resource_idx, cx, cy, radius_tiles,
            quantity, quality, width, height
        )

    # Group by resource type
    by_type = {i: [] for i in range(len(RESOURCE_TYPES))}
    for (ridx, y, x), (qty, qual) in accumulator.items():
        by_type[ridx].append((x, y, qty, qual))

    return by_type


def write_resources_bin(filepath, by_type, width, height):
    """Write resources.bin sparse binary format."""
    os.makedirs(os.path.dirname(filepath) or ".", exist_ok=True)

    with open(filepath, "wb") as f:
        f.write(struct.pack("<I", MAGIC))
        f.write(struct.pack("<I", VERSION))
        f.write(struct.pack("<I", width))
        f.write(struct.pack("<I", height))
        f.write(struct.pack("<I", len(RESOURCE_TYPES)))

        for rtype_idx in range(len(RESOURCE_TYPES)):
            entries = by_type[rtype_idx]
            f.write(struct.pack("<H", len(entries)))
            for x, y, qty, qual in entries:
                f.write(struct.pack("<HHH B", x, y, qty, qual))

    size_kb = os.path.getsize(filepath) / 1024
    total_entries = sum(len(v) for v in by_type.values())
    print(f"Written {filepath} ({size_kb:.1f} KB, {total_entries} total deposits)")


def main():
    parser = argparse.ArgumentParser(
        description="Generate resource distribution for Dominion"
    )
    parser.add_argument("--width", type=int, default=2048)
    parser.add_argument("--height", type=int, default=1024)
    parser.add_argument("--output", type=str, default="data/resources.bin")
    args = parser.parse_args()

    print(f"Processing {len(DEPOSITS)} deposits ({len(RESOURCE_TYPES)} resource types)...")
    by_type = generate_resource_map(DEPOSITS, args.width, args.height)
    write_resources_bin(args.output, by_type, args.width, args.height)
    print("Done.")


if __name__ == "__main__":
    main()

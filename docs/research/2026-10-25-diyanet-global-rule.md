# Diyanet global rule

**Date:** 2026-10-26

This note records how `HIJRI_AUTHORITY_DIYANET` computes Diyanet's calendar from `hijri.h` v0.5.0 on, and what that computation was measured against. It replaces the published table that v0.4.1 read, whose sources are in `docs/research/2026-10-17-diyanet-calendar.md`. That note's two source lists remain the fixtures.

## The rule as implemented

Diyanet's bulletin "Beginning of the month of Ramadan 1447 AH" (https://diyanetamerica.org/wp-content/uploads/2026/02/BEGINNING-OF-THE-MONTH-OF-RAMADAN-1447-AH-1.pdf) states the 2016 Istanbul rule as four conditions, quoted in the earlier note. It leaves the frame of each angle, the search window, and how the Americas condition combines with visibility elsewhere unstated. The library reads it as follows, and the score below is what chose that reading.

Day D begins a month when the evening of D-1 satisfies A or B.

- A: some point whose sunset falls before 00:00 UT of D passes.
- B: some point on the Americas mainland passes, and the conjunction precedes dawn in Wellington on its local date D.

A point passes when the conjunction precedes its sunset and `hijri_local_predicate_evaluate` returns 1 for the entry's predicate, `HIJRI_PREDICATE_ALTITUDE_5_ELONGATION_8`. That means a topocentric unrefracted centre altitude of at least 5 degrees and a geocentric elongation of at least 8 degrees, at a sunset found with `HIJRI_SUNSET_CONVENTION_ASTRONOMICAL` at sea level. The month starts on the first such D among the five days from the floor of the conjunction's Julian Day. The Hijri label comes from the tabular calendar five days into the month, as for every computed entry.

Wellington is 41.2865 S, 174.7762 E. Its dawn is the Sun's centre rising through -18 degrees, found by bisection between local mean midnight and noon. The angle is Diyanet's own. Its written statement, quoted by Takvim on 2013-07-17 (https://www.takvim.com.tr/Guncel/2013/07/17/diyanetten-imsak-vakti-aciklamasi-1374068706), reads "astronomik tanın başlangıcı olan 18º'yi esas almaktadır", and says 18 degrees replaced 19 degrees in 1982. Diyanet's published imsak times add temkin minutes, which the rule as stated does not mention. The dawn here applies none.

## The search

B is tried first. It evaluates the westernmost Americas mainland point at every whole degree of latitude for which the mainland has one: 125 points, 53 S to 71 N. Islands are excluded, Tierra del Fuego among them, and the search stops at the first point that passes.

A evaluates one point per latitude, every 4 degrees from 60 S to 60 N. At each latitude it bisects longitude over [-180, 0] eleven times for the westernmost point whose sunset is still before 00:00 UT of D, and evaluates that point. The search takes that point as the latitude's best case, since among the points that qualify it has the latest sunset and so the oldest Moon. The score below is what supports that choice.

Before either search, a candidate day is rejected outright when the conjunction is at or after its 00:00 UT. Both clauses need the conjunction before an earlier instant, so this changes no answer.

## The coast table

`hijri__americas_west_coast` was derived from Natural Earth 1:50m land, `ne_50m_land.geojson` from https://github.com/nvkelso/natural-earth-vector (raw file `geojson/ne_50m_land.geojson`), which is in the public domain. The file used has SHA-256 e874b27a51d146452be360cafb3cc50c86001074a67d534113e6534682f9826b. The script finds the one polygon ring containing both (-100, 40) and (-60, -10), which is the Americas mainland, ring 1200 with 9378 vertices. For each latitude it prints the smallest longitude at which an edge of that ring crosses it, or `nan` when none does. Run as `python3 -I coast.py ne_50m_land.geojson 1`. The rows printed as `nan` were 56 S, 55 S, 54 S and 72 N, and the table holds the other 125, rounded to 0.01 degrees as printed.

```python
import json, sys
d = json.load(open(sys.argv[1]))
rings = []
for f in d["features"]:
    g = f["geometry"]
    polys = [g["coordinates"]] if g["type"] == "Polygon" else g["coordinates"]
    for p in polys:
        rings.append(p[0])
def inside(r, x, y):
    c = False
    for i in range(len(r)):
        x1, y1 = r[i]; x2, y2 = r[i-1]
        if (y1 > y) != (y2 > y) and x < x1 + (y - y1) * (x2 - x1) / (y2 - y1):
            c = not c
    return c
na = [i for i, r in enumerate(rings) if inside(r, -100, 40)]
sa = [i for i, r in enumerate(rings) if inside(r, -60, -10)]
print("NA ring", na, "SA ring", sa, "sizes", [len(rings[i]) for i in set(na + sa)], file=sys.stderr)
ids = sorted(set(na + sa))
step = float(sys.argv[2])
lat = -56.0
while lat <= 72.0 + 1e-9:
    west = None
    for i in ids:
        r = rings[i]
        for k in range(len(r)):
            x1, y1 = r[k]; x2, y2 = r[k-1]
            if (y1 > lat) != (y2 > lat):
                x = x1 + (lat - y1) * (x2 - x1) / (y2 - y1)
                if west is None or x < west: west = x
    print("%.1f %s" % (lat, "nan" if west is None else "%.2f" % west))
    lat += step
```

## Score

The record is the union of the two Diyanet source lists, 186 distinct month starts from 2021-01-14 to 2035-12-31. A start counts as reproduced when it converts to day 1 of its Hijri month and the day before converts to a different month. `test_authority_registry` recomputes the count and asserts it equals the registry row.

The rule reproduces 184 of the 186. It starts 1 Zilkade 1442 (2021-06-11) a day late and 1 Zilkade 1444 (2023-05-21) a day early.

Variants measured on the way, all against the same 186 starts:

| Variant | Score |
|---|---|
| As shipped | 184 |
| Topocentric elongation, real coastline (measured 2026-10-17 with an earlier prototype) | 177 |
| Search starting the day after the conjunction's floor | 180 |
| Americas clause disabled (the recorded mutation) | 167 |

The earlier prototype's 183 with topocentric elongation came from a longitude and latitude box that counted open Pacific as mainland. With the real coastline topocentric elongation falls to 177, and geocentric elongation, the predicate's own frame, gives 184.

These 186 months do not test the Wellington clause. Neither a fixed 17:00 UT cutoff nor no cutoff at all changed the score in the 2026-10-17 runs. The clause is kept because the bulletin states it.

## Speed

Measured 2026-10-26 on one core at `nice -n 19`, `gcc -O2`, through `hijri_authority_from_gregorian`, over every row of both lists and the day before each: 422 conversions, mean 9.13 ms. The search order was chosen by timing the converter directly over the 186 starts and the days before them, 372 calls, all three scoring 184:

| Search | Mean per conversion |
|---|---|
| A first, 2 degree latitude step | 16.67 ms |
| A first, 4 degree latitude step | 9.54 ms |
| B first, 4 degree latitude step (shipped) | 7.40 ms |

The conjunction prune took the first row down from 26.96 ms. Nothing is cached, so every conversion repeats its search.

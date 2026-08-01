# A validated Kemenag/Indonesia reference for MABIMS 2021

**Date:** 2026-08-01

## What Kemenag actually publishes

The Ministry of Religious Affairs (Kemenag RI, Ditjen Bimas Islam) issues an
annual **Kalender Hijriah Indonesia** — a computed calendar prepared by its
Hisab Rukyat team with Islamic-astronomy experts from multiple organisations,
using the MABIMS 2021 criterion (altitude ≥ 3°, elongation ≥ 6.4°) evaluated
"di seluruh wilayah Indonesia" (across the whole territory, an aggregation
rule, not a single station). The 2026 edition is hosted on the official
e-literasi portal:
`https://simbi.kemenag.go.id/eliterasi/portal-web/buku-digital/kalender-hijriah-indonesia-2026-68e5d49985916`
(a JS flipbook; the raw PDF is not directly linkable, so month starts below
were taken from transcriptions and cross-checked as described).

Ramadan, Syawal, and Dzulhijjah remain subject to the itsbat session (rukyat
plus ruling); the printed calendar is the computed baseline.

## The reference table, and how it was validated

37 month starts, Rajab 1445 (2024-01-13) through Rajab 1448 (2026-12-10),
taken from al-habib.info's transcriptions of the official releases
(pages state "merujuk kepada rilis resmi Ditjen Bimas Islam / Kementerian
Agama RI") for 2024, 2025, 2026.

**Oracle rule applied before any measurement** (per
[2026-08-01-umm-al-qura-oracle.md](2026-08-01-umm-al-qura-oracle.md)):
cross-checked against SEVEN independently documented government announcements,
all matching the table exactly:

| Event | Announced | Table |
|---|---|---|
| 1 Ramadan 1445 (itsbat) | 2024-03-12 | ✓ |
| 1 Syawal 1445 (itsbat) | 2024-04-10 | ✓ |
| 1 Ramadan 1446 (itsbat) | 2025-03-01 | ✓ |
| 1 Syawal 1446 (itsbat) | 2025-03-31 | ✓ |
| 1 Ramadan 1447 (itsbat, setneg.go.id) | 2026-02-19 | ✓ |
| 1 Dzulhijjah 1447 (Kemenag) | 2026-05-18 | ✓ |
| 1 Muharram 1448 (Kemenag) | 2026-06-16 | ✓ |

The two months that later turned out astronomically puzzling (Rajab 1447 =
2025-12-21 and Rajab 1448 = 2026-12-10) were additionally confirmed against a
second independent transcription (detik.com's month-by-month listing: 1 Jan
2026 = 12 Rajab 1447 and 31 Dec 2026 = 22 Rajab 1448 imply exactly those
starts). Residual risk: an ordinary, never-announced month mis-transcribed
identically by both secondary sources; retrieving the official PDF would
close it and is left as a follow-up.

## Measurement: single-point MABIMS 2021 vs the official calendar

For each official start S, the decision evening is S−1. Measured with the
library's `HIJRI_PREDICATE_MABIMS_2021` (topocentric elongation, geometric
centre altitude):

```
                       decision evening      evening BEFORE that
                       passes (supports S)   passes (would be a day early)
  Sabang (95.32E, west tip)   33/37                 0/37
  Banda Aceh                  32/37                 0/37
  Jakarta                     32/37                 0/37
  Pelabuhan Ratu              32/37                 0/37
```

**Never one day early, anywhere, in 37 months.** The westernmost point
supports the most starts, as crescent geometry predicts for an
anywhere-in-Indonesia rule.

### The four unsupported months (Sabang, decision evening)

```
  start        alt      topo elong   geo elong
  2025-03-01   +4.51      +5.40        +6.40-   (Ramadan 1446; itsbat-confirmed)
  2025-12-21   +2.19      +5.97        +6.47    (Rajab 1447)
  2026-06-16   +3.82      +6.17        +6.97    (Muharram 1448; Kemenag-announced)
  2026-12-10   +2.09      +6.02        +6.50    (Rajab 1448)
```

Findings, stated carefully:

- All four fail the **topocentric** elongation threshold; scoring with
  **geocentric** elongation instead supports one more month (34/37) and puts
  1 Ramadan 1446 *exactly at* the 6.4° boundary (6.3996°) — within the
  library's ephemeris error of the threshold. The
  topocentric-vs-geocentric convention question for Kemenag's own
  implementation remains open and now has a measurable consequence.
- The two Rajab months fail **altitude** (≈2.1–2.2°) at Indonesia's
  westernmost point, ~0.85° below the criterion — too large for any
  refraction/limb convention to bridge (mar'i credits reach ~+0.65° at that
  altitude). Istikmal forcing is ruled out: neither follows a 30-day month.
  Conclusion: the official calendar contains months whose starts a
  single-point MABIMS-2021 computation does not support under any convention
  tested. Whether this reflects a different computational convention inside
  Kemenag's toolchain, a committee decision, or something else is unresolved.

## What a library claim can honestly say

- Supported: "never contradicts the official calendar in the early
  direction" (0/37 both conventions) and "supports ≥33 of 37 official month
  starts at the westernmost point" — both as committed fixture assertions.
- Not supported: any claim of reproducing the Kemenag calendar exactly. The
  README's existing stance — a local predicate is not an authority decision —
  is confirmed by measurement, with the four cases quantified.

## Follow-ups

- Retrieve the official PDF (simbi flipbook) to replace secondary
  transcriptions for never-announced months.
- Resolve Kemenag's elongation convention (topocentric per Djamaluddin's
  MABIMS statement, but geocentric fits the calendar slightly better) —
  primary source would be the Hisab Rukyat team's computation guide
  ("Ephemeris Hisab Rukyat" methodology).
- Extend the table back to 2023 (first year the new criterion was in force)
  when a validated source for it is found (the 2023 transcription page is no
  longer online).

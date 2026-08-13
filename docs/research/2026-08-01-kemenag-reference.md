# A validated Kemenag/Indonesia reference for MABIMS 2021

**Date:** 2026-08-01

## What Kemenag actually publishes

The Ministry of Religious Affairs (Kemenag RI, Ditjen Bimas Islam) issues an
annual **Kalender Hijriah Indonesia**, a computed calendar prepared by its
Hisab Rukyat team with Islamic-astronomy experts from multiple organisations,
using the MABIMS 2021 criterion (altitude ≥ 3°, elongation ≥ 6.4°) evaluated
"di seluruh wilayah Indonesia" (across the whole territory, an aggregation
rule, not a single station). The 2026 edition is hosted on the official
e-literasi portal:
`https://simbi.kemenag.go.id/eliterasi/portal-web/buku-digital/kalender-hijriah-indonesia-2026-68e5d49985916`
(a JS flipbook, and the raw PDF is not directly linkable, so month starts below
were taken from transcriptions and cross-checked as described).

Ramadan, Syawal, and Dzulhijjah remain subject to the itsbat session (rukyat
plus ruling). The printed calendar is the computed baseline.

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
identically by both secondary sources. Retrieving the official PDF would
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
  2025-03-01   +4.51      +5.40        +6.3952   (Ramadan 1446, itsbat-confirmed)
  2025-12-21   +2.19      +5.97        +6.47    (Rajab 1447)
  2026-06-16   +3.82      +6.17        +6.97    (Muharram 1448, Kemenag-announced)
  2026-12-10   +2.09      +6.02        +6.50    (Rajab 1448)
```

Findings, stated carefully:

- All four fail the **topocentric** elongation threshold. Scoring with
  **geocentric** elongation instead supports one more month (34/37) and puts
  1 Ramadan 1446 just below the 6.4° boundary (6.3952°, 0.005° short), within the
  library's ephemeris error of the threshold. **The topocentric-vs-geocentric
  convention question for Kemenag's own implementation, recorded here as open,
  is now RESOLVED to topocentric**, measured against Kemenag's own published
  hilal table for 2023: the topocentric elongation is contained in 11 of the 12
  published ranges and the geocentric elongation in 1 of 12, worst geocentric
  excursion 0.9637°. See
  [2026-08-08-kemenag-published-quantities.md](2026-08-08-kemenag-published-quantities.md).
  That the geocentric form scores one month better against the announced starts
  is a property of the 37-month sample, not evidence about the convention.
- The two Rajab months fail **altitude** (≈2.1 to 2.2°) at Indonesia's
  westernmost point, ~0.85° below the criterion, too large for any
  refraction/limb convention to bridge (mar'i credits reach ~+0.65° at that
  altitude). Istikmal forcing is ruled out: neither follows a 30-day month.
  Conclusion: the official calendar contains months whose starts a
  single-point MABIMS-2021 computation does not support under any convention
  tested. Whether this reflects a different computational convention inside
  Kemenag's toolchain, a committee decision, or something else is unresolved.

## What a library claim can honestly say

- Supported: "never contradicts the official calendar in the early
  direction" (0/37 both conventions) and "supports ≥33 of 37 official month
  starts at the westernmost point", both as committed fixture assertions.
- Not supported: any claim of reproducing the Kemenag calendar exactly. The
  README's existing stance, that a local predicate is not an authority
  decision, is confirmed by measurement, with the four cases quantified.

## Follow-ups

- Retrieve the official PDF (simbi flipbook) to replace secondary
  transcriptions for never-announced months.
- ~~Resolve Kemenag's elongation convention.~~ DONE, 2026-08-08. The primary
  source named here as the thing to find, the Hisab Rukyat team's computation
  guide, was found and read: *Ephemeris Hisab Rukyat 2023*, page 604. The
  convention is topocentric. See
  [2026-08-08-kemenag-published-quantities.md](2026-08-08-kemenag-published-quantities.md).
- Extend the table back to 2023 (first year the new criterion was in force)
  when a validated source for it is found (the 2023 transcription page is no
  longer online).

## Convention pinned by the itsbat announcement (added same day)

For the 28 February 2025 itsbat (1 Ramadan 1446), the official announcement
stated the hilal across Indonesia at altitude 3°5.91′ to 4°40.96′ and
elongation 4°47.03′ to 6°24.14′. It was assumed here that the maxima occur at
the westernmost point, which is the assumption the two conclusions below turn
on and the reason they do not hold. Compared with this library at Sabang that
evening:

```
                        announced max      library @ Sabang     delta
  elongation            6°24.14′ = 6.4023°  geocentric  6.3952°  0.43′
                                            topocentric 5.4027°  ~60′
  altitude              4°40.96′ = 4.683°   geometric   4.507°   ~10.6′
```

Two conclusions were drawn from this comparison. **Both were wrong, and both
were corrected on 2026-08-08.** They are kept here with their refutation rather
than deleted, because the error they share is easy to repeat.

- ~~**Kemenag's operational elongation is geocentric.**~~ The announced maximum
  matches the geocentric value to 0.43 arcminutes while the topocentric value
  is a full degree away. REFUTED. Measured against Kemenag's published 2023
  hilal table, the topocentric elongation is contained in 11 of 12 published
  ranges and the geocentric elongation in 1 of 12. T. Djamaluddin's published
  statement ("elongasi toposentrik") and Kemenag's own published quantities now
  agree, and `HIJRI_PREDICATE_MABIMS_2021` as shipped is on the right side of
  the question.
- ~~**Kemenag's altitude is likely mar'i (apparent).**~~ The ~10.6′ excess over
  the geometric value is consistent with refraction at 4.5° altitude
  (~10 to 11′). REFUTED. Measured against the same published table, the shipped
  centre geometric altitude is contained in 12 of 12 published ranges, upper
  limb geometric in 8 of 12, and mar'i in 2 of 12. This was recorded here as
  the open item blocking any future Kemenag-operational predicate. It is no
  longer open, and it does not block.

**Why both were wrong.** Each conclusion compared a library value computed at
Sabang against a published RANGE ENDPOINT, which is only meaningful if Sabang
is the location where Kemenag evaluates that endpoint. It is not, and the
location producing the extreme moves from month to month under an
anywhere-in-Indonesia rule. Containment drops that assumption and reverses both
answers. The full argument, the measured tables, and a third instance of the
same mistake are in
[2026-08-08-kemenag-published-quantities.md](2026-08-08-kemenag-published-quantities.md).

The comparison still corroborates the library's ephemeris against the
government's own hisab team at the sub-arcminute level on the quantity that
decides months. `test_kemenag_official_calendar` keeps the elongation pin as
a living assertion, and `test_kemenag_published_quantities` now asserts
containment against the published ranges directly.

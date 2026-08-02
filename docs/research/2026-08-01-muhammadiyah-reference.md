# A validated Muhammadiyah reference for Wujudul Hilal

**Date:** 2026-08-01

## Scope: the wujudul-hilal era ends at 1446 H

Muhammadiyah used hisab hakiki wujudul hilal through the end of **1446 H**
and switched to **KHGT** (Kalender Hijriah Global Tunggal, the Istanbul
global-calendar criterion) from **1447 H** (mid-2025) onward — confirmed by
muhammadiyah.or.id and the 1447 H Maklumat, which is KHGT-based. Any
validation of `HIJRI_PREDICATE_WUJUDUL_HILAL` against Muhammadiyah dates
must therefore use **1446 H and earlier**. Validating against 1447+ dates
would measure the wrong criterion.

## The reference: Maklumat PP Muhammadiyah, primary source

Unlike the Kemenag case (transcribed calendar), this reference is built
entirely from **primary sources**: the annual official Maklumat documents on
muhammadiyah.or.id, which set Ramadan, Syawal, and Zulhijah and publish the
underlying hisab (ijtimak time WIB and the Moon's altitude at sunset at
Yogyakarta, φ = −07°48′, λ = 110°21′ BT). Twelve month starts, 1443–1446 H:

| Start | Date | Published decision-evening altitude | Verdict |
|---|---|---|---|
| 1 Ramadan 1443 | 2022-04-02 | +2°18′12″ | wujud |
| 1 Syawal 1443 | 2022-05-02 | +4°50′25″ | wujud |
| 1 Zulhijah 1443 | 2022-06-30 | +1°58′28″ | wujud |
| 1 Ramadan 1444 | 2023-03-23 | (in Maklumat PDF, not extracted) | wujud |
| 1 Syawal 1444 | 2023-04-21 | (PDF) | wujud |
| 1 Zulhijah 1444 | 2023-06-19 | (PDF) | wujud |
| 1 Ramadan 1445 | 2024-03-11 | **+0°56′28″** | wujud (razor-thin) |
| 1 Syawal 1445 | 2024-04-10 | ijtimak 2024-04-09 01:23 | (30-day Ramadan; day-29 evening pre-ijtimak) |
| 1 Zulhijah 1445 | 2024-06-08 | **−3°32′39″** on 06-06 | belum wujud → 30 days |
| 1 Ramadan 1446 | 2025-03-01 | +4°11′08″ | wujud |
| 1 Syawal 1446 | 2025-03-31 | **−1°59′04″** on 03-29 | belum wujud → 30 days |
| 1 Zulhijah 1446 | 2025-05-28 | +1°27′07″ | wujud |

The set includes three negative/30-day cases and one sub-degree positive —
the discriminating cases a validation actually needs. Being Maklumat-sourced,
these dates need no separate anchor validation: they ARE the announcements.

## Measurement 1: the predicate vs all 12 starts

`HIJRI_PREDICATE_WUJUDUL_HILAL` evaluated at Yogyakarta (−7°48′, 110°21′,
90 m) on each decision evening (S−1) and the evening before (S−2):

```
  supported: 12/12        one day early: 0/12
```

Every Maklumat decision of the wujudul-hilal era is reproduced, including
all three 30-day months and the +0.94° razor case.

## Measurement 2: published verdicts and quantities

All **8/8** published wujud/belum-wujud verdicts match the predicate's.

The published "tinggi Bulan" quantity does not match any single library
quantity at sub-arcminute level (unlike Kemenag's clean geocentric
elongation pin). It sits consistently **between** our geocentric true-centre
altitude and our upper-limb quantity, within ~0.1–0.5° of each:

```
  evening      published   geo-true-ctr  topo-ctr(geom)  upper-limb
  2022-04-01   +2.3033     +2.7048       +1.7607         +2.5848
  2022-05-01   +4.8403     +5.3405       +4.4257         +5.2424
  2022-06-29   +1.9744     +2.3021       +1.4034         +2.2151
  2024-03-10   +0.9411     +1.2406       +0.2167         +1.0624
  2024-06-06   −3.5442     −3.0294       −3.9990         −3.1674
  2025-02-28   +4.1856     +4.7226       +3.7196         +4.5602
  2025-03-29   −1.9844     −1.4905       −2.5083         −1.6640
  2025-05-27   +1.4519     +1.7923       +0.7792         +1.6220
```

Interpretation, stated carefully: the Maklumat chain (Pedoman procedure with
Ephemeris Hisab Rukyat data tables, dip-delayed sunset) produces a related
but not identical altitude figure; no convention knob in the library lands
on it exactly. What matters — the **verdict** on every evening, including
two negatives and a +0.94° positive whose margins exceed the ~0.5° quantity
scatter — agrees 8/8.

## Measurement 3: ijtimak pins

Eight published ijtimak times (WIB) vs the library's conjunction:

```
  published              ours        delta
  2022-04-01 13:27:13    13:23      −3.3 min
  2022-05-01 03:31:02    03:27      −3.9
  2022-06-29 09:55:07    09:51      −3.8
  2024-03-10 16:07:42    16:00      −7.1
  2024-06-06 19:39:58    19:37      −2.2
  2025-02-28 07:46:49    07:45      −1.4
  2025-03-29 17:59:51    17:58      −1.2
  2025-05-27 10:04:18    10:02      −1.4
```

Minutes-level agreement with a small systematic early bias, consistent with
our geometric (no nutation/aberration) positions versus their
apparent-position ephemeris tables. None of the deltas approaches an
ijtimak-vs-sunset boundary in this set.

## What a library claim can honestly say

- Supported: `HIJRI_PREDICATE_WUJUDUL_HILAL` at Yogyakarta reproduces
  **all twelve** Maklumat-announced month starts of the wujudul-hilal era
  (1443–1446 H), never early, and matches all eight published
  wujud/belum-wujud verdicts. Together with the Pedoman worked-example
  fixture already committed, the predicate is now validated against both the
  method's manual and its real-world decisions.
- Scope limits: the record covers the three ubudiyah months per year (the
  ones Maklumats announce), not all twelve; and from 1447 H Muhammadiyah's
  calendar is KHGT, which this predicate does not and should not claim to
  reproduce.

## Follow-ups

- Extract the 1444 H Maklumat PDF's hisab numbers to complete the pin table.
- Extend to earlier Maklumats (1442 and before) for more negatives.
- KHGT is a separate criterion (global imkan-rukyat, Istanbul 2016); if
  Muhammadiyah-post-1447 support is ever wanted, it is a new predicate with
  its own primary-source research, not a change to Wujudul Hilal.

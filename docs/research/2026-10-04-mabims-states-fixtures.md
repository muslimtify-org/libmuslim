# Fixtures for Singapore, Malaysia and Brunei

**Date:** 2026-10-04

[2026-09-25-mabims-takwim-sources.md](2026-09-25-mabims-takwim-sources.md) admitted seven announced month starts for each of Singapore, Malaysia and Brunei and measured nothing against the library. This note is that measurement. The three fixtures it backs are `test_muis_announced_starts`, `test_malaysia_announced_starts` and `test_brunei_announced_starts` in `tests/test_hijri.c`.

## What was measured

For each announced start S, `HIJRI_PREDICATE_MABIMS_2021` was evaluated through `hijri_evaluate_evening` on the evening of S minus 1, the decision evening, and on S minus 2, the evening before it. A pass on S minus 1 supports the start. A pass on S minus 2 would have started the month a day early. The sunset convention is the one the library selects for this predicate, `HIJRI_SUNSET_CONVENTION_KEMENAG`. Whether the three states use that convention was not researched, and the comment above `test_mabims_states_diverge` records that the alternative moves no month start.

The seven starts are the same in all three countries: 2024-03-12, 2024-04-10, 2025-03-02, 2025-03-31, 2025-05-29, 2026-02-19 and 2026-03-21.

| Site | Latitude | Longitude | Supported | One day early |
|---|---|---|---|---|
| Singapore | 1.3521 | 103.8198 | 7 of 7 | 0 of 7 |
| Kuala Lumpur | 3.1390 | 101.6869 | 7 of 7 | 0 of 7 |
| Langkawi | 6.3500 | 99.8000 | 7 of 7 | 0 of 7 |
| Bandar Seri Begawan | 4.9031 | 114.9398 | 7 of 7 | 0 of 7 |

Langkawi is not a fixture. It was measured because Malaysia observes at 29 sites and a point in the west of the country is where a crescent passes first.

## Why the set discriminates

A table whose every decision evening passes easily and whose every previous evening has the Moon below the horizon would prove little. Three of the seven are not like that. On the evening before the decision evening the Moon was above the horizon and under threshold, so the month ran to 30 days:

| Evening | Site | Altitude | Topocentric elongation | What fails |
|---|---|---|---|---|
| 2025-02-28 | Singapore | 4.141 | 5.125 | elongation |
| 2025-02-28 | Kuala Lumpur | 4.242 | 5.188 | elongation |
| 2025-02-28 | Bandar Seri Begawan | 3.795 | 4.683 | elongation |
| 2025-05-27 | Singapore | 2.148 | 6.272 | both |
| 2025-05-27 | Kuala Lumpur | 2.440 | 6.311 | both |
| 2025-05-27 | Langkawi | 2.880 | 6.339 | both |
| 2025-05-27 | Bandar Seri Begawan | 2.128 | 5.987 | both |
| 2026-03-19 | Singapore | 2.286 | 5.152 | both |
| 2026-03-19 | Kuala Lumpur | 2.481 | 5.199 | both |
| 2026-03-19 | Bandar Seri Begawan | 2.160 | 4.740 | both |

2025-05-27 is the closest. Langkawi sits 0.120 degrees under the altitude threshold and 0.061 degrees under the elongation threshold. Both are far outside the 0.0070 degree error bar `hijri.h` documents, so the verdict is decided and not a coin toss. This is also the month where Indonesia and the other three states part: Kemenag's calendar starts Zulhijjah 1446 on 2025-05-28, and Singapore, Malaysia and Brunei announced 2025-05-29.

2025-02-28 is the evening where Indonesia started Ramadan 1446 and the other three did not, already recorded in the sources note.

On every decision evening the lowest altitude in the set is 6.725 degrees, at Bandar Seri Begawan on 2024-04-09, more than twice the threshold. That is why the fixtures assert equality and not a floor.

## Published quantities compared, and two gaps

Two announcements publish the numbers behind the decision for 2025-02-28.

| Source | Published altitude | Library | Published elongation | Library |
|---|---|---|---|---|
| MUIS, Singapore | 4.3 | 4.141 | 5.1 | 5.125 |
| JAKIM, site not named | 4 deg 15 min = 4.250 | 4.242 at Kuala Lumpur, 4.342 at Langkawi | 5 deg 17 min = 5.283 | 5.188 at Kuala Lumpur, 5.233 at Langkawi |

The MUIS elongation agrees within its one-decimal print precision. The MUIS altitude does not: 4.141 rounds to 4.1, and 4.3 was published, a difference of about 0.16 degrees. MUIS states no refraction, limb or site convention, so the cause is not known. Refraction at that altitude is of that order, which is a candidate and not a finding.

The JAKIM altitude is bracketed by the two Malaysian sites. The JAKIM elongation is not: the library is 0.095 degrees short at Kuala Lumpur and 0.050 short at Langkawi, against a figure printed to the arcminute. The site JAKIM computed for is not named in the material read for the sources note, and the reference frame is not stated.

Both are recorded as gaps. Neither is pinned in a test, because a tolerance wide enough to pass would have no source behind it. Neither bears on the verdict that evening, since the elongation is more than a degree under 6.4 on every reading.

## What this supports and what it does not

Supported: at one site per country, the shipped predicate agrees with every one of the seven announced starts and is never a day early.

Not supported: any claim that the library reproduces the Singapore, Malaysia or Brunei calendar. Seven starts cover Ramadan, Syawal and Zulhijjah only. The other nine month names of each year have no fixture. Each announced date is also a ruling, by a Mufti or by royal proclamation, and Malaysia's rests on physical observation at 29 sites as well as calculation.

A longer run for any of the three needs the annual printed calendar read in full. Brunei's Taqwim is a text-layer PDF and is the nearest to hand.

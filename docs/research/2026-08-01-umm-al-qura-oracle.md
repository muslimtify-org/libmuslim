# Umm al-Qura: the oracle, a one-day error, and a reverted fix

**Date:** 2026-08-01

## The oracle

The reference for `hijri_umm_al_qura_from_gregorian()` is the
`islamic-umalqura` calendar in ICU/CLDR — the Umm al-Qura table shipped in
browsers and operating systems. Enumerate month starts with:

```sh
node -e '
const f=new Intl.DateTimeFormat("en-u-ca-islamic-umalqura",{year:"numeric",month:"numeric",day:"numeric",timeZone:"UTC"});
const g=(dt,t)=>+f.formatToParts(dt).find(x=>x.type===t).value;
for(let t=Date.UTC(2015,0,1);t<=Date.UTC(2030,11,31);t+=86400000){
  const dt=new Date(t);
  if(g(dt,"day")===1) console.log(dt.toISOString().slice(0,10), g(dt,"year"), g(dt,"month"));}
'
```

The 198 rows this produces are committed in `tests/test_hijri.c` as
`HIJRI_UMM_OFFICIAL`.

**Validated against five Saudi dates known independently of any calendar
table** before being admitted:

| Gregorian | Hijri | Event |
|---|---|---|
| 2025-03-01 | 1 Ramadan 1446 | fasting began |
| 2025-03-30 | 1 Shawwal 1446 | Eid al-Fitr |
| 2024-03-11 | 1 Ramadan 1445 | fasting began |
| 2024-04-10 | 1 Shawwal 1445 | Eid al-Fitr |
| 2023-06-27 | 9 Dhu al-Hijjah 1444 | Day of Arafah |

Two independent published reproductions (al-habib.info, ummalquracalendar.org)
agree with ICU on the same months.

## The one-day error this replaced

An earlier analysis used R.H. van Gent's `ummalqura_dat` array, hand-parsed
with an assumed MJD epoch of 1858-11-17. That extraction produced dates one day
LATER than the real calendar for 191 of 198 months in 2015-2030.

Every conclusion drawn from it was false:

| Claimed | Actual |
|---|---|
| 94.9% of Umm al-Qura months one day early | 92.4% exact |
| errors always early, never late | 9 late, 6 early |
| decision evening should be S+27 | S+28 is correct; S+27 scores 3.6% |
| predicate saturated, 259/259 | artifact of the shifted table |
| MABIMS, Diyanet broken the same way | never established; void |

A design was written and approved on that basis which would have driven a
working function from 92.4% to 3.6%.

**The tell was visible and was misread.** A uniform one-directional error
across three criteria, four countries and 130 years is far better explained by
one broken reference than by six broken code paths. That reasoning was written
down at the time and the wrong conclusion drawn from it.

**Rule adopted:** no oracle is admitted without an independent cross-check
against values known from outside the reference itself. The five-date check
above takes under a minute and would have caught this immediately.

## Current accuracy

198 official month starts, 2015-2030, asking what Hijri day the library assigns:

```
  per-date conjunction scan (shipping)   183 exact   6 early    9 late   92.4%
```

## The chain: measured, better on paper, reverted

Chaining month starts sequentially — deciding each month's length from the
predicate on the evening of its 29th day (`S + 28`), seeded from the tabular
calendar a few months back — scores **191/198 (96.5%)**, removing 8 of the 9
late errors. It was implemented and reverted the same day.

**Why it was reverted.** The chain's answer depends on which tabular month it
is seeded from, and the seed is derived from the query date. Two adjacent
Gregorian days that straddle a tabular month boundary get different seeds, and
their chains can converge on different month starts. The result is
non-monotonic output — consecutive days returning the same Hijri day, or
skipping one:

```
  2025-11-22 gave 1447-05-29 after 1447-05-29
  2025-12-21 gave 1447-06-29 after 1447-06-27
```

Measured over 730 consecutive days:

```
  latitude     incoherent transitions
  21.4N Mecca            0
   6.2S Jakarta          0
  51.5N London           0
  60.0N                  4
  65.0N                  4
```

The per-date scan has **zero** incoherent transitions at every latitude tested,
from Jakarta to 72N. Trading silent wrong days at mid-high latitudes for +4.1
points at Mecca is a bad trade for a calendar library.

### Two repair attempts, both measured, both rejected

**Deeper burn-in.** Reduces but never eliminates, because it does not remove
the query-dependence:

```
  burn-in   60N   65N
     3        5     5
     4        4     4
     6        1     3
     8        0     2
```

**Seed agreement.** Run the chain from two consecutive seed depths and accept
the first depth where they agree. Fixed 60N but not 65N (3 remaining) — the
scheme stops at the *first* agreeing depth, which differs between adjacent
dates, so the seeds it compares do not overlap as intended.

**Agreement across all depths.** Requiring every seed depth in the window to
agree, and failing otherwise, does reach zero incoherence — by converting wrong
answers into failures:

```
  latitude   converted   incoherent
  21.4N        730/730        0
  51.5N        730/730        0
  60.0N        560/730        0     lost 170 conversions
  65.0N        411/730        0     lost 319 conversions
```

Sound, but ~20x the compute (roughly 160 evening evaluations per conversion
against 8) and a third of high-latitude coverage gone.

### What a correct chain would need

The month start must be a function of the *calendar*, not of the query date.
Any query-derived seed can produce query-dependent answers. The designs that
actually remove the dependence:

- Chain from a fixed epoch. Deterministic and provably coherent, but O(months
  since epoch) — roughly 630 months from 1400 AH to 2030, far too slow per
  conversion without a memoised table.
- Chain from a fixed epoch once into a lookup table. Correct and fast, but
  needs global mutable state, which this header deliberately avoids.

**The 8-point gap between 183 and 191 is real, unclaimed work.** It is not a
tolerance chosen to make a test pass, and the fixture now pins the baseline so
the gap stays visible.

## Why 198/198 is unreachable from the rule

Two months resist any rule-based implementation at any precision:

- `2024-12-02` — the Moon's upper limb sits at **-0.0131°**, 47 arcseconds
  below the horizon. Inside the library's own ephemeris error.
- Six months in 2029-2030 — conjunction 8-16 h before sunset, Moon 1.0-2.5°
  altitude, moonset lag 7-15 min. Both documented Umm al-Qura conditions are
  comfortably satisfied, yet the published table gives a 30-day month. The
  published table departs from its own stated rule for these months.

## Still void, pending re-measurement

The MABIMS (Indonesia, Malaysia, Singapore) and Diyanet comparisons produced on
2026-08-01 were computed against the misread van Gent extraction. No claim
about them should be repeated until they are re-measured against a validated
reference.

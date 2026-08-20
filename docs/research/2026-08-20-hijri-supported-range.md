# What bounds hijri.h, and where

**Date:** 2026-08-20

## Question

Issue #27 lists "document the supported date and geographic range" as the first thing a 1.0.0 would need, on the grounds that grepping `hijri.h` for a range returns one line and that line states the span the ephemeris oracle happened to cover rather than a contract. A user has no documented answer for year 1200 or year 2400.

This note measures what actually bounds the header, so the answer is a contract rather than a restatement of the test grid.

## Four bounds, binding in different places

### Validated: 1900 to 2100

`tests/test_ephemeris_oracle.c` compares against 24 JPL Horizons epochs across that span, worst case 0.0051 deg of lunar longitude. Outside it nothing here has been checked against an independent ephemeris. This is the only bound that is a measurement rather than a property of an algorithm.

### Table-bound: 1882-11-12 to 2174-11-25

`hijri_umm_al_qura_from_gregorian` answers from the published table inside that window and falls back to astronomical reconstruction outside it. A hard edge, already documented at the function, and `hijri_umm_al_qura_covers` distinguishes the two.

### Delta T: adequate 1600 to 2200, and not the binding constraint

The model is a polynomial for 1986 to 2050 and a long-term parabola outside it. Compared against the standard historical determinations, and converted into lunar displacement at the Moon's 13.176 deg per day:

```
year   actual s   header s   error s   lunar displacement
1600      120.0      134.1     +14.1    7.75 arcsec
1700        9.0       25.7     +16.7    9.15 arcsec
1800       13.7      -18.8     -32.5   17.84 arcsec
1900       -2.8        0.8      +3.6    1.95 arcsec
1950       29.1       34.5      +5.4    2.98 arcsec
2000       63.8       63.1      -0.7    0.39 arcsec
2100      202.0      231.9     +29.9   16.39 arcsec
2200      442.0      443.4      +1.4    0.77 arcsec
```

Worst case 0.005 deg, which sits below the 0.0051 deg the lunar series itself carries. Replacing the Delta T model without also improving the series would change nothing measurable.

This is worth stating because Delta T is the usual suspect for long-range degradation and here it is not the limit.

### The actual limit: margin, not date

Every criterion thresholds a continuous quantity. An evening whose value sits within the error bar of its threshold has an answer the library cannot stand behind, and that is true at any epoch including the present.

Counting evenings where `|altitude - 3|` or `|elongation - 6.4|` is under 0.0070 deg, the worst topocentric elongation error measured against DE440, at Mecca under MABIMS 2021, over every day 1 to 28 of every month:

```
century   evenings   inside the error bar   rate
1600s        33600       4                  1 in 8400
1700s        33600       7                  1 in 4800
1800s        33600       5                  1 in 6720
1900s        33600      10                  1 in 3360
2000s        33600       6                  1 in 5600
2100s        33600       5                  1 in 6720
2300s        33600       4                  1 in 8400
```

Between 4 and 10 per century, and flat. 1600 and 2300 both show 4, and the highest count is 1900, inside the validated span. There is no sign of the calculation degrading with distance from J2000 across 1600 to 2400.

The closest single margins found were 0.000192 deg in altitude across 2100-2150 and 0.001243 deg in elongation over the same span, both an order of magnitude inside the error bar.

## The contract this supports

Use the header freely from 1600 to 2200. Expect a handful of evenings per century where the verdict is a coin toss the arithmetic cannot settle, at any epoch. Treat 1882 to 2174 as the range where Umm al-Qura is a published table rather than a reconstruction. Outside 1600 to 2200 nothing fails loudly, and nothing has been checked.

What degrades with distance from J2000 is not the calculation but confidence in the error bar, because the error bar stops being measured outside 1900 to 2100.

## What this note does not settle

The geographic half of #27's first item. Every measurement here is at Mecca. The polar behaviour of the sunset and moonset finders is a separate question, and `prayertimes.h` has just had its own polar work, so the two should not be conflated.

It also does not extend the ephemeris oracle. Doing that needs JPL Horizons data for epochs outside 1900-2100, which has to be fetched and admitted under the usual provenance rules rather than generated here.

Reproduction programs are local and not committed. All are a few dozen lines against the public API.

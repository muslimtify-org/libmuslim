# Solar hour angle reference frame

**Date:** 2026-08-05
**Oracle:** Skyfield 1.54 on JPL DE421
**Fixture:** `tests/test_ephemeris_oracle.c`, `SKY_EQEQ`, the same 24 TT epochs used throughout this file, JD 2415020.5 to 2488069.5
**Issue:** #29

## The defect

`hijri_sun_position()` returns an apparent right ascension, referred to the true equinox of date, because Meeus ch. 25 applies both nutation and aberration to the solar longitude. Every solar altitude call in `hijri.h` paired that apparent right ascension with `hijri__gmst_deg()`, mean sidereal time, referred to the mean equinox instead. An hour angle is only meaningful when the sidereal time and the right ascension it is combined with are referred to the same equinox. Pairing an apparent right ascension with a mean sidereal time carries the difference between the two equinoxes, the equation of the equinoxes, straight into the hour angle as error.

The Moon was never affected. `hijri_moon_position()` is mean-of-date, using mean obliquity, so `hijri__gmst_deg()` was already the correct sidereal time for it. The fix therefore adds a second sidereal time, `hijri__gast_deg()`, apparent sidereal time, and routes only the four solar altitude call sites through it. Lunar paths keep using `hijri__gmst_deg()` unchanged.

## Measured effect on the library

Verbatim from the regenerated 2020-2025 research baseline, comparing before and after the fix:

```
sunset_jd                          max 1.05338 s
moonset_jd                         max 0.00000 s
moon_center_geometric_altitude     max 14.62320 arcsec
topocentric_elongation             max  0.43200 arcsec
yallop_model_q                     max  0.00041
odeh_model_v                       max  0.00410
decision / zone flips              none
```

`moonset_jd` moving by exactly zero is the expected consequence of the Moon never touching `hijri__gast_deg()`, and Task 3 of the implementing plan treated a nonzero value there as a stop-and-report condition. No `local_decision`, `yallop_model_zone`, or `odeh_model_zone` value changed anywhere in the 133-row baseline.

## Sunset decomposition against Skyfield on DE421

The 1.05338 s baseline movement at its worst site is the combined effect of three independent sources of disagreement between the library and Skyfield, not the equation of the equinoxes alone. Decomposed at four sites, verbatim:

```
site          total  Meeus sun  GMST/GAST  solar plx   (seconds)
yogyakarta    2.545      1.263      0.690      0.592
jakarta       1.638      0.952      0.086      0.600
mecca         1.777      0.930      0.161      0.687
london        3.132      1.693      0.440      0.999
```

The GMST/GAST column is what this fix addresses. It is a minority of the total at every site, smaller than the Meeus solar theory's own truncation error everywhere except Mecca, and smaller than the omitted solar parallax term everywhere. Fixing the frame pairing narrows the total sunset disagreement with Skyfield, it does not close it, because the other two columns are unaffected by this change.

## Oracle result

The library's one-term equation of the equinoxes, `hijri__eqeq_deg()`, uses only the leading 17.20 arcsec Omega term of nutation in longitude (Meeus 22.1 truncated). Compared against Skyfield 1.54's full IAU 2000B nutation series on JPL DE421 over the 24 committed epochs, the maximum deviation is 0.0004829 deg, against a true term that itself ranges from -16.01 to +15.99 arcsec across the set.

Before this fix, every solar hour angle carried the full equation of the equinoxes as error, up to about 16 arcsec. After it, at most the 0.0004829 deg residual of the one-term approximation remains. That is a 9x reduction, not elimination, and the test tolerance is pinned at 0.001 deg, a 2.07x margin over the measured maximum, so the honest description of the fix is "reduced," not "corrected."

## Solar parallax remains omitted, on purpose

`hijri_find_sunset()` continues to use a geocentric Sun, with no parallax correction, worth roughly 0.6 to 1.0 second of sunset at the four decomposition sites above. This is not an oversight left by this change. The official conventions the library exists to reproduce leave it out: the Pedoman Hisab Muhammadiyah computes sunset as `h = -(s.d. + R' + Dip)`, with no parallax term, documented at `docs/research/2026-08-01-wujudul-hilal-convention.md:128`. Adding solar parallax would move the library away from the published calendar it is validated against, not toward it. Task 5 of the implementing plan recorded the same decision in the `hijri.h` header, next to the two-sidereal-time documentation.

## This does not improve agreement with any official calendar

This fix should not be read as closing, or even meaningfully narrowing, any gap against an official calendar. The magnitude of the change is 0.004 deg, the largest movement measured anywhere in the criterion parameters above once converted from seconds and arcseconds back to degrees of altitude. The only unexplained gap this library currently has against an official calendar is the two Kemenag-published Rajab starts (1447, 2025-12-21, and 1448, 2026-12-10) that fail the MABIMS 2021 altitude threshold at Sabang by about 0.85 deg, documented in `docs/research/2026-08-01-kemenag-reference.md`. That gap is roughly 200 times larger than the size of this fix. Nothing in this note or in the fix it documents offers an explanation for it, and no claim to the contrary should be inferred from the fact that both concern solar or lunar frame conventions. This fix corrects a real reference-frame defect, measured and mutation-tested, and it is unrelated in scale and in cause to the Rajab discrepancy.

## Deferred item: horizon dip and the MABIMS 2021 threshold

Horizon dip is omitted from the local predicate solvers in `hijri.h`. For the Wujudul Hilal upper-limb quantity, this has been proven to cancel: `test_pedoman_worked_example` passes, because an elevated observer's later sunset lets the Moon descend by almost exactly the dip credit that the altitude side would otherwise gain, matching the cancellation documented in `docs/research/2026-08-01-wujudul-hilal-convention.md`.

That cancellation has NOT been shown to hold for the MABIMS 2021 altitude threshold, which uses a different quantity than the Wujudul Hilal upper-limb test. At Sabang, dip is worth roughly 22 seconds of sunset and roughly 0.09 deg of Moon altitude, a magnitude comparable to the two failing Rajab months' 0.85 deg gap only in the sense that both are altitude-threshold questions, not in size. This is recorded as an open item for a future measurement, not resolved by this fix.

## Reproducing these figures

```sh
gcc -std=c11 -Wall -Wextra -Wpedantic -O2 tests/test_ephemeris_oracle.c -lm -o /tmp/teo && /tmp/teo | tail -3
```

expects a line `eqeq max deviation: 0.0004829 deg` and a final line `Moon ephemeris tests: 448 checks, 0 failures`. The `SKY_EQEQ` table was generated by hand with Skyfield 1.54 on JPL DE421 on 2026-08-05, reading `t.gast - t.gmst` at each epoch, converted from hours to degrees and wrapped to (-180, 180], and is not itself reproducible from a checkout, matching the practice already used for the other hand-generated Skyfield tables in this file. The committed table is the record. The generator's own correctness rests on it being an independent derivation from the library's one-term formula, and the 0.0004829 deg agreement between the two is the check.

The sunset decomposition table and the baseline movement figures were measured before the fix landed, against the same 2020-2025 research baseline this project regenerates and byte-compares on every `make check`. The mutation record for the oracle fixture is M8 in `tests/test_ephemeris_oracle.c`, negating the sign of `hijri__eqeq_deg()`, which fails the group.

# Topocentric error bar and its decomposition

**Date:** 2026-08-07
**Oracles:** Skyfield 1.54 on JPL DE440 (`de440s.bsp`), cross-checked against JPL Horizons
**Fixture:** `tests/test_ephemeris_oracle.c`, tables `SKY_TOPO_JAKARTA`, `SKY_TOPO_MECCA`, `SKY_TOPO_MID45`, `SKY_TOPO_HIGH60`, `SKY_DELTA_T`, `HORIZONS_TOPO_JAKARTA`, `HORIZONS_TOPO_HIGH60`
**Groups:** 9 through 13 of `tests/test_ephemeris_oracle.c`
**Issue:** #17

Every number below is printed by `./build/tests/test_ephemeris_oracle`. Where a figure is a difference between two printed numbers it is labelled as computed, and the two inputs are shown next to it.

## What was unmeasured

MABIMS 2021 thresholds the topocentric altitude at 3 deg and the topocentric elongation at 6.4 deg. Those are the quantities the criteria consume. PR #16 measured positions and the geocentric elongation path, which reads 0.0065798 deg against DE440 on the shipped path. It did not measure the parallax correction that sits between a geocentric position and a topocentric observable, and it did not measure the hour angle chain that turns that correction into an altitude.

The only external anchor on the topocentric chain was a single number. `test_pedoman_worked_example()` compares `moon_upper_limb_apparent_altitude_deg` against the Pedoman Hisab Muhammadiyah worked example and lands 27.2 arcsec from the book, at one epoch, at one site, against a publication computed with its own ephemeris. That one residual jointly constrains the sunset instant, the geocentric Moon position, the parallax correction, the hour angle, the semidiameter and the refraction constant, and it had never been attributed to causes.

This note replaces that anecdote with a measured bar across four latitudes and 22 epochs, decomposed by cause. It does not close the 27.2 arcsec residual, because that residual includes the sunset sampling instant and this measurement deliberately excludes it. See the licensing section below.

## Method

Twenty four fixed TT epochs, running from JD 2415020.5 to JD 2488069.5, the same epochs the rest of `tests/test_ephemeris_oracle.c` uses. Four observer sites at elevation 0:

```
name      latitude   longitude
jakarta      -6.2       106.8
mecca        21.4        39.8
mid45        45.0         0.0
high60       60.0         0.0
```

Four choices in this design are load bearing.

**Fixed instants, not the library's computed sunset.** Sampling at the library's own sunset would couple a parallax error to a sunset time error, and neither could then be measured separately. This is the constraint #18 established on the solar side.

**Airless elevation.** The library computes a geometric altitude and folds refraction into its sunset target instead. A refracted reference would double count refraction and appear as a spurious discrepancy of about 0.57 deg near the horizon. Skyfield's `altaz()` is called with no arguments, and the Horizons query sets `APPARENT='AIRLESS'` explicitly. The generator checks this rather than assuming it, at an epoch where the Moon is 26.8 deg up so that refraction is visible.

**UT1 comes from IERS, not from `hijri_delta_t_seconds()`.** Groups 9 through 12 derive UT1 from the `SKY_DELTA_T` table, which is Skyfield's IERS-backed delta-T, and never from the library's own model. The library's delta-T model is wrong by up to 5.3532 s over the graded epochs, and an error of that size in the derived UT is an error of the same size in sidereal time, which is 80.52 arcsec of hour angle. Letting that stand in front of the parallax code would have measured the delta-T model instead of the thing under test. The delta-T model is measured separately, in its own section below.

**Earth rotation dependent groups run on the first 22 epochs, not all 24.** This is a finding in its own right and it has its own section below.

## The decomposition

Group 9 is the shipped topocentric altitude chain against airless DE440. Group 10 feeds the library and a rigorous WGS84 reference the same geocentric position, taken from `SKY_MOON_GEOMETRIC`, so lunar series truncation is removed from the input rather than left in the answer, and what remains is the spherical Earth approximation in `hijri_moon_topocentric()`.

Both are printed two ways, and the difference between the two ways matters more than it looks.

```
                    group 9 total        group 10 spherical Earth
site           overall  near-horizon     overall  near-horizon    rows in band
jakarta         10.31       2.18           2.18       0.35              5
mecca           12.07       2.85           6.77       1.86              3
mid45           17.50      13.27          12.63       8.11              5
high60          19.82      17.10          13.35      10.22              7
                              all figures in arcsec
```

The near-horizon column is the one MABIMS geometry actually consumes. The 24 fixture epochs were chosen to exercise geocentric position, so they sweep the whole sky, from -86.61 to +82.68 deg altitude at Jakarta. Only 5 of the 22 measured Jakarta rows fall within 15 deg of the horizon, and MABIMS thresholds altitude at 3 deg. The overall column is therefore dominated by geometry the criteria never see. It remains a valid upper bound and it is what the committed tolerances pin, because a bound should be conservative, but quoting it as the topocentric error bar would answer a different question than #17 asks.

Read plainly: near the horizon at Jakarta the shipped chain is within 2.18 arcsec of DE440, and at latitude 60 it is within 17.10 arcsec.

Subtracting group 10 from group 9 gives the share that is not the Earth shape approximation, that is, lunar series truncation plus frame effects plus anything else. These are computed differences between the printed numbers above, not printed by the binary:

```
site        overall difference    near-horizon difference
jakarta     10.31 - 2.18 = 8.13     2.18 - 0.35 = 1.83
mecca       12.07 - 6.77 = 5.30     2.85 - 1.86 = 0.99
mid45      17.50 - 12.63 = 4.87    13.27 - 8.11 = 5.16
high60     19.82 - 13.35 = 6.47    17.10 - 10.22 = 6.88
                          all figures in arcsec
```

**A caveat on that subtraction, stated plainly.** Each column is a maximum over epochs, and the subtraction is a maximum minus a maximum. The two maxima may be driven by different epochs, so the difference is an indication of the truncation and frame share, not an exact per epoch attribution. Nothing in this note should be read as claiming that at any single epoch the spherical Earth term is exactly the group 10 figure and the remainder is exactly the difference. What the subtraction does establish is that at Jakarta, where the criteria are applied, the Earth shape approximation is a small part of an already small bar, 0.35 arcsec of a 2.18 arcsec near-horizon total, and that at latitude 60 it becomes the larger share, 10.22 arcsec of 17.10 arcsec.

The group 10 measurement confirms the qualitative picture #17 recorded from an earlier estimate. #17 put the flattening term at 0.20 arcsec at Jakarta, 5.59 arcsec at latitude 45 and 8.56 arcsec at latitude 60. The near-horizon measurements are 0.35, 8.11 and 10.22 arcsec. The shape is the same, small at low latitude and growing with latitude, which is what entering through `rho*cos(phi')` predicts. The measured values are somewhat larger than the estimate, so the estimate should not be quoted further, but the conclusion #17 drew from it, that flattening is not worth fixing at MABIMS latitudes, survives the measurement.

## Elongation, two references

Two separate quantities are reported, and they must never be added together.

```
site      convention-matched residual    isolated cost of the convention
jakarta            25.17                            8.66
mecca              21.91                            8.49
mid45              23.09                            8.23
high60             23.28                            7.33
                        all figures in arcsec
```

The first column is library error and nothing else. The reference is a topocentric Moon against a geocentric Sun, which is exactly what the library computes, so the residual is implementation error measured against a matching convention. It is a maximum over the 22 measured epochs with no near-horizon restriction.

The second column involves no library code at all. It is the difference between two oracle columns, a fully topocentric elongation and one that omits solar parallax. It is the price of the Pedoman convention, isolated. The library is not wrong to differ from a fully topocentric reference. Omitting solar parallax is what the Pedoman Hisab Muhammadiyah does, recorded in `docs/research/2026-08-01-wujudul-hilal-convention.md`, and charging that deliberate choice as library error would misreport it.

Adding the two columns would be wrong twice over. They are maxima over epochs and may be driven by different rows, and more importantly the second is not an error at all, it is the cost of a convention the library adopts on purpose. The binary also prints a third quantity, `elong_full`, which is the library against the fully topocentric reference. That number is deliberately unasserted and is not quoted as a headline here, because it folds the implementation error and the convention cost together into a single figure that answers no question anyone asked.

For scale, the MABIMS 2021 topocentric elongation threshold is 6.4 deg. The largest convention-matched residual above, 25.17 arcsec at Jakarta, is 0.0069925 deg. The previously known geocentric elongation error on the shipped path is 0.0065798 deg, so the topocentric elongation is not materially worse than the geocentric one the library already had measured.

## The epoch limit, and why it is not a library defect

Topocentric altitude depends on where the Earth has rotated to, which means it depends on UT1. UT1 beyond the IERS prediction horizon is not a determinate quantity, and no library can make it one.

Measured consequence: Skyfield and Horizons, given identical TT epochs and identical sites, disagree by 384.04 arcsec at Jakarta in 2100 and 31.19 arcsec in 2050, while agreeing to better than 0.7 arcsec at every one of the 22 earlier epochs. That is not one engine being wrong. It is two extrapolations of Earth's spin, 74 years out, disagreeing. Horizons states its own limit in its response header:

```
EOP coverage: DATA-BASED 1962-JAN-20 TO 2026-AUG-06. PREDICTS-> 2026-NOV-01
```

Groups 9 through 12 therefore iterate `TOPO_EPOCH_COUNT`, which is 22. The two far future rows stay in the tables so that row i still means row i in `FIXTURE` and `SKY_MOON_GEOMETRIC`, they are simply not measured against. Geocentric position does not depend on Earth's rotation, which is why the pre-existing groups legitimately use all 24 epochs and why this went unnoticed until a topocentric quantity was measured.

The consequence for callers is worth stating directly, because it is not something a future release fixes. A Hijri date computed for the year 2100 carries an irreducible altitude uncertainty of order 0.1 deg from Earth rotation alone, against a MABIMS altitude threshold of 3 deg. That is small relative to the threshold but it is not zero, and near a boundary it is the dominant term in the error bar, larger than everything else this note measures put together.

## Oracle agreement

Over the 22 measured epochs, the two engines agree to 0.0001098 deg at Jakarta and 0.0001015 deg at latitude 60, evaluating the same airless topocentric altitude at the same TT. That is roughly 0.4 arcsec, well inside every library residual reported above, which is what makes the oracle usable as a reference at all.

A degeneracy guard counts rows that differ, and reports 43 distinct of 44. The one collision is not a transcription fault. Horizons prints 6 decimal places against Skyfield's 7, and at JD 2458853.5 for Jakarta both read -86.611709, so two independently retrieved engines land bit identical purely by rounding. An earlier version of the guard required the minimum difference across all rows to be non-zero, and that version failed on correct data for exactly this reason. Counting distinct rows still catches a wholesale copy of one table into the other, which is the failure mode the guard exists for, while tolerating an incidental rounding collision.

## Delta-T

`hijri_delta_t_seconds()` applies the Espenak and Meeus 2005 to 2050 polynomial, which is a 2007 vintage forecast. Earth has since rotated faster than that forecast, so the model runs high. Against Skyfield's IERS-backed values the maximum error over the graded epochs is 5.3532 s, and it grows monotonically across the modern set, from 2.2587 s at JD 2458853.5 to 5.3532 s at JD 2460699.5. This is a real and growing model defect, not noise.

It is reported two ways, because one number alone would mislead in either direction.

```
delta_t max error IERS-graded     = 5.3532 s
delta_t TT-specified hour angle  = 80.52 arcsec
delta_t sunset-anchored residual = 2.93 arcsec
```

**TT specified, 80.52 arcsec.** If the instant of interest is known in TT, as it is for every fixture in this file, a UT must be derived to get sidereal time. Deriving it with a model that is wrong by dT seconds puts the sidereal time off by dT seconds of Earth rotation, which is about 15 times dT in arcsec of hour angle. This is the artifact groups 9 through 12 avoid by taking UT1 from `SKY_DELTA_T`.

**Sunset anchored, 2.93 arcsec.** On the shipped path nobody supplies a UT. `hijri_find_sunset()` bisects the solar altitude to find one. A delta-T error moves the Sun's evaluated position by dT seconds of solar motion, which is a fraction of an arcsec in right ascension, so the recovered sunset instant expressed in UT is barely affected. The Moon is then evaluated at that essentially correct UT with a TT that is dT late, costing dT seconds of lunar motion, while the sidereal time is computed from the correct UT and is clean. The large hour angle term cancels because the solver recovers the instant itself rather than being handed one.

The cancellation is asserted, not merely argued. `delta_t_anchored_below_fixed` in group 13 requires the anchored residual to be strictly smaller than the TT specified one, so a regression that broke the cancellation would fail the suite rather than survive in prose.

**The 2.93 arcsec figure is the one that belongs in a user-facing error bar.** The 80.52 arcsec figure describes a usage pattern the shipped path does not have.

Four of the 24 epochs are ungraded, and the reason is that the reference is not IERS data there. Rows 0 and 1, JD 2415020.5 and JD 2433282.5, predate IERS coverage, which begins in 1962, so Skyfield's value is a reconstruction. Rows 22 and 23, JD 2469807.5 and JD 2488069.5, are past the prediction horizon, so both sides are forecasts. The binary prints all four with the reason attached. The last of them shows the library at 230.8053 s against a reference of 95.9271 s, a difference of 134.8783 s, which measures the disagreement between two guesses about Earth's rotation in 2100 and says nothing about this library. Both arcsec figures above are accumulated over the graded rows only. An earlier version accumulated them over all 24 rows and reported 2028.71 arcsec and 79.02 arcsec, both of which were artifacts of that single forecast row.

## What this does and does not license

**These bars are for fixed-instant sampling.** Every figure in this note is measured at fixed TT epochs, by construction, for the reason stated in the method section. The sunset sampling instant is a separate term and it is not measured here. It is owned by #18, which found the sampling instant carries a 1.6 to 3.1 second error whose largest component is the Meeus ch. 25 solar truncation. A caller who wants the total error on `moon_center_geometric_altitude_deg` at the library's own computed sunset must combine this note's bar with #18's sampling term. Nothing here permits quoting the near-horizon Jakarta figure of 2.18 arcsec as the end-to-end accuracy of a calendar decision.

**Solar parallax remains deliberately omitted.** The elongation section reports its isolated cost, 8.66 arcsec at Jakarta at worst, as a convention price and not as an error. The library omits it because the Pedoman Hisab Muhammadiyah omits it, and adding it would move the library away from the published calendars it is validated against. This measurement is not an argument for adding it.

**What is licensed.** The claim in the `hijri.h` header that the topocentric parallax correction is "not oracle-measured" is retired by this work. The convention-matched topocentric elongation residual is 25.17 arcsec at worst across four sites, and the near-horizon topocentric altitude residual runs from 2.18 arcsec at Jakarta to 17.10 arcsec at latitude 60. Those are upper bounds over the measured epoch set, not point estimates for any particular date.

**What is not established.** The per-epoch attribution of the group 9 total between truncation, frame and Earth shape, for the reason given in the decomposition section. The behaviour at latitudes beyond 60 deg, which is not sampled. The behaviour at nonzero observer elevation, which is not sampled, since every site here sits at elevation 0. Any claim about a specific calendar decision, since none of the epochs here is a calendar boundary.

**Fix gate outcome: no fix applied.** Three candidates were put to the gate's four conditions and none warranted a change to `hijri.h`.

The spherical Earth approximation was measured on the sunset anchored path rather than estimated. Replacing the bare `sin(phi)` and `cos(phi)` in `hijri_moon_topocentric()` with the WGS84 `rho` terms and differencing `moon_center_geometric_altitude_deg` at the library's own computed sunset at Jakarta, over the 22 graded fixture dates, moves the altitude by at most 0.39 arcsec within 15 deg of the horizon and 1.82 arcsec over the full altitude sweep. Both are below the 2 arcsec bar, so the candidate fails condition 2 at the place the criteria are applied. This confirms by measurement what the group 10 near-horizon figure of 0.35 arcsec already indicated. The patch was reverted.

The group 9 minus group 10 remainder at Jakarta near the horizon is 1.83 arcsec, also below the bar, and it is lunar series truncation plus frame terms rather than a single correctable cause, so it fails condition 1 as well as condition 2.

The delta-T model is the only candidate that clears condition 2. It is worth 2.93 arcsec by the group 13 anchored measure and 3.05 arcsec measured directly as a change in `moon_center_geometric_altitude_deg` at the Jakarta sunset, both above the bar. It was therefore put to the bounding experiment of amendment A16. Perturbing `hijri_delta_t_seconds()` by 5.5 s, which exceeds the model's entire IERS graded error range of 2.2587 s in 2020 to 5.3532 s in 2024, moved no official calendar fixture. Kemenag topocentric support held at 33 of 37 with an early count of exactly 0, Muhammadiyah held at 12 of 12, and Umm al-Qura held at 198 of 198. The tightest Kemenag support margin moved from 0.0072696 deg to 0.0063748 deg, which is the closest anything came to flipping and it did not flip.

Conditions 3 and 4 are therefore met, but they are met vacuously, and that is the finding. A shift that bounds the model's whole modern error changes no calendar decision this library is validated against, so a more accurate delta-T would be cosmetic here. The `test_pedoman_worked_example` residual is the one quantity that moved in the wrong direction, growing from 16.77 arcsec to 19.37 arcsec against a fixture tolerance of 180 arcsec. That is a further reason not to reach for a replacement model on the strength of the arcsec figure alone, since the direction of improvement is not uniform across epochs.

The only checks that failed under the perturbation were the three `conj_bitident_*` pins in `tests/test_hijri.c`, which compare a conjunction instant expressed in UT against a value captured before a signature change, at a tolerance of 1e-9 day, or 86 microseconds. A 5.5 s shift in delta-T moves a UT instant by 0.0000637 day, which is exactly what the three FAIL lines reported. These are API stability pins and not calendar fixtures, and they would trip under a delta-T change of any size, so they carry no information about whether the model error is decision relevant. They do record a real cost of any future delta-T change, which is that those three captured constants must be recaptured.

Choosing a replacement delta-T model is a design decision this work never scoped. It involves table size in a single header library, extrapolation policy past the 2026 IERS horizon, and reproducibility of already published dates. No such model was invented here and no numeric change was made to `hijri.h`. The perturbation and the flattening patch were both reverted, and the suite returns to 502 checks in `test_hijri` and 782 checks in `test_ephemeris_oracle`, all passing, with no compiler warnings.

## Reproducing these figures

The committed tables are the record. The generators below are recorded so that the tables can be regenerated and rechecked from a checkout, following the practice already used for `SKY_MOON_APPARENT` and the Horizons curl elsewhere in `tests/test_ephemeris_oracle.c`.

The Skyfield side, run under a throwaway venv with `skyfield==1.54`, which downloads `de440s.bsp` and a timescale file on first run:

```python
from skyfield.api import load, wgs84

AU_KM = 149597870.7

EPOCHS = [
    2415020.5, 2433282.5, 2458853.5, 2458931.7, 2459044.2, 2459122.9,
    2459201.3, 2459318.6, 2459407.1, 2459502.8, 2459613.4, 2459688.2,
    2459777.9, 2459860.5, 2459955.1, 2460048.7, 2460133.3, 2460229.6,
    2460322.4, 2460451.8, 2460577.2, 2460699.5, 2469807.5, 2488069.5,
]

SITES = [
    ("jakarta", -6.2, 106.8),
    ("mecca",   21.4,  39.8),
    ("mid45",   45.0,   0.0),
    ("high60",  60.0,   0.0),
]

ts = load.timescale()
eph = load('de440s.bsp')
earth, moon, sun = eph['earth'], eph['moon'], eph['sun']

# Sanity read, printed before the tables so a wrong assumption is caught early.
t0 = ts.tt_jd(2458853.5)
print("SANITY delta_t at 2458853.5 = %.6f s" % t0.delta_t)
# The refraction sanity epoch is NOT 2458853.5. At that epoch the Jakarta Moon
# is 86.6 deg BELOW the horizon, and skyfield/earthlib.py returns exactly zero
# refraction outside [-1.0, 89.9] deg altitude, so airless and refracted come
# back byte-identical and the guard proves nothing. 2459318.6 puts the Moon
# 26.8 deg up, where refraction is a clearly visible ~2 arcmin.
t1 = ts.tt_jd(2459318.6)
site0 = earth + wgs84.latlon(-6.2, 106.8, elevation_m=0)
a0 = site0.at(t1).observe(moon).apparent()
alt0, _, _ = a0.altaz()
print("SANITY jakarta airless altitude at 2459318.6 = %.7f deg" % alt0.degrees)
alt0r, _, _ = a0.altaz(temperature_C=10.0, pressure_mbar=1010.0)
print("SANITY jakarta REFRACTED altitude at 2459318.6 = %.7f deg" % alt0r.degrees)

print()
print("BLOCK delta_t")
for jd in EPOCHS:
    t = ts.tt_jd(jd)
    print("  {%.1f, %.7f}," % (jd, t.delta_t))

for name, lat, lon in SITES:
    print()
    print("BLOCK %s" % name)
    observer = earth + wgs84.latlon(lat, lon, elevation_m=0)
    for jd in EPOCHS:
        t = ts.tt_jd(jd)
        app_moon = observer.at(t).observe(moon).apparent()
        alt, _, _ = app_moon.altaz()

        # Convention matched: topocentric Moon against a GEOCENTRIC Sun,
        # which is what hijri.h:1220 computes.
        geo_sun = earth.at(t).observe(sun).apparent()
        elong_conv = app_moon.separation_from(geo_sun).degrees

        # Fully topocentric: both bodies seen from the site.
        topo_sun = observer.at(t).observe(sun).apparent()
        elong_full = app_moon.separation_from(topo_sun).degrees

        print("  {%.1f, %.7f, %.7f, %.7f}," %
              (jd, alt.degrees, elong_conv, elong_full))
```

The Horizons side, one call per cross-checked site. This is the Jakarta call. The latitude 60 table uses the identical command with `SITE_COORD='0,60,0'`:

```sh
curl -s -G "https://ssd.jpl.nasa.gov/api/horizons.api" \
  --data-urlencode "format=text" \
  --data-urlencode "COMMAND='301'" \
  --data-urlencode "OBJ_DATA='NO'" \
  --data-urlencode "MAKE_EPHEM='YES'" \
  --data-urlencode "EPHEM_TYPE='OBSERVER'" \
  --data-urlencode "CENTER='coord@399'" \
  --data-urlencode "COORD_TYPE='GEODETIC'" \
  --data-urlencode "SITE_COORD='106.8,-6.2,0'" \
  --data-urlencode "APPARENT='AIRLESS'" \
  --data-urlencode "TLIST=2415020.5 2433282.5 2458853.5 2458931.7 2459044.2 2459122.9 2459201.3 2459318.6 2459407.1 2459502.8 2459613.4 2459688.2 2459777.9 2459860.5 2459955.1 2460048.7 2460133.3 2460229.6 2460322.4 2460451.8 2460577.2 2460699.5 2469807.5 2488069.5" \
  --data-urlencode "TLIST_TYPE='JD'" \
  --data-urlencode "TIME_TYPE='TT'" \
  --data-urlencode "QUANTITIES='4'" \
  --data-urlencode "CSV_FORMAT='YES'" \
  > horizons_jakarta.txt
```

The elevation column must be read from the response header rather than assumed. The header line sits above the `$$SOE` marker, so a grep that prints only lines at and below the marker never shows it. The header returned by this query is:

```
 Date__(TT)__HR:MN:SC.fff, , ,Azi_(a-app), Elev_(a-app),
```

Elevation in degrees is comma field 4, counting from 0, the two unlabeled fields being the daylight and moon visibility flag columns.

To reproduce the measurements themselves from a checkout:

```sh
make test && ./build/tests/test_ephemeris_oracle
```

Every figure quoted in this note appears in that output, except the labelled subtractions in the decomposition section, which are computed from two printed numbers each and are shown with their inputs. The mutation records for the tables added by this work are M10 through M14 in `tests/test_ephemeris_oracle.c`, each with the verbatim FAIL line it produced.

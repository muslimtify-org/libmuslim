# Set-solver error bar, and where the sunset residual comes from

**Date:** 2026-08-08
**Oracles:** Skyfield 1.54 on JPL DE440 (`de440s.bsp`), cross-checked against JPL Horizons
**Fixture:** `tests/test_ephemeris_oracle.c`, tables `SKY_SETSOLVE_JAKARTA`, `SKY_SETSOLVE_MECCA`, `SKY_SETSOLVE_MID45`, `SKY_SETSOLVE_HIGH60`, `HORIZONS_SETSOLVE_SUN`
**Group:** 14 of `tests/test_ephemeris_oracle.c`
**Issue:** #18

Every number below was measured. Three kinds appear and they are not interchangeable. Most are printed by `./build/tests/test_ephemeris_oracle` on every run, and those are quoted as the binary prints them. Some were printed once by a temporary harness or by a temporarily zeroed tolerance, read off, and then recorded in the comment beside the constant they pin, and those say so where they appear. A few are computed from two printed numbers, and those are labelled as computed with their inputs shown. Nothing here is estimated.

## What was unmeasured

Sunset is the sampling instant for every official calendar this library is validated against. Kemenag, Muhammadiyah and Umm al-Qura are all evaluated at the moment the Sun sets, so every altitude, every elongation and every lag the library reports is read at an instant `hijri_find_sunset()` computed. Until this work that instant had no oracle at all. `hijri_find_moonset()` had none either.

The size of the gap is worth stating before the method, because it is not small relative to the things that were measured. Near the horizon the Moon falls at 0.00403 deg per second, so one second of sunset error is 14.51 arcsec of Moon altitude, computed as 0.00403 times 3600. The measured Jakarta sunset residual below is 1.5623 s, which is 22.67 arcsec of Moon altitude on the same conversion. The entire topocentric chain measured in #32, the whole parallax and hour angle path, contributes 2.18 arcsec at Jakarta. The sampling instant is therefore worth 10.4 times the chain that was measured first, computed as 22.67 divided by 2.18.

That is the reason this fixture exists. The largest single term in the chain that decides a date was the one term with no external anchor.

## Method

Four sites at elevation 0, matching #32 so the fixtures compose:

```
name      latitude   longitude
jakarta      -6.2       106.8
mecca        21.4        39.8
mid45        45.0         0.0
high60       60.0         0.0
```

Twelve dates, each one day after a 2025 conjunction, giving 48 rows. The choice of grid is a finding in itself and has its own section below.

A Skyfield generator solves the same two crossings the library solves, by the same bisection on the same bracket shape, and its instants are transcribed into the fixture as Julian Day in UT1 to nine decimals, which is 86 microseconds against a second-scale residual. The library is then run on the same dates and the two instants are differenced.

**The convention is held equal on both sides, on purpose.** The oracle solves for the same target the library does, a geocentric Sun with apparent right ascension and apparent sidereal time, against the same negated sum of refraction and solar semidiameter. That is what separates this measurement from issue #33, which asks whether those two constants are the right ones. Holding the convention equal means the table keeps its meaning after #33 changes them, because #33 changes both sides together. What is measured here is implementation error against a matching convention, not error against physical truth.

**The scan starts at local midnight expressed in UT, not at 0h UT minus half a day.** The first draft of this work started the scan at `hijri_jd_from_gregorian(...) - 0.5`, which is wrong twice over. `hijri_jd_from_gregorian` already returns 0h UT, and `hijri_find_sunset` scans forward 24 hours from whatever instant it is handed, so subtracting half a day starts the scan at noon on the previous day. Measured with the draft as written, three of four sites reported roughly one whole day of sunset error, 86437 to 86555 seconds, and 12 of 48 rows found no moonset inside the window. Jakarta escaped only by accident of its +106.8 longitude. The correct form is the one the library's own caller uses at `hijri.h:1182`, subtracting `longitude_deg / 360.0`. `hijri.h:1163-1181` is a nineteen line comment explaining that passing 0h UT directly caused a one-day error across the Americas and the Pacific, and this draft reintroduced the same class of defect from the opposite direction.

**One row was read by hand before 96 numbers were transcribed.** At Jakarta on the first grid date the library returns sunset 2460705.970476800 against the oracle's 2460705.970467385, a difference of +0.813434 s, and moonset 2460706.005166175 against 2460706.005166394, a difference of -0.018950 s. Seconds, not minutes. Minutes would have meant the generator's frame was wrong, and no table would have been worth transcribing.

## The Moon frame

The moonset side is the easy thing to get wrong, and getting it wrong would have misreported deliberate design as library error.

`hijri__altitude_deg` pairs a mean-of-date right ascension with MEAN sidereal time, deliberately, and the header documents that choice. Skyfield's `altaz()` returns an apparent position paired with true sidereal time. Nutation in right ascension reaches roughly 17 arcsec, and the fixture comment records that comparing against the unmatched frame would report on the order of 1.3 s of that deliberate choice as library error. Against a measured moonset residual of 0.3862 s at Jakarta, an unmatched comparison would have been dominated by the mismatch rather than by the library.

The generator therefore subtracts nutation in right ascension to reach the mean equinox and uses GMST. The subtraction has a sign, and the sign was checked rather than assumed. At the first grid instant Skyfield's own `gast` minus `gmst` is 1.491386 arcsec, and the generator's `dpsi` times `cos(eps)` is 1.491690 arcsec. Same sign, agreeing to 0.0003 arcsec. A sign error here would have been a 3 arcsec error in hour angle in the wrong direction and would have shown up as roughly twice the correction it was meant to remove.

## Results

Printed by the binary, one line per site:

```
setsolve max jakarta  sunset 1.5623 s  moonset 0.3862 s
setsolve max mecca    sunset 1.8397 s  moonset 0.6073 s
setsolve max mid45    sunset 2.2017 s  moonset 1.7952 s
setsolve max high60   sunset 2.7380 s  moonset 7.2926 s
```

Read plainly: against an oracle using the library's own convention, the sunset solver is within 1.6 s at Jakarta and within 2.8 s at latitude 60, and the moonset solver is within 0.4 s at Jakarta and within 7.3 s at latitude 60.

The two quantities spread differently across latitude, and that shaped the bounds. Sunset spreads 1.75x from Jakarta to latitude 60, so it takes one shared tolerance of 5.5 s. Moonset spreads 18.88x, so it takes a per-site tolerance, 0.8, 1.3, 3.6 and 15.0 s. A single moonset bound sized for high60 would have left the Jakarta assertion roughly 39x slack, which is not an assertion.

The high60 moonset figure is not anomalous and is not excluded. Three of its twelve rows are ones where the Moon set before sunset, so the solver walked to the following night. That is a different regime with a different error scale, and those rows are kept because they exercise the 24 hour scan that no other row reaches.

Separately, the bisection convergence is bounded with no oracle at all. The check evaluates the library's own altitude function at the instant the library reports and measures the distance from the target it solved for. The measured maximum over all 48 rows is 1.296e-07 deg, printed by the temporary harness of the transcription commit and recorded in the comment on `TOL_SETSOLVE_CONVERGE_DEG`, against a bound of 1e-6 deg. That bound is deliberately far above the measurement, because the assertion exists to catch a solver regression such as a reduced iteration count rather than to certify the last bit. It is also the only assertion in group 14 that would survive deleting every table, so it pins the solver independently of the ephemeris and a regression in one cannot hide inside the other's tolerance.

The solar position the sunset crossing is solved from is cross-checked against Horizons at three of the solved Jakarta instants. The per-row residuals are 0.0031450, 0.0034349 and 0.0021063 deg in right ascension and 0.0009276, 0.0000433 and 0.0001147 deg in declination, against a pinned bound of 0.007 deg. Those six were read off by setting the bound to zero for one run and are recorded in the comment on `TOL_SETSOLVE_HORIZONS_DEG`. The binary prints their extremes on every run, as `setsolve_horizons residual min 0.0000433 max 0.0034349 deg`.

## Attribution

The residual is attributed, not left as a remainder. That is the point of the convergence measurement.

Forty halvings of a one hour bracket reach about 3e-9 seconds. The solver's own contribution to the reported instant is therefore nine orders of magnitude below the residual, which is 1.5623 to 2.7380 s. Whatever the residual is, it is not the root finder.

What remains is the position the root finder is handed. The library's solar theory is Meeus ch. 25, and this file's header already records it as reaching 0.0084042 deg against DE440, which is the library's dominant documented error term. The Horizons cross-check above measures the same theory at the three solved instants and finds it 0.0021063 to 0.0034349 deg from an independent source in right ascension, inside that documented bound.

So the convention-matched sunset residual is solar ephemeris truncation. This is not a new defect discovered by this work, it is the already documented dominant term of the library appearing in a new place and being measured there for the first time. A caller who wants a smaller sunset residual needs a longer solar series, not a better solver.

The moonset side is bounded but not decomposed, deliberately. It decides nothing in any validated path. It feeds `lag_time_minutes` and `moonset_after_sunset`, which are read only by two research predicates and by the Umm al-Qura fallback at `hijri.h:1760`, and that fallback fires only outside 1882-11-12 to 2174-11-25, so it never runs in the 198 of 198 table fixture. One observation is recorded without being claimed as an attribution: the library's lunar truncation is 0.0012755 deg in longitude against the solar 0.0084042 deg, a factor of 6.6, and the moonset residual is smaller than the sunset residual at three of the four sites. That is consistent with the same cause but it is not established here, because the moonset path also carries parallax, semidiameter and a different frame.

## The grid, and a correction

The obvious grid for a lunar visibility library is the conjunction evening. It is the wrong grid, and this was measured rather than reasoned. The counts in this section come from a throwaway harness run against the library over the three candidate grids, not from the committed suite, which runs only the grid that was chosen.

```
grid                    rows with a moonset
conjunction evening           28 of 48
conjunction plus one          48 of 48
conjunction plus two          47 of 48
```

A conjunction falling after local sunset leaves the old moon in the evening sky, and the old moon sets before the Sun, so `hijri_find_moonset` finds nothing in the window. Twenty of 48 rows would have been holes in the table, and a fixture with holes at exactly the rows where the geometry is hardest is worse than no fixture.

Conjunction plus one is used. At that grid the lag distribution across the 48 rows is minimum 2.4, median 62.2 and maximum 1424.8 minutes, with 39 rows under 120 minutes and 3 over 600. The 3 long ones are the high-latitude rows where the Moon set before sunset and the solver found the following night's moonset. They are kept, for the reason given in the results section.

## What this does and does not license

**The convention gap belongs to #33, and this note asserts nothing about it.** Issue #33 measures the cost of the refraction and semidiameter constants themselves and reports a total sunset shift of -0.454 to +4.541 s, split into an ephemeris component of -0.378 to +1.880 s and a convention component of -0.912 to +3.440 s. Those are #33's measurements, quoted here as #33's and not re-derived, because this fixture holds the convention equal on both sides by construction and therefore cannot see it. Nothing here says whether the library's constants are right.

**This fixture is the baseline #33 verifies against.** Because the convention is held equal, a change to `HIJRI__REFRACTION_AT_HORIZON_DEG` or `HIJRI__SOLAR_SEMIDIAMETER_DEG` moves both sides together and these tables keep their meaning. If #33 changes those constants and group 14 moves, the change did something other than what it claims.

**What is licensed.** The claim that the sunset sampling instant is unmeasured is retired. Against a convention-matched DE440 oracle the sunset solver is within 2.7380 s and the moonset solver within 7.2926 s across four sites and twelve dates, the bisection is converged to 1.296e-07 deg, and the sunset residual is attributed to Meeus ch. 25 solar truncation. #32's note says a caller who wants the total error at the library's own sunset must combine its bar with this sampling term. That term now has a number.

**What is not established.** Error against physical truth, since the convention is held equal on both sides on purpose. Behaviour at latitudes beyond 60 deg, which is not sampled. Behaviour at nonzero observer elevation, since every site here sits at elevation 0. Any claim about a specific calendar decision, since none of the 48 rows is a calendar boundary. The per-row attribution of the sunset residual, since the maxima quoted are maxima over twelve dates and the Horizons cross-check reads only three instants at one site. And the moonset residual's cause, for the reason given in the attribution section.

**A gap in the mutation coverage, recorded rather than smoothed over.** Two of the five original mutations were not caught. Perturbing a stored moonset by 8.64 s at high60 stays inside that site's 15.0 s bound, and the same perturbation at Jakarta against 0.8 s does fail, so the gap is the price of an envelope bound at one site rather than a missing assertion. Perturbing a Horizons right ascension by +0.01 deg is not caught because the row's own residual runs the other way, leaving 45 microdegrees of margin inside the 0.007 deg bound. The same cell in the minus direction is caught. A third, replacing the Horizons declination column with the library's own values, exposed a real defect in the degeneracy guard and is described below. All five are recorded verbatim as S1 through S5 in `tests/test_ephemeris_oracle.c`.

**The degeneracy guard was replaced after a mutation showed it does not guard.** The guard as first written counted distinct rows on the right ascension column only, so the declination column had no degeneracy check at all, and counting distinctness is the wrong test for this table regardless. The precedent it borrowed from guards one stored table being copied byte-exact from another, and this table has no sibling to copy from. Its realistic failure is being regenerated from the library instead of from Horizons, which yields seven-decimal values that never equal the library's doubles bit for bit, so a distinctness count sails past it. The replacement asserts a FLOOR of 1e-5 deg on the MINIMUM of the six differences, which is 4.33x below the measured minimum of 0.0000433 deg and roughly 200x above the rounding of a seven-decimal transcription. A floor on the maximum was written first and executed, and it did not catch the declination-only mutation, because the untouched right ascension column held the maximum up. Both records are S5b in the fixture.

## Reproducing these figures

The committed tables are the record. The generator below is recorded so they can be regenerated and rechecked from a checkout, following the practice already used for `SKY_MOON_APPARENT` and the Horizons curl elsewhere in `tests/test_ephemeris_oracle.c`. It was run by hand under a throwaway venv with `skyfield==1.54`, which downloads `de440s.bsp` and a timescale file on first run.

```python
from skyfield.api import load, wgs84
from skyfield.nutationlib import iau2000b
import math

REFR = 0.5667          # HIJRI__REFRACTION_AT_HORIZON_DEG
SUN_SD = 0.2667        # HIJRI__SOLAR_SEMIDIAMETER_DEG
MOON_SD_FACTOR = 0.2725076   # hijri.h:996

DATES = [(2025,1,30),(2025,3,1),(2025,3,30),(2025,4,28),(2025,5,28),(2025,6,26),
         (2025,7,25),(2025,8,24),(2025,9,22),(2025,10,22),(2025,11,21),(2025,12,21)]
SITES = [("jakarta",-6.2,106.8),("mecca",21.4,39.8),
         ("mid45",45.0,0.0),("high60",60.0,0.0)]

ts = load.timescale()
eph = load('de440s.bsp')
earth, sun, moon = eph['earth'], eph['sun'], eph['moon']

def sun_alt(lat, lon, t):
    """Geocentric Sun projected into the site horizon frame, apparent RA/Dec,
    apparent sidereal time. This is exactly what hijri_sun_altitude does."""
    a = earth.at(t).observe(sun).apparent()
    ra, dec, _ = a.radec(epoch='date')
    H = math.radians((t.gast * 15.0 + lon - ra._degrees) % 360.0)
    phi, d = math.radians(lat), math.radians(dec.degrees)
    return math.degrees(math.asin(
        math.sin(phi)*math.sin(d) + math.cos(phi)*math.cos(d)*math.cos(H)))

def moon_limb_alt(lat, lon, t):
    """Topocentric Moon upper limb, MEAN equinox of date, MEAN sidereal time.

    The mean frame is the load-bearing step and is easy to get wrong. The
    library pairs mean-of-date RA/Dec with GMST deliberately, and the header
    documents that choice. Comparing against Skyfield's apparent/GAST would
    charge roughly 1.3 s of deliberate design as library error.
    iau2000b returns 0.1 microarcseconds, hence the 1e7/3600 scaling."""
    obs = earth + wgs84.latlon(lat, lon, elevation_m=0)
    a = obs.at(t).observe(moon).apparent()
    ra, dec, dist = a.radec(epoch='date')
    dpsi, _ = iau2000b(t.tt)
    eps = math.radians(23.4393)
    ra_mean = ra._degrees - (float(dpsi) / 1e7 / 3600.0) * math.cos(eps)
    gmst = t.gmst * 15.0
    H = math.radians((gmst + lon - ra_mean) % 360.0)
    phi, d = math.radians(lat), math.radians(dec.degrees)
    alt = math.degrees(math.asin(
        math.sin(phi)*math.sin(d) + math.cos(phi)*math.cos(d)*math.cos(H)))
    parallax = math.degrees(math.asin(6378.14 / dist.km))
    return alt + MOON_SD_FACTOR * parallax + REFR

def solve(f, lo, hi, target):
    """Bisection on the same bracket shape the library uses."""
    flo = f(ts.ut1_jd(lo)) - target
    for _ in range(60):
        mid = 0.5 * (lo + hi)
        fmid = f(ts.ut1_jd(mid)) - target
        if (flo < 0) == (fmid < 0):
            lo, flo = mid, fmid
        else:
            hi = mid
    return 0.5 * (lo + hi)

print("SANITY see the task report")
for name, lat, lon in SITES:
    print()
    print("BLOCK %s" % name)
    for (y, m, d) in DATES:
        t0 = ts.utc(y, m, d).ut1
        # scan hour by hour for the sunset crossing, mirroring the library
        ss = None
        prev = sun_alt(lat, lon, ts.ut1_jd(t0)) + REFR + SUN_SD
        for h in range(1, 25):
            cur = sun_alt(lat, lon, ts.ut1_jd(t0 + h/24.0)) + REFR + SUN_SD
            if prev > 0 and cur <= 0:
                ss = solve(lambda tt: sun_alt(lat, lon, tt),
                           t0 + (h-1)/24.0, t0 + h/24.0, -(REFR + SUN_SD))
                break
            prev = cur
        ms = None
        if ss is not None:
            prev = moon_limb_alt(lat, lon, ts.ut1_jd(ss))
            for h in range(1, 25):
                cur = moon_limb_alt(lat, lon, ts.ut1_jd(ss + h/24.0))
                if prev > 0 and cur <= 0:
                    ms = solve(lambda tt: moon_limb_alt(lat, lon, tt),
                               ss + (h-1)/24.0, ss + h/24.0, 0.0)
                    break
                prev = cur
        print("  {%.9f, %.9f},   /* %04d-%02d-%02d */" %
              (ss if ss else 0.0, ms if ms else 0.0, y, m, d))
```

The Horizons cross-check is one call. `TIME_TYPE='UT'` because the stored instants are UT1, and the three instants in `TLIST` are rows 0, 5 and 11 of `SKY_SETSOLVE_JAKARTA` copied verbatim from the fixture:

```sh
curl -s -G "https://ssd.jpl.nasa.gov/api/horizons.api" \
  --data-urlencode "format=text" \
  --data-urlencode "COMMAND='10'" \
  --data-urlencode "OBJ_DATA='NO'" \
  --data-urlencode "MAKE_EPHEM='YES'" \
  --data-urlencode "EPHEM_TYPE='OBSERVER'" \
  --data-urlencode "CENTER='500@399'" \
  --data-urlencode "APPARENT='AIRLESS'" \
  --data-urlencode "TLIST=2460705.970467385 2460852.950441329 2461030.962099322" \
  --data-urlencode "TLIST_TYPE='JD'" \
  --data-urlencode "TIME_TYPE='UT'" \
  --data-urlencode "QUANTITIES='2'" \
  --data-urlencode "CSV_FORMAT='YES'" \
  > horizons_setsolve.txt
```

That returns right ascension and declination in sexagesimal, which is what the fixture's degree columns are converted from:

```
 Date__(UT)__HR:MN:SC.fff, , , R.A.__(a-app), DEC_(a-app),
 2025-Jan-30 11:17:28.382, , ,   20 53 04.95, -17 31 09.9,
 2025-Jun-26 10:48:38.131, , ,   06 22 12.03, +23 20 24.9,
 2025-Dec-21 11:05:25.381, , ,   17 59 16.02, -23 26 17.3,
```

To reproduce the measurements themselves from a checkout:

```sh
make test && ./build/tests/test_ephemeris_oracle
```

The four `setsolve max` lines, the `setsolve_horizons residual min ... max ...` line and the 1029 check count appear in that output on every run. The figures that do not, and which the preamble flags as read off once, are the 1.296e-07 deg convergence maximum, the six per-row Horizons residuals, the Task 1 sanity row, the nutation sign check and the grid counts, each of which is recorded beside the constant or in the section it justifies. The labelled conversions in the first section are computed from printed numbers and shown with their inputs, and #33's convention figures belong to #33. The mutation records for the tables added by this work are S1 through S5 and S5b in `tests/test_ephemeris_oracle.c`, each with the verbatim FAIL line it produced or an explicit record that no failure was observed.

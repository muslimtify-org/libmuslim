# How the Hijri calendar is calculated

This document explains the astronomy and logic behind `hijri.h`, what is computed at each step, and why. It reads on its own, without the source.

## 1. The core problem

A Hijri month begins at sunset on the evening the new crescent is visible, or, depending on the method, is deemed to be. That sentence hides three separate problems:

1. When is the new moon (conjunction)? That is the moment the Moon and Sun share the same ecliptic longitude, with the Moon between Earth and Sun and invisible.
2. When does the Sun set, at a given place, on a given evening?
3. Given those two facts, does this particular evening start the new month? This is where countries and organisations genuinely disagree, because no single physical threshold for "visible" is universally agreed.

Everything else in the library answers these three questions, in order.

## 2. Two calculation styles

### 2a. Tabular (arithmetic) calendar, no astronomy

Some calendars, and most simple software, do not look at the sky at all. The tabular or "Kuwaiti" algorithm runs a fixed 30-year cycle. Months alternate 30 and 29 days, and 11 of every 30 years add a day to the last month (355 days instead of 354), which keeps the average month close to the true synodic month of 29.530589 days.

```
is_leap_year(y)  =  ((11*y + 14) mod 30) < 11
month_length(y, m) = 30 if m is odd
                    = 29 if m is even and m != 12
                    = 30 if m == 12 and is_leap_year(y)
                    = 29 if m == 12 and not is_leap_year(y)
```

It is cheap, deterministic and reversible in closed form, but disconnected from the sky. It tracks the astronomical calendar to within roughly one or two days, which is enough for a fast fallback or a sanity check. Religious authorities do not declare months by it.

### 2b. Astronomical calendar

The rest of this document covers the astronomical calendar, which decides each month from the actual positions of the Sun and Moon.

## 3. Step 1: time bookkeeping

Everything below works in Julian Day (JD), a single running count of days with a decimal fraction for the time of day. Astronomy uses it instead of calendar dates because it makes date arithmetic trivial.

`hijri_jd_from_gregorian()` and `hijri_gregorian_from_jd()` convert between a Gregorian date and JD, using the standard algorithm from Meeus, *Astronomical Algorithms*, ch. 7.

The formulas for the Sun and Moon are defined in Terrestrial Time (TT), a uniform clock unaffected by Earth's slightly irregular rotation. Sunset and moonset happen in Universal Time (UT), the time civil time zones are built on. The difference between them, ΔT, has drifted from a few seconds to about a minute over the last few centuries. `hijri_delta_t_seconds()` approximates it with a small polynomial fit. The correction shifts positions by only a few arcseconds, which is irrelevant next to the degree-scale thresholds used later, but it is applied for correctness.

## 4. Step 2: where are the Sun and Moon?

### 4a. Sun position

`hijri_sun_position()` implements Meeus's low-precision solar theory (ch. 25). It starts from the Sun's mean longitude and mean anomaly, both simple polynomials in time, and applies the equation of the centre, a few sine terms of the mean anomaly that account for Earth's orbit being an ellipse. That gives the Sun's true ecliptic longitude. A small further correction for nutation and aberration gives the apparent longitude, which is rotated into right ascension and declination, the coordinates a telescope is pointed by, using Earth's axial tilt (obliquity). The result is accurate to about 0.01°, far more than needed here.

### 4b. Moon position

`hijri_moon_position()` follows the same recipe: mean orbital elements, then periodic corrections, then the same rotation. The Moon's motion is much more irregular than the Sun's apparent motion, because the Sun's gravity and Earth's oblateness perturb it, so its theory needs many more periodic terms. The library uses the complete series of Meeus ch. 47, with the precision given in the accuracy summary at the end of this document.

### 4c. Topocentric correction (parallax)

Both positions above are geocentric, as seen from the centre of the Earth. A real observer is up to about 6,378 km off-centre. The Moon is close, about 385,000 km away against the Sun's 150,000,000 km, so that offset shifts where the Moon appears in the sky by up to about a degree. `hijri_moon_topocentric()` applies this correction for a given observer, using the Moon's horizontal parallax (derived from its distance) and the local sidereal time. It matters because several visibility criteria work in fractions of a degree.

## 5. Step 3: sunset, moonset and conjunction

All three are the same computational problem: find the time some quantity crosses zero. Each is solved by scanning forward for a sign change and bisecting the interval that contains it.

| Event | Function | Quantity that crosses zero |
|---|---|---|
| Sunset | `hijri_find_sunset()` | The altitude of the Sun's upper limb, including atmospheric refraction at the horizon. Its centre then sits about 0.83° below the true horizon, from about 34′ of refraction and the Sun's apparent radius of about 16′. |
| Moonset | `hijri_find_moonset()` | The Moon's topocentric upper-limb altitude, with the same refraction and the Moon's own semidiameter. |
| Conjunction | `hijri_find_previous_conjunction()`, `hijri_find_next_conjunction()`, `hijri_find_relevant_conjunction()` | The difference in ecliptic longitude between Moon and Sun, which grows by roughly 360° every 29.53 days as the Moon laps the Sun. |

For conjunction the library searches backward from a reference time for the most recent crossing, or forward for the next. A search over a fixed window around a guess can return either of two neighbouring conjunctions, depending on exactly where the guess falls. `hijri_find_relevant_conjunction()` picks whichever of the previous and next conjunctions is nearer to a given evening.

## 6. Step 4: the quantities every criterion is built from

Given a candidate sunset, `hijri_compute_evening_parameters()` derives everything the country rules check:

| Quantity | What it means | How it is computed |
|---|---|---|
| Moon altitude | How high the Moon sits above the horizon at sunset | Topocentric altitude from the hour-angle formula |
| ARCV (arc of vision) | Moon altitude minus Sun altitude at sunset, roughly how far above and behind the Sun the Moon is | `moon_altitude − sun_altitude` |
| Elongation | The full angular separation between Moon and Sun, not just their altitude difference | Spherical law of cosines on their RA and Dec |
| Crescent width | The apparent width of the lit sliver, which is what makes a crescent visible or not | `W ≈ SD·(1 − cos(elongation))`, where `SD` is the Moon's topocentric semidiameter (from its parallax) |
| Moon age | Hours since the most recent conjunction | `(sunset time − conjunction time) × 24` |
| Lag time | Minutes between sunset and moonset, how much longer the Moon stays up | `(moonset time − sunset time) × 1440` |

Every criterion below is written in these six quantities.

## 7. Step 5: the criteria, where countries diverge

Given the same sky, the same sunset and the same numbers, authorities draw the line for "the month starts tomorrow" in genuinely different places.

### Umm al-Qura (Saudi Arabia, civil calendar)

```
month starts  ⟺  conjunction occurred before sunset
              AND moonset occurs after sunset
```

This is evaluated at Mecca. It has no altitude or elongation requirement at all. It is a pure geometry test, whether the young Moon is born and lingers above the horizon after sunset, and not a visibility prediction.

`hijri_umm_al_qura_from_gregorian()` does not evaluate this criterion for dates in 1300 to 1600 AH. It reads the official published table, embedded in about 600 bytes and derived from ICU/CLDR, because the published calendar departs from its own stated criterion in some months and so cannot be computed exactly. The criterion remains available through the predicate API and is the fallback outside the table's range. For years before about 1423 AH (2002 CE) the table is a modern retro-computation, and documented Saudi correspondences show it can differ by a day from the calendar in use at the time. Three of five documented anchors differ. See `docs/research/2026-08-01-umm-al-qura-oracle.md`.

### MABIMS (Indonesia, Malaysia, Brunei, Singapore)

The criterion before 2021:

```
month starts  ⟺  (altitude ≥ 2° AND elongation ≥ 3°)  OR  moon age ≥ 8 hours
```

The criterion since December 2021:

```
month starts  ⟺  altitude ≥ 3°  AND  elongation ≥ 6.4°
```

Both are minimum thresholds. The numbers were chosen, and later raised, from accumulated records of what was actually reported as visible.

### Wujudul Hilal (Muhammadiyah, Indonesia)

```
month starts  ⟺  conjunction occurred before sunset  AND  moon altitude > 0°
```

The most permissive criterion here. The Moon only has to be above the horizon, with no visibility margin, which is why Muhammadiyah's calendar sometimes starts a month a day before MABIMS from the same sky.

### Turkey (Diyanet), the 2016 Istanbul criterion, ECFR and the Fiqh Council of North America

```
month starts  ⟺  altitude ≥ 5°  AND  elongation ≥ 8°
```

The thresholds are stricter than MABIMS, and the criterion is meant to be checked globally. If it is met anywhere on Earth at that lunar cycle's relevant sunset, the month starts everywhere, not just at the site where it was checked. The predicate `HIJRI_PREDICATE_ALTITUDE_5_ELONGATION_8` evaluates the threshold for whichever single location you pass in.

For Diyanet the global search is built in. `HIJRI_AUTHORITY_DIYANET` searches the whole Earth for a point that passes before 00:00 UT, and also checks the Americas mainland, which Diyanet's bulletin allows provided the conjunction falls before dawn in Wellington. Which points it checks is the library's reading of the bulletin, not a fixed astronomical fact. That reading reproduces 184 of Diyanet's 186 published month starts from 2021 to 2035. See `docs/research/2026-10-25-diyanet-global-rule.md`. ECFR and the Fiqh Council of North America have no registry entry, so for them the sweep is still yours to run.

### Egypt

```
month starts  ⟺  lag time ≥ 5 minutes
```

A pure moonset-timing rule.

## 8. Step 6: quantitative visibility models (Yallop, Odeh)

These two produce a graded classification instead of one fixed threshold. Both come from published research that correlated historical sighting reports with ARCV and crescent width.

Yallop (1997) combines ARCV and crescent width `W`, in arcminutes, into one score:

```
q = [ ARCV − (11.8371 − 6.3226·W + 0.7319·W² − 0.1018·W³) ] / 10
```

It then classifies:

| q range | Zone |
|---|---|
| q > 0.216 | Easily visible |
| −0.014 < q ≤ 0.216 | Visible under perfect conditions |
| −0.160 < q ≤ −0.014 | May need optical aid |
| −0.232 < q ≤ −0.160 | Needs optical aid |
| q ≤ −0.232 | Not visible (below the Danjon limit) |

Odeh (2004), built from 737 observation records, uses the same two inputs with a different polynomial:

```
V = ARCV − ( −0.1018·W³ + 0.7319·W² − 6.3226·W + 7.1651 )
```

| V range | Zone |
|---|---|
| V ≥ 5.65 | Visible, naked eye |
| 2.0 ≤ V < 5.65 | Visible with optical aid, could be naked eye |
| −0.96 ≤ V < 2.0 | Visible with optical aid only |
| V < −0.96 | Not visible |

Because they return a zone rather than yes or no, they are exposed as `hijri_yallop_classify()` and `hijri_odeh_classify()`, not through the boolean `hijri_local_predicate_evaluate()`. You decide which zones count as "the month starts" for your purposes.

## 9. Step 7: from one evening to a full calendar

`hijri_evaluate_evening()` runs steps 3 to 7 for one evening and returns whether that evening starts a month, plus every intermediate number, so a caller or a UI can show its work instead of a bare yes or no.

`hijri_from_gregorian_with_local_predicate()` builds a full date from that:

1. Find the conjunction nearest the target date.
2. Walk forward evening by evening from that conjunction, not from the target date, checking `hijri_evaluate_evening()` until the chosen criterion first holds. The day after that evening is the first day of the month. If the target date comes before it, repeat from the conjunction before.

   Starting from the conjunction matters. Most thresholds, once met a few days after new moon, stay met for most of the month, because the Moon keeps moving away from the Sun until about full moon. A backward scan from an arbitrary date for the most recent evening where the threshold holds would nearly always land on yesterday, and report a mid-month date as day 1.
3. The day of the month is `target_date − month_start_date + 1`.
4. The month and year numbers come from the tabular calendar, sampled a few days into the resolved month rather than on its first day. The tabular calendar's own boundary can sit a day away from the astronomical one, and sampling on the boundary would then name the wrong month even though the day count is right.

## 10. Why criteria disagree

Given identical sky data, different criteria normally report different dates around a month boundary. That is why the library makes the criterion a runtime choice. Three mechanisms drive it:

- Stricter thresholds (Turkey and the Istanbul criterion, ECFR) need the Moon further from the Sun than looser ones (MABIMS), which means one evening later in borderline cases.
- Wujudul Hilal drops the visibility margin entirely, so it can start a month a day before a threshold criterion looking at the same sky.
- Longitude relative to the conjunction's UT time matters. If a conjunction happens late in the UT day, a far-east location such as Jakarta (UTC+7) can have its sunset before the conjunction, while a location further west sets after it. For that evening the two places can sit in different lunar months.

## 11. Accuracy summary

| Component | Precision | Notes |
|---|---|---|
| Julian Day conversion | Exact | Standard algorithm, no approximation |
| Delta T | Seconds, historically minutes | Negligible next to degree-scale thresholds |
| Sun position | About 0.01° | Meeus low-precision theory |
| **Moon position** | **0.0051° lon, 0.0006° lat, 41.9 km** | **Full Meeus ch. 47 series, measured against 24 JPL Horizons epochs from 1900 to 2100** |
| Rise and set times | Limited by the Moon position above | Refraction from a fixed constant per sunset convention |
| Parallax | Spherical-Earth approximation | Elevation correction omitted (sub-arcsecond effect) |
| Umm al-Qura dates | Exact against the published table, 1300 to 1600 AH | Table lookup, no astronomy. See the Umm al-Qura part of §7. |
| Month and year numbering (predicate paths) | Reliable in normal cases | Taken from the tabular calendar mid-month |

The Moon position uses the complete Meeus ch. 47 series: 60 longitude and distance terms, 60 latitude terms, the eccentricity correction and the A1, A2 and A3 additive terms. `tests/test_ephemeris_oracle.c` validates it against a vendored JPL Horizons fixture, Meeus's own printed worked example and published ΔT values. Nutation and aberration are not applied, so positions are geometric and referred to the mean equinox of date. That, not truncation of the series, is the 0.005° floor in the table. Every other function depends only on the contents of the `HijriMoonPosition` struct, not on how it was computed, so a higher-precision ephemeris can be swapped in without touching anything else.

## References

- Meeus, J. *Astronomical Algorithms*, 2nd ed., Willmann-Bell, 1998.
- Yallop, B.D. "A Method for Predicting the First Sighting of the New Crescent Moon", RGO NAO Technical Note No. 69, 1997.
- Odeh, M.S. "New Criterion for Lunar Crescent Visibility", *Experimental Astronomy* 18, 2004.
- MABIMS ad-referendum, 8 December 2021 (3° and 6.4° criterion).
- International Hijri Calendar Union Congress, Istanbul, 2016 (5° and 8° unified criterion).
- KACST, Umm al-Qura calendar rules documentation.

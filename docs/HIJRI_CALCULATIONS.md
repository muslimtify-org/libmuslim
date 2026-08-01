# How the Hijri Calendar Is Calculated

This document explains the astronomy and logic behind `hijri.h`, what's
actually being computed at each step, and why. It's meant to be readable
on its own, without needing to read the source first.

## 1. The core problem

A Hijri month begins at sunset on the evening the new crescent moon is
(or, depending on the method, is deemed to be) visible. That single
sentence hides three separate sub-problems:

1. **When is the new moon (conjunction)?**, the moment the Moon and Sun
   share the same ecliptic longitude, i.e. the Moon is between Earth and
   Sun, invisible.
2. **When does the Sun set, at a given place, on a given evening?**
3. **Given those two facts, does *this particular* evening qualify as
   the start of the new month?**, and this is where different
   countries and organizations genuinely disagree, because there's no
   single universally-agreed physical threshold for "visible."

Everything else in the library is built to answer these three
questions, in order.

## 2. Two fundamentally different calculation styles

### 2a. Tabular (arithmetic) calendar, no astronomy

Some calendars (and most simple software) don't look at the sky at all.
The **tabular/"Kuwaiti" algorithm** just runs a fixed 30-year cycle:
months alternate 30 and 29 days, and 11 out of every 30 years get an
extra day tacked onto the last month (355 days instead of 354) to keep
the average month length close to the true synodic month
(29.530589 days).

```
is_leap_year(y)  =  ((11*y + 14) mod 30) < 11
month_length(y, m) = 30 if m is odd
                    = 29 if m is even and m != 12
                    = 30 if m == 12 and is_leap_year(y)
                    = 29 if m == 12 and not is_leap_year(y)
```

This is cheap, deterministic, and reversible in closed form, but it's
disconnected from the actual sky. It tracks the real astronomical
calendar to within roughly ±1–2 days, which is good enough as a fast
fallback or a sanity check, but not what religious authorities actually
use to declare a month.

### 2b. Astronomical calendar, the real thing

This is the interesting part, and the rest of this document is about it.

## 3. Step 1: Time bookkeeping

Everything below works in **Julian Day (JD)**, a single running count
of days (with a decimal fraction for time-of-day) used throughout
astronomy instead of calendar dates, because it makes date arithmetic
trivial.

- `hijri_jd_from_gregorian()` / `hijri_gregorian_from_jd()` convert
  between a Gregorian calendar date and JD, using the standard algorithm
  from Meeus, *Astronomical Algorithms*, ch. 7.
- Astronomical formulas for the Sun and Moon are defined in **Terrestrial
  Time (TT)**, a uniform clock unaffected by Earth's slightly irregular
  rotation, while sunset/moonset are naturally things that happen in
  **Universal Time (UT)**, the time zone system is built on. The
  difference between them, **ΔT**, has drifted from a few seconds to
  about a minute over the last few centuries. `hijri_delta_t_seconds()`
  approximates it with a small polynomial fit. This correction only
  shifts positions by a few arcseconds, irrelevant next to the
  multi-degree thresholds used later, but it's there for correctness.

## 4. Step 2: Where are the Sun and Moon?

### 4a. Sun position

`hijri_sun_position()` implements Meeus's **low-precision solar theory**
(ch. 25 of his book): starting from the Sun's mean longitude and mean
anomaly (both simple polynomials in time), it applies the **equation of
the center**, a correction (built from a few sine terms of the mean
anomaly) that accounts for Earth's orbit being an ellipse, not a circle
— to get the Sun's true ecliptic longitude. A small further correction
for nutation and aberration gives the *apparent* longitude, which is
then converted into right ascension/declination (the coordinates you'd
need to point a telescope) via the standard ecliptic-to-equatorial
rotation using Earth's axial tilt (obliquity). This is accurate to about
0.01°, which is far more precision than needed here.

### 4b. Moon position

`hijri_moon_position()` follows the same general recipe, mean orbital
elements, then periodic corrections, then the same coordinate rotation,
but the Moon's motion is much more irregular than the Sun's apparent
motion (because it's perturbed by the Sun's gravity, Earth's oblateness,
etc.), so its full theory (ELP2000-82B) has on the order of a hundred
periodic correction terms.

**This library currently implements only the five or six
largest-amplitude terms**, the ones driven by the Moon's mean anomaly,
its elongation from the Sun, and its argument of latitude. That's
enough to get the right *shape* of answer (accurate to a few tenths of a
degree) and to exercise every other part of the pipeline correctly, but
it is **not** full observational precision. See the accuracy section
near the end of this document, and the note at the top of `hijri.h`, for
what to do about that if you need it.

### 4c. Topocentric correction (parallax)

Both of the above give **geocentric** positions, as if you were
standing at the center of the Earth. A real observer is up to ~6,378 km
off-center, and because the Moon is close (haep ~385,000 km away, versus
the Sun's ~150,000,000 km), this offset noticeably shifts where the Moon
*appears* to be in the sky, by up to about a degree. `hijri_moon_topocentric()`
applies this correction (parallax) for a specific observer location,
using the Moon's horizontal parallax (derived from its distance) and the
local sidereal time to work out the geometry. This matters here because
several visibility criteria work in fractions of a degree.

## 5. Step 3: Sunset, moonset, and conjunction

All three of these are really the same computational problem, **find
the time some quantity crosses zero**, solved with a bisection
root-finder (`hijri_find_sunset`, `hijri_find_moonset`,
`hijri_find_conjunction` / `hijri_find_previous_conjunction`):

- **Sunset**: scan forward through the day in coarse steps computing the
  Sun's topocentric altitude at each step, once it's found to cross from
  above the horizon to below, bisect that interval down to
  ~1-second precision. The "horizon" used is not 0°, but about −0.83°
  (standard atmospheric refraction of ~34′, plus the Sun's own apparent
  radius of ~16′), this is the conventional definition of sunset, when
  the *upper limb* of the Sun's disk touches the true horizon as seen
  through the atmosphere.
- **Moonset**: identical idea, applied to the Moon's topocentric
  altitude, using just the refraction correction (the Moon's own
  semidiameter is a smaller, second-order effect that's omitted here for
  simplicity).
- **Conjunction (new moon)**: instead of altitude, track the *difference
  in ecliptic longitude* between Moon and Sun. This difference
  increases by roughly 360°/29.53 days as the Moon laps the Sun each
  month, conjunction is the moment it crosses 0° (Moon "catching up" to
  Sun). `hijri_find_previous_conjunction()` scans backward from a
  reference time and bisects the crossing, which is unambiguous,
  unlike naively searching a fixed window around a guess, which can pick
  up either of two neighboring conjunctions depending on exactly where
  the guess falls (a real bug encountered and fixed during development
  of this library, see the doc comment on `hijri_find_conjunction()`
  for why the "previous conjunction" variant exists separately).

## 6. Step 4: the quantities every visibility criterion is built from

Once you have a candidate sunset time, `hijri_compute_hilal_parameters()`
derives everything the various country rules actually check:

| Quantity | What it means | How it's computed |
|---|---|---|
| **Moon altitude** | How high the Moon sits above the horizon at sunset | Topocentric altitude via the hour-angle formula |
| **ARCV** (arc of vision) | Moon altitude minus Sun altitude at sunset, roughly, how far "above and behind" the Sun the Moon is | `moon_altitude − sun_altitude` |
| **Elongation** | The true angular separation between Moon and Sun (not just altitude difference, the full 3D angle) | Spherical law of cosines on their RA/Dec |
| **Crescent width** | The apparent width of the illuminated sliver, which is what actually makes a crescent visible or not | Approximated from the Moon's angular size and elongation: `W ≈ SD·(1 − cos(elongation))`, where `SD` is the Moon's topocentric semidiameter (from its parallax) |
| **Moon age** | Hours elapsed since the most recent conjunction | `(sunset time − conjunction time) × 24` |
| **Lag time** | Minutes between sunset and moonset (how much longer the Moon stays up after the Sun sets) | `(moonset time − sunset time) × 1440` |

These six numbers are the entire vocabulary that every criterion below
is written in.

## 7. Step 5: the criteria, where countries actually diverge

Given the same sky, the same sunset, the same numbers above, different
authorities draw the line for "the month starts tomorrow" in genuinely
different places:

**Umm al-Qura (Saudi Arabia, civil calendar)**
```
month starts  ⟺  conjunction occurred before sunset
              AND moonset occurs after sunset
```
evaluated specifically at Mecca. Notice this has *no altitude or
elongation requirement at all*, it's a pure geometry test ("is the
young moon born, and does it linger above the horizon after the sun
goes down"), not an actual visibility prediction.

> **Implementation note:** since 2026-08 the library's
> `hijri_umm_al_qura_from_gregorian()` does not evaluate this criterion at
> all for dates in 1300–1600 AH — it reads the official published table
> (embedded, ~600 bytes, derived from ICU/CLDR), because the published
> calendar deviates from its own stated criterion for some months and is
> therefore not exactly computable. The criterion machinery described here
> remains available through the predicate API and is used as the fallback
> outside the table's range.

**MABIMS (Indonesia / Malaysia / Brunei / Singapore)**

Pre-2021:
```
month starts  ⟺  (altitude ≥ 2° AND elongation ≥ 3°)  OR  moon age ≥ 8 hours
```
Since December 2021:
```
month starts  ⟺  altitude ≥ 3°  AND  elongation ≥ 6.4°
```
Both are minimum-threshold tests, the numbers were chosen (and later
revised upward) based on accumulated observational records of what's
actually reported as visible.

**Wujudul Hilal (Muhammadiyah, Indonesia)**
```
month starts  ⟺  conjunction occurred before sunset  AND  moon altitude > 0°
```
The most permissive criterion here, "the moon merely exists above the
horizon," no visibility margin at all. This is why Muhammadiyah's
calendar sometimes starts a month a day earlier than MABIMS, from
identical underlying sky data.

**Turkey (Diyanet) / 2016 Istanbul ICOP unified criterion, and
ECFR / Fiqh Council of North America**
```
month starts  ⟺  altitude ≥ 5°  AND  elongation ≥ 8°
```
Stricter thresholds than MABIMS, and, importantly, intended to be
checked *globally*: if the criterion is met **anywhere on Earth** at
that lunar cycle's relevant sunset, the month starts everywhere, not
just at the site where it happened to be checked. (This library
evaluates the threshold for whichever single location you pass in,
reproducing the "anywhere on Earth" logic means sweeping a set of
candidate longitudes yourself and OR-ing the results, a policy choice
about which longitudes to check, not a fixed astronomical fact.)

**Egypt**
```
month starts  ⟺  lag time ≥ 5 minutes
```
A pure moonset-timing rule.

## 8. Step 6: quantitative visibility models (Yallop, Odeh)

Rather than a single fixed threshold, these two produce a **graded
classification** from published research correlating actual historical
sighting reports against ARCV and crescent width.

**Yallop (1997)** combines ARCV and crescent width `W` (in arcminutes)
into a single score:

```
q = [ ARCV − (11.8371 − 6.3226·W + 0.7319·W² − 0.1018·W³) ] / 10
```

then classifies:

| q range | Zone |
|---|---|
| q > 0.216 | Easily visible |
| −0.014 < q ≤ 0.216 | Visible under perfect conditions |
| −0.160 < q ≤ −0.014 | May need optical aid |
| −0.232 < q ≤ −0.160 | Needs optical aid |
| q ≤ −0.232 | Not visible (below the Danjon limit) |

**Odeh (2004)**, built from 737 observation records, uses the same two
inputs with a different polynomial:

```
V = ARCV − ( −0.1018·W³ + 0.7319·W² − 6.3226·W + 7.1651 )
```

| V range | Zone |
|---|---|
| V ≥ 5.65 | Visible, naked eye |
| 2.0 ≤ V < 5.65 | Visible with optical aid, could be naked eye |
| −0.96 ≤ V < 2.0 | Visible with optical aid only |
| V < −0.96 | Not visible |

Because these return a graded zone rather than a plain yes/no, they're
exposed as `hijri_yallop_classify()` / `hijri_odeh_classify()` rather
than through the boolean `hijri_criterion_evaluate()` path, you decide
which zone(s) count as "the month starts" for your purposes.

## 9. Step 7: putting it together, walking from sunset to a full calendar

`hijri_evaluate_evening()` runs steps 3–7 for one specific evening and
returns whether that evening triggers month-start, plus every
intermediate number, so a caller (or a UI) can show its work rather than
just a bare yes/no.

`hijri_from_gregorian()` builds a full calendar date from that:

1. Find the geocentric conjunction most recently before the target date.
2. Walk forward evening by evening **from that conjunction** (not from
   the target date) checking `hijri_evaluate_evening()`, until the
   chosen criterion first triggers, that's the actual first day of the
   month.

   This "walk forward from the conjunction" detail matters: most
   visibility thresholds, once satisfied a few days after new moon, stay
   satisfied for most of the rest of the month (the Moon just keeps
   getting further from the Sun until roughly full moon). Naively
   scanning *backward* from an arbitrary target date for "the most
   recent evening where the threshold holds" would almost always land
   on "yesterday," misreporting an arbitrary mid-month date as day 1.
   Anchoring the search at the actual conjunction avoids that.
3. The day-of-month is just `target_date − month_start_date + 1`.
4. For the month/year *number* (as opposed to the day boundary, which
   is fully astronomical), the tabular calendar is sampled a few days
   *into* the resolved month, not exactly on the boundary day, since
   the tabular calendar's own boundary can itself sit a day off from the
   astronomically-resolved one, which would otherwise risk mislabeling
   which month it is even though the day *count* is correct.

## 10. Why disagreement between criteria is expected, not a bug

Given identical sky data, it's normal and expected for different
criteria to report different dates around a month boundary, that's the
whole reason this library makes the criterion a runtime choice rather
than hardcoding one. A few concrete mechanisms behind that:

- Stricter altitude/elongation thresholds (Turkey/ICOP, ECFR) simply
  require the Moon to be further from the Sun than looser ones (MABIMS),
  which mechanically means "one evening later" in borderline cases.
- Wujudul Hilal drops the visibility margin entirely, so it can call a
  month a day earlier than a threshold-based criterion looking at
  exactly the same sky.
- Time zone relative to the conjunction's UT time matters a lot: if a
  given lunar cycle's conjunction happens late in the UT day, a
  far-east location (e.g. Jakarta, UTC+7) may have its sunset *before*
  the conjunction even occurs, while a location further west hasn't had
  sunset yet and sees the conjunction as already past, so the "current"
  lunar month can genuinely differ by a full cycle between the two
  places for that specific evening.

## 11. Accuracy summary

| Component | Precision | Notes |
|---|---|---|
| Julian Day conversion | Exact | Standard algorithm, no approximation |
| Delta T | ~seconds, historically ~minutes | Negligible vs. degree-scale thresholds |
| Sun position | ~0.01° | Meeus low-precision theory |
| **Moon position** | **~0.1–0.3°** | **Only 5–6 leading ELP2000-82B terms, see caveat below** |
| Rise/set times | Limited by Moon-position precision above | Refraction modeled with a fixed 34′ constant |
| Parallax | Spherical-Earth approximation | Elevation correction omitted (sub-arcsecond effect) |
| Month/year numbering | Reliable in normal cases | Falls back on the tabular calendar's mid-month assignment |

**The one component to be careful with for real observational-grade
use is lunar position.** The current implementation trades off full
ELP2000-82B precision (~60 terms) for a compact, easy-to-follow
implementation using only the handful of largest terms. This is enough
to get qualitatively correct answers and to validate the rest of the
pipeline (as demonstrated by the worked timezone-boundary example
earlier), but genuinely borderline real-world cases can come out
differently with the full series. If you need that level of precision,
swap the body of `hijri_moon_position()` for the full Meeus ch. 47
tables, or link against an existing full-precision implementation (e.g.
[mygulamali/meeus](https://github.com/mygulamali/meeus) or libnova),
nothing else in the library needs to change, since every other function
only depends on the `HijriMoonPosition` struct's contents, not on how it
was computed.

## References

- Meeus, J. *Astronomical Algorithms*, 2nd ed., Willmann-Bell, 1998.
- Yallop, B.D. "A Method for Predicting the First Sighting of the New
  Crescent Moon", RGO NAO Technical Note No. 69, 1997.
- Odeh, M.S. "New Criterion for Lunar Crescent Visibility", *Experimental
  Astronomy* 18, 2004.
- MABIMS ad-referendum, 8 December 2021 (3°/6.4° criterion).
- International Hijri Calendar Union Congress, Istanbul, 2016 (5°/8°
  unified criterion).
- KACST / Umm al-Qura calendar rules documentation.

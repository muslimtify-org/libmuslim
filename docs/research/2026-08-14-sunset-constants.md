# The sunset constants: where 959.63 comes from, and what changing the horizon cost

**Date:** 2026-08-14
**Oracles:** Skyfield 1.55 on JPL DE440 (`de440s.bsp`), JPL Horizons, and the library itself
**Code:** `hijri.h` (`HijriSunsetConvention`, `hijri__sun_upper_limb_altitude`)
**Fixtures:** `tests/test_hijri.c`, `tests/test_ephemeris_oracle.c` group 14
**Issue:** #33

Every number in this note was measured on 2026-08-14 unless it is quoted as a prior measurement, in which case its source is named. Nothing is estimated.

## What changed

The library used to solve sunset against a fixed target, 34' 00.12" of refraction plus a fixed 16' 00.12" of solar semidiameter, spelled as two macros. Both official methods the library reproduces disagree with that on both terms. Each now carries its own `HijriSunsetConvention`, a value type of two double fields, `refraction_at_horizon_deg` and `solar_semidiameter_arcsec_at_1au`, with three instances, `MUHAMMADIYAH {0.575, 959.63}`, `KEMENAG {0.575, 959.63}` and `ASTRONOMICAL {0.5667, 959.63}`. `hijri_find_sunset()`, `hijri_find_moonset()` and `hijri_compute_evening_parameters()` all changed signature to take a `const HijriSunsetConvention *conv`, which is documented non-NULL with no runtime check, exactly as `loc` already is. Selection happens in one place, `hijri_evaluate_evening()`, which already holds the predicate.

The fixed semidiameter is gone. `hijri__sun_upper_limb_altitude()` computes `959.63 / 3600.0 / sun.distance_au` at the sampled instant and the solver targets that sum at zero, so the crossing target now breathes over the year the way both books' tables do. The two published methods do not differ from each other today, and the type exists anyway, because they are separate published methods that happen to agree rather than one method with three names.

## Step 1: the provenance of 959.63

This is the part of the change with the weakest paper trail, so it was re-derived from physics rather than inherited from the issue body.

**Neither book states the constant.** Both the Pedoman Hisab Muhammadiyah and Kemenag's Ephemeris Hisab Rukyat supply the solar semidiameter as per-date tabulated input data, not as a constant at unit distance. So there is no sentence in either primary source to cite for 959.63. What can be established is whether their tables behave as `959.63 / r` does, and that is what was tested.

The method inverts each published figure. If a book prints a semidiameter `s` for an instant whose true Earth-Sun distance is `r` astronomical units, then the constant it is implicitly using is `s * r`, and the prediction to check is `959.63 / r` against `s`. Distances came from DE440 through Skyfield 1.55, apparent geocentric, and were cross-checked against the library's own `hijri_sun_position().distance_au`.

Kemenag's tabulated extremes, quoted as 15' 43.90" at aphelion and 16' 15.89" at perihelion, against the 2023 extremes of the edition they appear in:

| instant | DE440 distance, au | predicted from 959.63 | published | residual | implied constant |
|---|---|---|---|---|---|
| aphelion 2023-07-06 20:00 UT | 1.01668064 | 943.8854" = 15' 43.89" | 943.90" | -0.0146" | 959.6449 |
| perihelion 2023-01-04 16:30 UT | 0.98329553 | 975.9324" = 16' 15.93" | 975.89" | +0.0424" | 959.5883 |

The Pedoman's worked figures. The date 2008-12-06 is the one the issue records, and the book gives no time of day, so the whole day was sampled:

| instant | DE440 distance, au | predicted from 959.63 | published | residual | implied constant |
|---|---|---|---|---|---|
| 2008-12-06 12:00 UT | 0.98521239 | 974.0336" = 16' 14.03" | 974.03" | +0.0036" | 959.6264 |
| 2008-12-06 00:00 UT | 0.98528458 | 973.9623" = 16' 13.96" | 973.95" | +0.0123" | 959.6179 |

The third Pedoman figure, 16' 04.01", carries no date in the issue. Inverted it implies 0.995456 au, a distance the Earth occupies on 2008-10-21, where DE440 gives 0.99544893 au and the prediction is 964.017" against the published 964.01", a residual of +0.007". That row is offered as consistent rather than as a dated check, because the date was found from the figure and not the other way round.

**Worst residual across all five figures: 0.0424 arcsec.** For scale, the crossing target itself is 34' 30" plus about 16', roughly 3030 arcsec, so the worst residual is 1.4e-5 of the quantity being pinned. Converted through the Sun's own vertical rate at the horizon it is far below a millisecond of sunset.

The largest residual, the Kemenag perihelion row, is not evidence against 959.63, because perihelion distance itself moves year to year and Kemenag's tabulated extreme need not come from 2023. Over 2020 to 2030 the perihelion semidiameter computed from 959.63 ranges 975.8918" to 975.9840" and the aphelion one ranges 943.8406" to 943.9196". Both published extremes, 975.89" and 943.90", fall inside those ranges. The interannual spread, about 0.09" and 0.08", is larger than either residual, so the residuals are within the ambiguity of which year the table was built for.

The library corroborates independently. Its own `hijri_sun_position()` at the same four instants gives 0.98511740, 0.98518221, 0.98330903 and 1.01669594 au, which is at most 9.5e-5 au from DE440 and moves the derived semidiameter by at most 0.10 arcsec. That is the library's solar distance error showing up in the semidiameter, and it is a quarter of an order below the 34' 30" refraction difference this change is really about.

**This is a consistency check, not a citation, and the distinction is the finding.** The honest claim is: 959.63 arcsec at unit distance reproduces every published semidiameter figure recorded from both primary sources to within 0.05 arcsec, which is inside the year-to-year variation of the extremes themselves, and the implied constants recovered from those figures span 959.588 to 959.645 with 959.63 sitting inside that span. The claim NOT made is that either book specifies 959.63. Neither does. The figure itself is the standard solar semidiameter at unit distance, the IAU value, and it is cited to that, not to the pedoman. What the pedoman evidence establishes is that both books are using the same physical constant the library now uses, which is the question that actually mattered, since the library has to reproduce their arithmetic and not their bibliography.

Halt condition 3 was checked and did not fire.

## The binding fix gate

Measured before the numeric change and again after, on the same fixtures.

| gate metric | required | before | after |
|---|---|---|---|
| kemenag support_topo | at or above 33 | 33 | 33 |
| kemenag support_geo | at or above 34 | 34 | 34 |
| kemenag early_topo | exactly 0 | 0 | 0 |
| kemenag early_geo | exactly 0 | 0 | 0 |
| muhammadiyah support | exactly 12 | 12 | 12 |
| muhammadiyah early | exactly 0 | 0 | 0 |
| pedoman worked-example residual | not worse | 0.279564 arcmin | 0.236703 arcmin |

No month start moved in either official calendar. The one gate metric that moved, moved the right way: the Pedoman worked example is the only external anchor in the suite that is sensitive to the sunset convention at all, and it improved by 0.042861 arcmin, about 2.6 arcsec. That is a small number and it is the entire external evidence that the new constants are closer to the books than the old ones. The calendars did not move because a 34.5 arcsec change in the horizon moves sunset by about 2.3 s, and no month start in the fixture sits that close to a threshold. That is a statement about how far the fixture rows sit from their boundaries, not about the change being inert.

`test_umm_al_qura_official_calendar` is unchanged. `test_kemenag_published_quantities` altitude containment holds for every row with no tolerance.

## Group 14, regenerated

The set-solver fixture from issue #37 holds the convention equal on both sides by construction, so it had to be regenerated against the new convention or it would have measured the convention change instead of implementation error. Both sides moved together, which is the result that says the regeneration was done right.

| site | sunset before | sunset after | moonset before | moonset after |
|---|---|---|---|---|
| jakarta | 1.5623 s | 1.5945 s | 0.3862 s | 0.3864 s |
| mecca | 1.8397 s | 1.8741 s | 0.6073 s | 0.6075 s |
| mid45 | 2.2017 s | 2.2487 s | 1.7952 s | 1.7954 s |
| high60 | 2.7380 s | 2.8107 s | 7.2926 s | 7.3076 s |

The sunset maximum rose 2.7 percent and the moonset maximum rose 0.2 percent. `TOL_SETSOLVE_SUNSET_S` moved from 5.5 to 5.7 to keep roughly 2x margin on the new maximum. Every other tolerance in the file, including all four per-site moonset bounds, is unchanged. Had the regeneration got the convention wrong on one side these would have moved by seconds, not by hundredths.

## The Skyfield version difference is zero at the stored resolution

The committed tables were generated under Skyfield 1.54. The regeneration ran under 1.55, so a version change is confounded with the constant change unless it is separated. It was separated by a control run: the generator was re-run with the OLD constants under Skyfield 1.55 and reproduced all 96 committed values bit for bit at nine decimals.

That single run carries two conclusions and it is the strongest evidence in the change.

1. The frame replication is correct. The generator, rebuilt for the new convention, still reproduces the old table exactly when handed the old constants, so nothing in the regeneration path drifted.
2. The Skyfield 1.54 to 1.55 difference collapses to zero at the resolution the tables store. Every difference visible in the regenerated tables is therefore attributable to the constants and to nothing else.

Without that control the 2.7 percent sunset movement above would have been uninterpretable.

## The near-cancellation on the Moon's limb

The naive prediction is wrong and it is worth writing down why, because the naive prediction is what a reviewer will compute in their head.

Refraction rises by 0.5 arcmin, from 34' 00.12" to 34' 30", and in the Moon's upper-limb altitude that term is added straight to the limb. So the naive expectation is that every reported Moon altitude at sunset rises by about +0.5 arcmin. It does not. The measured shift is under 0.25 arcmin in either direction.

The missing term is the sampling instant. The same larger target also delays sunset, by about 2.3 s. Near the horizon the Moon falls at 0.00403 deg per second, so 2.3 s of delay costs about 0.56 arcmin of Moon altitude. The two effects are opposite in sign and nearly equal in size, so they very nearly cancel, and which one wins at a given epoch depends on the Moon's actual vertical rate there, which is why the residual goes both ways.

Two measured pins, both from the Muhammadiyah maklumat fixture:

| row | limb altitude before | after | shift |
|---|---|---|---|
| muh_maklumat_2024_03_10 | 63.669 arcmin | 63.582 arcmin | -0.087 arcmin |
| muh_maklumat_2025_05_27 | 97.345 arcmin | 97.583 arcmin | +0.238 arcmin |

This is also why the official calendars did not move. A convention change that looks like half an arcminute on paper delivers a quarter of that or less to the quantity the criteria actually read.

## New assertions, and one that did not earn its keep

Two assertions were added to hold the split load-bearing.

1. The two pedoman conventions solve bit-identical sunsets across all 48 rows. They carry equal constants today, so this asserts that the plumbing routes them separately and that any future divergence in one shows up rather than being silently shared.
2. The astronomical convention separates from Kemenag by a measured minimum of 2.0035 s and a maximum of 6.9093 s, floored at 1.0 s. The floor is well below the measured minimum on purpose, since the assertion exists to catch a collapse of the two conventions into one, not to pin the separation.

The suite is now 1126 checks in the oracle binary.

**The recorded finding, which is not flattering to the new assertions.** The mutation that restores a fixed solar semidiameter WAS caught, but by the pre-existing convergence assertion, not by either assertion added above. Assertion 2 does not catch it because the mutation moves both the astronomical and Kemenag sides identically, so their separation survives. Assertion 1 does not catch it because both pedoman conventions stay identical to each other under it. Both new assertions are aimed at the convention split, and the semidiameter is the one term the split does not vary. Recorded rather than papered over, because a reader deciding whether to trust the new assertions should know what they do not cover.

## What is still deliberately omitted

Solar parallax, about 8.8 arcsec and worth up to about 1 s of sunset, and horizon dip. Both are left out and both omissions are unchanged by this work. The reasons are unchanged too and are recorded in `hijri.h` and in `docs/research/2026-08-01-wujudul-hilal-convention.md`: the published methods this library reproduces omit parallax, and dip has been measured twice as making agreement worse.

The 0.5 arcmin refraction gap that `docs/research/2026-08-01-wujudul-hilal-convention.md` recorded as negligible is now closed rather than negligible. The library carries 34' 30" for both pedoman methods.

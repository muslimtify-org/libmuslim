# Cross-engine error bar

**Date:** 2026-08-02
**Oracles:** JPL Horizons (retrieved 2026-07-30) and Skyfield 1.54 / JPL DE440 (de440s.bsp)
**Fixture:** `tests/test_ephemeris_oracle.c`, 24 TT epochs, JD 2415020.5 - 2488069.5

Every number below was printed by `./build/tests/test_ephemeris_oracle` over all
24 epochs. None is a single-epoch spot check and none is carried over from an
earlier document. The exact printed lines are quoted under
[Reproducing these figures](#reproducing-these-figures).

## Headline error bar

Total deviation from apparent truth, which is where the body actually is:

| body | quantity | error bar |
|---|---|---|
| Moon | ecliptic longitude | **0.0051 deg** |
| Moon | ecliptic latitude | 0.0006 deg |
| Moon | distance | 41.9 km |
| Sun | apparent ecliptic longitude | **0.0084042 deg** |

The Moon figures are the pre-existing comparison against apparent JPL Horizons,
unchanged by this work and reconfirmed by this run. The Sun figure is new. It is
the first time `hijri_sun_position()` has been compared against any ephemeris
oracle at all.

**The Sun is the larger error source.** Its 0.0084042 deg exceeds the Moon's
0.0012755 deg series-truncation error by 0.0084042 / 0.0012755 = 6.6x, and it
exceeds even the Moon's total apparent longitude residual by
0.0084042 / 0.0051 = 1.6x. Sunset drives every predicate in this
library and elongation is a Sun-minus-Moon angle, so this is the most
consequential single result of the run.

The Sun's residual is nonetheless inside the roughly 0.01 deg that `hijri.h`
documents for Meeus ch. 25 low-precision solar theory. No defect is established.
See [Is the solar residual a defect](#is-the-solar-residual-a-defect) below.

## Why the headline includes the deliberate omissions

The Moon headline is 0.0051 deg against apparent Horizons, not the smaller
0.0012755 deg measured against mean-of-date DE440. The larger figure is the
right one to quote because a visibility criterion thresholds a physical
configuration. MABIMS 2021 asks whether the topocentric elongation reaches 6.4
deg and the topocentric altitude reaches 3 deg, and those are questions about
where the Moon and Sun actually are as seen from the site. An error of a given
size moves the computed answer by exactly that much whether it arose from a
truncated series or from a term this library deliberately does not evaluate. The
distinction matters for deciding what to improve. It does not matter for stating
how far the answer can be off.

The decomposition below is what separates the two. It is reported alongside the
headline rather than instead of it.

## Decomposition

| Comparison | Measured |
|---|---|
| Group 1, Skyfield vs Horizons, both apparent | lon 0.0000668 deg, lat 0.0000113 deg, dist 0.0 km |
| Group 2, library vs mean-of-date DE440 (Moon) | lon 0.0012755 deg, lat 0.0005528 deg, dist 5.0 km |
| Group 3, library Sun vs apparent DE440 | lon 0.0084042 deg (signed: mean +0.0020391, min -0.0024953, max +0.0084042) |
| Group 4, elongation error | 0.0070530 deg |
| Group 4, frame mismatch (apparent vs mean reference) | 0.0055998 deg |

**Group 1 is harness verification.** Two independently written clients, two
independent time-scale conversions and two independent frame transformations
agree on the apparent Moon to 0.0000668 deg. That is 0.0051 / 0.0000668 = 76x
below the residual being measured, so the comparison machinery is not a limiting
term in anything below it. This group compares two compiled-in tables and never
calls the library, which is why it also carries an explicit non-degeneracy check
against a table accidentally pasted twice.

**Group 2 isolates what the truncated series costs.** Removing nutation and
aberration from the reference rather than leaving them in the residual gives
0.0012755 deg. Compared against the 0.0051 deg apparent residual that is
0.0051 / 0.0012755 = 4.0x. So roughly three quarters of the Moon's apparent
longitude error is the omitted physics and the remaining quarter is the series.

This confirms the claim already written at `hijri.h:71-73`, that the apparent
residual "is essentially that omitted nutation, not series error". That claim
was previously an inference from the size of nutation. It is now a measurement,
and the measured ratio is 4x rather than anything larger.

**Group 4 measures the frame question directly** rather than arguing it. See
[Frame consistency](#frame-consistency).

### Is the solar residual a defect

No, on the evidence available. The signed residuals over the 24 epochs run from
-0.0024953 deg to +0.0084042 deg with a mean of +0.0020391 deg. Both signs
occur and the mean is about a quarter of the extreme, so this is a two-sided
spread around approximately zero rather than a one-sided offset. That is the
same discriminator this codebase already applies to the 385000.56 km lunar
distance constant: a one-sided offset would indicate a structural error in a
coefficient, a sign or a convention, while a two-sided spread indicates series
truncation noise. The magnitude also sits inside the roughly 0.01 deg the header
documents for Meeus ch. 25.

The precondition for a solar code change is therefore not met. Nothing here
demonstrates a defect in `hijri_sun_position()`.

## Propagation to criterion parameters

Altitude is the body's direction expressed in a rotated frame. A rotation
preserves angles, so an angular position error of epsilon produces at most
epsilon of altitude error. The bound is analytic and requires no perturbation
study or numerical experiment. Likewise the error in an angular separation
between two bodies is bounded by the sum of the two bodies' individual angular
errors, by the triangle inequality on the sphere.

Resulting bounds:

| criterion parameter | error bar | threshold | ratio |
|---|---|---|---|
| Moon altitude | 0.0051 deg | 3 deg (MABIMS 2021) | 3.0 / 0.0051 = 588x |
| Sun altitude (sunset) | 0.0084042 deg | n/a, sets the evening | see below |
| Sun-Moon elongation, analytic bound | 0.0084042 + 0.0051 = 0.0135042 deg | 6.4 deg (MABIMS 2021) | 6.4 / 0.0135042 = 474x |
| Sun-Moon elongation, measured | 0.0070530 deg | 6.4 deg | 6.4 / 0.0070530 = 907x |

The analytic bound and the direct measurement agree in the expected direction:
the measured 0.0070530 deg is smaller than the 0.0135042 deg worst case, by
0.0135042 / 0.0070530 = 1.9x, because the two bodies' errors do not attain their
maxima at the same epoch and do not always carry the same sign. Quote the
measured figure where the epochs are these 24, and the analytic bound where they
are not.

The sunset case is the one that does not reduce to a direct angle comparison.
A solar longitude error propagates into the sunset instant through the Sun's
apparent motion. The Sun moves about 0.9856 deg per day in longitude, a standard
published rate and not a measurement from this run, so 0.0084042 deg is about
0.0085 days of solar motion, but the relevant rate at the horizon is the Earth's
rotation at roughly 15 deg per hour of hour angle, likewise standard, which
converts 0.0084042 deg of position error into under 3 seconds of sunset time.
That is not a separate measurement, only the same figure expressed in the unit
the sunset search uses.

## Frame consistency

`hijri_sun_position()` returns an **apparent** longitude. It applies both the
aberration term and the nutation term at `hijri.h:504-506`.
`hijri_moon_position()` returns the **mean equinox of date** with neither, and
uses the mean obliquity at `hijri.h:760`. The file header at `hijri.h:71`
previously stated that no nutation and no aberration are applied, a claim
accurate for the Moon and inaccurate for the Sun. That header block has since
been corrected on this branch and now distinguishes the two bodies explicitly.

The elongation at `hijri.h:1115-1118` is then computed as the angular separation
between these two differently framed directions. Nutation in longitude shifts
every ecliptic longitude by the same dpsi, so it cancels exactly in a
Sun-minus-Moon difference provided both bodies share a frame. Here they do not,
so dpsi is injected into the difference instead of cancelling.

The inconsistency is real. Its measured consequence is 0.0070530 deg, which is
6.4 / 0.0070530 = 907x below the MABIMS 2021 elongation threshold. The companion
frame-mismatch figure, the same reference computed both-apparent versus
both-mean, is 0.0055998 deg, and it confirms the mechanism: the injected term is
of exactly the size the frame difference predicts, and it accounts for
0.0055998 / 0.0070530 = 79 percent of the total elongation error.

**Conclusion: a real inconsistency, measured as immaterial at the 6.4 deg
threshold.** At 907x below the threshold it cannot move a criterion outcome
except on an evening already within 0.007 deg of the boundary, which is far
tighter than the roughly 0.15 deg window that `2026-07-30-findings.md` identifies
as the band where an ephemeris change can flip a decision. The proportionate
remedy is therefore a correction to the header text at `hijri.h:71-73`, which is
currently wrong about the Sun, and not a change to the code. No code change is
recommended and none was made. This is recorded as an open question below.

An honest reading of this section is that the frame question was raised, was
measured, and came back immaterial. That is a valid result and it is not being
inflated into a problem.

## Method and reproducibility

The four Skyfield tables were produced by a generator run once by hand in the
scratchpad and deliberately not committed, matching how the Horizons `curl`
invocation is recorded in the test file rather than automated. The repository
stays pure C with no network dependency and no Python in `make check`. All
comparison happens at test time against compiled-in constants.

- **Skyfield 1.54.**
- **Kernel: `de440s.bsp`**, spanning JD 2396752.50 to 2506352.50. The fixture
  epochs span JD 2415020.5 to 2488069.5 and so lie inside the kernel.
- **24 TT epochs**, the same set the Horizons fixture already used.
- **Nutation removed explicitly.** Skyfield's `framelib.ecliptic_frame` is the
  *true* equinox of date, not the mean equinox. Meeus ch. 47 targets the mean
  equinox, so the geometric tables subtract nutation in longitude computed by
  `skyfield.nutationlib.iau2000b`. The generator asserts a fixed reproduction
  value before emitting any table, so a change in this convention halts
  generation rather than silently shifting a fixture.
- **IAU 2000B accuracy.** It is the truncated nutation model, accurate to about
  0.001 arcsec, which is 2.8e-7 deg. Against the smallest quantity it is used to
  measure, the 0.0012755 deg Moon truncation error, that is
  0.0012755 / 2.8e-7 = 4555x smaller. It is not a limiting term anywhere in this
  note.

### Reproducing these figures

Build and run the oracle test from a current checkout:

```sh
make test && ./build/tests/test_ephemeris_oracle
```

The lines this run printed, verbatim:

```
sky_vs_horizons max deviation: lon 0.0000668 deg lat 0.0000113 deg dist 0.0 km
moon_truncation max: lon 0.0012755 deg lat 0.0005528 deg dist 5.0 km
sun_apparent max lon: 0.0084042 deg
sun_apparent signed lon: mean 0.0020391 deg min -0.0024953 deg max 0.0084042 deg
elongation_err max: 0.0070530 deg
frame_mismatch max (apparent vs mean reference): 0.0055998 deg
elongation_frame_unified max: 0.0083813 deg
max_lon_err=0.0051 deg  max_lat_err=0.0006 deg  max_dist_err=41.9 km
Moon ephemeris tests: 299 checks, 0 failures
```

Every measured figure in this note is one of those numbers or a division between
two of them, and every division is written out where it is claimed. The note
also cites figures that are not measurements from this run, and each is labelled
with its source where it appears: the 3 deg and 6.4 deg MABIMS 2021 thresholds,
IAU 2000B's 0.001 arcsec accuracy, the roughly 1.2 arcsec solar ecliptic
latitude the library models as zero, the 0.15 deg near-boundary band from
docs/research/2026-07-30-findings.md, the 0.003 deg TOL_GEOM_LON_DEG bound, the
1e-6 deg per-coefficient sensitivity, and the two standard rates used in the
sunset conversion below.

The Skyfield tables themselves are not reproducible from a checkout, because the
generator is not committed. They are reproducible from the recorded method above
given Skyfield 1.54 and `de440s.bsp`. The committed tables are the record.

### Fixture sensitivity, and one known gap

Each of the four new tables was deliberately mutated, the suite rebuilt, and the
result recorded verbatim in `tests/test_ephemeris_oracle.c`. One mutation per
table, M1 through M3 and M5, plus M4 which mutates a `hijri.h` coefficient
rather than a table. Four of the five were caught:

- **M1**, mean-of-date Moon longitude moved +0.01 deg: caught by `moon_geom_lon`.
- **M2**, apparent Sun longitude moved +0.05 deg: caught twice, by
  `sun_apparent_lon` and again by `elongation_vs_apparent_ref`, because the Sun
  table also feeds the group 4 reference.
- **M3**, the apparent-Moon table replaced with a verbatim copy of the Horizons
  fixture: caught only by the non-degeneracy guard. Every per-epoch comparison
  passed with a residual of exactly zero. Without that guard the suite would
  have reported 275 checks and 0 failures on a table pasted twice.
- **M4**, one `hijri__moon_lr` longitude coefficient changed by +1 unit:
  **NOT caught.** The suite reported 275 checks, 0 failures and exited 0. Two
  printed maxima moved, both far below any tolerance: group 2 shifted from
  0.0012755 to 0.0012752 deg, and group 4 from 0.0070530 to 0.0070533 deg,
  each about 3e-7 deg.
- **M5**, mean-of-date Sun longitude moved +0.05 deg: caught by
  `elongation_frame_unified`, with the suite reporting 299 checks and
  1 failure. M5 was added after a verification pass observed that
  `SKY_SUN_GEOMETRIC` was the one table no assertion read, so the claim that
  all four tables were mutation-proven was false when first written.
  `check_group5_frame_counterfactual()` now reads it. The check counts differ
  between M1 to M4 and M5, 275 against 299, because that group was added later.

M4 is a genuine limit of this fixture and is recorded as such rather than
smoothed over. The check that would have to tighten is `moon_geom_lon`, bounded
at 0.003 deg against a measured 0.0012755 deg residual. One coefficient unit
moves lambda by at most 1e-6 deg, so the residual is about
0.0012755 / 1e-6 = 1275x one coefficient unit. Catching a single unit would mean
pinning the residual to within 3e-7 deg of its current value, which asserts the
truncation error rather than bounding it and would then fail on any legitimate
reference update or compiler change. Per-coefficient sensitivity is covered
instead by the 1e-6 deg assertion against Meeus's own printed worked Example
47.a, which remains the only unit-sensitive check in the file.

## Open questions

- **The `hijri.h:71-73` header text was wrong about the Sun, and has been
  corrected.** It stated the file applies no nutation and no aberration. That
  holds for the Moon and not for `hijri_sun_position()`, which applies both at
  `hijri.h:504-506`. The header now distinguishes the two bodies. The change is
  comment-only and the committed 2020-2025 baseline is byte-identical across it,
  which is the evidence that no behaviour moved.
- **The mixed-frame elongation at `hijri.h:1115-1118` is left as it stands, and
  unifying the frames is measured to be a REGRESSION rather than an
  improvement.** Making both bodies mean-of-date, by removing the nutation and
  aberration terms the library itself applies at `hijri.h:505-506` and
  recomputing against a both-mean DE440 reference, gives 0.0083813 deg of
  elongation error against 0.0070530 deg for the current mixed frame. That is
  about 19 percent worse. The mismatch partially cancels the solar truncation
  error, and removing it exposes the full solar residual of 0.0084042 deg, which
  the 0.0083813 deg figure lands almost exactly on. So frame unification is a
  correctness-of-description change carrying a small accuracy cost, not an
  accuracy gain. The quantity that actually limits elongation accuracy is the
  Meeus ch. 25 low-precision solar theory, and only replacing that theory moves
  the number. The tolerance `TOL_ELONG_DEG` pins current behaviour, so if the
  frame is ever unified the diff will be deliberate and visible rather than
  silent drift.
  This comparison was first computed with a throwaway probe. It is now a
  committed standing assertion, `check_group5_frame_counterfactual()` in
  `tests/test_ephemeris_oracle.c`, which prints `elongation_frame_unified max`
  and bounds it with `TOL_ELONG_UNIFIED_DEG`. The figure is therefore
  reproducible from a checkout by running the binary, not merely described here.
  That group is also the only assertion reading `SKY_SUN_GEOMETRIC`, which is
  what holds that table to the same mutation-proven standard as the other three
  (mutation M5).
- **Solar latitude is modelled as zero.** The Sun tables carry an ecliptic
  latitude column for format symmetry but no assertion is made against it. Zero
  is correct to about 1.2 arcsec, which is 3.3e-4 deg and well inside the
  0.0084042 deg longitude residual, so it is not currently a limiting term. It
  is recorded rather than asserted.
- **The M4 gap above.** No achievable bound on this fixture reaches single
  coefficient sensitivity for the lunar series. Only the worked-example check
  does.
- **Neither oracle validates the criteria themselves.** This note bounds where
  the library thinks the bodies are. It says nothing about whether MABIMS 2021,
  Odeh or Yallop correctly predict a sighting. The open items in
  [`2026-07-30-findings.md`](2026-07-30-findings.md) on the missing primary
  MABIMS standard and the paywalled Odeh paper are untouched by this work.

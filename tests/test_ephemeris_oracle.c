/* test_ephemeris_oracle.c -- validates hijri_sun_position() and
 * hijri_moon_position() against two independent oracles: JPL Horizons and
 * Skyfield/JPL DE440.
 *
 * Build:  gcc -std=c11 -Wall -Wextra -Wpedantic -O2 tests/test_ephemeris_oracle.c -lm \
 *             -o /tmp/libmuslim-test-ephemeris
 *
 * The fixture below was retrieved on 2026-07-30 with:
 *
 *   curl -s -G "https://ssd.jpl.nasa.gov/api/horizons.api" \
 *     --data-urlencode "format=text" \
 *     --data-urlencode "COMMAND='301'" \
 *     --data-urlencode "OBJ_DATA='NO'" \
 *     --data-urlencode "MAKE_EPHEM='YES'" \
 *     --data-urlencode "EPHEM_TYPE='OBSERVER'" \
 *     --data-urlencode "CENTER='500@399'" \
 *     --data-urlencode "TLIST=<the 24 JD values below>" \
 *     --data-urlencode "TLIST_TYPE='JD'" \
 *     --data-urlencode "TIME_TYPE='TT'" \
 *     --data-urlencode "QUANTITIES='31,20'" \
 *     --data-urlencode "CSV_FORMAT='YES'"
 *
 * Columns are geocentric apparent ecliptic longitude and latitude of date, and
 * apparent range. Range was converted from AU to km with 1 AU = 149597870.7 km.
 *
 * The SECOND oracle, Skyfield evaluating JPL DE440, has its provenance and
 * generator transcription recorded at the SKY_MOON_APPARENT table further
 * down, alongside the tables it produced; the Horizons Sun fixture
 * (HORIZONS_SUN, COMMAND='10', retrieved 2026-08-02) is recorded at its own
 * table. Two engines, both bodies, same 24 epochs.
 *
 * TOLERANCES -- these are measured, not guessed.
 *
 *   Horizons reports APPARENT positions. Meeus ch. 47 yields GEOMETRIC longitude
 *   referred to the mean equinox of date, and this library applies neither
 *   nutation nor aberration. The residual is therefore dominated by nutation in
 *   longitude, which reaches about 0.005 deg.
 *
 *   Measured maximum error over these 24 epochs:
 *     current 6/4/4-term series : lon 0.1678 deg, lat 0.1068 deg, dist 982.2 km
 *     full Meeus 47 series      : lon 0.0051 deg, lat 0.0006 deg, dist  41.9 km
 *
 *   0.02 deg leaves about 4x margin on longitude and 33x on latitude, while the
 *   old series misses by 8x the bound. 100 km on distance gives 2.4x margin over
 *   the measured 41.9 km worst absolute case. Signed distance errors over these
 *   24 epochs run -41.8864 km to +40.4101 km with a mean of +1.9650 km. The
 *   near-zero mean against a two-sided spread of that size rules out a
 *   structural error in the 385000.56 km constant or the AU conversion -- either
 *   would show as a large one-sided offset -- and identifies the spread as
 *   Sigma-r truncation noise.
 */

#define HIJRI_IMPLEMENTATION
#include "../hijri.h"

#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define TOL_LON_DEG 0.02
#define TOL_LAT_DEG 0.02
#define TOL_DIST_KM 100.0

/* Group 2, library against mean-of-date DE440 -- the truncation error alone,
 * with nutation and aberration removed from the reference rather than left in
 * the residual. Measured maximum over these 24 epochs, printed by this binary
 * as "moon_truncation max": lon 0.0012755 deg, lat 0.0005528 deg, dist 5.0 km.
 * Each bound below is that measured maximum rounded up to leave roughly 2x
 * margin -- 2.35x on longitude, 2.17x on latitude, 2x on distance. */
#define TOL_GEOM_LON_DEG 0.003
#define TOL_GEOM_LAT_DEG 0.0012
#define TOL_GEOM_DIST_KM 10.0

/* Group 3, library Sun against apparent DE440 -- the first oracle comparison
 * hijri_sun_position() has ever had. Measured maximum over these 24 epochs,
 * printed by this binary as "sun_apparent max lon": 0.0084042 deg. That sits
 * inside the ~0.01 deg the header documents for Meeus ch. 25 low-precision
 * solar theory, so the theory is performing as specified.
 *
 * Signed residuals over the same 24 epochs run -0.0024953 deg to +0.0084042
 * deg with a mean of +0.0020391 deg. Both signs occur and the mean is about a
 * quarter of the extreme, so this is a two-sided spread rather than a
 * one-sided offset -- series truncation noise, not a structural error in a
 * coefficient, sign or convention. Same discriminator this file already
 * applies to the 385000.56 km distance constant.
 *
 * 0.017 deg is the measured maximum rounded up to leave 2.02x margin. */
#define TOL_SUN_LON_DEG 0.017

/* Group 4, the library's Sun-minus-Moon longitude difference against a
 * consistently framed both-apparent DE440 reference. Measured maximum over
 * these 24 epochs, printed by this binary as "elongation_err max": 0.0070530
 * deg. The companion "frame_mismatch max" line -- the same reference computed
 * both-apparent versus both-mean -- measures 0.0055998 deg over the same
 * epochs.
 *
 * Both sit far below the 6.4 deg MABIMS 2021 elongation threshold: 6.4 /
 * 0.0070530 = 907x and 6.4 / 0.0055998 = 1143x, i.e. three orders of
 * magnitude. So the mixed-frame difference this library computes is not a
 * source of criterion-outcome error at that threshold. The bound below exists
 * to pin current behaviour, so a later frame change shows up as a deliberate
 * diff rather than silent drift.
 *
 * 0.015 deg is the measured maximum rounded up to leave 2.13x margin. */
#define TOL_ELONG_DEG 0.015

/* GROUP 1, oracle-versus-oracle. These bound the agreement between Skyfield
 * and Horizons, not between the library and anything, so they are deliberately
 * far tighter than every other bound in this file: the two oracles derive from
 * the same JPL ephemeris family and should agree to the fixtures' own printing
 * resolution. Measured over these 24 epochs:
 *
 *     lon 0.0000668 deg      lat 0.0000113 deg      dist 0.0 km
 *
 * The longitude bound keeps only 1.50x margin, which is intentional. A loose
 * bound here would defeat the point, since this is the check that would catch
 * a time-scale or frame mistake in the comparison harness. The distance bound
 * is not a 2x rule: both tables print distance to 0.1 km, so 0.0 km means
 * "agrees to the printing resolution", and 0.5 km is five printing units.
 *
 * GROUP 5, the frame-unification counterfactual. Measured 0.0083813 deg,
 * rounded up to leave 2.03x margin.
 *
 * GROUP 6, the solar harness, reuses the TOL_ORACLE_* bounds: measured
 * lon 0.0000675 deg, lat 0.0000118 deg, dist 0.0 km, so the longitude bound
 * keeps 1.48x margin, same deliberate tightness as group 1.
 *
 * GROUP 7, the shipped elongation path. Measured 0.0065798 deg; 0.014 leaves
 * 2.13x margin.
 *
 * Coupling guard in group 5: the library's two solar frame terms against the
 * same physics recorded independently in the tables. Measured 0.0004844 deg,
 * which is the nutation Meeus ch. 25 truncates away (it keeps only the omega
 * term of dpsi; the discarded terms reach about 0.0005 deg). 0.001 leaves
 * 2.06x margin. */
#define TOL_ORACLE_LON_DEG 0.0001
#define TOL_ORACLE_LAT_DEG 0.0001
#define TOL_ORACLE_DIST_KM 0.5
#define TOL_ELONG_UNIFIED_DEG 0.017
#define TOL_ELONG_SHIPPED_DEG 0.014
#define TOL_SUN_TERMS_DEG 0.001
#define TOL_EQEQ_DEG 0.001

/* Group 9, library topocentric altitude against airless DE440. Measured
 * maximum over 4 sites and the first TOPO_EPOCH_COUNT epochs, printed by
 * this binary as
 * "topo_alt max": jakarta 0.0028647 deg, mecca 0.0033521 deg, mid45 0.0048619
 * deg, high60 0.0055047 deg. The bound below is the largest of those rounded
 * up to leave roughly 2x margin. */
#define TOL_TOPO_ALT_DEG 0.011

/* Group 10, the spherical-Earth approximation isolated. Measured maximum,
 * printed as "topo_counterfactual max": jakarta 2.18, mecca 6.77, mid45 12.63,
 * high60 13.35 arcsec. Bound is the largest rounded up to roughly 2x margin.
 * This is the flattening term issue #17 records as not worth fixing at MABIMS
 * latitudes, now measured rather than asserted. */
#define TOL_TOPO_CF_DEG 0.0075

/* Group 11, topocentric elongation against a convention-matched reference.
 * Measured maximum, printed as "elong_conv max": jakarta 0.0069925, mecca
 * 0.0060859, mid45 0.0064134, high60 0.0064678 deg. Bound is the largest
 * rounded up to roughly 2x margin. For scale, the MABIMS 2021 topocentric
 * elongation threshold is 6.4 deg. The separately printed "elong_full max" is
 * deliberately unbounded, see the group comment. */
#define TOL_ELONG_CONV_DEG 0.014

/* Group 12, Skyfield DE440 against Horizons, airless topocentric altitude.
 * Measured maximum, printed as "sky_vs_horizons_topo max": jakarta 0.0001098
 * deg, high60 0.0001015 deg. Bound is the larger rounded up to roughly 2x
 * margin. This bounds oracle agreement, not library accuracy. */
#define TOL_ORACLE_TOPO_DEG 0.00025

/* Group 13, hijri_delta_t_seconds() against IERS. The bound applies only to
 * the 20 epochs where the reference is IERS data, rows 2 through 21, printed
 * as "delta_t max error IERS-graded": 5.3532 s. The all-epoch figure
 * 134.8783 s is printed alongside but not asserted, because rows 0 and 1
 * predate IERS coverage and rows 22 and 23 are past it.
 *
 * The bound exists to catch a regression in the model, not to certify its
 * accuracy. The accuracy statement that matters is the sunset-anchored
 * residual printed by this group, 2.93 arcsec. */
#define TOL_DELTA_T_SEC 11.0

/* Group 14, the set solvers against oracle-solved instants with the convention
 * held equal on both sides. Measured maxima, printed by the temporary harness
 * in the preceding commit as "setsolve max": sunset 2.7380 s, moonset 7.2926 s.
 * Each bound below is that rounded up to leave roughly 2x margin.
 *
 * These are NOT the library's error against physical truth. They are its error
 * against an oracle using the library's own convention, which is the quantity
 * issue #18 asks for. The convention gap itself belongs to issue #33. */
#define TOL_SETSOLVE_SUNSET_S 5.5

/* Moonset is bounded per site, not once. Measured maxima: jakarta 0.3862 s,
 * mecca 0.6073 s, mid45 1.7952 s, high60 7.2926 s, an 18.88x spread against
 * sunset's 1.75x. A single bound sized for high60 would leave the Jakarta
 * assertion roughly 39x slack, which is not an assertion. The high60 figure is
 * not anomalous: three of its twelve rows are ones where the Moon set before
 * sunset and the solver walked to the following night, a different regime with
 * a different error scale, kept because they exercise the 24 hour scan.
 * Each site's bound sits in tol_moonset_s in SETSOLVE_SITES. */

/* Bisection convergence, measured as 1.296e-07 deg by the same harness. The bound is
 * deliberately far above that, because this assertion exists to catch a solver
 * regression such as a reduced iteration count, not to certify the last bit.
 * 40 halvings of a one hour bracket reach about 3e-9 seconds, so any plausible
 * regression is orders of magnitude away from this bound. */
#define TOL_SETSOLVE_CONVERGE_DEG 1e-6

/* Horizons cross-check on the solar position the sunset crossing is solved
 * from. Measured maxima over the three rows, printed by this binary when the
 * bound was set to zero to read them off: right ascension 0.0034349 deg,
 * declination 0.0009276 deg. 0.007 is the larger of the two rounded up to leave
 * 2.04x margin, consistent with the roughly 2x margin used elsewhere in this
 * file. The figure sits below the 0.0084042 deg maximum solar position error
 * this file's header records against DE440, which is what a matching query
 * convention should produce. A residual an order of magnitude larger would mean
 * the query convention, not the library, had drifted. */
#define TOL_SETSOLVE_HORIZONS_DEG 0.007

/* The FLOOR every one of the same six cells must clear, which is a different
 * assertion from the ceiling above and exists for a different reason. The
 * ceiling bounds accuracy. This bounds INDEPENDENCE: it fails if the stored
 * table stops being an outside source.
 *
 * Measured over all three rows and both columns, printed by this binary as
 * "setsolve_horizons residual min ... max ...": the maximum is 0.0034349 deg
 * and the MINIMUM is 0.0000433 deg, the row 1 declination difference. The
 * floor binds against the minimum, so 1e-5 deg is 4.33x below the measured
 * minimum and 343x below the measured maximum.
 *
 * The other side of the choice is what the floor must separate from. A table
 * regenerated from the library and transcribed at the seven decimals Horizons
 * prints would differ from the library's own doubles by at most 5e-8 deg,
 * which is 200x below this floor. The floor therefore sits between the two
 * populations with two orders of magnitude of clearance on each side, and it
 * asserts that the sources disagree at all rather than asserting how much.
 * Sizing it near the measured residual would make it a second accuracy bound
 * that fails whenever the library improves. */
#define TOL_SETSOLVE_HORIZONS_FLOOR_DEG 1e-5

/* MEASURED MUTATION SENSITIVITY of the four tables above.
 *
 * The Meeus 47.a block further down records an exhaustive 120-mutation sweep of
 * the Sigma-l coefficients. The four Skyfield tables added here are held to the
 * same standard: each was deliberately mutated, the suite was rebuilt and run,
 * and the result below is what the binary actually printed. Every mutation was
 * reverted afterwards; nothing in this file or in hijri.h carries a mutation.
 *
 * Coverage, one mutation per table: M1 SKY_MOON_GEOMETRIC, M2 SKY_SUN_APPARENT,
 * M3 SKY_MOON_APPARENT, M5 SKY_SUN_GEOMETRIC, M6 HORIZONS_SUN. M4 mutates a
 * hijri.h coefficient rather than a table and is recorded as an uncaught gap.
 * The check counts differ across the records -- 275 for M1 through M4, 299 for
 * M5, 419 for M6 -- because the suite grew between rounds: M5 and group 5 were
 * added after a verification pass found SKY_SUN_GEOMETRIC had no assertion
 * reading it, and M6 with groups 6 and 7 after a review found the Sun rested
 * on a single generation harness and the shipped elongation path was untested.
 *
 * M1  SKY_MOON_GEOMETRIC row 0 longitude, 272.4120166 -> 272.4220166 (+0.01)
 *     CAUGHT
 *       FAIL moon_geom_lon at jd=2415020.5: got 272.4122702 want 272.4220166 (err 0.0097464 > tol 0.0030000)
 *
 * M2  SKY_SUN_APPARENT row 0 longitude, 280.1533846 -> 280.2033846 (+0.05)
 *     CAUGHT, and caught twice -- the Sun table feeds group 4 as well, so the
 *     elongation reference moves with it:
 *       FAIL sun_apparent_lon at jd=2415020.5: got 280.1535706 want 280.2033846 (err 0.0498140 > tol 0.0170000)
 *       FAIL elongation_vs_apparent_ref at jd=2415020.5: got 7.7413005 want 7.7867292 (err 0.0454287 > tol 0.0150000)
 *
 * M3  SKY_MOON_APPARENT body replaced with a verbatim copy of FIXTURE, i.e.
 *     the same table pasted twice.
 *     CAUGHT by the non-degeneracy guard, which is the only check in group 1
 *     that can see this: every per-epoch comparison passes with a residual of
 *     exactly zero, and the printed maximum collapses to zero:
 *       sky_vs_horizons max deviation: lon 0.0000000 deg lat 0.0000000 deg dist 0.0 km
 *       FAIL sky_vs_horizons_nondegenerate: expected a non-zero value, got 0.000000000
 *     This is the mutation that justifies check_true_nonzero() existing. Without
 *     it, a table pasted twice would report 275 checks and 0 failures.
 *
 *     Scope note, added after a review observed the max-only guard was weaker
 *     than this record implied: the guard M3 exercised catches only a
 *     byte-exact paste. A table copied with one row regenerated kept the max
 *     nonzero and passed. Groups 1 and 6 now also assert the MINIMUM
 *     per-epoch deviation nonzero, which fails if any single row is copied
 *     verbatim (measured minima 6.6e-6 and 5.4e-6 deg).
 *
 * M4  hijri__moon_lr row 0 Sigma-l coefficient, 6288774 -> 6288775 (+1 unit).
 *     NOT CAUGHT -- recorded as a gap, not as a pass.
 *     The suite printed "Moon ephemeris tests: 275 checks, 0 failures" and
 *     exited 0. The only visible movement anywhere in the output was the group 2
 *     maximum, 0.0012755 -> 0.0012752 deg, a shift of 3e-7 deg; group 4's
 *     maximum moved 0.0070530 -> 0.0070533 deg. moon_geom_lon did not fail.
 *
 *     Which check would need tightening, and why it cannot be: moon_geom_lon.
 *     Its bound is TOL_GEOM_LON_DEG = 0.003 deg against a measured residual of
 *     0.0012755 deg. A one-unit Sigma-l change moves lambda by at most 1e-6 deg,
 *     so catching it would mean pinning the residual to within 3e-7 deg of its
 *     unmutated value -- asserting the truncation error itself rather than
 *     bounding it, which would then fail on any legitimate reference or
 *     compiler change. The residual is about 1275x one coefficient unit, so no
 *     achievable bound on this fixture reaches unit sensitivity.
 *
 *     This is the same hard limit the 47.a block already documents for the
 *     Horizons fixture, and it is why the 1e-6 deg assertion against the
 *     publisher's worked example remains the only per-coefficient check in this
 *     file. The DE440 tables broaden the epoch coverage; they do not sharpen it.
 *     Note that the 47.a check did not catch this mutation either -- the run
 *     reported 0 failures overall, not merely 0 in groups 1 through 4 -- which
 *     puts row 1 in the +1 direction among the MISSED majority the sweep below
 *     counts at 87 / 120.
 *
 * M5  SKY_SUN_GEOMETRIC row 0 longitude, 280.1543331 -> 280.2043331 (+0.05).
 *     CAUGHT by elongation_frame_unified:
 *       FAIL elongation_frame_unified at jd=2415020.5: got 7.7422959
 *       want 7.7923165 (err 0.0500206 > tol 0.0170000)
 *     Suite went to "299 checks, 1 failures", exit 1.
 *
 *     M5 was added after a verification pass observed that SKY_SUN_GEOMETRIC
 *     was the one table no assertion read -- it fed only a printf, so the claim
 *     above that all four tables are mutation-proven was false when written.
 *     check_group5_frame_counterfactual() now reads it, which both closes that
 *     gap and makes the "unifying the frames measures worse" result
 *     reproducible from a checkout rather than resting on an uncommitted
 *     probe.
 *
 * M6  HORIZONS_SUN row 0 longitude, 280.1533171 -> 280.1633171 (+0.01).
 *     CAUGHT by the solar harness:
 *       FAIL sky_vs_horizons_sun_lon at jd=2415020.5: got 280.1533846
 *       want 280.1633171 (err 0.0099325 > tol 0.0001000)
 *     Suite went to "419 checks, 1 failures", exit 1.
 *
 * M7  SKY_MOON_APPARENT row 0 lon and lat replaced with the FIXTURE row 0
 *     values, a single row copied verbatim rather than the whole table.
 *     CAUGHT by the per-row minimum guard, and by nothing else -- every
 *     per-epoch tolerance check passed:
 *       FAIL sky_vs_horizons_min_nondegenerate: expected a non-zero value, got 0.000000000
 *     Suite went to "422 checks, 1 failures", exit 1. This is the case the
 *     M3 scope note records as invisible to the max-only guard.
 *
 * M10 through M14 cover the seven tables added for the topocentric error bar:
 * SKY_TOPO_JAKARTA, SKY_TOPO_MECCA, SKY_TOPO_MID45, SKY_TOPO_HIGH60,
 * SKY_DELTA_T, HORIZONS_TOPO_JAKARTA and HORIZONS_TOPO_HIGH60. They cover the
 * distinct failure modes rather than one mutation per table, which is the
 * pattern M1 through M9 already follow. Row 0 is jd 2415020.5 throughout, and
 * it is inside TOPO_EPOCH_COUNT, so a row 0 mutation is visible to groups 9
 * through 12. Rows 22 and 23 are excluded from those groups by amendment A4
 * and a mutation there would not be caught, by design. All five below were run
 * against the suite at 782 checks and every one was reverted.
 *
 * M10 SKY_TOPO_JAKARTA row 0 altitude, 23.9726098 -> 23.9826098 (+0.01).
 *     CAUGHT, by the oracle-versus-oracle harness and by it alone:
 *       FAIL sky_vs_horizons_topo_jakarta at jd=2415020.5: got 23.9826098 want 23.9725000 (err 0.0101098 > tol 0.0002500)
 *     Suite went to "782 checks, 1 failures", exit 1. Group 9 did not fire:
 *     TOL_TOPO_ALT_DEG is 0.011 deg and a 0.01 deg shift stays inside it. What
 *     pins each row of this table individually is group 12's 0.00025 deg
 *     oracle-agreement bound, which is two orders tighter because it compares
 *     two references to each other rather than a truncated model to a
 *     reference.
 *
 * M11 SKY_TOPO_HIGH60 row 0 altitude, -52.5432747 -> -52.5332747 (+0.01).
 *     CAUGHT twice, by the library check and by the oracle harness:
 *       FAIL topo_alt_high60 at jd=2415020.5: got -52.5475899 want -52.5332747 (err 0.0143152 > tol 0.0110000)
 *       FAIL sky_vs_horizons_topo_high60 at jd=2415020.5: got -52.5332747 want -52.5432710 (err 0.0099963 > tol 0.0002500)
 *     Suite went to "782 checks, 2 failures", exit 1.
 *     Recorded separately from M10 because a single shared tolerance could in
 *     principle be dominated by one site and leave another effectively
 *     unchecked. Group 9 fires here and not at M10 because the library
 *     residual at this row is already 0.0043152 deg and the shift adds to it
 *     rather than cancelling against it.
 *
 * M12 SKY_TOPO_MECCA row 0 convention-matched elongation, 7.1372572 ->
 *     7.1472572 (+0.01).
 *     NOT CAUGHT -- recorded as executed, not as a pass. The suite printed
 *     "782 checks, 0 failures" and exited 0. The reason is directional, not a
 *     dead column: the library reads 7.1417163 at this row, so its unmutated
 *     residual is +0.0044591 deg, and adding 0.01 to the reference moves the
 *     reference TOWARD the library, leaving err 0.0055409 against
 *     TOL_ELONG_CONV_DEG = 0.014.
 *
 *     The same cell IS pinned in the other direction, and at twice the size in
 *     this one. Both were run:
 *       -0.01, 7.1372572 -> 7.1272572:
 *       FAIL elong_conv_mecca at jd=2415020.5: got 7.1417163 want 7.1272572 (err 0.0144591 > tol 0.0140000)
 *       +0.02, 7.1372572 -> 7.1572572:
 *       FAIL elong_conv_mecca at jd=2415020.5: got 7.1417163 want 7.1572572 (err 0.0155409 > tol 0.0140000)
 *     Each of those went to "782 checks, 1 failures", exit 1.
 *
 *     Which check would need tightening, and why it cannot be:
 *     elong_conv_mecca. Its bound has to clear the library's own elongation
 *     error, printed as "elong_conv max mecca" at 0.0060859 deg, 21.91 arcsec,
 *     which is truncation in the shipped series and not something a fixture
 *     can remove. A bound tight enough to catch 0.01 deg in the shrinking
 *     direction at every row would sit below that residual and fail on correct
 *     data. This is the same hard limit M4 records for the Sigma-l
 *     coefficients, so it is a bounded gap rather than a fixable weakness.
 *
 * M13 HORIZONS_TOPO_JAKARTA body replaced with a verbatim copy of the first
 *     two columns of SKY_TOPO_JAKARTA.
 *     CAUGHT by the row-counting degeneracy guard, and by nothing else --
 *     every per-epoch tolerance check in group 12 passed, with a residual of
 *     exactly zero at every jakarta row:
 *       sky_vs_horizons_topo distinct rows = 22 of 44
 *       FAIL sky_vs_horizons_topo_nondegenerate_rows: expected a non-zero value, got -44.000000000
 *     Suite went to "782 checks, 1 failures", exit 1.
 *     This is the mode M7 documents and this mutation is what justifies the
 *     guard existing. Copying one of the two site tables zeroes 22 of the 44
 *     compared rows, so 4*22 - 3*44 = -44, and check_true_nonzero rejects a
 *     non-positive value. Note the guard was reformulated from a per-row
 *     minimum to a row count by amendment A13, because Horizons prints 6
 *     decimals and Skyfield 7 and the two collide exactly at jd 2458853.5.
 *     The count form still catches the wholesale copy, which is the failure
 *     mode it exists for.
 *
 * M14 SKY_DELTA_T row 2, 69.3630814 -> 74.3630814 (+5.0 s).
 *     CAUGHT by group 9, at three of the four sites:
 *       FAIL topo_alt_jakarta at jd=2458853.5: got -86.5941837 want -86.6117090 (err 0.0175253 > tol 0.0110000)
 *       FAIL topo_alt_mecca at jd=2458853.5: got -16.3053050 want -16.3240946 (err 0.0187896 > tol 0.0110000)
 *       FAIL topo_alt_mid45 at jd=2458853.5: got 18.7998061 want 18.7855430 (err 0.0142631 > tol 0.0110000)
 *     Suite went to "782 checks, 3 failures", exit 1.
 *
 *     Which groups caught it and which did not is the informative part, since
 *     groups 9 through 11 all derive UT1 from this table.
 *
 *     Group 9 caught it because the library is driven at a UT1 taken from this
 *     table while the Skyfield altitude it is compared against is fixed, so
 *     5 s of Earth rotation, 75.2 arcsec of hour angle, lands as an altitude
 *     error. high60 did not fire, the hour-angle-to-altitude projection there
 *     leaving 0.0093031 deg against the 0.011 deg bound.
 *
 *     Group 10 did NOT catch it, and correctly so. Its reference is computed
 *     in this file from the same perturbed UT1, so the shift cancels on both
 *     sides and every printed counterfactual maximum was identical to
 *     baseline, jakarta 0.0006067 deg through high60 0.0037087 deg. That
 *     cancellation is the whole reason group 10 can isolate the
 *     spherical-Earth term from Earth rotation.
 *
 *     Group 11 did not catch it either. Elongation is an angle between two
 *     bodies and is nearly independent of the observer's rotation phase.
 *
 *     Group 13 did not catch it. A 5 s shift leaves that row's model error at
 *     about 5.0 s, inside TOL_DELTA_T_SEC = 11.0 s and below the unmutated
 *     graded maximum of 5.3532 s, so neither the assertion nor the two printed
 *     arcsec figures moved. That bound is deliberately loose, existing to
 *     catch a model regression rather than to certify accuracy, and group 9 is
 *     what actually pins this table's values. */

static int checks;
static int failures;

/* Smallest absolute difference between two angles, in degrees. */
static double angdiff(double a, double b) {
  double d = fmod(a - b + 540.0, 360.0) - 180.0;
  return fabs(d);
}

/* Signed difference between two angles, in degrees, wrapped to [-180, 180).
 * The sign is what separates a structural offset from truncation noise. */
static double angsigned(double a, double b) {
  return fmod(a - b + 540.0, 360.0) - 180.0;
}

static void check_within(const char *name, double jd, double actual,
                         double expected, double tol) {
  double err = fabs(actual - expected);
  checks++;
  if (!(err <= tol)) {
    failures++;
    printf("FAIL %s at jd=%.1f: got %.7f want %.7f (err %.7f > tol %.7f)\n",
           name, jd, actual, expected, err, tol);
  }
}

static void check_true_nonzero(const char *name, double value) {
  checks++;
  if (!(value > 0.0)) {
    failures++;
    printf("FAIL %s: expected a non-zero value, got %.9f\n", name, value);
  }
}

static void check_angle_within(const char *name, double jd, double actual,
                               double expected, double tol) {
  double err = angdiff(actual, expected);
  checks++;
  if (!(err <= tol)) {
    failures++;
    printf("FAIL %s at jd=%.1f: got %.7f want %.7f (err %.7f > tol %.7f)\n",
           name, jd, actual, expected, err, tol);
  }
}

/* jd_tt, ecliptic longitude (deg), ecliptic latitude (deg), distance (km) */
static const double FIXTURE[24][4] = {
  {2415020.5, 272.4165886,  1.1082961,   368384.7},
  {2433282.5,  61.4113240,  3.7815845,   399627.0},
  {2458853.5,  33.9142526, -4.7720034,   399050.0},
  {2458931.7, 350.0058726, -4.8406280,   405997.3},
  {2459044.2,  29.6109850, -4.5133431,   403651.7},
  {2459122.9, 351.3590490, -5.0051181,   402694.7},
  {2459201.3, 307.2360025, -3.8917788,   379131.4},
  {2459318.6,  46.1467562, -2.2062479,   405972.5},
  {2459407.1, 127.3842980,  4.3442962,   392638.4},
  {2459502.8, 319.3933932, -5.0379149,   381789.9},
  {2459613.4, 336.2772407, -4.9759126,   371948.6},
  {2459688.2, 234.2535523, -0.1535543,   365670.3},
  {2459777.9, 345.1493094, -4.5958723,   371014.5},
  {2459860.5, 350.7275114, -4.1560745,   373354.7},
  {2459955.1, 149.5662649,  4.7492624,   404593.7},
  {2460048.7, 304.7981835, -5.2706635,   369277.7},
  {2460133.3, 345.6392462, -3.5820605,   366205.2},
  {2460229.6, 168.9985402,  3.1010587,   403593.9},
  {2460322.4, 311.4838341, -4.6555373,   362505.5},
  {2460451.8, 214.3795062, -1.7207032,   398518.7},
  {2460577.2,  77.6242509,  5.0077504,   377036.8},
  {2460699.5, 237.7144760, -4.3684690,   399260.0},
  {2469807.5,  18.6647902,  3.3919518,   378705.4},
  {2488069.5, 157.4003861,  1.0927180,   371679.8},
};

/* Second oracle: Skyfield 1.54 evaluating JPL DE440 (de440s.bsp,
 * JD 2396752.50 - 2506352.50). Generated offline; the generator is not
 * committed, matching how the Horizons curl above is recorded rather than
 * automated. Columns: jd_tt, ecliptic longitude, latitude, distance km.
 *
 * The tables below were produced with:
 *
 *   python3 -m venv venv && ./venv/bin/pip install skyfield   # 1.54
 *   ./venv/bin/python gen_oracle.py
 *
 * where gen_oracle.py, for each jd_tt in the FIXTURE epoch list above, is:
 *
 *   from skyfield.api import load
 *   from skyfield.framelib import ecliptic_frame
 *   from skyfield.nutationlib import iau2000b
 *   AU_KM = 149597870.7
 *   ts = load.timescale(); eph = load('de440s.bsp')
 *   earth, moon, sun = eph['earth'], eph['moon'], eph['sun']
 *   t = ts.tt_jd(jd)
 *
 *   # APPARENT: light-time, aberration and deflection; true equinox of date.
 *   a = earth.at(t).observe(body).apparent()
 *   lat, lon, dist = a.frame_latlon(ecliptic_frame)
 *
 *   # GEOMETRIC: instantaneous vector, no light-time, no aberration, and
 *   # nutation in longitude removed to reach the MEAN equinox of date.
 *   # iau2000b returns 0.1 microarcseconds, hence the 1e7 / 3600 scaling.
 *   g = (body - earth).at(t)
 *   lat, lon, dist = g.frame_latlon(ecliptic_frame)
 *   dpsi, _ = iau2000b(t.tt)
 *   lon_mean = lon.degrees - float(dpsi) / 1e7 / 3600.0
 *
 * The nutation subtraction is the load-bearing step and is easy to get wrong.
 * skyfield.framelib.ecliptic_frame is the TRUE equinox of date, not the mean,
 * so using it directly would compare the library against a reference carrying
 * nutation the library does not apply, and would report that nutation as
 * library error. Verified at jd_tt 2458853.5 during development: true-of-date
 * 33.9144543, dpsi -0.0046361 deg, mean-of-date 33.9190904, against
 * hijri_moon_position()'s 33.9187415 -- a residual of 0.00035 deg rather than
 * the 0.0043 deg the unsubtracted comparison would have shown.
 *
 * SKY_MOON_APPARENT is the true equinox of date with light-time, aberration
 * and deflection applied -- the same convention as the Horizons FIXTURE above,
 * which is what makes the two directly comparable.
 *
 * This table is compared against FIXTURE, not against the library. No change
 * to hijri.h can alter that comparison; it is deliberately a check that two
 * independently written clients, time-scale conversions and frame
 * transformations agree. Its value was realised at generation time. It is
 * committed so the agreement is a recorded, re-checkable fact and so a later
 * edit to either table is caught. */
static const double SKY_MOON_APPARENT[24][4] = {
  {2415020.5, 272.4166554, 1.1082848, 368384.7},
  {2433282.5, 61.4113490, 3.7815908, 399627.0},
  {2458853.5, 33.9142664, -4.7720029, 399050.0},
  {2458931.7, 350.0058863, -4.8406354, 405997.3},
  {2459044.2, 29.6109988, -4.5133435, 403651.7},
  {2459122.9, 351.3590626, -5.0051253, 402694.7},
  {2459201.3, 307.2360153, -3.8917898, 379131.4},
  {2459318.6, 46.1467695, -2.2062451, 405972.5},
  {2459407.1, 127.3843109, 4.3443072, 392638.4},
  {2459502.8, 319.3934063, -5.0379255, 381789.9},
  {2459613.4, 336.2772541, -4.9759217, 371948.6},
  {2459688.2, 234.2535652, -0.1535586, 365670.3},
  {2459777.9, 345.1493230, -4.5958804, 371014.5},
  {2459860.5, 350.7275249, -4.1560818, 373354.7},
  {2459955.1, 149.5662781, 4.7492722, 404593.7},
  {2460048.7, 304.7981964, -5.2706746, 369277.7},
  {2460133.3, 345.6392595, -3.5820685, 366205.2},
  {2460229.6, 168.9985534, 3.1010663, 403593.9},
  {2460322.4, 311.4838468, -4.6555482, 362505.5},
  {2460451.8, 214.3795185, -1.7207037, 398518.7},
  {2460577.2, 77.6242628, 5.0077584, 377036.8},
  {2460699.5, 237.7144878, -4.3684739, 399260.0},
  {2469807.5, 18.6647836, 3.3919492, 378705.4},
  {2488069.5, 157.4003361, 1.0927267, 371679.8},
};

/* SKY_MOON_GEOMETRIC is the MEAN equinox of date: instantaneous geocentric
 * vector, no light-time, no aberration, and nutation in longitude subtracted
 * via IAU 2000B. This is the convention hijri_moon_position() actually
 * targets (Meeus ch. 47), so this table -- not the apparent one -- measures
 * what the truncated series really costs.
 *
 * IAU 2000B is accurate to about 0.001 arcsec (2.8e-7 deg), three orders of
 * magnitude below the residual measured here, so it is not a limiting term. */
static const double SKY_MOON_GEOMETRIC[24][4] = {
  {2415020.5, 272.4120166, 1.1083031, 368389.7},
  {2433282.5, 61.4124549, 3.7816016, 399601.8},
  {2458853.5, 33.9190904, -4.7719955, 399012.2},
  {2458931.7, 350.0108155, -4.8406393, 406005.8},
  {2459044.2, 29.6157657, -4.5133349, 403690.6},
  {2459122.9, 351.3640981, -5.0051258, 402682.9},
  {2459201.3, 307.2408668, -3.8918022, 379106.7},
  {2459318.6, 46.1518018, -2.2062300, 405957.0},
  {2459407.1, 127.3886479, 4.3443166, 392626.7},
  {2459502.8, 319.3980522, -5.0379296, 381755.9},
  {2459613.4, 336.2809758, -4.9759193, 371934.2},
  {2459688.2, 234.2579205, -0.1535774, 365686.5},
  {2459777.9, 345.1527443, -4.5958729, 371042.3},
  {2459860.5, 350.7312116, -4.1560708, 373339.1},
  {2459955.1, 149.5691689, 4.7492669, 404619.6},
  {2460048.7, 304.8012322, -5.2706743, 369313.4},
  {2460133.3, 345.6415830, -3.5820558, 366236.2},
  {2460229.6, 169.0009665, 3.1010524, 403614.2},
  {2460322.4, 311.4853337, -4.6555416, 362493.4},
  {2460451.8, 214.3811407, -1.7207197, 398501.7},
  {2460577.2, 77.6250778, 5.0077643, 377072.9},
  {2460699.5, 237.7144129, -4.3684826, 399296.7},
  {2469807.5, 18.6607667, 3.3919358, 378667.7},
  {2488069.5, 157.3996232, 1.0927093, 371711.2},
};

/* hijri_sun_position() returns APPARENT longitude -- Meeus ch. 25 applies both
 * the aberration term and the nutation term (hijri.h:504-506) -- so it is
 * compared against SKY_SUN_APPARENT. SKY_SUN_GEOMETRIC is committed alongside
 * it so the Sun's own decomposition is available on the same footing as the
 * Moon's, and so the frame-consistency check in group 4 has both frames
 * available without regenerating.
 *
 * Before this table existed, hijri_sun_position() had never been compared
 * against any oracle: the previous test file was moon-only and the suite's
 * only other use of it (tests/test_hijri.c:334) is unchecked. Since sunset
 * drives every predicate in the library and elongation is a Sun-Moon angle,
 * that was the larger of the two gaps.
 *
 * Column usage, stated so unread data does not look load-bearing. All four
 * columns of SKY_SUN_APPARENT are read: longitude by groups 3 through 5 and 7,
 * latitude and distance by the group 6 harness cross-check against
 * HORIZONS_SUN. SKY_SUN_GEOMETRIC is read only in its jd and longitude
 * columns; its latitude and distance are retained because the generator emits
 * all four and a hand-trimmed table would no longer be its verbatim output.
 * The library models solar ecliptic latitude as zero, which is correct to
 * about 1.2 arcsec; that is recorded in the research note rather than
 * asserted here. */
static const double SKY_SUN_APPARENT[24][4] = {
  {2415020.5, 280.1533846, 0.0000530, 147094540.8},
  {2433282.5, 280.0045145, -0.0000206, 147091153.4},
  {2458853.5, 284.0860830, -0.0001191, 147091188.1},
  {2458931.7, 3.0186935, -0.0000756, 149116951.8},
  {2459044.2, 111.7344421, -0.0001621, 152072021.9},
  {2459122.9, 187.6835096, -0.0002389, 149796568.8},
  {2459201.3, 266.3145434, -0.0001726, 147205533.1},
  {2459318.6, 24.3673652, 0.0000231, 150043846.8},
  {2459407.1, 109.5025501, 0.0001991, 152086401.4},
  {2459502.8, 202.1524413, -0.0002218, 149172562.6},
  {2459613.4, 314.0162387, -0.0002237, 147441091.8},
  {2459688.2, 28.6315322, -0.0000323, 150208731.6},
  {2459777.9, 114.8096371, -0.0000655, 152042538.3},
  {2459860.5, 194.6961027, -0.0000870, 149496953.6},
  {2459955.1, 290.0642766, 0.0001134, 147115383.1},
  {2460048.7, 23.9874874, -0.0001832, 150013224.9},
  {2460133.3, 105.4237298, -0.0000388, 152092994.9},
  {2460229.6, 198.5049047, 0.0002229, 149337680.3},
  {2460322.4, 292.1683498, -0.0001163, 147133997.9},
  {2460451.8, 60.7289375, -0.0001067, 151410581.1},
  {2460577.2, 181.1438937, 0.0001909, 150086332.2},
  {2460699.5, 304.2385149, -0.0001856, 147262963.2},
  {2469807.5, 280.7475596, 0.0001027, 147106965.9},
  {2488069.5, 280.6033268, 0.0000785, 147108222.9},
};

static const double SKY_SUN_GEOMETRIC[24][4] = {
  {2415020.5, 280.1543331, 0.0000532, 147094536.4},
  {2433282.5, 280.0112187, -0.0000204, 147091155.6},
  {2458853.5, 284.0965068, -0.0001190, 147091188.1},
  {2458931.7, 3.0291486, -0.0000757, 149116944.7},
  {2459044.2, 111.7446231, -0.0001620, 152072021.1},
  {2459122.9, 187.6940387, -0.0002389, 149796575.7},
  {2459201.3, 266.3249832, -0.0001727, 147205538.2},
  {2459318.6, 24.3778893, 0.0000233, 150043839.7},
  {2459407.1, 109.5122957, 0.0001992, 152086396.8},
  {2459502.8, 202.1625968, -0.0002219, 149172568.6},
  {2459613.4, 314.0255348, -0.0002237, 147441095.3},
  {2459688.2, 28.6413487, -0.0000326, 150208726.3},
  {2459777.9, 114.8184544, -0.0000655, 152042531.5},
  {2459860.5, 194.7052804, -0.0000869, 149496955.6},
  {2459955.1, 290.0727669, 0.0001133, 147115390.6},
  {2460048.7, 23.9959961, -0.0001832, 150013223.6},
  {2460133.3, 105.4314457, -0.0000387, 152092987.2},
  {2460229.6, 198.5128351, 0.0002228, 149337679.0},
  {2460322.4, 292.1754179, -0.0001162, 147134004.9},
  {2460451.8, 60.7359924, -0.0001069, 151410579.2},
  {2460577.2, 181.1501822, 0.0001910, 150086326.5},
  {2460699.5, 304.2440353, -0.0001857, 147262969.0},
  {2469807.5, 280.7491331, 0.0001026, 147106964.8},
  {2488069.5, 280.6081995, 0.0000782, 147108215.8},
};

/* Second oracle for the SUN, so the body with the largest measured error does
 * not rest on a single generation harness the way it briefly did. Retrieved on
 * 2026-08-02 with the same curl as the Moon FIXTURE at the top of this file,
 * changing only COMMAND='301' to COMMAND='10' (the Sun). Columns: jd_tt,
 * apparent ecliptic longitude and latitude of date, apparent range converted
 * from AU with 1 AU = 149597870.7 km.
 *
 * This table is compared against SKY_SUN_APPARENT, not against the library:
 * it is the solar counterpart of group 1, closing the circularity in which
 * the solar tables were only ever compared against the library whose residual
 * set their own tolerances. */
static const double HORIZONS_SUN[24][4] = {
  {2415020.5, 280.1533171, 0.0000648, 147094540.8},
  {2433282.5, 280.0044888, -0.0000093, 147091153.4},
  {2458853.5, 284.0860699, -0.0001084, 147091188.1},
  {2458931.7, 3.0186804, -0.0000703, 149116951.8},
  {2459044.2, 111.7344289, -0.0001731, 152072021.9},
  {2459122.9, 187.6834965, -0.0002434, 149796568.8},
  {2459201.3, 266.3145303, -0.0001635, 147205533.1},
  {2459318.6, 24.3673521, 0.0000245, 150043846.8},
  {2459407.1, 109.5025371, 0.0001882, 152086401.4},
  {2459502.8, 202.1524282, -0.0002236, 149172562.6},
  {2459613.4, 314.0162256, -0.0002128, 147441091.8},
  {2459688.2, 28.6315189, -0.0000318, 150208731.6},
  {2459777.9, 114.8096238, -0.0000766, 152042538.3},
  {2459860.5, 194.6960894, -0.0000902, 149496953.6},
  {2459955.1, 290.0642635, 0.0001243, 147115383.1},
  {2460048.7, 23.9874743, -0.0001817, 150013224.9},
  {2460133.3, 105.4237166, -0.0000495, 152092994.9},
  {2460229.6, 198.5048917, 0.0002204, 149337680.3},
  {2460322.4, 292.1683368, -0.0001053, 147133997.9},
  {2460451.8, 60.7289246, -0.0001121, 151410581.1},
  {2460577.2, 181.1438807, 0.0001853, 150086332.2},
  {2460699.5, 304.2385020, -0.0001745, 147262963.2},
  {2469807.5, 280.7475650, 0.0001128, 147106965.9},
  {2488069.5, 280.6033753, 0.0000877, 147108222.9},
};

/* Equation of the equinoxes, GAST - GMST, in degrees, at the same 24 epochs
 * as the tables above.
 *
 * PROVENANCE: generated 2026-08-05 with Skyfield 1.54 on JPL DE421, reading
 * t.gast - t.gmst for t = ts.tt_jd(<epoch>), converted from hours to degrees
 * and wrapped to (-180, 180]. Skyfield evaluates the full IAU 2000B nutation
 * series; hijri__eqeq_deg() uses only the leading 17.20" Omega term, so these
 * two are INDEPENDENT derivations and their agreement is the check.
 *
 * The generator was run by hand and is not committed, matching the practice
 * already used for SKY_MOON_APPARENT in this file. */
static const double SKY_EQEQ[24][2] = {
  { 2415020.5, +0.004441918},
  { 2433282.5, -0.000841633},
  { 2458853.5, -0.004252976},
  { 2458931.7, -0.004350645},
  { 2459044.2, -0.004202831},
  { 2459122.9, -0.004446576},
  { 2459201.3, -0.004268986},
  { 2459318.6, -0.004446504},
  { 2459407.1, -0.003802165},
  { 2459502.8, -0.004081034},
  { 2459613.4, -0.003227657},
  { 2459688.2, -0.003807395},
  { 2459777.9, -0.002952766},
  { 2459860.5, -0.003196435},
  { 2459955.1, -0.002481261},
  { 2460048.7, -0.002597950},
  { 2460133.3, -0.001943961},
  { 2460229.6, -0.002042573},
  { 2460322.4, -0.001172969},
  { 2460451.8, -0.001315363},
  { 2460577.2, -0.000565305},
  { 2460699.5, +0.000240783},
  { 2469807.5, +0.003866134},
  { 2488069.5, +0.000838030},
};

/* Sunset and moonset instants solved on the oracle side, for issue #18.
 *
 * PROVENANCE: generated with Skyfield 1.54 on JPL DE440 (de440s.bsp). Columns
 * are the sunset instant and the moonset instant, both as Julian Day in UT1 to
 * nine decimals, which is 86 microseconds against a second-scale residual. One
 * table per observer site, all at elevation 0. Grid dates are one day after
 * each 2025 conjunction. The generator was run by hand and is not committed,
 * matching every other oracle table in this file. Its full text is recorded in
 * docs/research/2026-08-08-set-solver-oracle.md.
 *
 * THE CONVENTION IS HELD EQUAL ON BOTH SIDES, ON PURPOSE. The oracle solves for
 * the same target the library does, using a GEOCENTRIC Sun with apparent
 * right ascension and apparent sidereal time, and the same
 * -(REFRACTION + SOLAR_SEMIDIAMETER) target. That is what separates this
 * fixture from issue #33, which is about whether those two constants are the
 * right ones. Holding the convention equal means this table keeps its meaning
 * after #33 changes them, because #33 changes both sides together.
 *
 * THE MOON SIDE REPLICATES THE MEAN FRAME AND THIS IS THE EASY THING TO GET
 * WRONG. hijri__altitude_deg pairs mean-of-date right ascension with MEAN
 * sidereal time, deliberately, and the header documents that choice. Skyfield's
 * altaz returns apparent with true sidereal time. Nutation in right ascension
 * reaches roughly 17 arcsec, and near the horizon the Moon falls at about
 * 0.0036 deg per second, so comparing against an unmatched frame would report
 * on the order of 1.3 s of deliberate design as library error. The generator
 * subtracts nutation to reach the mean equinox and uses GMST. */
static const double SKY_SETSOLVE_JAKARTA[12][2] = {
  {2460705.970467385, 2460706.005166394},   /* 2025-01-30 */
  {2460735.966437230, 2460736.012782676},   /* 2025-03-01 */
  {2460764.957508930, 2460764.987453242},   /* 2025-03-30 */
  {2460793.949553166, 2460793.966749458},   /* 2025-04-28 */
  {2460823.947139859, 2460823.998588262},   /* 2025-05-28 */
  {2460852.950441329, 2460852.990654236},   /* 2025-06-26 */
  {2460881.954225177, 2460881.978115114},   /* 2025-07-25 */
  {2460911.953985053, 2460911.991356533},   /* 2025-08-24 */
  {2460940.950524119, 2460940.965760513},   /* 2025-09-22 */
  {2460970.948337770, 2460970.972346232},   /* 2025-10-22 */
  {2461000.952330614, 2461000.988626409},   /* 2025-11-21 */
  {2461030.962099322, 2461031.010110631},   /* 2025-12-21 */
};
static const double SKY_SETSOLVE_MECCA[12][2] = {
  {2460706.131631374, 2460706.174817780},   /* 2025-01-30 */
  {2460736.142414247, 2460736.208249921},   /* 2025-03-01 */
  {2460765.149332837, 2460765.199414176},   /* 2025-03-30 */
  {2460794.156236909, 2460794.194021995},   /* 2025-04-28 */
  {2460824.165061587, 2460824.238505167},   /* 2025-05-28 */
  {2460853.171294338, 2460853.225059390},   /* 2025-06-26 */
  {2460882.168821229, 2460882.200234720},   /* 2025-07-25 */
  {2460912.155539094, 2460912.189597008},   /* 2025-08-24 */
  {2460941.136838004, 2460941.149751297},   /* 2025-09-22 */
  {2460971.118712745, 2460971.135562280},   /* 2025-10-22 */
  {2461001.109516486, 2461001.138223613},   /* 2025-11-21 */
  {2461031.113796608, 2461031.161757674},   /* 2025-12-21 */
};
static const double SKY_SETSOLVE_MID45[12][2] = {
  {2460706.211959834, 2460706.262619581},   /* 2025-01-30 */
  {2460736.241376039, 2460736.329060860},   /* 2025-03-01 */
  {2460765.267621290, 2460765.340735438},   /* 2025-03-30 */
  {2460794.293062627, 2460794.354716877},   /* 2025-04-28 */
  {2460824.316790390, 2460824.413162007},   /* 2025-05-28 */
  {2460853.327019621, 2460853.389818894},   /* 2025-06-26 */
  {2460882.315769845, 2460882.348273627},   /* 2025-07-25 */
  {2460912.285542799, 2460912.308896410},   /* 2025-08-24 */
  {2460941.248116870, 2460941.252271402},   /* 2025-09-22 */
  {2460971.210514179, 2460971.212179313},   /* 2025-10-22 */
  {2461001.184467469, 2461001.195873260},   /* 2025-11-21 */
  {2461031.181372040, 2461031.223635422},   /* 2025-12-21 */
};
static const double SKY_SETSOLVE_HIGH60[12][2] = {
  {2460706.173261592, 2460706.226433615},   /* 2025-01-30 */
  {2460736.227652046, 2460736.335782412},   /* 2025-03-01 */
  {2460765.277377197, 2460765.374065649},   /* 2025-03-30 */
  {2460794.327049147, 2460794.420246489},   /* 2025-04-28 */
  {2460824.375462957, 2460824.516601558},   /* 2025-05-28 */
  {2460853.394095609, 2460853.463548038},   /* 2025-06-26 */
  {2460882.365617409, 2460882.392235455},   /* 2025-07-25 */
  {2460912.310187954, 2460912.316264119},   /* 2025-08-24 */
  {2460941.249482482, 2460942.238925292},   /* 2025-09-22 */
  {2460971.187900848, 2460972.165052444},   /* 2025-10-22 */
  {2461001.137552446, 2461002.123227193},   /* 2025-11-21 */
  {2461031.121036874, 2461031.137639221},   /* 2025-12-21 */
};

/* The grid dates, one day after each 2025 conjunction. Kept beside the tables
 * because the fixture is meaningless without knowing which evening each row
 * describes, and because the conjunction-evening grid that an earlier draft
 * used leaves 20 of 48 rows with no moonset at all. */
static const int SETSOLVE_DATES[12][3] = {
  {2025,  1, 30}, {2025,  3,  1}, {2025,  3, 30}, {2025,  4, 28},
  {2025,  5, 28}, {2025,  6, 26}, {2025,  7, 25}, {2025,  8, 24},
  {2025,  9, 22}, {2025, 10, 22}, {2025, 11, 21}, {2025, 12, 21},
};

typedef struct {
  const char *name;
  double lat_deg;
  double lon_deg;
  const double (*table)[2];
  double tol_moonset_s;   /* per-site, see the comment on the bounds below */
} SetSolveSite;

static const SetSolveSite SETSOLVE_SITES[4] = {
  {"jakarta", -6.2, 106.8, SKY_SETSOLVE_JAKARTA, 0.8},
  {"mecca",   21.4,  39.8, SKY_SETSOLVE_MECCA,   1.3},
  {"mid45",   45.0,   0.0, SKY_SETSOLVE_MID45,   3.6},
  {"high60",  60.0,   0.0, SKY_SETSOLVE_HIGH60,  15.0},
};

/* Horizons cross-check on the POSITION the sunset crossing is solved from,
 * retrieved with the query recorded in
 * docs/research/2026-08-08-set-solver-oracle.md. Columns are jd_ut1, apparent
 * geocentric solar right ascension and declination in degrees, at three of the
 * solved jakarta sunset instants.
 *
 * WHAT THIS DOES NOT COVER, stated plainly rather than implied away. It
 * validates the position, not the generator's frame replication, because a
 * geocentric Sun projected into a site horizon frame is not a standard Horizons
 * output. Two other things cover the frame instead: the library's apparent
 * sidereal time is pinned independently by SKY_EQEQ at 0.0004829 deg, and a
 * frame error in the generator would land in seconds to minutes rather than
 * hiding, which the group 14 maxima would show immediately. */
static const double HORIZONS_SETSOLVE_SUN[3][3] = {
  {2460705.970467385, 313.2706250, -17.5194167},
  {2460852.950441329, 95.5501250, 23.3402500},
  {2461030.962099322, 269.8167500, -23.4381389},
};

/* MEASURED MUTATION SENSITIVITY of the set-solver fixtures above.
 *
 * S1 through S5 cover SKY_SETSOLVE_JAKARTA, SKY_SETSOLVE_HIGH60, the bisection
 * solver itself and HORIZONS_SETSOLVE_SUN, following the same rule the M-series
 * follows: every mutation below was applied, built and run, and only lines the
 * binary actually printed are quoted. Two of the five were NOT caught and are
 * recorded as gaps rather than replaced with mutations that work, following M4
 * and M12. Every mutation was reverted, including the one to hijri.h, and
 * neither this file nor hijri.h carries a mutation. All runs are against the
 * suite at 1029 checks, whose unmutated result is "1029 checks, 0 failures".
 *
 * S1  SKY_SETSOLVE_JAKARTA row 0 sunset, 2460705.970467385 -> 2460705.970567385
 *     (+0.0001 d, 8.64 s) against TOL_SETSOLVE_SUNSET_S = 5.5 s.
 *     CAUGHT
 *       FAIL setsolve_sunset_jakarta at jd=2460706.0: got 7.8265920 want 0.0000000 (err 7.8265920 > tol 5.5000000)
 *     Suite went to "1029 checks, 1 failures", exit 1. The reported 7.83 s
 *     rather than 8.64 s is the row's own 0.81 s residual signing against the
 *     perturbation, which is the same directional effect M12 documents.
 *
 * S2  SKY_SETSOLVE_HIGH60 row 0 moonset, 2460706.226433615 ->
 *     2460706.226533615 (+0.0001 d, 8.64 s).
 *     NOT CAUGHT -- recorded as executed, not as a pass. The suite printed
 *     "1029 checks, 0 failures" and exited 0. The only visible movement was the
 *     printed maximum:
 *       setsolve max high60   sunset 2.7380 s  moonset 9.0879 s
 *     against the unmutated 7.2926 s. This is not a defect in the mutation, it
 *     is what the bound says: high60's tol_moonset_s is 15.0 s because three of
 *     its twelve rows are ones where the Moon set before sunset and the solver
 *     walked to the following night, and 8.64 s is inside that. A bound tight
 *     enough to catch this at high60 would sit below the site's own 7.2926 s
 *     residual and fail on correct data, which is the same hard limit M4 and
 *     M12 record.
 *
 *     The same perturbation IS pinned where the bound is an assertion rather
 *     than an envelope. SKY_SETSOLVE_JAKARTA row 0 moonset, 2460706.005166394
 *     -> 2460706.005266394, same +0.0001 d, against tol_moonset_s = 0.8 s:
 *       FAIL setsolve_moonset_jakarta at jd=2460706.0: got 8.6589351 want 0.0000000 (err 8.6589351 > tol 0.8000000)
 *     That went to "1029 checks, 1 failures", exit 1. Both runs are recorded
 *     because the per-site bound of amendment A2 is exactly what makes the
 *     difference between them, and a single shared bound would have left all
 *     four sites in high60's position.
 *
 * S3  hijri__bisect_crossing at hijri.h:948, the iteration bound 40 -> 4. This
 *     is the mutation check_group14_setsolve's convergence check exists for.
 *     CAUGHT, by the convergence check at all 48 rows and by the oracle
 *     comparisons as well:
 *       FAIL setsolve_sunset_jakarta at jd=2460706.0: got 17.8820997 want 0.0000000 (err 17.8820997 > tol 5.5000000)
 *       FAIL setsolve_converge_jakarta at jd=2460706.0: got -0.7596316 want -0.8334000 (err 0.0737684 > tol 0.0000010)
 *       FAIL setsolve_moonset_jakarta at jd=2460706.0: got 21.6234997 want 0.0000000 (err 21.6234997 > tol 0.8000000)
 *     Suite went to "1029 checks, 138 failures", exit 1. setsolve_converge_*
 *     failed 48 of 48 times, one per row across all four sites.
 *
 *     What this says about coverage, stated plainly. For THIS mutation the
 *     convergence check adds no coverage the oracle comparison did not already
 *     provide: the sunset comparison fails at the same rows, and it fails
 *     first. Four halvings of a one hour bracket leave 3.75 minutes of bracket,
 *     so the instant moves by tens of seconds, far outside a 5.5 s bound. What
 *     the convergence check does add is independence, not sensitivity: it reads
 *     the library's own altitude at the instant the library reports and needs
 *     no table at all, so it is the one assertion in group 14 that survives
 *     deleting every fixture, and it cannot be satisfied by a solver regression
 *     that happens to cancel against an ephemeris change. It is a different
 *     kind of coverage, not a strictly larger one, and S3 is the evidence for
 *     that rather than for the stronger claim.
 *
 * S4  HORIZONS_SETSOLVE_SUN row 0 right ascension, 313.2706250 -> 313.2806250
 *     (+0.01 deg) against TOL_SETSOLVE_HORIZONS_DEG = 0.007 deg.
 *     NOT CAUGHT -- recorded as executed, not as a pass. The suite printed
 *     "1029 checks, 0 failures" and exited 0.
 *
 *     The reason is directional, exactly as at M12, and it was measured rather
 *     than assumed. Temporarily setting the tolerance to 1e-9 prints the
 *     unmutated residuals:
 *       FAIL setsolve_horizons_sun_ra at jd=2460706.0: got 313.2737700 want 313.2706250 (err 0.0031450 > tol 0.0000000)
 *     The library sits 0.0031450 deg ABOVE this Horizons row, so adding 0.01 to
 *     the row moves it toward the library and leaves err 0.0068550, which is
 *     inside 0.007 by 45 microdegrees. The cell IS pinned in the other
 *     direction, and that run was executed too, 313.2706250 -> 313.2606250:
 *       FAIL setsolve_horizons_sun_ra at jd=2460706.0: got 313.2737700 want 313.2606250 (err 0.0131450 > tol 0.0070000)
 *     which went to "1029 checks, 1 failures", exit 1. The bound cannot be
 *     tightened past the library's own 0.0034349 deg maximum residual against
 *     Horizons without failing on correct data, so this is a bounded gap of
 *     roughly one part in a thousand of a degree in one direction at one row.
 *
 * S5  HORIZONS_SETSOLVE_SUN declination column replaced with the library's own
 *     values, -17.5184891, 23.3402933, -23.4382536, so every declination
 *     difference collapses to rounding.
 *     NOT CAUGHT -- recorded as executed, not as a pass. The suite printed
 *       setsolve_horizons distinct rows = 3 of 3
 *       Moon ephemeris tests: 1029 checks, 0 failures
 *     and exited 0. The per-row tolerance checks passing is expected and is the
 *     point of the mutation. The guard failing to fire is not: it counts
 *     distinctness on the RIGHT ASCENSION column only, `if (dra > 0.0)`, so the
 *     declination column has no degeneracy guard at all. That is the finding,
 *     and it is left recorded rather than quietly fixed here because Task 5
 *     changes no assertion.
 *
 *     A second run bounds how much the guard covers even on the column it does
 *     read. Replacing BOTH columns with the library's printed values,
 *     313.2737700, 95.5535599, 269.8188563 and the three declinations above,
 *     still printed "setsolve_horizons distinct rows = 3 of 3" and
 *     "1029 checks, 0 failures". Values transcribed at seven decimals never
 *     equal the library's doubles bit for bit, so `dra > 0.0` holds and the
 *     guard sees nothing. What it catches is the byte-exact array copy that M3
 *     and M13 record, not a hand-transcribed one. The M3 scope note documents
 *     this same weakness in the max-only form, and groups 1 and 6 answered it
 *     with a per-row minimum, which is the shape this guard would need.
 *
 * S5b re-runs S5 against the replacement guard. The S5 record above is kept
 * unchanged as the evidence for why the guard was replaced, not as a live
 * result. The distinctness counter it describes no longer exists. In its place
 * check_group14_setsolve asserts a FLOOR, TOL_SETSOLVE_HORIZONS_FLOOR_DEG =
 * 1e-5 deg, on the MINIMUM of the six differences. Both runs below are against
 * the suite at 1029 checks, whose unmutated result is "1029 checks, 0
 * failures" with "setsolve_horizons residual min 0.0000433 max 0.0034349 deg".
 *
 * S5b-a  HORIZONS_SETSOLVE_SUN declination column replaced with the library's
 *     own values, -17.5184891, 23.3402933, -23.4382536, which is exactly the
 *     mutation S5 above ran and survived.
 *     CAUGHT
 *       setsolve_horizons residual min 0.0000000 max 0.0034349 deg
 *       FAIL setsolve_horizons_independent: expected a non-zero value, got -0.000009986
 *     Suite went to "1029 checks, 1 failures", exit 1.
 *
 * S5b-b  BOTH columns replaced with the library's own values, right ascensions
 *     313.2737700, 95.5535599, 269.8188563 and the three declinations above,
 *     which is the S5 second run.
 *     CAUGHT
 *       setsolve_horizons residual min 0.0000000 max 0.0000000 deg
 *       FAIL setsolve_horizons_independent: expected a non-zero value, got -0.000009987
 *     Suite went to "1029 checks, 1 failures", exit 1.
 *
 *     Both mutations were reverted. The two failure values, -0.000009986 and
 *     -0.000009987, are the floor minus a residual of order 1e-8, which is the
 *     rounding of a seven-decimal transcription against the library's doubles.
 *     That is the population the floor separates from, and the measured gap to
 *     it is roughly 200x, against 4.33x of headroom above the real minimum.
 *
 *     One intermediate result is recorded because it changed the guard's
 *     shape. The floor was first written over the MAXIMUM of the six
 *     differences and executed against S5b-a: the untouched right ascension
 *     column kept the maximum at 0.0034349 deg, the suite printed "1029
 *     checks, 0 failures" and exited 0. A maximum only guards the whole table
 *     going degenerate at once, so the guard was rewritten over the minimum
 *     before the mutations above were run.
 */

/* Topocentric fixtures for issue #17.
 *
 * PROVENANCE: generated 2026-08-07 with Skyfield 1.54 on JPL DE440
 * (de440s.bsp), one table per observer site, all at elevation 0. Columns are
 * jd_tt, airless topocentric altitude of the Moon in degrees, elongation
 * between the topocentric Moon and the GEOCENTRIC Sun, and elongation between
 * the topocentric Moon and the topocentric Sun. All in degrees.
 *
 * The third column deliberately mirrors this library's own convention, which
 * pairs a topocentric Moon with a geocentric Sun at hijri.h:1220 because the
 * Pedoman Hisab convention omits solar parallax. Comparing against the fourth
 * column instead would charge that deliberate choice as library error. Both
 * are committed so the implementation error and the cost of the convention can
 * be reported as two separate numbers.
 *
 * Sites: jakarta (-6.2, 106.8), mecca (21.4, 39.8), mid45 (45.0, 0.0),
 * high60 (60.0, 0.0).
 *
 * altaz() is called with no arguments, which is airless. The library computes
 * geometric altitude and folds refraction into its sunset target at
 * hijri.h:912-913 and hijri.h:958, so a refracted reference would double count
 * and show as a spurious 0.57 deg discrepancy.
 *
 * The generator was run by hand and is not committed, matching the practice
 * already used for SKY_MOON_APPARENT and the Horizons curl in this file. Its
 * full text is recorded in docs/research/2026-08-07-topocentric-error-bar.md. */
static const double SKY_TOPO_JAKARTA[24][4] = {
  {2415020.5, 23.9726098, 6.9105472, 6.9129388},
  {2433282.5, -55.2123304, 140.7542108, 140.7518455},
  {2458853.5, -86.6117090, 109.7044169, 109.7020107},
  {2458931.7, 82.6827749, 13.9728851, 13.9732619},
  {2459044.2, -10.1579578, 81.2911161, 81.2909534},
  {2459122.9, -3.9264361, 163.6945176, 163.6963612},
  {2459201.3, -60.6189138, 41.0074376, 41.0062161},
  {2459318.6, 26.8118952, 22.6868325, 22.6851908},
  {2459407.1, -31.9519982, 17.6044698, 17.6060718},
  {2459502.8, 5.8925277, 118.0363850, 118.0376342},
  {2459613.4, -43.0056754, 23.4860320, 23.4837625},
  {2459688.2, 60.6799821, 153.8929349, 153.8927263},
  {2459777.9, -66.4823674, 129.1461816, 129.1439441},
  {2459860.5, -39.0296353, 155.1662247, 155.1643468},
  {2459955.1, 5.9286803, 139.4355855, 139.4339590},
  {2460048.7, 17.9115629, 80.1502263, 80.1506575},
  {2460133.3, 61.4615788, 119.2867686, 119.2880251},
  {2460229.6, 74.0535195, 29.6421382, 29.6433892},
  {2460322.4, -36.8633726, 20.5958816, 20.5936615},
  {2460451.8, -25.5525048, 154.3089790, 154.3101308},
  {2460577.2, 9.6816084, 102.5171842, 102.5169899},
  {2460699.5, 71.5337108, 66.4722941, 66.4746141},
  {2469807.5, -78.1118239, 98.0512976, 98.0489160},
  {2488069.5, 39.1643192, 123.9558617, 123.9582612},
};
static const double SKY_TOPO_MECCA[24][4] = {
  {2415020.5, -46.8762790, 7.1372572, 7.1386511},
  {2433282.5, 15.2407090, 140.3797870, 140.3784391},
  {2458853.5, -16.3240946, 108.8799340, 108.8785676},
  {2458931.7, 23.1242996, 13.6253519, 13.6261831},
  {2459044.2, -58.6560203, 82.1063464, 82.1042967},
  {2459122.9, -67.6190975, 163.0147428, 163.0141919},
  {2459201.3, -20.8848655, 40.1765965, 40.1778594},
  {2459318.6, -27.0546116, 22.4664899, 22.4646206},
  {2459407.1, 39.2647754, 17.6792265, 17.6813973},
  {2459502.8, -64.1279465, 117.4902792, 117.4886922},
  {2459613.4, -66.8969802, 22.4059436, 22.4060358},
  {2459688.2, -10.9582824, 153.3998596, 153.3975019},
  {2459777.9, -42.0420657, 130.2387855, 130.2387920},
  {2459860.5, 20.9025474, 154.7858254, 154.7841981},
  {2459955.1, -41.8269745, 139.7005545, 139.6983276},
  {2460048.7, 43.4399389, 79.5051455, 79.5066955},
  {2460133.3, -9.0253236, 118.9784455, 118.9771790},
  {2460229.6, 15.5024440, 28.7919656, 28.7943029},
  {2460322.4, -71.7904414, 19.5326851, 19.5327108},
  {2460451.8, -82.0483122, 153.5633460, 153.5621308},
  {2460577.2, -29.1230198, 102.9869391, 102.9854806},
  {2460699.5, 6.2841576, 65.7208140, 65.7219816},
  {2469807.5, -27.7042197, 97.0587501, 97.0573967},
  {2488069.5, 67.7002498, 122.8333409, 122.8347115},
};
static const double SKY_TOPO_MID45[24][4] = {
  {2415020.5, -66.8716568, 7.6869971, 7.6869256},
  {2433282.5, 49.9926044, 140.7684469, 140.7686942},
  {2458853.5, 18.7855430, 108.9627942, 108.9629137},
  {2458931.7, -13.7870749, 13.8592911, 13.8590916},
  {2459044.2, -33.8639192, 82.6532008, 82.6517175},
  {2459122.9, -49.4787814, 162.4227323, 162.4207894},
  {2459201.3, 0.2297160, 40.3384518, 40.3399131},
  {2459318.6, -29.9084542, 21.9012587, 21.9007826},
  {2459407.1, 64.2972996, 18.2344553, 18.2354401},
  {2459502.8, -63.3987265, 116.8534941, 116.8512074},
  {2459613.4, -29.4708980, 22.0777854, 22.0790125},
  {2459688.2, -44.2677226, 153.7573229, 153.7552358},
  {2459777.9, -8.6302195, 130.3868544, 130.3880195},
  {2459860.5, 32.6975508, 155.0414078, 155.0405293},
  {2459955.1, -29.3031720, 140.1331414, 140.1321295},
  {2460048.7, 12.9663543, 79.0831086, 79.0836992},
  {2460133.3, -37.9326667, 119.4730406, 119.4714394},
  {2460229.6, -12.8653977, 28.8291877, 28.8308280},
  {2460322.4, -37.2118004, 19.2130770, 19.2140936},
  {2460451.8, -42.5292543, 153.3020733, 153.3006110},
  {2460577.2, -18.0612196, 103.5883998, 103.5877462},
  {2460699.5, -32.0474879, 65.9110558, 65.9105539},
  {2469807.5, 10.1896324, 97.0232165, 97.0234371},
  {2488069.5, 27.6526863, 122.4072841, 122.4071252},
};
static const double SKY_TOPO_HIGH60[24][4] = {
  {2415020.5, -52.5432747, 7.6997330, 7.6996556},
  {2433282.5, 43.4455046, 140.8683633, 140.8686722},
  {2458853.5, 16.0279310, 109.0745215, 109.0746089},
  {2458931.7, -13.0875932, 14.0906511, 14.0898554},
  {2459044.2, -20.7826710, 82.6237855, 82.6227881},
  {2459122.9, -36.4502330, 162.3620836, 162.3601097},
  {2459201.3, -8.0591760, 40.4979869, 40.4988915},
  {2459318.6, -15.4751959, 21.8430087, 21.8428672},
  {2459407.1, 50.6794611, 18.2957849, 18.2965281},
  {2459502.8, -49.5356163, 116.8142785, 116.8122416},
  {2459613.4, -26.1033484, 22.2293872, 22.2302783},
  {2459688.2, -38.1048510, 153.8479424, 153.8461267},
  {2459777.9, -10.0774525, 130.2434783, 130.2443497},
  {2459860.5, 19.1216784, 155.0377311, 155.0366025},
  {2459955.1, -14.5398190, 140.0842135, 140.0834596},
  {2460048.7, 0.1766407, 79.2161017, 79.2161076},
  {2460133.3, -29.8114237, 119.6704020, 119.6692865},
  {2460229.6, -6.6888936, 28.9156615, 28.9172278},
  {2460322.4, -34.5850465, 19.3852683, 19.3858824},
  {2460451.8, -35.0859490, 153.4736682, 153.4727199},
  {2460577.2, -3.1106005, 103.5892151, 103.5891624},
  {2460699.5, -31.7983023, 66.0516007, 66.0509639},
  {2469807.5, 10.7645064, 97.1497503, 97.1500123},
  {2488069.5, 22.6711368, 122.5088872, 122.5087142},
};

/* True delta-T at the same 24 epochs, TT minus UT1 in seconds, read from
 * Skyfield's bundled IERS data.
 *
 * This table exists because hijri_delta_t_seconds() at hijri.h:558 applies the
 * Espenak-Meeus 2005-2050 polynomial across 1986-2050, and that polynomial is
 * a 2007-vintage forecast. Earth has since rotated faster than forecast, so
 * the model reads about 2.2 s high in the 2020s. 2.2 s of UT is 33.7 arcsec of
 * hour angle, which is larger than the entire 27.2 arcsec residual issue #17
 * exists to explain.
 *
 * Every group below that grades the topocentric chain therefore derives UT1
 * from THIS table, never from hijri_delta_t_seconds(), so that the parallax
 * and hour-angle code is measured without a delta-T artifact standing in front
 * of it. Group 13 measures the delta-T model separately, which is the only
 * honest way to report both. */
static const double SKY_DELTA_T[24][2] = {
  {2415020.5, -1.9754351},
  {2433282.5, 28.9320000},
  {2458853.5, 69.3630814},
  {2458931.7, 69.4054885},
  {2459044.2, 69.4104422},
  {2459122.9, 69.3576225},
  {2459201.3, 69.3628483},
  {2459318.6, 69.3616458},
  {2459407.1, 69.3394758},
  {2459502.8, 69.2887904},
  {2459613.4, 69.2911019},
  {2459688.2, 69.2827043},
  {2459777.9, 69.2372800},
  {2459860.5, 69.1875428},
  {2459955.1, 69.2009568},
  {2460048.7, 69.2130511},
  {2460133.3, 69.2152108},
  {2460229.6, 69.1694982},
  {2460322.4, 69.1757138},
  {2460451.8, 69.2052242},
  {2460577.2, 69.1268053},
  {2460699.5, 69.1392032},
  {2469807.5, 71.4428713},
  {2488069.5, 95.9270748},
};
/* Horizons cross-check for the topocentric altitude, retrieved 2026-08-07
 * with the observer-site query recorded in
 * docs/research/2026-08-07-topocentric-error-bar.md. Columns are jd_tt and
 * airless elevation in degrees. APPARENT='AIRLESS' is explicit in the query,
 * because the refracted column would differ by up to 0.57 deg near the
 * horizon and the discrepancy would look like a library defect.
 *
 * Two sites only, jakarta and high60. These tables are compared against
 * SKY_TOPO_*, not against the library. No change to hijri.h can alter that
 * comparison. It exists so that a transcription or convention error in either
 * oracle is caught by disagreement rather than trusted, which is the same role
 * group 1 plays for the geocentric tables. Bracketing the latitude range is
 * sufficient for that role, so mecca and mid45 are not fetched. */
static const double HORIZONS_TOPO_JAKARTA[24][2] = {
  {2415020.5, 23.972500},
  {2433282.5, -55.212337},
  {2458853.5, -86.611709},
  {2458931.7, 82.682774},
  {2459044.2, -10.157960},
  {2459122.9, -3.926439},
  {2459201.3, -60.618915},
  {2459318.6, 26.811896},
  {2459407.1, -31.951995},
  {2459502.8, 5.892524},
  {2459613.4, -43.005674},
  {2459688.2, 60.679984},
  {2459777.9, -66.482372},
  {2459860.5, -39.029630},
  {2459955.1, 5.928681},
  {2460048.7, 17.911560},
  {2460133.3, 61.461577},
  {2460229.6, 74.053515},
  {2460322.4, -36.863375},
  {2460451.8, -25.552503},
  {2460577.2, 9.681605},
  {2460699.5, 71.533711},
  {2469807.5, -78.103159},
  {2488069.5, 39.057641},
};
static const double HORIZONS_TOPO_HIGH60[24][2] = {
  {2415020.5, -52.543271},
  {2433282.5, 43.445448},
  {2458853.5, 16.027865},
  {2458931.7, -13.087502},
  {2459044.2, -20.782715},
  {2459122.9, -36.450266},
  {2459201.3, -8.059234},
  {2459318.6, -15.475172},
  {2459407.1, 50.679424},
  {2459502.8, -49.535635},
  {2459613.4, -26.103422},
  {2459688.2, -38.104758},
  {2459777.9, -10.077554},
  {2459860.5, 19.121654},
  {2459955.1, -14.539826},
  {2460048.7, 0.176695},
  {2460133.3, -29.811329},
  {2460229.6, -6.688829},
  {2460322.4, -34.585092},
  {2460451.8, -35.086032},
  {2460577.2, -3.110602},
  {2460699.5, -31.798232},
  {2469807.5, 10.759672},
  {2488069.5, 22.723118},
};
/* Meeus, "Astronomical Algorithms" 2nd ed., worked Example 47.a:
 * 1992 April 12 at 0h TD, i.e. JDE 2448724.5. The book prints
 *   lambda = 133.162655 deg, beta = -3.229126 deg, Delta = 368409.7 km.
 *
 * This is the sharpest check in the file. The Horizons fixture below is bounded
 * at 0.02 deg -- about 20000 units of Sigma-l -- so it proves the series is
 * broadly right while being nearly blind to a single mistyped coefficient.
 * Asserting lambda and beta to 1e-6 deg pins Sigma-l and Sigma-b to roughly one
 * unit.
 *
 * MEASURED SENSITIVITY, so nobody mistakes this for a complete guarantee.
 *
 * Exhaustive sweep: every one of the 60 Sigma-l coefficients was changed by
 * +1 and by -1 in turn, 120 mutations, and this file's own 1e-6 deg assertion
 * applied. Result:
 *
 *   CAUGHT  33 / 120   (27.5 %)
 *   MISSED  87 / 120   (72.5 %)
 *
 *   rows caught in BOTH directions      0
 *   rows caught in ONE  direction      33
 *   rows caught in NEITHER direction   27
 *
 * TWO mechanisms are at work, and it matters not to confuse them.
 *
 * First, visibility. Our unmutated lambda sits 3.1484e-7 deg below the printed
 * figure, and a one-unit change moves lambda by |sin(arg) * E| * 1e-6 deg. So a
 * row can be seen at all only when |sin(arg) * E| exceeds
 * (1e-6 - 3.1484e-7) / 1e-6 = 0.6852. Measured: the 27 invisible rows all have
 * |sin(arg) * E| <= 0.6760, the 33 detectable rows all have >= 0.7049. Those 27
 * rows are undetectable by this check in either direction -- a one-unit typo in
 * any of them passes the whole suite silently.
 *
 * Second, direction. For the 33 rows that clear the threshold, exactly one
 * direction is ever caught, never both, decided by whether the typo pushes the
 * residual away from zero:
 *
 *   47.A row 17 sigma_l  10675 -> 10676  residual +6.85e-7  MISSED
 *   47.A row 17 sigma_l  10675 -> 10674  residual -1.31e-6  CAUGHT
 *   47.A row 59 sigma_l    294 -> 295    residual -1.21e-6  CAUGHT
 *   47.A row 59 sigma_l    294 -> 293    residual +5.77e-7  MISSED
 *   47.B row  1 sigma_b 5128122 -> ...23 residual -1.06e-6  CAUGHT
 *
 * Both rows above are in the detectable minority (|sin*E| = 0.999993 and
 * 0.892090), so they show the direction effect but are not representative.
 *
 * Full sweep, all three columns, 60 rows each by +/-1:
 *
 *   Sigma-l   CAUGHT  33 / 120   baseline residual -3.1484e-7 deg
 *   Sigma-b   CAUGHT  41 / 120   baseline residual -4.1922e-7 deg
 *   Sigma-r   CAUGHT   0 / 120   baseline residual -1.5184e-2 km
 *
 * Sigma-r is blind, not merely weak: one unit is one metre while the book
 * prints Delta only to 0.1 km, so no single-unit distance typo is detectable in
 * any row or direction, and the 100 km Horizons bound does not rescue it.
 *
 * This is a hard limit, not an unfinished task. A coefficient unit is 1e-6 deg.
 * Correcting the Horizons comparison for nutation was tried and does work --
 * worst-case longitude error drops from 0.00514 to 0.00150 deg -- but that is
 * still 1500x coarser than one unit, the rest being aberration and truncation.
 * No Horizons bound reaches unit sensitivity; only a reference printed at unit
 * precision can, and Meeus prints one worked example for the Moon.
 *
 * So this is a strong smoke test, not per-coefficient coverage.
 *
 * It is also the provenance check: reproducing the publisher's own worked
 * example from the repository is stronger evidence that these are the
 * published coefficients than any claim about where they were copied from. */
static void check_meeus_example_47a(void) {
  HijriMoonPosition m = hijri_moon_position(2448724.5);
  check_within("meeus_47a_longitude", 2448724.5, m.geocentric_longitude_deg,
               133.162655, 1e-6);
  check_within("meeus_47a_latitude", 2448724.5, m.geocentric_latitude_deg,
               -3.229126, 1e-6);
  check_within("meeus_47a_distance", 2448724.5, m.distance_km, 368409.7, 0.1);
}

/* The fixture above is expressed in TT, so it calls hijri_moon_position()
 * directly and never touches Delta T. Every caller in hijri.h goes the other
 * way -- hijri_sun_altitude(), hijri_moon_altitude(), the evening parameters,
 * and both visibility evaluators all convert with hijri_jd_tt_from_ut() first.
 * Without the checks below, that conversion is untested and a regression in it
 * would leave this whole file green while shifting every real result.
 *
 * Delta T is not negligible next to the series accuracy this file asserts:
 * 73 s in 2023 moves the Moon 0.0112 deg, over twice the 0.0051 deg worst-case
 * series error. The header calls the routine "coarse but adequate"; these
 * checks are what makes "adequate" falsifiable.
 *
 * Published values are the usual reference figures for Delta T at those epochs.
 * The 8 s bound is deliberately loose -- it is not a precision claim, it bounds
 * the induced lunar error at about 0.0012 deg, a quarter of the series error.
 * Measured at the time of writing: worst deviation 5.01 s at 1950, i.e.
 * 0.00076 deg of lunar motion. */
static void check_delta_t(void) {
  struct { double jd; double published_s; } v[] = {
    {2415020.5, -2.80},  /* 1900.0 */
    {2433282.5, 29.10},  /* 1950.0 */
    {2451544.5, 63.83},  /* 2000.0 */
    {2455197.5, 66.07},  /* 2010.0 */
    {2458849.5, 69.36},  /* 2020.0 */
  };
  for (unsigned i = 0; i < sizeof v / sizeof *v; i++) {
    check_within("delta_t_seconds", v[i].jd, hijri_delta_t_seconds(v[i].jd),
                 v[i].published_s, 8.0);
  }
}

/* Proves the UT -> TT conversion is actually wired into the position path, not
 * merely present. Feeding the same number as UT and as TT must differ by
 * exactly the Moon's motion over Delta T, so this fails both if the conversion
 * is dropped (difference collapses to zero) and if it gains a wrong scale. */
static void check_ut_to_tt_path(void) {
  double jd_ut = 2460000.5;
  double dt_days = hijri_delta_t_seconds(jd_ut) / 86400.0;

  double as_ut = hijri_moon_position(jd_ut).geocentric_longitude_deg;
  double via_tt = hijri_moon_position(hijri_jd_tt_from_ut(jd_ut))
                      .geocentric_longitude_deg;
  double moved = angdiff(via_tt, as_ut);

  /* Mean lunar motion is 13.176 deg/day; over ~73 s that is ~0.0111 deg.
   * Allow 10 percent for the true instantaneous rate. */
  check_within("ut_to_tt_shifts_moon", jd_ut, moved, 13.176 * dt_days,
               0.1 * 13.176 * dt_days);
  check_true_nonzero("ut_to_tt_conversion_is_applied", moved);
}

/* Rigorous geocentric-to-topocentric reference, Meeus 11.1 and 40.6/40.7.
 *
 * This is the library's own algorithm done exactly. hijri_moon_topocentric()
 * substitutes bare sin(phi) and cos(phi) where Meeus has rho*sin(phi') and
 * rho*cos(phi'), dropping Earth flattening. Feeding both this function and the
 * library the same geocentric position isolates that approximation, with lunar
 * series truncation removed from the input rather than left in the answer.
 *
 * WGS84: a = 6378137.0 m, 1/f = 298.257223563, so b/a = 1 - f. */
static void ref_rho_phi(double lat_deg, double *rho_sin_phi,
                        double *rho_cos_phi) {
  const double f = 1.0 / 298.257223563;
  double phi = lat_deg * M_PI / 180.0;
  double b_over_a = 1.0 - f;
  double u = atan(b_over_a * tan(phi));
  *rho_sin_phi = b_over_a * sin(u);
  *rho_cos_phi = cos(u);
}

/* Topocentric right ascension and declination from a geocentric position,
 * using the ellipsoidal rho terms. Mirrors hijri_moon_topocentric() exactly
 * except for those two substitutions. */
static void ref_topocentric(double ra_deg, double dec_deg,
                            double parallax_deg, double jd_ut,
                            double lat_deg, double lon_deg,
                            double *ra_topo_deg, double *dec_topo_deg) {
  double rho_sin_phi, rho_cos_phi;
  double H, dec, pi_r, delta_ra, ra_topo, dec_topo;

  ref_rho_phi(lat_deg, &rho_sin_phi, &rho_cos_phi);

  H = hijri__hour_angle_deg(jd_ut, lon_deg, ra_deg) * M_PI / 180.0;
  dec = dec_deg * M_PI / 180.0;
  pi_r = parallax_deg * M_PI / 180.0;

  delta_ra = atan2(-rho_cos_phi * sin(pi_r) * sin(H),
                   cos(dec) - rho_cos_phi * sin(pi_r) * cos(H));
  ra_topo = ra_deg * M_PI / 180.0 + delta_ra;
  dec_topo = atan2((sin(dec) - rho_sin_phi * sin(pi_r)) * cos(delta_ra),
                   cos(dec) - rho_cos_phi * sin(pi_r) * cos(H));

  *ra_topo_deg = hijri__norm_deg(ra_topo * 180.0 / M_PI);
  *dec_topo_deg = dec_topo * 180.0 / M_PI;
}

/* Group 10a -- the reference's own Earth geometry, checked before it is
 * trusted to grade anything.
 *
 * At the equator and at the pole the ellipsoid and the sphere agree exactly.
 * In between, rho*cos(phi') must EXCEED cos(phi), and the direction is worth
 * stating because it is easy to get backwards. rho*cos(phi') is the point's
 * distance from the rotation axis in equatorial radii. Flattening shortens
 * the radius, rho = 0.998331 at latitude 45, but it also drops the geocentric
 * latitude below the geodetic one, 44.8076 against 45, and that second effect
 * dominates for axis distance. Measured: rho*cos(phi') = 0.708293171 against
 * cos(45) = 0.707106781, a surplus of 0.001186390. The companion term
 * rho*sin(phi') does fall below sin(phi), 0.703552 against 0.707107, which is
 * the flattening pulling the surface in along the axis. */
static void check_group10_ref_selftest(void) {
  double rs, rc;

  ref_rho_phi(0.0, &rs, &rc);
  check_within("ref_rho_sin_phi_equator", 0.0, rs, 0.0, 1e-12);
  check_within("ref_rho_cos_phi_equator", 0.0, rc, 1.0, 1e-12);

  ref_rho_phi(45.0, &rs, &rc);
  check_true_nonzero("ref_rho_cos_phi_45_exceeds_spherical",
                     rc - cos(45.0 * M_PI / 180.0));

  ref_rho_phi(90.0, &rs, &rc);
  check_within("ref_rho_cos_phi_pole", 0.0, rc, 0.0, 1e-12);
}

/* The 24 epochs in this file were chosen to exercise GEOCENTRIC position, so
 * they sweep the whole sky. Measured altitudes run from -86.61 to +82.68 deg
 * at Jakarta. MABIMS 2021 thresholds altitude at 3 deg, so the geometry the
 * criteria actually consume is a narrow band near the horizon, and only 4 of
 * the 24 Jakarta rows fall in it.
 *
 * A maximum over all 24 rows is a valid upper bound and is what the pinned
 * tolerances use, because a bound should be conservative. But quoting that
 * number as "the topocentric error bar" when the row driving it sits 86 deg
 * below the horizon would answer a different question than issue #17 asks.
 * Every altitude group therefore prints BOTH the overall maximum and the
 * maximum restricted to this band, with the row count and the driving epoch,
 * so the research note can quote the relevant one and say which it is.
 *
 * 15 deg is deliberately wider than the 3 deg threshold. Parallax and its
 * error vary smoothly with zenith distance, so a band that hugged 3 deg would
 * report a near-empty sample. */
#define HORIZON_BAND_DEG 15.0

/* Earth-rotation-dependent groups run on the FIRST 22 epochs, not all 24.
 *
 * The last two fixture epochs are jd 2469807.5 (2050) and 2488069.5 (2100).
 * Topocentric altitude depends on where the Earth has rotated to, which means
 * it depends on UT1, and UT1 beyond the IERS prediction horizon is not a
 * determinate quantity. Measured consequence: Skyfield and Horizons, given
 * identical TT epochs and identical sites, disagree by 384.04 arcsec at
 * Jakarta in 2100 and 31.19 arcsec in 2050, while agreeing to better than
 * 0.7 arcsec at all 22 earlier epochs. The Horizons response header states
 * its own limit, "EOP coverage: DATA-BASED 1962-JAN-20 TO 2026-AUG-06.
 * PREDICTS-> 2026-NOV-01". 384 arcsec of altitude is about 25 seconds of
 * Earth rotation, which is two extrapolations of Earth's spin 74 years out
 * disagreeing, not a library error and not something any library can fix.
 *
 * Those two epochs stay in the tables so that row i here still means row i in
 * FIXTURE and SKY_MOON_GEOMETRIC, which group 10 relies on. They are simply
 * not measured against. Nothing that reaches altitude through sidereal time
 * may use them.
 *
 * Geocentric position does not depend on Earth's rotation, which is why the
 * pre-existing groups legitimately use all 24 and why this went unnoticed
 * until a topocentric quantity was measured. */
#define TOPO_EPOCH_COUNT 22

typedef struct {
  const char *name;
  double lat_deg;
  double lon_deg;
  const double (*table)[4];
} TopoSite;

static const TopoSite TOPO_SITES[4] = {
  {"jakarta", -6.2, 106.8, SKY_TOPO_JAKARTA},
  {"mecca",   21.4,  39.8, SKY_TOPO_MECCA},
  {"mid45",   45.0,   0.0, SKY_TOPO_MID45},
  {"high60",  60.0,   0.0, SKY_TOPO_HIGH60},
};

/* Group 9 -- the shipped topocentric altitude, against Skyfield.
 *
 * The chain is composed by hand rather than calling hijri_moon_altitude(),
 * and the reason is load-bearing. hijri_moon_altitude() takes a UT and derives
 * TT with hijri_delta_t_seconds(), so calling it here would fold the delta-T
 * model's error into a measurement of the parallax and hour-angle code. UT1 is
 * taken from SKY_DELTA_T instead, so the position is evaluated at the intended
 * TT and the sidereal time at the intended UT1. What hijri_moon_altitude()
 * itself costs is group 13's subject, not this one's. */
static void check_group9_topo_altitude(void) {
  int s, i;
  for (s = 0; s < 4; s++) {
    const TopoSite *site = &TOPO_SITES[s];
    HijriLocation loc;
    double max_err = 0.0, max_band = 0.0, band_jd = 0.0;
    int n_band = 0;
    char label[64];

    loc.latitude_deg = site->lat_deg;
    loc.longitude_deg = site->lon_deg;
    loc.elevation_m = 0.0;
    loc.name = NULL; /* HijriLocation has a fourth member, hijri.h:175 */

    for (i = 0; i < TOPO_EPOCH_COUNT; i++) {
      double jd_tt = site->table[i][0];
      double jd_ut = jd_tt - SKY_DELTA_T[i][1] / 86400.0;
      HijriMoonPosition geo = hijri_moon_position(jd_tt);
      double ra_topo, dec_topo, alt, err;

      hijri_moon_topocentric(&geo, jd_ut, site->lat_deg, site->lon_deg, 0.0,
                             &ra_topo, &dec_topo);
      alt = hijri__altitude_deg(ra_topo, dec_topo, jd_ut, &loc);
      err = fabs(alt - site->table[i][1]);
      if (err > max_err) max_err = err;

      /* Near-horizon band, see HORIZON_BAND_DEG. */
      if (fabs(site->table[i][1]) < HORIZON_BAND_DEG) {
        n_band++;
        if (err > max_band) max_band = err;
        if (err >= max_band) band_jd = jd_tt;
      }

      sprintf(label, "topo_alt_%s", site->name);
      check_within(label, jd_tt, alt, site->table[i][1], TOL_TOPO_ALT_DEG);
    }
    printf("topo_alt max %-8s = %.7f deg (%.2f arcsec)  "
           "near-horizon %.7f deg (%.2f arcsec) over %d rows, worst at jd %.1f\n",
           site->name, max_err, max_err * 3600.0,
           max_band, max_band * 3600.0, n_band, band_jd);
  }
}

/* Group 10 -- the parallax and hour-angle code alone.
 *
 * Both sides are fed the SAME geocentric position, taken from
 * SKY_MOON_GEOMETRIC, which is already mean-of-date geocentric at these exact
 * epochs. Lunar series truncation is therefore removed from the INPUT rather
 * than left in the answer, and what remains is the spherical-Earth
 * approximation in hijri_moon_topocentric() plus any coding defect.
 *
 * Subtracting this from group 9 gives the truncation and frame share by
 * measurement instead of by argument. That subtraction is the whole reason
 * issue #17 asks for a decomposition rather than a single number. */
static void check_group10_counterfactual(void) {
  int s, i;
  for (s = 0; s < 4; s++) {
    const TopoSite *site = &TOPO_SITES[s];
    HijriLocation loc;
    double max_err = 0.0, max_band = 0.0;
    int n_band = 0;
    char label[64];

    loc.latitude_deg = site->lat_deg;
    loc.longitude_deg = site->lon_deg;
    loc.elevation_m = 0.0;
    loc.name = NULL; /* HijriLocation has a fourth member, hijri.h:175 */

    for (i = 0; i < TOPO_EPOCH_COUNT; i++) {
      double jd_tt = SKY_MOON_GEOMETRIC[i][0];
      double jd_ut = jd_tt - SKY_DELTA_T[i][1] / 86400.0;
      HijriMoonPosition geo = hijri_moon_position(jd_tt);
      double ra_lib, dec_lib, ra_ref, dec_ref;
      double alt_lib, alt_ref, err;

      /* Same geocentric input to both, the library's own, so the only
       * difference between the two sides is the Earth-shape substitution. */
      hijri_moon_topocentric(&geo, jd_ut, site->lat_deg, site->lon_deg, 0.0,
                             &ra_lib, &dec_lib);
      ref_topocentric(geo.right_ascension_deg, geo.declination_deg,
                      geo.horizontal_parallax_deg, jd_ut,
                      site->lat_deg, site->lon_deg, &ra_ref, &dec_ref);

      alt_lib = hijri__altitude_deg(ra_lib, dec_lib, jd_ut, &loc);
      alt_ref = hijri__altitude_deg(ra_ref, dec_ref, jd_ut, &loc);
      err = fabs(alt_lib - alt_ref);
      if (err > max_err) max_err = err;

      if (fabs(site->table[i][1]) < HORIZON_BAND_DEG) {
        n_band++;
        if (err > max_band) max_band = err;
      }

      sprintf(label, "topo_cf_%s", site->name);
      check_within(label, jd_tt, alt_lib, alt_ref, TOL_TOPO_CF_DEG);
    }
    printf("topo_counterfactual max %-8s = %.7f deg (%.2f arcsec)  "
           "near-horizon %.7f deg (%.2f arcsec) over %d rows\n",
           site->name, max_err, max_err * 3600.0,
           max_band, max_band * 3600.0, n_band);
  }
}

/* Group 11 -- topocentric elongation, against two references.
 *
 * Column 2 of the site table is the convention-matched reference, a
 * topocentric Moon against a geocentric Sun, which is what hijri.h:1220
 * computes. Its residual is implementation error and nothing else.
 *
 * Column 3 is fully topocentric, both bodies seen from the site. The library
 * is NOT wrong to differ from it. Omitting solar parallax is the Pedoman
 * convention, recorded in docs/research/2026-08-01-wujudul-hilal-convention.md
 * and reaffirmed in PR #30. The gap between the two columns is the price of
 * that convention, roughly the 8.8 arcsec solar parallax, and it is measured
 * here so that issue #23 can report it rather than discover it.
 *
 * Reporting only one of these would either hide the convention or charge it as
 * a defect. Both are printed, separately, on purpose. */
static void check_group11_elongation(void) {
  int s, i;
  for (s = 0; s < 4; s++) {
    const TopoSite *site = &TOPO_SITES[s];
    double max_conv = 0.0, max_full = 0.0, max_cost = 0.0;
    char label[64];

    for (i = 0; i < TOPO_EPOCH_COUNT; i++) {
      double jd_tt = site->table[i][0];
      double jd_ut = jd_tt - SKY_DELTA_T[i][1] / 86400.0;
      HijriMoonPosition geo = hijri_moon_position(jd_tt);
      HijriSunPosition sun = hijri_sun_position(jd_tt);
      double ra_topo, dec_topo, elong, dc, df;

      hijri_moon_topocentric(&geo, jd_ut, site->lat_deg, site->lon_deg, 0.0,
                             &ra_topo, &dec_topo);
      elong = hijri__angular_separation_deg(ra_topo, dec_topo,
                                            sun.right_ascension_deg,
                                            sun.declination_deg);

      dc = fabs(elong - site->table[i][2]);
      df = fabs(elong - site->table[i][3]);
      if (dc > max_conv) max_conv = dc;
      if (df > max_full) max_full = df;

      /* Oracle-internal, no library code involved: the difference between a
       * fully topocentric elongation and one that omits solar parallax. This
       * is the price of the Pedoman convention, isolated. The printed
       * elong_full above is NOT this quantity, because it also carries the
       * library's own implementation error. */
      {
        double cost = fabs(site->table[i][3] - site->table[i][2]);
        if (cost > max_cost) max_cost = cost;
      }

      sprintf(label, "elong_conv_%s", site->name);
      check_within(label, jd_tt, elong, site->table[i][2], TOL_ELONG_CONV_DEG);
    }
    printf("elong_conv max %-8s = %.7f deg (%.2f arcsec)\n",
           site->name, max_conv, max_conv * 3600.0);
    printf("elong_full max %-8s = %.7f deg (%.2f arcsec)  <- cost of the "
           "geocentric-Sun convention, not an error\n",
           site->name, max_full, max_full * 3600.0);
    printf("elong_convention_cost max %-8s = %.7f deg (%.2f arcsec)  "
           "<- oracle-internal, the isolated price of omitting solar parallax\n",
           site->name, max_cost, max_cost * 3600.0);
  }
}

/* Group 12 -- two engines, same convention, same epochs, no library code. */
static void check_group12_oracle_vs_oracle(void) {
  const double (*sky[2])[4] = {SKY_TOPO_JAKARTA, SKY_TOPO_HIGH60};
  const double (*hor[2])[2] = {HORIZONS_TOPO_JAKARTA, HORIZONS_TOPO_HIGH60};
  const char *names[2] = {"jakarta", "high60"};
  int n_rows = 0, n_distinct = 0;
  int s, i;

  for (s = 0; s < 2; s++) {
    double max_err = 0.0;
    char label[64];
    for (i = 0; i < TOPO_EPOCH_COUNT; i++) {
      double d = fabs(sky[s][i][1] - hor[s][i][1]);
      if (d > max_err) max_err = d;
      n_rows++;
      if (d > 0.0) n_distinct++;
      sprintf(label, "sky_vs_horizons_topo_%s", names[s]);
      check_within(label, sky[s][i][0], sky[s][i][1], hor[s][i][1],
                   TOL_ORACLE_TOPO_DEG);
    }
    printf("sky_vs_horizons_topo max %-8s = %.7f deg\n", names[s], max_err);
  }

  /* Guard against one table having been copied from the other, which every
   * per-epoch tolerance check above would happily pass. This is the failure
   * mode mutation M7 records and mutation M13 exercises.
   *
   * An earlier version of this guard required the MINIMUM difference across
   * all rows to be non-zero. That is wrong, and the reason is worth keeping.
   * Horizons prints 6 decimal places and Skyfield 7, so the two engines can
   * land bit-identical at a row purely by rounding. They do exactly that at
   * jd 2458853.5 for jakarta, where both read -86.611709. One coincident row
   * zeroed the minimum and failed the guard on genuinely good data.
   *
   * Count instead. A copied table makes EVERY row identical, so requiring
   * more than three quarters of rows to differ separates a real copy from
   * incidental rounding collisions. Written as 4*n_distinct - 3*n_rows so it
   * can be handed to check_true_nonzero, which passes only on a positive
   * value. */
  printf("sky_vs_horizons_topo distinct rows = %d of %d\n",
         n_distinct, n_rows);
  check_true_nonzero("sky_vs_horizons_topo_nondegenerate_rows",
                     (double)(4 * n_distinct - 3 * n_rows));
}

/* Group 13 -- what hijri_delta_t_seconds() costs, reported two ways.
 *
 * Reporting only the first number would be alarming and misleading. Reporting
 * only the second would hide a real model defect. Both are printed.
 *
 * TT SPECIFIED: if the instant of interest is known in TT, as it is for every
 * fixture in this file, a UT must be derived to get sidereal time. Deriving it
 * with a delta-T model that is wrong by dT seconds puts the sidereal time off
 * by dT seconds of Earth rotation, which is 15*dT arcsec of hour angle. This
 * is the artifact groups 9 through 11 avoid by taking UT1 from SKY_DELTA_T,
 * and it is by far the larger of the two numbers.
 *
 * SUNSET ANCHORED: on the shipped path nobody supplies a UT. hijri_find_sunset
 * bisects hijri_sun_altitude to FIND one. A delta-T error moves the Sun's
 * evaluated position by dT seconds of solar motion, about 0.09 arcsec in right
 * ascension for dT = 2.2 s, so the recovered sunset instant in UT is barely
 * affected. The Moon is then evaluated at that essentially correct UT with a
 * TT that is dT late, costing dT seconds of lunar motion, while the sidereal
 * time is computed from the correct UT and is clean.
 *
 * The second number is the one that belongs in any user-facing error bar.
 *
 * CAVEAT ON THE REFERENCE. SKY_DELTA_T is Skyfield's delta-T, which is IERS
 * data only from 1962 onward. Before that it is a reconstruction, and it is
 * not tight: it reads -1.9754351 s at 1900 against the widely accepted
 * -2.72 s, and 28.9320000 s at 1950 against 29.15 s. That 0.745 s gap at 1900
 * is a third of the 2.2 s model error this group exists to attribute, so the
 * two pre-1962 epochs cannot grade hijri_delta_t_seconds() and are reported
 * separately rather than folded into the bound. The two post-2050 epochs are
 * forecasts on both sides and are excluded from the bound for the same reason
 * in the opposite direction.
 *
 * This does NOT affect groups 9 through 11. Those derive UT1 from this same
 * table, and Skyfield placed its own observer with the same value, so the two
 * sides are self-consistent whatever the absolute truth is. */
static void check_group13_delta_t(void) {
  double max_dt_err = 0.0, max_dt_err_graded = 0.0;
  double max_ha_arcsec = 0.0, max_anchored_arcsec = 0.0;
  int i;

  for (i = 0; i < 24; i++) {
    double jd_tt = SKY_DELTA_T[i][0];
    double true_dt = SKY_DELTA_T[i][1];
    double jd_ut = jd_tt - true_dt / 86400.0;
    double lib_dt = hijri_delta_t_seconds(jd_ut);
    double dt_err = fabs(lib_dt - true_dt);
    double ha_arcsec, anchored_arcsec;
    HijriMoonPosition a, b;

    if (dt_err > max_dt_err) max_dt_err = dt_err;

    /* TT specified: the derived UT is wrong by dt_err, so the sidereal time
     * is wrong by dt_err of Earth rotation. 1.00273790935 converts a solar
     * interval to a sidereal one, 15 converts seconds of time to arcsec. */
    ha_arcsec = dt_err * 1.00273790935 * 15.0;

    /* Sunset anchored: lunar motion over dt_err seconds, measured rather
     * than assumed, by evaluating the Moon at both times. */
    a = hijri_moon_position(jd_tt);
    b = hijri_moon_position(jd_tt + dt_err / 86400.0);
    anchored_arcsec =
        hijri__angular_separation_deg(a.right_ascension_deg,
                                      a.declination_deg,
                                      b.right_ascension_deg,
                                      b.declination_deg) * 3600.0;

    /* Bound only the epochs where the reference is IERS data. Rows 0 and 1
     * are 1900 and 1950, before IERS coverage. Rows 22 and 23 are 2050 and
     * 2100, past it. Both regimes are printed below but not asserted. */
    if (i >= 2 && i < TOPO_EPOCH_COUNT) {
      check_within("delta_t_model", jd_tt, lib_dt, true_dt, TOL_DELTA_T_SEC);
      if (dt_err > max_dt_err_graded) max_dt_err_graded = dt_err;
      /* A14: the two arcsec figures below are only meaningful where the
       * reference is IERS data. Accumulating them over the forecast rows
       * reported 2028.71 arcsec, which measures the disagreement between two
       * guesses about Earth's rotation in 2100 and nothing about this
       * library. */
      if (ha_arcsec > max_ha_arcsec) max_ha_arcsec = ha_arcsec;
      if (anchored_arcsec > max_anchored_arcsec)
        max_anchored_arcsec = anchored_arcsec;
    } else {
      printf("delta_t ungraded epoch jd %.1f: library %.4f s, reference "
             "%.4f s, difference %.4f s (reference is reconstruction or "
             "forecast, not IERS data)\n", jd_tt, lib_dt, true_dt, dt_err);
    }
  }

  printf("delta_t max error all epochs     = %.4f s\n", max_dt_err);
  printf("delta_t max error IERS-graded     = %.4f s\n", max_dt_err_graded);
  printf("delta_t TT-specified hour angle  = %.2f arcsec\n", max_ha_arcsec);
  printf("delta_t sunset-anchored residual = %.2f arcsec\n",
         max_anchored_arcsec);

  /* The whole argument for not treating delta-T as the dominant error rests
   * on the anchored residual being far smaller than the TT-specified one.
   * Assert it rather than leaving it as prose. */
  check_true_nonzero("delta_t_anchored_below_fixed",
                     max_ha_arcsec - max_anchored_arcsec);
}

/* Group 1 -- harness verification. Two oracles, same convention, same epochs. */
static void check_group1_harness(void) {
  double max_lon = 0.0, max_lat = 0.0, max_dist = 0.0;
  double min_lon = 999.0;
  int i;
  for (i = 0; i < 24; i++) {
    double dlon = angdiff(SKY_MOON_APPARENT[i][1], FIXTURE[i][1]);
    double dlat = fabs(SKY_MOON_APPARENT[i][2] - FIXTURE[i][2]);
    double ddist = fabs(SKY_MOON_APPARENT[i][3] - FIXTURE[i][3]);
    if (dlon > max_lon) max_lon = dlon;
    if (dlon < min_lon) min_lon = dlon;
    if (dlat > max_lat) max_lat = dlat;
    if (ddist > max_dist) max_dist = ddist;
    check_angle_within("sky_vs_horizons_lon", FIXTURE[i][0],
                       SKY_MOON_APPARENT[i][1], FIXTURE[i][1],
                       TOL_ORACLE_LON_DEG);
    check_within("sky_vs_horizons_lat", FIXTURE[i][0],
                 SKY_MOON_APPARENT[i][2], FIXTURE[i][2], TOL_ORACLE_LAT_DEG);
    check_within("sky_vs_horizons_dist", FIXTURE[i][0],
                 SKY_MOON_APPARENT[i][3], FIXTURE[i][3], TOL_ORACLE_DIST_KM);
  }
  printf("sky_vs_horizons max deviation: lon %.7f deg lat %.7f deg dist %.1f km\n",
         max_lon, max_lat, max_dist);
  /* Non-degeneracy, per row rather than in aggregate. A max-only guard passes
   * on a table copied from FIXTURE with a single row regenerated; asserting
   * the MINIMUM per-epoch deviation nonzero catches any row copied verbatim.
   * Valid because the measured per-epoch minimum is 6.6e-6 deg, comfortably
   * nonzero at every epoch. */
  check_true_nonzero("sky_vs_horizons_nondegenerate", max_lon);
  check_true_nonzero("sky_vs_horizons_min_nondegenerate", min_lon);
}

/* Group 2 -- what the truncated Meeus series actually costs, with the
 * deliberate omissions removed rather than inferred. */
static void check_group2_truncation(void) {
  double max_lon = 0.0, max_lat = 0.0, max_dist = 0.0;
  int i;
  for (i = 0; i < 24; i++) {
    HijriMoonPosition m = hijri_moon_position(SKY_MOON_GEOMETRIC[i][0]);
    double dlon = angdiff(m.geocentric_longitude_deg, SKY_MOON_GEOMETRIC[i][1]);
    double dlat = fabs(m.geocentric_latitude_deg - SKY_MOON_GEOMETRIC[i][2]);
    double ddist = fabs(m.distance_km - SKY_MOON_GEOMETRIC[i][3]);
    if (dlon > max_lon) max_lon = dlon;
    if (dlat > max_lat) max_lat = dlat;
    if (ddist > max_dist) max_dist = ddist;
    check_angle_within("moon_geom_lon", SKY_MOON_GEOMETRIC[i][0],
                       m.geocentric_longitude_deg, SKY_MOON_GEOMETRIC[i][1],
                       TOL_GEOM_LON_DEG);
    check_within("moon_geom_lat", SKY_MOON_GEOMETRIC[i][0],
                 m.geocentric_latitude_deg, SKY_MOON_GEOMETRIC[i][2],
                 TOL_GEOM_LAT_DEG);
    check_within("moon_geom_dist", SKY_MOON_GEOMETRIC[i][0], m.distance_km,
                 SKY_MOON_GEOMETRIC[i][3], TOL_GEOM_DIST_KM);
  }
  printf("moon_truncation max: lon %.7f deg lat %.7f deg dist %.1f km\n",
         max_lon, max_lat, max_dist);
}

/* Group 3 -- the Sun, measured against an oracle for the first time. */
static void check_group3_sun(void) {
  double max_lon = 0.0;
  double signed_sum = 0.0, signed_min = 0.0, signed_max = 0.0;
  int i;
  for (i = 0; i < 24; i++) {
    HijriSunPosition s = hijri_sun_position(SKY_SUN_APPARENT[i][0]);
    double dlon = angdiff(s.apparent_longitude_deg, SKY_SUN_APPARENT[i][1]);
    double sdlon = angsigned(s.apparent_longitude_deg, SKY_SUN_APPARENT[i][1]);
    if (dlon > max_lon) max_lon = dlon;
    if (i == 0 || sdlon < signed_min) signed_min = sdlon;
    if (i == 0 || sdlon > signed_max) signed_max = sdlon;
    signed_sum += sdlon;
    check_angle_within("sun_apparent_lon", SKY_SUN_APPARENT[i][0],
                       s.apparent_longitude_deg, SKY_SUN_APPARENT[i][1],
                       TOL_SUN_LON_DEG);
  }
  printf("sun_apparent max lon: %.7f deg\n", max_lon);
  /* One-sided offset => structural error. Two-sided spread with near-zero
   * mean => truncation noise. Same discriminator this file already applies
   * to the 385000.56 km distance constant. */
  printf("sun_apparent signed lon: mean %.7f deg min %.7f deg max %.7f deg\n",
         signed_sum / 24.0, signed_min, signed_max);
}

/* Group 4 -- elongation, the quantity MABIMS 2021 thresholds at 6.4 deg.
 *
 * Nutation in longitude shifts every ecliptic longitude by the same dpsi, so
 * it cancels exactly in a Sun-minus-Moon difference PROVIDED both bodies are
 * expressed in the same frame. This library does not do that: the Sun is
 * apparent (nutation applied) and the Moon is mean-of-date (nutation not
 * applied), so dpsi is injected into the difference rather than cancelling.
 * This group measures that injection directly instead of arguing about it. */
static void check_group4_elongation(void) {
  double max_err = 0.0, max_mismatch = 0.0;
  int i;
  for (i = 0; i < 24; i++) {
    HijriMoonPosition m = hijri_moon_position(SKY_SUN_APPARENT[i][0]);
    HijriSunPosition s = hijri_sun_position(SKY_SUN_APPARENT[i][0]);
    /* As the library computes it: apparent Sun minus mean Moon. */
    double lib_diff = angdiff(s.apparent_longitude_deg,
                              m.geocentric_longitude_deg);
    /* Consistent reference, both bodies apparent. */
    double ref_app = angdiff(SKY_SUN_APPARENT[i][1], SKY_MOON_APPARENT[i][1]);
    /* Consistent reference, both bodies mean-of-date. */
    double ref_geo = angdiff(SKY_SUN_GEOMETRIC[i][1],
                             SKY_MOON_GEOMETRIC[i][1]);
    double err = fabs(lib_diff - ref_app);
    double mismatch = fabs(ref_app - ref_geo);
    if (err > max_err) max_err = err;
    if (mismatch > max_mismatch) max_mismatch = mismatch;
    check_within("elongation_vs_apparent_ref", SKY_SUN_APPARENT[i][0], lib_diff,
                 ref_app, TOL_ELONG_DEG);
  }
  printf("elongation_err max: %.7f deg\n", max_err);
  printf("frame_mismatch max (apparent vs mean reference): %.7f deg\n",
         max_mismatch);
}

/* Group 5 -- the counterfactual, and the answer to "should we just fix the
 * frame mismatch group 4 measures".
 *
 * Undoes the aberration and nutation-in-longitude terms that
 * hijri_sun_position() applies at hijri.h:505-506, putting the Sun in the same
 * mean-of-date frame as the Moon, then compares that unified difference
 * against a both-mean DE440 reference.
 *
 * The result is counterintuitive and is the reason this check is committed
 * rather than left as a one-off experiment: unifying the frames measures
 * WORSE, not better. 0.0083813 deg here against 0.0070530 deg for the mixed
 * frame group 4 measures, about 19 percent worse. The mismatch partially
 * cancels the solar truncation error, so removing it exposes the full solar
 * residual -- note 0.0083813 lands almost exactly on the 0.0084042 deg the Sun
 * misses by on its own in group 3. The quantity that actually limits
 * elongation accuracy is the Meeus ch. 25 solar theory, not the frame.
 *
 * This is also the only assertion that reads SKY_SUN_GEOMETRIC, which is what
 * holds that table to the same mutation-proven standard as the other three. */
static void check_group5_frame_counterfactual(void) {
  double max_err = 0.0, max_coupling = 0.0;
  int i;
  for (i = 0; i < 24; i++) {
    double jd = SKY_SUN_GEOMETRIC[i][0];
    double t = (jd - 2451545.0) / 36525.0;
    double omega = 125.04 - 1934.136 * t;
    /* The two terms from hijri.h:505-506, subtracted back out. */
    double nut_ab = -0.00569 - 0.00478 * sin(omega * M_PI / 180.0);
    HijriSunPosition s = hijri_sun_position(jd);
    HijriMoonPosition m = hijri_moon_position(jd);
    double sun_mean = s.apparent_longitude_deg - nut_ab;
    double unified = angdiff(sun_mean, m.geocentric_longitude_deg);
    double ref_geo = angdiff(SKY_SUN_GEOMETRIC[i][1],
                             SKY_MOON_GEOMETRIC[i][1]);
    double err = fabs(unified - ref_geo);
    if (err > max_err) max_err = err;
    check_within("elongation_frame_unified", jd, unified, ref_geo,
                 TOL_ELONG_UNIFIED_DEG);
    /* Coupling guard. nut_ab above duplicates the two constants at
     * hijri.h:505-506, and nothing else would notice if either side moved:
     * with 2x margin on the assertion above, a changed library term drifts
     * sun_mean out of the mean-of-date frame while everything still passes.
     * The tables record the same physics independently -- apparent minus
     * geometric solar longitude IS the applied nutation plus aberration, per
     * the generator -- so asserting nut_ab against that difference fails if
     * either the library constants or the tables move. */
    {
      /* SIGNED difference -- angdiff() takes an absolute value, which would
       * compare a negative nut_ab against a positive magnitude and report a
       * spurious doubling. */
      double table_nut_ab = fmod(SKY_SUN_APPARENT[i][1] -
                                 SKY_SUN_GEOMETRIC[i][1] + 540.0, 360.0) -
                            180.0;
      double coupling = fabs(nut_ab - table_nut_ab);
      if (coupling > max_coupling) max_coupling = coupling;
      check_within("sun_terms_vs_tables", jd, nut_ab, table_nut_ab,
                   TOL_SUN_TERMS_DEG);
    }
  }
  printf("elongation_frame_unified max: %.7f deg\n", max_err);
  printf("sun_terms_vs_tables max: %.7f deg\n", max_coupling);
}

/* Group 6 -- solar harness verification, the Sun's counterpart of group 1.
 * SKY_SUN_APPARENT against HORIZONS_SUN, two independently written clients on
 * the same convention. Neither table involves the library, so like group 1
 * this is deliberately a check of the comparison machinery, committed so the
 * agreement is a recorded fact and a later edit to either table is caught. */
static void check_group6_sun_harness(void) {
  double max_lon = 0.0, max_lat = 0.0, max_dist = 0.0;
  double min_lon = 999.0;
  int i;
  for (i = 0; i < 24; i++) {
    double dlon = angdiff(SKY_SUN_APPARENT[i][1], HORIZONS_SUN[i][1]);
    double dlat = fabs(SKY_SUN_APPARENT[i][2] - HORIZONS_SUN[i][2]);
    double ddist = fabs(SKY_SUN_APPARENT[i][3] - HORIZONS_SUN[i][3]);
    if (dlon > max_lon) max_lon = dlon;
    if (dlon < min_lon) min_lon = dlon;
    if (dlat > max_lat) max_lat = dlat;
    if (ddist > max_dist) max_dist = ddist;
    check_angle_within("sky_vs_horizons_sun_lon", HORIZONS_SUN[i][0],
                       SKY_SUN_APPARENT[i][1], HORIZONS_SUN[i][1],
                       TOL_ORACLE_LON_DEG);
    check_within("sky_vs_horizons_sun_lat", HORIZONS_SUN[i][0],
                 SKY_SUN_APPARENT[i][2], HORIZONS_SUN[i][2],
                 TOL_ORACLE_LAT_DEG);
    check_within("sky_vs_horizons_sun_dist", HORIZONS_SUN[i][0],
                 SKY_SUN_APPARENT[i][3], HORIZONS_SUN[i][3],
                 TOL_ORACLE_DIST_KM);
  }
  printf("sky_vs_horizons_sun max deviation: lon %.7f deg lat %.7f deg "
         "dist %.1f km\n", max_lon, max_lat, max_dist);
  /* Per-row non-degeneracy, as in group 1. Measured per-epoch minimum
   * 5.4e-6 deg, nonzero at every epoch. */
  check_true_nonzero("sky_vs_horizons_sun_nondegenerate", max_lon);
  check_true_nonzero("sky_vs_horizons_sun_min_nondegenerate", min_lon);
}

/* Spherical separation between two ecliptic (lon, lat) directions, degrees.
 * Reference-side math only; the library side goes through its own RA/Dec
 * path below. */
static double ecl_separation_deg(double lon1, double lat1, double lon2,
                                 double lat2) {
  double l1 = lon1 * M_PI / 180.0, b1 = lat1 * M_PI / 180.0;
  double l2 = lon2 * M_PI / 180.0, b2 = lat2 * M_PI / 180.0;
  double c = sin(b1) * sin(b2) + cos(b1) * cos(b2) * cos(l1 - l2);
  if (c > 1.0) c = 1.0;
  if (c < -1.0) c = -1.0;
  return acos(c) * 180.0 / M_PI;
}

/* Group 7 -- the SHIPPED elongation path. Groups 4 and 5 recompute a
 * longitude difference in test code; nothing there executes the code the
 * criteria actually run. hijri.h computes elongation with
 * hijri__angular_separation_deg on RA/Dec (hijri.h:1115-1118), which goes
 * through the ecliptic-to-equatorial conversion of both bodies. This group
 * feeds the library's own RA/Dec through that same private function and
 * compares against the spherical separation of the DE440 apparent directions,
 * so a defect in the obliquity term or the separation formula is no longer
 * invisible.
 *
 * Scope, stated so the published claims stay honest: this measures the
 * GEOCENTRIC elongation path. The topocentric value adds the parallax
 * correction, which depends on an observer location and is not measured
 * here. */
static void check_group7_shipped_elongation(void) {
  double max_err = 0.0;
  int i;
  for (i = 0; i < 24; i++) {
    double jd = SKY_SUN_APPARENT[i][0];
    HijriSunPosition s = hijri_sun_position(jd);
    HijriMoonPosition m = hijri_moon_position(jd);
    /* The same call hijri.h:1115-1118 makes, on the same inputs. */
    double lib_sep = hijri__angular_separation_deg(
        m.right_ascension_deg, m.declination_deg, s.right_ascension_deg,
        s.declination_deg);
    double ref_sep = ecl_separation_deg(SKY_SUN_APPARENT[i][1],
                                        SKY_SUN_APPARENT[i][2],
                                        SKY_MOON_APPARENT[i][1],
                                        SKY_MOON_APPARENT[i][2]);
    double err = fabs(lib_sep - ref_sep);
    if (err > max_err) max_err = err;
    check_within("elongation_shipped_path", jd, lib_sep, ref_sep,
                 TOL_ELONG_SHIPPED_DEG);
  }
  printf("elongation_shipped_path max: %.7f deg\n", max_err);
}

/* Group 14 -- the sunset and moonset solvers against oracle-solved instants.
 *
 * Sunset is the sampling instant for every official calendar this library is
 * validated against, and near the horizon the Moon falls at about 0.004 deg per
 * second, so a second of sunset error is roughly 14 arcsec of Moon altitude.
 * For scale, the entire topocentric chain measured in issue #32 contributes
 * 2.18 arcsec at Jakarta. This is the largest single term in the chain that
 * decides a date, and until this group existed it was unmeasured.
 *
 * Moonset is bounded but not decomposed, deliberately. It decides nothing in
 * any validated path: it feeds lag_time_minutes and moonset_after_sunset, read
 * only by two research predicates and by the Umm al-Qura fallback at
 * hijri.h:1760, which fires only outside 1882-11-12 to 2174-11-25 and so never
 * runs in the 198 of 198 table fixture.
 *
 * The convergence check uses no oracle at all. It evaluates the library's own
 * altitude function at the instant the library reports and bounds the distance
 * from the target it solved for. That pins the solver independently of the
 * ephemeris, so a regression in one cannot hide inside the other's tolerance,
 * and it is the only assertion here that would survive deleting every table. */
static void check_group14_setsolve(void) {
  int s, i;
  for (s = 0; s < 4; s++) {
    const SetSolveSite *site = &SETSOLVE_SITES[s];
    HijriLocation loc;
    double max_ss = 0.0, max_ms = 0.0;
    char label[64];

    loc.latitude_deg = site->lat_deg;
    loc.longitude_deg = site->lon_deg;
    loc.elevation_m = 0.0;
    loc.name = NULL;

    for (i = 0; i < 12; i++) {
      /* LOCAL midnight expressed in UT, matching hijri.h:1182. NOT -0.5.
       * hijri_jd_from_gregorian already returns 0h UT, and hijri_find_sunset
       * scans forward 24 hours from whatever it is handed, so subtracting half
       * a day starts the scan at noon on the PREVIOUS day and returns the
       * previous evening's sunset at most longitudes. hijri.h:1163-1181
       * documents this exact off-by-one-day class, which the library was
       * already bitten by once. */
      double jd_midnight = hijri_jd_from_gregorian(
          SETSOLVE_DATES[i][0], SETSOLVE_DATES[i][1],
          (double)SETSOLVE_DATES[i][2]) - site->lon_deg / 360.0;
      double lib_ss, lib_ms, err;

      sprintf(label, "setsolve_sunset_available_%s", site->name);
      if (hijri_find_sunset(jd_midnight, &loc, &lib_ss) != HIJRI_EVENT_OK) {
        check_true_nonzero(label, 0.0);
        continue;   /* lib_ss is not written on failure, so do not read it */
      }
      check_true_nonzero(label, 1.0);

      err = fabs(lib_ss - site->table[i][0]) * 86400.0;
      if (err > max_ss) max_ss = err;
      sprintf(label, "setsolve_sunset_%s", site->name);
      check_within(label, site->table[i][0], err, 0.0,
                   TOL_SETSOLVE_SUNSET_S);

      sprintf(label, "setsolve_converge_%s", site->name);
      check_within(label, site->table[i][0],
                   hijri_sun_altitude(lib_ss, &loc),
                   -(HIJRI__REFRACTION_AT_HORIZON_DEG +
                     HIJRI__SOLAR_SEMIDIAMETER_DEG),
                   TOL_SETSOLVE_CONVERGE_DEG);

      sprintf(label, "setsolve_moonset_available_%s", site->name);
      if (hijri_find_moonset(lib_ss, &loc, &lib_ms) != HIJRI_EVENT_OK) {
        check_true_nonzero(label, 0.0);
        continue;   /* lib_ms is not written on failure, so do not read it */
      }
      check_true_nonzero(label, 1.0);

      err = fabs(lib_ms - site->table[i][1]) * 86400.0;
      if (err > max_ms) max_ms = err;
      sprintf(label, "setsolve_moonset_%s", site->name);
      check_within(label, site->table[i][1], err, 0.0,
                   site->tol_moonset_s);
    }
    printf("setsolve max %-8s sunset %.4f s  moonset %.4f s\n",
           site->name, max_ss, max_ms);
  }

  {
    int k;
    double max_resid = 0.0, min_resid = 1e9;
    for (k = 0; k < 3; k++) {
      double jd = HORIZONS_SETSOLVE_SUN[k][0];
      HijriSunPosition p = hijri_sun_position(hijri_jd_tt_from_ut(jd));
      double dra = fabs(p.right_ascension_deg - HORIZONS_SETSOLVE_SUN[k][1]);
      double ddec = fabs(p.declination_deg - HORIZONS_SETSOLVE_SUN[k][2]);
      if (dra > max_resid) max_resid = dra;
      if (ddec > max_resid) max_resid = ddec;
      if (dra < min_resid) min_resid = dra;
      if (ddec < min_resid) min_resid = ddec;
      check_angle_within("setsolve_horizons_sun_ra", jd,
                         p.right_ascension_deg, HORIZONS_SETSOLVE_SUN[k][1],
                         TOL_SETSOLVE_HORIZONS_DEG);
      check_within("setsolve_horizons_sun_dec", jd, p.declination_deg,
                   HORIZONS_SETSOLVE_SUN[k][2], TOL_SETSOLVE_HORIZONS_DEG);
    }
    /* A FLOOR, not a distinctness count, and the difference matters.
     *
     * Mutation S5 showed the distinctness count does not guard this table. It
     * counted only the right ascension column, leaving declination unguarded,
     * and counting distinctness is the wrong test here regardless: the M7
     * precedent guards one stored table being copied byte-exact from another,
     * and this table has no sibling to copy from. Its realistic failure is
     * being regenerated from the LIBRARY instead of from Horizons, which
     * yields seven-decimal values that never equal the library's doubles bit
     * for bit, so a distinctness count passes.
     *
     * Two genuinely independent sources must disagree by a non-trivial amount.
     * The library's ch. 25 solar theory reaches 0.0084042 deg against DE440 by
     * this file's own header, and the measured residual here runs to
     * 0.0034349 deg. A table regenerated from the library would sit at
     * roughly zero and fail this floor.
     *
     * The floor is taken over the MINIMUM of the six differences, not the
     * maximum, and that was measured rather than assumed. A floor on the
     * maximum was written first and executed: replacing the declination column
     * alone left the right ascension column's 0.0034349 deg standing, the
     * maximum was unchanged, and the suite stayed at "1029 checks, 0
     * failures". A maximum guards only the whole table going degenerate at
     * once. The minimum guards every cell, which is the shape groups 1 and 6
     * already use and the shape the S5 record says this guard needed. */
    printf("setsolve_horizons residual min %.7f max %.7f deg\n",
           min_resid, max_resid);
    check_true_nonzero("setsolve_horizons_independent",
                       min_resid - TOL_SETSOLVE_HORIZONS_FLOOR_DEG);
  }
}

/* MUTATION M8 (2026-08-05): negating the sign of hijri__eqeq_deg() fails this
 * group with, verbatim:
 *   FAIL eqeq at jd=2415020.5: got -0.0043072 want 0.0044419 (err 0.0087491 > tol 0.0010000)
 * Reverting restores 448 checks, 0 failures. The fixture therefore detects a
 * sign error, which is the most likely way this term is got wrong.
 *
 * MUTATION M9 (2026-08-05): making hijri__eqeq_deg() return 0.0, which is
 * exactly the pre-fix behaviour of pairing an apparent solar right ascension
 * with MEAN sidereal time, fails this group with, verbatim:
 *   FAIL eqeq at jd=2415020.5: got 0.0000000 want 0.0044419 (err 0.0044419 > tol 0.0010000)
 * 20 of the 24 epochs fail, not all 24: at the remaining four the true
 * equation of the equinoxes is itself smaller than TOL_EQEQ_DEG, so zero is
 * within tolerance there. That is the honest detection rate, and it is the
 * more important of the two mutations, because it proves this fixture would
 * catch a silent regression to the old frame pairing rather than only a
 * fat-fingered sign. Reverting restores 448 checks, 0 failures. */

/* Group 8: the equation of the equinoxes the library applies to solar hour
 * angles, against Skyfield's full IAU 2000B value.
 *
 * Measured maximum over these 24 epochs, printed by this binary as
 * "eqeq max deviation": 0.0004829 deg. The true term ranges over -16.01 to
 * +15.99 arcsec across the set, so the one-term approximation captures it to
 * about 11% worst case. Before this fix the solar hour angle carried the FULL
 * term as error; after it, at most the residual. That is a 9x reduction, not
 * elimination, and the tolerance below is sized to say so honestly.
 *
 * TOL_EQEQ_DEG 0.001 is the measured 0.0004829 rounded up to leave 2.07x
 * margin, consistent with the roughly 2x margin used elsewhere in this file. */
static void check_group8_eqeq(void) {
  double worst = 0.0;
  for (int i = 0; i < 24; i++) {
    double jd = SKY_EQEQ[i][0];
    double lib = hijri__eqeq_deg(jd);
    double dev = fabs(lib - SKY_EQEQ[i][1]);
    if (dev > worst) worst = dev;
    check_within("eqeq", jd, lib, SKY_EQEQ[i][1], TOL_EQEQ_DEG);
  }
  printf("eqeq max deviation: %.7f deg\n", worst);

  /* Non-degeneracy: a table of zeros, or a formula that returned zero, would
   * satisfy the bound above on a set whose values are all small. Assert the
   * fixture actually spans both signs and reaches the term's real amplitude. */
  {
    double lo = SKY_EQEQ[0][1], hi = SKY_EQEQ[0][1];
    for (int i = 1; i < 24; i++) {
      if (SKY_EQEQ[i][1] < lo) lo = SKY_EQEQ[i][1];
      if (SKY_EQEQ[i][1] > hi) hi = SKY_EQEQ[i][1];
    }
    /* check_true_nonzero is this file's only boolean helper, so express each
     * condition as a quantity that is positive exactly when it holds. */
    check_true_nonzero("eqeq_fixture_spans_negative", -0.004 - lo);
    check_true_nonzero("eqeq_fixture_spans_positive", hi - 0.004);
  }
}

int main(void) {
  double max_lon = 0.0, max_lat = 0.0, max_dist = 0.0;

  check_group1_harness();
  check_group2_truncation();
  check_group3_sun();
  check_group4_elongation();
  check_group5_frame_counterfactual();
  check_group6_sun_harness();
  check_group7_shipped_elongation();
  check_group14_setsolve();
  check_group8_eqeq();
  check_group10_ref_selftest();
  check_group9_topo_altitude();
  check_group10_counterfactual();
  check_group11_elongation();
  check_group12_oracle_vs_oracle();
  check_group13_delta_t();
  check_meeus_example_47a();
  check_delta_t();
  check_ut_to_tt_path();

  for (int i = 0; i < 24; i++) {
    double jd = FIXTURE[i][0];
    HijriMoonPosition m = hijri_moon_position(jd);

    double e_lon = angdiff(m.geocentric_longitude_deg, FIXTURE[i][1]);
    double e_lat = fabs(m.geocentric_latitude_deg - FIXTURE[i][2]);
    double e_dist = fabs(m.distance_km - FIXTURE[i][3]);

    if (e_lon > max_lon) max_lon = e_lon;
    if (e_lat > max_lat) max_lat = e_lat;
    if (e_dist > max_dist) max_dist = e_dist;

    check_angle_within("moon_longitude", jd, m.geocentric_longitude_deg,
                       FIXTURE[i][1], TOL_LON_DEG);
    check_within("moon_latitude", jd, m.geocentric_latitude_deg,
                 FIXTURE[i][2], TOL_LAT_DEG);
    check_within("moon_distance", jd, m.distance_km, FIXTURE[i][3],
                 TOL_DIST_KM);
  }

  printf("max_lon_err=%.4f deg  max_lat_err=%.4f deg  max_dist_err=%.1f km\n",
         max_lon, max_lat, max_dist);
  printf("Moon ephemeris tests: %d checks, %d failures\n", checks, failures);
  return failures == 0 ? 0 : 1;
}

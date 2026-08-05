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
 *     M3 scope note records as invisible to the max-only guard. */

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
  check_group8_eqeq();
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

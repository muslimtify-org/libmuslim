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
 * Both sit four orders of magnitude below the 6.4 deg MABIMS 2021 elongation
 * threshold, so the mixed-frame difference this library computes is not a
 * source of criterion-outcome error at that threshold. The bound below exists
 * to pin current behaviour, so a later frame change shows up as a deliberate
 * diff rather than silent drift.
 *
 * 0.015 deg is the measured maximum rounded up to leave 2.13x margin. */
#define TOL_ELONG_DEG 0.015

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
 * the aberration term and the nutation term (hijri.h:485-502) -- so it is
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
 * Both Sun tables carry an ecliptic latitude column for format symmetry with
 * the Moon tables. The library models solar ecliptic latitude as zero, which
 * is correct to about 1.2 arcsec; that is recorded in the research note rather
 * than asserted here. */
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
  int i;
  for (i = 0; i < 24; i++) {
    double dlon = angdiff(SKY_MOON_APPARENT[i][1], FIXTURE[i][1]);
    double dlat = fabs(SKY_MOON_APPARENT[i][2] - FIXTURE[i][2]);
    double ddist = fabs(SKY_MOON_APPARENT[i][3] - FIXTURE[i][3]);
    if (dlon > max_lon) max_lon = dlon;
    if (dlat > max_lat) max_lat = dlat;
    if (ddist > max_dist) max_dist = ddist;
    check_angle_within("sky_vs_horizons_lon", FIXTURE[i][0],
                       SKY_MOON_APPARENT[i][1], FIXTURE[i][1], 1e-4);
    check_within("sky_vs_horizons_lat", FIXTURE[i][0],
                 SKY_MOON_APPARENT[i][2], FIXTURE[i][2], 1e-4);
    check_within("sky_vs_horizons_dist", FIXTURE[i][0],
                 SKY_MOON_APPARENT[i][3], FIXTURE[i][3], 1.0);
  }
  printf("sky_vs_horizons max deviation: lon %.7f deg lat %.7f deg dist %.1f km\n",
         max_lon, max_lat, max_dist);
  /* A table pasted twice would print exactly zero. */
  check_true_nonzero("sky_vs_horizons_nondegenerate", max_lon);
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

int main(void) {
  double max_lon = 0.0, max_lat = 0.0, max_dist = 0.0;

  check_group1_harness();
  check_group2_truncation();
  check_group3_sun();
  check_group4_elongation();
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

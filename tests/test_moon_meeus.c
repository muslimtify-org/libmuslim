/* test_moon_meeus.c -- validates hijri_moon_position() against JPL Horizons.
 *
 * Build:  gcc -std=c11 -Wall -Wextra -Wpedantic -O2 tests/test_moon_meeus.c -lm \
 *             -o /tmp/libmuslim-test-moon
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

static int checks;
static int failures;

/* Smallest absolute difference between two angles, in degrees. */
static double angdiff(double a, double b) {
  double d = fmod(a - b + 540.0, 360.0) - 180.0;
  return fabs(d);
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
 * The book prints lambda to six decimals, and our unmutated value sits
 * 3.15e-7 deg below the printed figure. That offset is what decides whether a
 * given typo is caught, because a one-unit change in a Sigma-l coefficient
 * moves lambda by at most 1e-6 deg -- the same order as the tolerance itself.
 * So a typo is caught when it pushes the residual AWAY from zero past 1e-6,
 * and missed when it pushes toward zero. Single-unit mutations, both
 * directions:
 *
 *   47.A row 17 sigma_l  10675 -> 10676  residual +6.85e-7  MISSED
 *   47.A row 17 sigma_l  10675 -> 10674  residual -1.31e-6  CAUGHT
 *   47.A row 59 sigma_l    294 -> 295    residual -1.21e-6  CAUGHT
 *   47.A row 59 sigma_l    294 -> 293    residual +5.77e-7  MISSED
 *   47.B row  1 sigma_b 5128122 -> ...23 residual  1.06e-6  CAUGHT
 *
 * Note what this does NOT say. Row 17 has sin(arg) = +0.999993 at this epoch,
 * i.e. maximum sensitivity, and it is still missed in one direction. The miss
 * is not a property of the row; both rows above are caught one way and missed
 * the other. Roughly half of single-unit Sigma-l typos escape, selected by
 * sign rather than by how sensitive the coefficient is. Do not read an
 * uncaught row as a low-sensitivity row.
 *
 * Sigma-r is a separate and harder case: one unit is one metre, while the book
 * prints Delta only to 0.1 km, so distance errors below about 100 units are
 * invisible in either direction.
 *
 * So this catches many single-digit transcription errors but by no means all.
 * Real per-coefficient coverage would need worked examples at several epochs,
 * and Meeus prints only this one.
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

int main(void) {
  double max_lon = 0.0, max_lat = 0.0, max_dist = 0.0;

  check_meeus_example_47a();

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

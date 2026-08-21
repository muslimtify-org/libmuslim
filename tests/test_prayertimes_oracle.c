/* test_prayertimes_oracle.c -- cross-header oracle: validates
 * calculate_prayer_times()'s maghrib against hijri_find_sunset(), an
 * independent bisection root-finder on topocentric solar altitude that
 * shares no code path with prayertimes.h's closed-form hour-angle solver.
 *
 * Build:  gcc -std=c11 -Wall -Wextra -Wpedantic -O2 tests/test_prayertimes_oracle.c \
 *             -lm -o /tmp/libmuslim-test-prayertimes-oracle
 *
 * WHY THIS EXISTS
 *
 *   prayertimes.h computes sunset from a closed-form hour-angle formula,
 *   refined once at the event's own instant (see prayertimes.h's
 *   refine_event(), added to fix the bug this oracle's Step 7 mutation
 *   record reproduces below). hijri.h computes sunset by bisecting
 *   hijri_sun_altitude() to a target altitude over a 24-hour window. The two
 *   have never been compared against each other before this file.
 *
 * PAIRING THE CONVENTIONS
 *
 *   prayertimes.h always evaluates sunset with REFRACTION_CORRECTION, 0.833
 *   deg (prayertimes.h:81), applied as a pure horizon-dip angle with no
 *   separate semidiameter term.
 *
 *   hijri.h's HIJRI_SUNSET_CONVENTION_ASTRONOMICAL (hijri.h:388) is
 *   {0.5667, 959.63}: 0.5667 deg of refraction plus a solar semidiameter of
 *   959.63 arcsec at 1 au, applied as a separate term by hijri__find_event's
 *   caller so it varies with the Sun's actual distance over the year (see
 *   hijri.h's HijriSunsetConvention comment). At 1 au the two sum to
 *
 *     0.5667 + 959.63 / 3600 = 0.5667 + 0.266563... = 0.833263... deg
 *
 *   which is REFRACTION_CORRECTION to within 0.00026 deg (under 1
 *   arcsecond). The two conventions are the same physical horizon to that
 *   precision, so a bound on the difference between the two libraries'
 *   sunsets measures agreement between two independent solvers, not a
 *   convention mismatch absorbed as a constant offset. Away from 1 au the
 *   two diverge by a fraction of an arcsecond more (Earth's orbit is not
 *   circular), which is why the measured bound below carries margin rather
 *   than being pinned at zero.
 *
 * SUBTRACTING IHTIYAT
 *
 *   prayertimes.h adds a precautionary margin, params->ihtiyat minutes, to
 *   every prayer time it returns (prayertimes.h:580-587), maghrib included.
 *   hijri.h applies no such margin. This oracle uses CALC_MWL, whose
 *   ihtiyat is 0 minutes and whose maghrib_interval is 0 (so maghrib equals
 *   raw sunset, not sunset-plus-offset), but subtracts params->ihtiyat/60.0
 *   anyway so the comparison stays correct if the method used here ever
 *   changes.
 *
 * SHARED LOCAL-TIME FRAME
 *
 *   calculate_prayer_times() takes an explicit civil `timezone` (hours) and
 *   returns local decimal hours in that zone. hijri_find_sunset() has no
 *   notion of civil time at all: its jd_local_midnight_ut parameter is mean
 *   SOLAR local midnight expressed in UT, built as
 *   hijri_jd_from_gregorian(y, m, d) - longitude_deg / 360.0 (hijri.h:1493-
 *   1513 documents why: passing UT midnight instead silently returns the
 *   previous evening west of about 90 deg W).
 *
 *   To compare the two outputs in one frame without adding a civil-timezone
 *   dependency to either header, this oracle calls calculate_prayer_times()
 *   with timezone = longitude / 15.0 -- mean solar time used AS the civil
 *   offset. That makes prayertimes.h's "local decimal hours" and hijri.h's
 *   "hours since mean solar local midnight" the same frame by construction,
 *   so the comparison measures the two sunset solvers against each other
 *   and nothing about timezone conversion.
 *
 * GRID
 *
 *   Latitudes: -60, -50, -40, -30, -20, -10, 0, 10, 20, 30, 40, 50, 60
 *   degrees (13 points). The upper bound was chosen, not assumed: an
 *   earlier draft of this grid ran out to +-66 (just inside the 66.5622 deg
 *   Arctic/Antarctic circle, where hijri_find_sunset() still reports
 *   HIJRI_EVENT_OK all year). Near the circle the Sun's altitude crosses
 *   the horizon at a grazing angle, so dt/dAltitude is large there and the
 *   ~1 arcsecond convention gap from PAIRING THE CONVENTIONS above (plus
 *   refine_event()'s single-iteration truncation) is amplified into tens of
 *   seconds of TIME difference even though both solvers agree on the
 *   ANGLE to well under an arcminute. Measured at lat=-66, lon=0,
 *   2025-12-12: 123.6495 s, which alone would exceed the 60-second halt
 *   threshold. That is a property of comparing two solvers in the time
 *   domain at grazing incidence, not the astronomy defect this oracle
 *   exists to catch, so the grid is capped at +-60 deg, comfortably inside
 *   the circle, where every point still solves and the amplification is
 *   negligible (see MEASURED BOUND below).
 *
 *   Longitudes: -120, 0, 120 degrees (3 points) -- spread east and west so
 *   the jd_local_midnight_ut construction is exercised on both sides of the
 *   prime meridian, not just at it.
 *
 *   Dates: every day of 2025-01-01 through 2025-12-31 (365 points, a
 *   non-leap year) -- a full seasonal cycle, so the declination swing that
 *   drives the equation-of-time defect this repository already fixed is
 *   covered at every latitude in the grid.
 *
 *   Total grid points: 13 * 3 * 365 = 14235.
 *
 * VACUOUS-PASS GUARD (Step 3)
 *
 *   An oracle that silently skips every point where either solver fails
 *   would pass even if both solvers were broken. usable_points counts grid
 *   points where BOTH calculate_prayer_times() produced a finite maghrib
 *   AND hijri_find_sunset() returned HIJRI_EVENT_OK, and the count is asserted
 *   against USABLE_FLOOR_FRACTION * total grid points, not skipped. Inside
 *   +-60 deg neither solver ever legitimately fails (measured: 14235 of
 *   14235 usable), so USABLE_FLOOR_FRACTION is pinned high, at 0.95, and
 *   still leaves room for a platform whose libm disagrees at a handful of
 *   points without that alone tripping the guard. See the Step 8 mutation
 *   record below for the observed FAIL line this guard produces when
 *   nothing is solvable.
 *
 * MEASURED BOUND (Steps 4-5)
 *
 *   With the tolerance set arbitrarily wide (1.0e9 s) to measure rather than
 *   guess, this oracle's maximum observed |difference| over the full 14235-
 *   point, +-60 deg grid was 6.5966 s, at latitude -60.0, longitude -120.0,
 *   2025-12-17 (see the printed "prayertimes_oracle max diff" line for the
 *   exact date each run, since the day of maximum disagreement can shift a little
 *   with libm). Well under the 60-second halt threshold and under the
 *   123.6495 s measured just outside the grid at +-66 deg (GRID above).
 *   TOL_SUNSET_S is pinned at 15.0 s, leaving about 2.3x margin over the
 *   measured maximum.
 *
 * MUTATION RECORD, Step 7 -- reverting the Task 1 refinement
 *
 *   prayertimes.h's sunset was temporarily reverted from
 *
 *     sunset = refine_event(jd, latitude, longitude, timezone,
 *                           REFRACTION_CORRECTION, +1.0, sunset);
 *
 *   back to the unrefined value (the refine_event() call deleted, sunset
 *   left as the plain hour-angle result). `make test` then produced,
 *   verbatim (the last 20 lines of the failing binary's output, which is
 *   all `make test` prints):
 *
 *         FAIL prayertimes_oracle_sunset_agrees_s at jd=2461005.2: got 27.8152 want 0.0000 (err 27.8152 > tol 15.0000)
 *         FAIL prayertimes_oracle_sunset_agrees_s at jd=2461006.2: got 26.5628 want 0.0000 (err 26.5628 > tol 15.0000)
 *         FAIL prayertimes_oracle_sunset_agrees_s at jd=2461007.2: got 25.2867 want 0.0000 (err 25.2867 > tol 15.0000)
 *         FAIL prayertimes_oracle_sunset_agrees_s at jd=2461008.2: got 23.9867 want 0.0000 (err 23.9867 > tol 15.0000)
 *         FAIL prayertimes_oracle_sunset_agrees_s at jd=2461009.2: got 22.6626 want 0.0000 (err 22.6626 > tol 15.0000)
 *         FAIL prayertimes_oracle_sunset_agrees_s at jd=2461010.2: got 21.3144 want 0.0000 (err 21.3144 > tol 15.0000)
 *         FAIL prayertimes_oracle_sunset_agrees_s at jd=2461011.2: got 19.9422 want 0.0000 (err 19.9422 > tol 15.0000)
 *         FAIL prayertimes_oracle_sunset_agrees_s at jd=2461012.2: got 18.5461 want 0.0000 (err 18.5461 > tol 15.0000)
 *         FAIL prayertimes_oracle_sunset_agrees_s at jd=2461013.2: got 17.1264 want 0.0000 (err 17.1264 > tol 15.0000)
 *         FAIL prayertimes_oracle_sunset_agrees_s at jd=2461014.2: got 15.6835 want 0.0000 (err 15.6835 > tol 15.0000)
 *         FAIL prayertimes_oracle_sunset_agrees_s at jd=2461034.2: got 16.3226 want 0.0000 (err 16.3226 > tol 15.0000)
 *         FAIL prayertimes_oracle_sunset_agrees_s at jd=2461035.2: got 17.9382 want 0.0000 (err 17.9382 > tol 15.0000)
 *         FAIL prayertimes_oracle_sunset_agrees_s at jd=2461036.2: got 19.5384 want 0.0000 (err 19.5384 > tol 15.0000)
 *         FAIL prayertimes_oracle_sunset_agrees_s at jd=2461037.2: got 21.1210 want 0.0000 (err 21.1210 > tol 15.0000)
 *         FAIL prayertimes_oracle_sunset_agrees_s at jd=2461038.2: got 22.6834 want 0.0000 (err 22.6834 > tol 15.0000)
 *         FAIL prayertimes_oracle_sunset_agrees_s at jd=2461039.2: got 24.2236 want 0.0000 (err 24.2236 > tol 15.0000)
 *         FAIL prayertimes_oracle_sunset_agrees_s at jd=2461040.2: got 25.7396 want 0.0000 (err 25.7396 > tol 15.0000)
 *         prayertimes_oracle usable points = 14235 of 14235 (floor 13523)
 *         prayertimes_oracle max diff = 203.6417 s at lat=60.0 lon=-120.0 2025-09-03
 *         prayertimes_oracle checks: 14236 checks, 9933 failures
 *
 *   9933 of 14235 checks failed with the refinement gone, at up to 203.6417
 *   seconds of disagreement -- the class of defect Task 1 fixed, caught here
 *   for the first time by a solver this repository had never compared
 *   prayertimes.h's sunset against. prayertimes.h was then restored exactly
 *   (confirmed with `git diff --exit-code prayertimes.h`).
 *
 * MUTATION RECORD, Step 8 -- proving the vacuous-pass guard fires
 *
 *   With the loop body's usability test temporarily forced to
 *   `if (0 && pt_ok && hijri_ok)` (both solvers treated as having failed on
 *   every grid point, so no per-point check ever runs), `make test`
 *   produced, verbatim:
 *
 *         prayertimes_oracle usable points = 0 of 14235 (floor 13523)
 *         FAIL prayertimes_oracle_usable_floor: expected a non-zero value, got -13523.000000000
 *         prayertimes_oracle max diff = 0.0000 s at lat=0.0 lon=0.0 0000-00-00
 *         prayertimes_oracle checks: 1 checks, 1 failures
 *
 *   The forced override was then removed, restoring the file to what is
 *   committed here.
 */

#define PRAYERTIMES_IMPLEMENTATION
#include "../prayertimes.h"

#define HIJRI_IMPLEMENTATION
#include "../hijri.h"

#include <math.h>
#include <stdio.h>

static int checks;
static int failures;

/* Pinned from the Step 4/5 measurement in the file header: max observed
 * 6.5966 s over the 14235-point grid, ~2.3x margin. To re-measure, set this
 * to something absurd (e.g. 1.0e9) and re-run before ever moving this. */
#define TOL_SUNSET_S 15.0

/* Fraction of the 14235-point grid that must yield a usable comparison from
 * BOTH solvers. Measured at 14235 of 14235 (100%) inside +-60 deg, see the
 * file header's VACUOUS-PASS GUARD note. 0.95 stays a hard floor while
 * leaving room for a platform whose libm disagrees at a handful of points. */
#define USABLE_FLOOR_FRACTION 0.95

static const double LATS[] = {-60.0, -50.0, -40.0, -30.0, -20.0, -10.0,
                              0.0,   10.0,  20.0,  30.0,  40.0,  50.0,
                              60.0};
#define N_LATS ((int)(sizeof(LATS) / sizeof(LATS[0])))

static const double LONS[] = {-120.0, 0.0, 120.0};
#define N_LONS ((int)(sizeof(LONS) / sizeof(LONS[0])))

/* 2025: non-leap, 365 days, one full seasonal cycle. */
static const int DAYS_IN_MONTH[12] = {31, 28, 31, 30, 31, 30,
                                      31, 31, 30, 31, 30, 31};

static void check_within(const char *name, double jd, double got,
                         double want, double tol) {
  double err = fabs(got - want);
  checks++;
  if (!(err <= tol)) {
    failures++;
    printf("FAIL %s at jd=%.1f: got %.4f want %.4f (err %.4f > tol %.4f)\n",
           name, jd, got, want, err, tol);
  }
}

static void check_true_nonzero(const char *name, double value) {
  checks++;
  if (!(value > 0.0)) {
    failures++;
    printf("FAIL %s: expected a non-zero value, got %.9f\n", name, value);
  }
}

/* ---------------------------------------------------------------------------
 * POLAR EXTENSION, ANGLE DOMAIN
 *
 * The grid above stops at +-60 deg because it compares two solvers in the TIME
 * domain, and near the polar circle the Sun crosses the horizon so shallowly
 * that a sub-arcminute angle disagreement becomes tens of seconds. That is a
 * property of the comparison, not of the astronomy, and it left everything
 * above 60 deg unvalidated even though that is exactly where this library's
 * high-latitude behaviour lives.
 *
 * This check asks the question the other way round and does not convert an
 * angle back into a time. It takes the instant prayertimes.h reports for
 * maghrib, asks the DE440-validated solver where the Sun is at that instant,
 * and compares that against where the same solver puts the Sun at its own
 * sunset. Grazing incidence does not amplify anything here: both quantities
 * are altitudes, and no refraction or semidiameter constant enters, because
 * the two are differenced against each other rather than against a target.
 *
 * MEASURED BOUND. |lat| in {62, 66, 70, 75, 78}, longitudes -120, 0, 120,
 * every day of 2025, both hemispheres. 7278 solved points, mean 0.1632
 * arcmin, max 0.4680 arcmin at lat -62.0 lon -120.0 on 2025-12-13. Not one
 * solved point exceeded 1 arcmin. TOL_POLAR_ARCMIN is pinned at 1.5, a
 * margin of about 3.2x.
 *
 * GRAZING, SEPARATED. 172 points sit within GRAZING_MARGIN_DEG of the
 * altitude that defines sunset, meaning the Sun's whole daily arc passes
 * within half a degree of it. There the crossing instant is extremely
 * sensitive and the two solvers land far apart in time while both stay
 * correct in angle. Max 9.3160 arcmin at lat 75.0 lon 0.0 on 2025-11-05,
 * where the Sun peaks at -0.6837 deg, 0.1493 deg above the -0.833 deg that
 * defines sunset. TOL_POLAR_GRAZING_ARCMIN is pinned at 15.0.
 *
 * These points were not visible when this check was written. hijri.h's
 * finder scanned hourly and stepped straight over such short excursions, so
 * the day returned no sunset and the comparison skipped it. Issue #82 gave
 * the finder a four minute rescan, which found them, which is what surfaced
 * this band. The count moved from 7448 comparable to 7278 solved plus 172
 * grazing.
 *
 * EXCLUDED, AND WHY. On the first and last day of the polar period the two
 * solvers are not describing the same event. prayertimes.h substitutes from
 * its reference latitude as soon as its own sunrise hour angle stops solving,
 * while hijri_find_sunset can still find a grazing sunset minutes before
 * midnight. At lat 66 on 2025-06-29 that is a 4.1 hour difference between two
 * individually correct answers. Those days are skipped by testing the same
 * condition prayertimes.h itself branches on, rather than by widening the
 * tolerance until they pass. 30 of 7478 points are excluded this way.
 *
 * Latitudes beyond 78 are deliberately not covered. At 88 deg the seasonal
 * boundary lasts longer than a day and the two solvers pick different
 * crossings, which produced a 13.63 arcmin outlier at lat -88 on 2025-03-27.
 * That is the same class of disagreement as the excluded boundary days rather
 * than a new one, and no inhabited location sits there.
 * ------------------------------------------------------------------------ */
/* How far the Sun's arc must clear the altitude a check is about before
   that check treats the crossing as decisively solved. Shared by the polar
   and twilight grids, which meet the same band at different altitudes. */
#define GRAZING_MARGIN_DEG 0.5

#define TOL_POLAR_ARCMIN 1.5
#define TOL_POLAR_GRAZING_ARCMIN 15.0

static void check_polar_angle_agreement(const MethodParams *params,
                                        double iht_hours) {
  static const double POLAR_LATS[] = {-78.0, -75.0, -70.0, -66.0, -62.0,
                                      62.0,  66.0,  70.0,  75.0,  78.0};
  static const double POLAR_LONS[] = {-120.0, 0.0, 120.0};
  long comparable = 0, excluded = 0, grazing = 0;
  double sum = 0.0, max_arcmin = 0.0, max_grazing = 0.0;
  double worst_lat = 0.0, worst_lon = 0.0;
  double graze_lat = 0.0, graze_lon = 0.0;
  int worst_m = 0, worst_d = 0, graze_m = 0, graze_d = 0;

  for (size_t i = 0; i < sizeof POLAR_LATS / sizeof *POLAR_LATS; i++) {
    for (size_t j = 0; j < sizeof POLAR_LONS / sizeof *POLAR_LONS; j++) {
      double lat = POLAR_LATS[i], lon = POLAR_LONS[j];
      double timezone = lon / 15.0; /* mean solar time, as above */
      HijriLocation loc = {lat, lon, 0.0, "polar oracle"};

      for (int month = 1; month <= 12; month++) {
        for (int day = 1; day <= DAYS_IN_MONTH[month - 1]; day++) {
          struct PrayerTimes pt = calculate_prayer_times(
              2025, month, day, lat, lon, timezone, params);
          double maghrib_raw = pt.maghrib - iht_hours;
          if (!isfinite(maghrib_raw)) continue;

          double jd_midnight =
              hijri_jd_from_gregorian(2025, month, (double)day) - lon / 360.0;
          double jd_sunset;
          if (hijri_find_sunset(jd_midnight, &loc,
                                &HIJRI_SUNSET_CONVENTION_ASTRONOMICAL,
                                &jd_sunset) != HIJRI_EVENT_OK) {
            continue;
          }

          /* The boundary-day exclusion, testing the same condition
             prayertimes.h branches on rather than a tolerance. */
          double decl, eqt;
          sun_position(julian_day(2025, month, day), &decl, &eqt);
          if (isnan(hour_angle(lat, decl, REFRACTION_CORRECTION))) {
            excluded++;
            continue;
          }

          double alt_pt =
              hijri_sun_altitude(jd_midnight + maghrib_raw / 24.0, &loc);
          double alt_hijri = hijri_sun_altitude(jd_sunset, &loc);
          double arcmin = fabs(alt_pt - alt_hijri) * 60.0;

          /* How decisively the Sun crosses the altitude that defines
             sunset. On a day where its whole arc sits within a fraction of
             a degree of that altitude, the crossing instant is extremely
             sensitive and the two solvers can land far apart in time while
             both remain correct. That is the same grazing band the twilight
             check below separates, reaching sunset rather than a twilight
             angle, and it is separated here for the same reason. */
          double event_alt = -REFRACTION_CORRECTION;
          double max_alt = 90.0 - fabs(lat - decl);
          double min_alt = fabs(lat + decl) - 90.0;
          double clearance = max_alt - event_alt;
          if (event_alt - min_alt < clearance)
            clearance = event_alt - min_alt;

          if (clearance < GRAZING_MARGIN_DEG) {
            grazing++;
            if (arcmin > max_grazing) {
              max_grazing = arcmin;
              graze_lat = lat;
              graze_lon = lon;
              graze_m = month;
              graze_d = day;
            }
            check_within("prayertimes_oracle_polar_grazing_arcmin",
                         jd_midnight, arcmin, 0.0, TOL_POLAR_GRAZING_ARCMIN);
            continue;
          }

          comparable++;
          sum += arcmin;
          if (arcmin > max_arcmin) {
            max_arcmin = arcmin;
            worst_lat = lat;
            worst_lon = lon;
            worst_m = month;
            worst_d = day;
          }
          check_within("prayertimes_oracle_polar_angle_arcmin", jd_midnight,
                       arcmin, 0.0, TOL_POLAR_ARCMIN);
        }
      }
    }
  }

  printf("prayertimes_oracle polar: %ld comparable, %ld boundary excluded, "
         "%ld grazing\n",
         comparable, excluded, grazing);
  printf("prayertimes_oracle polar grazing max = %.4f arcmin at lat=%.1f "
         "lon=%.1f 2025-%02d-%02d\n",
         max_grazing, graze_lat, graze_lon, graze_m, graze_d);
  printf("prayertimes_oracle polar max = %.4f arcmin at lat=%.1f lon=%.1f "
         "2025-%02d-%02d (mean %.4f)\n",
         max_arcmin, worst_lat, worst_lon, worst_m, worst_d,
         comparable ? sum / (double)comparable : 0.0);

  /* A vacuous pass guard of the same kind the seconds grid uses: if the grid
     ever stops producing comparable points, the check above passes silently. */
  check_true_nonzero("prayertimes_oracle_polar_nonvacuous",
                     (double)(comparable - 7000));
  /* The grazing branch needs its own guard for the same reason: if the band
     ever empties, its tolerance stops being exercised and its check passes
     without comparing anything. */
  check_true_nonzero("prayertimes_oracle_polar_grazing_nonvacuous",
                     (double)(grazing - 100));
}

/* ---------------------------------------------------------------------------
 * TWILIGHT ORACLE, ANGLE DOMAIN
 *
 * Closes #52. Sunset has been validated against hijri.h since this file was
 * written, but fajr and isha had no oracle of any kind, and #49 moved isha by
 * up to 16 minutes at Stockholm on the strength of a sunset measurement that
 * does not transfer to them.
 *
 * hijri.h exposes no twilight solver, which is why #52 records this as needing
 * either a published schedule or a new implementation. It turns out neither is
 * needed. hijri_sun_altitude() returns a geometric altitude with no refraction
 * term, which is exactly how prayertimes.h defines fajr and isha, so the two
 * are directly comparable: at the instant this header reports for fajr, ask
 * the DE440-validated solver where the Sun is, and compare against the
 * depression angle the event is defined by.
 *
 * Days where the fallback supplied the value are excluded, because on those
 * the Sun never reaches the angle and there is no crossing to check. So are
 * polar days, where the whole schedule is solved at the reference latitude
 * rather than at the location.
 *
 * TWO POPULATIONS, BOTH PINNED. The remainder splits cleanly by how far the
 * Sun goes past the required depression.
 *
 *   comfortably solved, deepest >= angle + 0.5 deg
 *     17676 points, mean 0.0426 arcmin, max 0.2312 arcmin. Pinned at 1.0, a
 *     margin of about 4.3x. Both figures improved when refine_event was
 *     iterated to convergence rather than taking a single step, from a mean
 *     of 0.0996 and a max of 0.7095.
 *
 *   grazing band, deepest within 0.5 deg of the angle
 *     186 points, mean 2.2351 arcmin, max 30.1964 arcmin at lat 70.0 on
 *     2025-03-27, roughly 6 minutes of time. Pinned at 45.0. Unchanged to
 *     the last digit by iterating refine_event, because the limit there is
 *     not convergence, see below.
 *
 * The grazing band is asserted rather than excluded, and it is not a
 * convergence failure. At lat 70 on 2025-03-27 the Sun clears 17 degrees by
 * 0.39, one step moves the estimate 38 minutes, and at that new instant the
 * declination has shifted just enough that the Sun no longer reaches 17
 * degrees at all. hour_angle returns NaN and the previous value stands.
 * Iterating changes this band by nothing at all, which is measured rather
 * than assumed. The limit is a nearly tangent crossing, not an early stop. That is a real accuracy limit of this
 * header and hiding it behind an exclusion would misrepresent the coverage.
 *
 * This also explains the observation that opened #52. Stockholm on 2026-08-17
 * is a grazing day, the one where the fallback engages, so the 16 minute
 * movement #49 introduced is this band rather than an unexplained defect. It
 * is now measured instead of merely suspected, which is what that issue asked
 * for.
 *
 * Grid: |lat| in {70, 66, 60, 50, 30, 0}, both hemispheres, longitudes -120,
 * 0 and 120, every day of 2025.
 * ------------------------------------------------------------------------ */
#define TOL_TWILIGHT_ARCMIN 1.0
#define TOL_TWILIGHT_GRAZING_ARCMIN 45.0

static void check_twilight_angle_agreement(const MethodParams *params,
                                           double iht_hours) {
  static const double TW_LATS[] = {-70.0, -66.0, -60.0, -50.0, -30.0, 0.0,
                                   30.0,  50.0,  60.0,  66.0,  70.0};
  static const double TW_LONS[] = {-120.0, 0.0, 120.0};
  long solved = 0, grazing = 0;
  double max_solved = 0.0, max_grazing = 0.0;

  for (size_t i = 0; i < sizeof TW_LATS / sizeof *TW_LATS; i++) {
    for (size_t j = 0; j < sizeof TW_LONS / sizeof *TW_LONS; j++) {
      double lat = TW_LATS[i], lon = TW_LONS[j], timezone = lon / 15.0;
      HijriLocation loc = {lat, lon, 0.0, "twilight oracle"};

      for (int month = 1; month <= 12; month++) {
        for (int day = 1; day <= DAYS_IN_MONTH[month - 1]; day++) {
          struct PrayerTimes pt = calculate_prayer_times(
              2025, month, day, lat, lon, timezone, params);
          double decl, eqt;
          sun_position(julian_day(2025, month, day), &decl, &eqt);

          /* Polar days are solved at the reference latitude, not this one. */
          if (isnan(hour_angle(lat, decl, REFRACTION_CORRECTION))) continue;

          double jd_midnight =
              hijri_jd_from_gregorian(2025, month, (double)day) - lon / 360.0;
          double deepest = 90.0 - fabs(lat + decl);

          struct {
            double hours, angle;
          } events[2] = {{pt.fajr - iht_hours, params->fajr_angle},
                         {pt.isha - iht_hours, params->isha_angle}};

          for (int k = 0; k < 2; k++) {
            bool failed = false;
            hour_angle_safe(lat, decl, events[k].angle, &failed);
            if (failed || !isfinite(events[k].hours)) continue;

            double alt =
                hijri_sun_altitude(jd_midnight + events[k].hours / 24.0, &loc);
            double arcmin = fabs(alt + events[k].angle) * 60.0;

            if (deepest >= events[k].angle + GRAZING_MARGIN_DEG) {
              solved++;
              if (arcmin > max_solved) max_solved = arcmin;
              check_within("prayertimes_oracle_twilight_arcmin", jd_midnight,
                           arcmin, 0.0, TOL_TWILIGHT_ARCMIN);
            } else {
              grazing++;
              if (arcmin > max_grazing) max_grazing = arcmin;
              check_within("prayertimes_oracle_twilight_grazing_arcmin",
                           jd_midnight, arcmin, 0.0,
                           TOL_TWILIGHT_GRAZING_ARCMIN);
            }
          }
        }
      }
    }
  }

  printf("prayertimes_oracle twilight: %ld solved (max %.4f arcmin), "
         "%ld grazing (max %.4f arcmin)\n",
         solved, max_solved, grazing, max_grazing);

  /* Vacuous-pass guards. Both populations must keep producing points, or the
     assertions above would pass by never running. */
  check_true_nonzero("prayertimes_oracle_twilight_solved_nonvacuous",
                     (double)(solved - 17000));
  check_true_nonzero("prayertimes_oracle_twilight_grazing_nonvacuous",
                     (double)(grazing - 150));
}

int main(void) {
  const MethodParams *params = method_params_get(CALC_MWL);
  double iht_hours = (double)params->ihtiyat / 60.0;

  long total_points = 0;
  long usable_points = 0;
  double max_diff_s = 0.0;
  double max_lat = 0.0, max_lon = 0.0;
  int max_y = 0, max_m = 0, max_d = 0;

  for (int li = 0; li < N_LATS; li++) {
    double lat = LATS[li];
    for (int oi = 0; oi < N_LONS; oi++) {
      double lon = LONS[oi];
      double timezone = lon / 15.0; /* mean solar time, see header comment */

      HijriLocation loc;
      loc.latitude_deg = lat;
      loc.longitude_deg = lon;
      loc.elevation_m = 0.0;
      loc.name = NULL;

      int year = 2025;
      for (int month = 1; month <= 12; month++) {
        for (int day = 1; day <= DAYS_IN_MONTH[month - 1]; day++) {
          total_points++;

          struct PrayerTimes pt = calculate_prayer_times(
              year, month, day, lat, lon, timezone, params);
          double maghrib_raw = pt.maghrib - iht_hours;
          int pt_ok = isfinite(maghrib_raw) ? 1 : 0;

          double jd_midnight =
              hijri_jd_from_gregorian(year, month, (double)day) -
              lon / 360.0;
          double jd_sunset;
          HijriEventStatus status = hijri_find_sunset(
              jd_midnight, &loc, &HIJRI_SUNSET_CONVENTION_ASTRONOMICAL,
              &jd_sunset);
          int hijri_ok = (status == HIJRI_EVENT_OK) ? 1 : 0;

          if (pt_ok && hijri_ok) {
            usable_points++;

            double hijri_hours = (jd_sunset - jd_midnight) * 24.0;
            double diff_s = fabs(hijri_hours - maghrib_raw) * 3600.0;

            if (diff_s > max_diff_s) {
              max_diff_s = diff_s;
              max_lat = lat;
              max_lon = lon;
              max_y = year;
              max_m = month;
              max_d = day;
            }

            /* diff_s is already in seconds; comparing it against 0.0 keeps
             * check_within's unconverted got/want in the same unit as
             * TOL_SUNSET_S, rather than comparing an hours-scale difference
             * to a seconds-scale tolerance. */
            check_within("prayertimes_oracle_sunset_agrees_s", jd_midnight,
                         diff_s, 0.0, TOL_SUNSET_S);
          }
        }
      }
    }
  }

  long floor_points = (long)(USABLE_FLOOR_FRACTION * (double)total_points);
  printf("prayertimes_oracle usable points = %ld of %ld (floor %ld)\n",
         usable_points, total_points, floor_points);
  check_true_nonzero("prayertimes_oracle_usable_floor",
                     (double)(usable_points - floor_points));

  printf("prayertimes_oracle max diff = %.4f s at lat=%.1f lon=%.1f "
         "%04d-%02d-%02d\n",
         max_diff_s, max_lat, max_lon, max_y, max_m, max_d);

  check_polar_angle_agreement(params, iht_hours);
  check_twilight_angle_agreement(params, iht_hours);

  printf("prayertimes_oracle checks: %d checks, %d failures\n", checks,
         failures);
  return failures == 0 ? 0 : 1;
}

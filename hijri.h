/*
 * MIT License
 *
 * Copyright (c) 2025-2026 muslimtify-org
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* hijri.h -- v1.0.0-rc1 -- stb-style single-file astronomical Hijri calendar library
 *
 * Do this:
 *      #define HIJRI_IMPLEMENTATION
 * before you include this file in *one* C or C++ file to create the
 * implementation.
 *
 *   // i.e. it should look like this in exactly one source file:
 *   #define HIJRI_IMPLEMENTATION
 *   #include "hijri.h"
 *
 * In every other file that needs the API, just:
 *   #include "hijri.h"
 *
 * Optionally #define HIJRI_STATIC before including to make all functions
 * `static` instead of externally linked (useful if you're dropping this
 * into a single-TU project and want to avoid any symbol export at all).
 *
 * -----------------------------------------------------------------------
 * WHAT THIS IS
 *
 * A from-scratch astronomical Hijri (Islamic lunar) calendar library:
 * Julian Day handling, low/medium-precision Sun & Moon ephemerides,
 * sunset/moonset/conjunction solvers, explicit local evening parameters,
 * documented MABIMS and Wujudul Hilal local predicates, neutral research
 * predicates, dedicated Yallop (1997) and Odeh (2004) visibility models,
 * and an opt-in Mecca-based Umm al-Qura policy. Local predicate results are
 * not, by themselves, complete national or global authority decisions. A
 * non-astronomical fixed-cycle tabular ("Kuwaiti algorithm") calendar is
 * included too, as a fast dependency-free fallback.
 *
 * -----------------------------------------------------------------------
 * ACCURACY CAVEAT -- PLEASE READ
 *
 * hijri_moon_position() implements the full Meeus ch. 47 lunar series --
 * Table 47.A (60 terms, longitude and distance) and Table 47.B (60 terms,
 * latitude), with the E eccentricity factor and the A1/A2/A3 additive
 * corrections.
 *
 * Measured against 24 JPL Horizons epochs spanning 1900-2100, worst case:
 *
 *     longitude  0.0051 deg      latitude  0.0006 deg      distance  41.9 km
 *
 * It also reproduces Meeus's own printed worked Example 47.a to every digit
 * the book gives. See tests/test_ephemeris_oracle.c, which carries both checks.
 *
 * WHAT THIS STILL DOES NOT DO, AND WHERE THE TWO BODIES DIFFER.
 *
 * The LUNAR position applies neither nutation nor aberration, so
 * hijri_moon_position() returns geometric coordinates referred to the mean
 * equinox of date. The 0.0051 deg residual above is mostly that omitted
 * nutation rather than series error, and this is now measured rather than
 * inferred: against a mean-of-date DE440 reference the Meeus ch. 47
 * truncation error is 0.0012755 deg, about 4x smaller.
 *
 * The SOLAR position is NOT the same, and an earlier version of this comment
 * wrongly claimed it was. Meeus ch. 25 applies both nutation and aberration
 * (see hijri_sun_position below), so apparent_longitude_deg is referred to the
 * TRUE equinox of date. Measured against DE440 its error is 0.0084042 deg,
 * within the ~0.01 deg the theory documents, but larger than the lunar
 * truncation error -- the Sun is the dominant term in this library's error
 * bar, not the Moon.
 *
 * A consequence worth knowing before "fixing" it: elongation is computed
 * between an apparent Sun and a mean-of-date Moon, which are different
 * frames. The shipped geocentric elongation path (the RA/Dec angular
 * separation the criteria run) measures 0.0065798 deg against DE440, 972x
 * below the 6.4 deg MABIMS 2021 threshold. The topocentric value is now
 * oracle-measured too, against a topocentric Moon and a geocentric Sun paired
 * exactly as this file pairs them: 0.0069925 deg at worst (Jakarta), 21.91 to
 * 25.17 arcsec across the four measured sites. That sits alongside the
 * geocentric 0.0065798 deg, so the parallax correction adds very little error
 * of its own, and both stay about three orders of magnitude below the 6.4 deg
 * threshold. The geocentric-Sun convention the Pedoman deliberately keeps
 * costs 7.33 to 8.66 arcsec of omitted solar parallax on its own, an
 * oracle-internal quantity that involves no library code. Making the frames
 * consistent measures WORSE, 0.0083813 deg in longitude difference, because
 * the mismatch partially cancels the solar truncation error. The real limit
 * is the ch. 25 solar theory, not the frame.
 * See docs/research/2026-08-02-cross-engine-error-bar.md and
 * docs/research/2026-08-07-topocentric-error-bar.md.
 *
 * Judgement is still required near a criterion boundary. This ephemeris is
 * far tighter than the thresholds the visibility criteria in this file use,
 * but a calculated result is not an observation, and no calculation here
 * decides religious validity.
 *
 * See the explicitly documented local predicates below.
 *
 * TWO SIDEREAL TIMES, DELIBERATELY. Solar hour angles use apparent sidereal
 * time (hijri__gast_deg) and lunar ones use mean sidereal time
 * (hijri__gmst_deg). That is not an inconsistency to be tidied up: the Sun's
 * right ascension here is apparent, referred to the true equinox, and the
 * Moon's is mean-of-date, so each is paired with the sidereal time referred to
 * the same equinox it is. Collapsing them to one clock reintroduces an error
 * equal to the equation of the equinoxes, up to about 16 arcsec of hour angle
 * or 1.05 s of sunset. Measured before the fix: see issue #29 and
 * docs/research/2026-08-05-solar-hour-angle-frame.md.
 *
 * Do not read that 1.05 s as the library's sunset accuracy. Sidereal time is a
 * function of UT1, and these entry points take a Julian date callers will in
 * practice populate from UTC. UT1 minus UTC ranges over plus or minus 0.9 s by
 * construction, the same order as the entire correction above, and no table
 * this library carries can remove it. Correcting the frame removes a
 * deterministic bias, it does not buy sub-second sunset precision.
 *
 * THE SUNSET HORIZON IS A PER-CRITERION VALUE, NOT A PAIR OF MACROS. Two
 * constants define it, the refraction at the horizon and the Sun's
 * semidiameter, and both official methods this library reproduces disagree
 * with the textbook astronomical figures on both. They now travel together in
 * a HijriSunsetConvention, which hijri_find_sunset(), hijri_find_moonset() and
 * hijri_compute_evening_parameters() all take by pointer and which
 * hijri_evaluate_evening() selects once from the predicate. Both pedoman use
 * 34' 30" of refraction, the research predicates and the Umm al-Qura fallback
 * use 34' 00.12" because they claim no authority. The semidiameter is no
 * longer fixed: it is stored at unit distance and divided by the Sun's
 * distance at the sampled instant, so the crossing target varies over the year
 * the way the books' tables do. The 959.63 arcsec figure is not stated by
 * either book, and it is confirmed here as consistent with every semidiameter
 * both books publish, to within 0.05 arcsec. See
 * docs/research/2026-08-14-sunset-constants.md for that derivation, the
 * unchanged calendar fixtures, and why a 0.5 arcmin refraction change delivers
 * under 0.25 arcmin to the Moon's limb.
 *
 * SOLAR PARALLAX IS OMITTED FROM SUNSET, ON PURPOSE. hijri_find_sunset() uses
 * a geocentric Sun. The roughly 8.8 arcsec of solar horizontal parallax would
 * move sunset by up to about 1 s. It is left out because the official
 * conventions this library exists to reproduce leave it out: the Pedoman Hisab
 * Muhammadiyah computes sunset as h = -(s.d. + R' + Dip), with no parallax
 * term (docs/research/2026-08-01-wujudul-hilal-convention.md). Adding it would
 * move the library AWAY from the published calendars it is validated against.
 *
 * HORIZON DIP IS OMITTED FROM THE SET SOLVERS, ALSO ON PURPOSE. loc->elevation_m
 * does not lower the sunset target. Do not "fix" this. It has been measured
 * twice and both times the correction was the error. For Wujudul Hilal the dip
 * cancels, because the Pedoman applies it to sunset AND to the altitude, so
 * adding it on one side only lands 17 arcmin out (see
 * docs/research/2026-08-01-wujudul-hilal-convention.md). For MABIMS 2021 it
 * does not cancel, but applying it makes agreement with the official Kemenag
 * calendar WORSE, 33 of 37 official month starts supported dropping to 32,
 * which fails the committed fixture floor at tests/test_hijri.c. Dip delays
 * sunset, so the Moon is lower, so a threshold read from below can only lose
 * months. Measured 2026-08-05, see
 * docs/research/2026-08-05-solar-hour-angle-frame.md.
 *
 * -----------------------------------------------------------------------
 * THREAD SAFETY
 *
 * Every public function in this file is a pure function of its arguments.
 * None retains state between calls, and none touches a mutable object at
 * file scope, so calls from any number of threads at once are safe, with no
 * locking required by the caller. This rests on a sweep run for this claim,
 * not on an impression from reading: `grep -nE '^[[:space:]]*static'
 * hijri.h prayertimes.h timezone.h | grep -v 'static const'` was run over
 * all three headers this repository ships, and every line it reported names
 * a function definition or declaration, none names a mutable object. See
 * issue #26, where this contract started as an impression rather than a
 * checked claim.
 *
 * -----------------------------------------------------------------------
 * DETERMINISM
 *
 * These are three separate claims, not one.
 *
 * Same platform, same compiler, same flags gives bit-identical results for
 * hijri.h. This is enforced on every run by the byte-exact comparison the
 * `make baseline` target runs against the committed
 * docs/research/hijri-2020-2025-baseline.csv.
 *
 * Across libm implementations on the same architecture, measured between
 * glibc 2.44 and musl 1.2.6 on x86-64, both at -O2, built from the same
 * source with gcc and musl-gcc: the full 132-row
 * docs/research/hijri-2020-2025-baseline.csv is byte-identical between the
 * two, and a wider sweep of raw double bit patterns (336 rows, 4 sites, 12
 * months a year, 2020 to 2026) found no Julian Day instant differed. Angular
 * quantities (moon altitude, elongation) differed on some rows, by 1 to 2
 * units in the last place, worst case 1.203e-13 deg at Jakarta in November
 * 2020, which is 4.0e-14 of the 3 degree MABIMS 2021 altitude threshold. Age
 * and lag are arithmetic on Julian Days, so with no Julian Day differing,
 * neither did they.
 *
 * This does not show the library is reproducible across platforms in
 * general. It shows two implementations agreeing on one architecture. macOS,
 * BSD, Windows, non-x86-64 architectures, and other optimisation levels are
 * not measured and no claim is made about them. The residual risk that
 * matters even within what was measured: a one-unit-in-the-last-place
 * angular difference could in principle land next to a rounding boundary
 * and move a printed digit at a date not among the 336 sampled here.
 *
 * This determinism contract covers hijri.h only. prayertimes.h and
 * timezone.h are explicitly not audited for it.
 *
 * -----------------------------------------------------------------------
 * LICENSE
 *
 * MIT -- see the full license text at the top of this file. Provided as-is,
 * no warranty, use at your own risk -- especially for anything where getting
 * the date wrong matters religiously; see the accuracy caveat above.
 */

#ifndef HIJRI_H
#define HIJRI_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HIJRIDEF
#ifdef HIJRI_STATIC
#define HIJRIDEF static
#else
#define HIJRIDEF extern
#endif
#endif

/* ---- Common types ------------------------------------------------------ */

/* Observer location. Longitude positive EAST, latitude positive NORTH,
 * elevation in meters above sea level. */
typedef struct {
  double latitude_deg;
  double longitude_deg;
  double elevation_m;
  const char *name; /* optional, for logging/debugging */
} HijriLocation;

/* A calculated Hijri calendar date. */
typedef struct {
  int year;
  int month; /* 1 = Muharram ... 12 = Dhu al-Hijjah */
  int day;   /* 1..30 */
} HijriDate;

/* Convenience constant: Mecca, used by the Umm al-Qura criterion. */
static const HijriLocation HIJRI_LOCATION_MECCA = {21.4225, 39.8262, 240.0,
                                                   "Mecca"};

/* The two quantities that define the sunset horizon. They are separate
 * constants per criterion because they are separate published methods, not
 * because they all differ today.
 *
 * The semidiameter is stored at unit distance, in arcseconds, because both
 * pedoman supply it as per-date input data rather than as a constant. It is
 * divided by the Sun's distance in au at the sampled instant, so the crossing
 * target varies over the year the way the books' tables do. 959.63 arcsec is
 * the IAU value of the solar semidiameter at 1 au.
 *
 * conv is documented non-NULL everywhere it is taken, with no runtime check,
 * matching how loc is already treated throughout this file. */
typedef struct {
  double refraction_at_horizon_deg;
  double solar_semidiameter_arcsec_at_1au;
} HijriSunsetConvention;

/* Pedoman Hisab Muhammadiyah (Majelis Tarjih dan Tajdid, 2009), p. 57:
 * h = -(s.d. + R' + Dip), with R' = 34' 30" = 0.575 deg and s.d. supplied as
 * per-date input data. */
static const HijriSunsetConvention HIJRI_SUNSET_CONVENTION_MUHAMMADIYAH =
    {0.575, 959.63};

/* Ephemeris Hisab Rukyat 2023 (Kementerian Agama RI), step 14 of the worked
 * hilal calculation: h = -(SD + 00 34' 30" + Dip), with SD read from Kemenag's
 * own hourly solar table, which runs 15' 43.90" at aphelion to 16' 15.89" at
 * perihelion. */
static const HijriSunsetConvention HIJRI_SUNSET_CONVENTION_KEMENAG =
    {0.575, 959.63};

/* Standard astronomical horizon, 34' 00.12" of refraction. Claims no
 * authority, and is what the research predicates and the Umm al-Qura fallback
 * take, because they disclaim one. The semidiameter is the same physical
 * quantity either way, so it does not vary with the criterion. */
static const HijriSunsetConvention HIJRI_SUNSET_CONVENTION_ASTRONOMICAL =
    {0.5667, 959.63};

/* ---- Julian Day ---------------------------------------------------------
 * Algorithms: Meeus, "Astronomical Algorithms" 2nd ed., ch. 7. */

HIJRIDEF double hijri_jd_from_gregorian(int year, int month, double day);
HIJRIDEF void hijri_gregorian_from_jd(double jd, int *year, int *month,
                                      double *day_frac);
HIJRIDEF int hijri_jd_weekday(double jd); /* 0 = Sunday ... 6 = Saturday */
HIJRIDEF double
hijri_julian_centuries(double jd); /* Julian centuries since J2000.0 */

/* ---- Delta T (TT - UT) approximation ------------------------------------
 * Coarse but adequate: at most a few arcseconds of ephemeris error, far
 * smaller than the multi-degree thresholds every criterion here uses. */

HIJRIDEF double hijri_delta_t_seconds(double jd);
HIJRIDEF double hijri_jd_tt_from_ut(double jd_ut);

/* ---- Solar position ------------------------------------------------------
 * Meeus low-precision solar theory (ch. 25), accurate to ~0.01 degree. */

typedef struct {
  double apparent_longitude_deg; /* geocentric ecliptic longitude, apparent:
                                   * nutation and the -20.4"/R aberration term
                                   * are both applied, see the ACCURACY CAVEAT
                                   * above for what that costs and buys. */
  /* APPARENT, referred to the TRUE equinox of date, degrees. This is the
   * "SOLAR position is NOT the same" fact from the file header: this Sun is
   * apparent while HijriMoonPosition's RA/Dec below are mean of date. */
  double right_ascension_deg;
  double declination_deg; /* Same apparent, true-equinox-of-date frame as
                            * right_ascension_deg above. */
  double distance_au; /* Geocentric distance, astronomical units. */
  double obliquity_deg; /* true obliquity of the ecliptic, i.e. mean obliquity
                          * plus nutation in obliquity, degrees. */
} HijriSunPosition;

HIJRIDEF HijriSunPosition hijri_sun_position(double jd_tt);

/* ---- Lunar position -------------------------------------------------- */

typedef struct {
  double geocentric_longitude_deg; /* Ecliptic longitude, mean equinox of
                                     * date, no nutation or aberration. */
  double geocentric_latitude_deg; /* Ecliptic latitude, same mean-of-date
                                    * frame as geocentric_longitude_deg. */
  double distance_km; /* Geocentric distance, kilometres. */
  /* MEAN OF DATE, not apparent: derived from a mean-of-date eps0 with no
   * nutation term added, unlike HijriSunPosition's right_ascension_deg
   * above, which is apparent. See the ACCURACY CAVEAT and "WHAT THIS STILL
   * DOES NOT DO" sections in the file header for why. */
  double right_ascension_deg;
  double declination_deg; /* Same mean-of-date frame as right_ascension_deg
                            * above. */
  double horizontal_parallax_deg; /* Geocentric horizontal parallax, degrees,
                                    * from an Earth-radius/distance ratio,
                                    * hijri_moon_topocentric() consumes it. */
} HijriMoonPosition;

/* See the ACCURACY CAVEAT at the top of this file. */
HIJRIDEF HijriMoonPosition hijri_moon_position(double jd_tt);

/* Topocentric RA/Dec, correcting for parallax at a specific location. */
HIJRIDEF void hijri_moon_topocentric(const HijriMoonPosition *geo, double jd_ut,
                                     double latitude_deg, double longitude_deg,
                                     double elevation_m, double *ra_topo_deg,
                                     double *dec_topo_deg);

/* ---- Rise/set/altitude solver -------------------------------------------
 * Generic bisection root-finder on topocentric altitude, used for both
 * sunset and moonset. */

typedef enum {
  HIJRI_EVENT_OK = 0,
  HIJRI_EVENT_NEVER_RISES,
  HIJRI_EVENT_NEVER_SETS,
  /* Search failure: non-finite input or no event in the search window.
   * Returned by the conjunction finders; the circumpolar statuses above
   * remain specific to the rise/set solvers. */
  HIJRI_EVENT_NOT_FOUND
} HijriEventStatus;

HIJRIDEF double hijri_sun_altitude(double jd_ut, const HijriLocation *loc);
HIJRIDEF double hijri_moon_altitude(double jd_ut, const HijriLocation *loc);

HIJRIDEF HijriEventStatus hijri_find_sunset(double jd_local_midnight_ut,
                                            const HijriLocation *loc,
                                            const HijriSunsetConvention *conv,
                                            double *result_jd);
HIJRIDEF HijriEventStatus hijri_find_moonset(double jd_after,
                                             const HijriLocation *loc,
                                             const HijriSunsetConvention *conv,
                                             double *result_jd);

/* ---- Conjunction (new moon) finder --------------------------------------- */

/* All three finders return HIJRI_EVENT_OK with the conjunction's Julian
 * Day in *result_jd, or HIJRI_EVENT_NOT_FOUND (leaving *result_jd
 * untouched) on non-finite input or an empty search window. With finite
 * civil dates the windows always contain a conjunction -- a lunation is
 * 29.5 days -- so NOT_FOUND from calendar code indicates a caller bug,
 * which is exactly why it is reported instead of echoed back as a
 * plausible-looking Julian Day (the pre-1.0 behaviour). */

/* Most recent geocentric conjunction at or before jd_before. This is what
 * you want when asking "what new moon does this evening's month count
 * belong to". */
HIJRIDEF HijriEventStatus
hijri_find_previous_conjunction(double jd_before, double *result_jd);
HIJRIDEF HijriEventStatus
hijri_find_next_conjunction(double jd_after, double *result_jd);
/* Selects the conjunction associated with the candidate crescent: the
 * nearer of previous/next to jd_evening. Meaningful for evenings near the
 * new-moon window. */
HIJRIDEF HijriEventStatus
hijri_find_relevant_conjunction(double jd_evening, double *result_jd);

/* Criterion thresholds, public because they are part of each criterion's
 * published definition. hijri_local_predicate_evaluate and
 * hijri_predicate_margins both read these, so the decision and the reported
 * margin cannot disagree about a number.
 *
 * Mutation record: HIJRI_MABIMS_2021_ALTITUDE_DEG was changed from 3.0 to
 * 4.0 and `make test` was run. The observed failures, verbatim:
 *   FAIL exact/mabims_2021_topocentric_equal actual=0 expected=1
 *   FAIL exact/mabims_2021_altitude_above actual=0 expected=1
 *   FAIL exact/mabims_2021_elongation_above actual=0 expected=1
 *   Hijri tests: 570 checks, 3 failures
 * This records that the existing calendar fixtures are the net that catches
 * a threshold edit. A later invariant test recombines per-term margins with
 * each predicate's boolean structure, but by construction both sides of
 * that invariant read the same constant, so a threshold change moves them
 * together and the invariant cannot catch it. These fixtures are the ones
 * that do. */
#define HIJRI_MABIMS_1992_ALTITUDE_DEG     2.0
#define HIJRI_MABIMS_1992_ELONGATION_DEG   3.0
#define HIJRI_MABIMS_1992_AGE_HOURS        8.0
#define HIJRI_MABIMS_2021_ALTITUDE_DEG     3.0
#define HIJRI_MABIMS_2021_ELONGATION_DEG   6.4
#define HIJRI_WUJUDUL_HILAL_LIMB_DEG       0.0
#define HIJRI_LAG_THRESHOLD_MINUTES        5.0
#define HIJRI_RESEARCH_ALTITUDE_DEG        5.0
#define HIJRI_RESEARCH_ELONGATION_DEG      8.0

typedef enum {
  HIJRI_PREDICATE_MABIMS_1992,
  HIJRI_PREDICATE_MABIMS_2021,
  HIJRI_PREDICATE_WUJUDUL_HILAL,
  HIJRI_PREDICATE_LAG_AT_LEAST_5_MINUTES,
  HIJRI_PREDICATE_ALTITUDE_5_ELONGATION_8,
  HIJRI_PREDICATE_CONJUNCTION_AND_MOONSET
} HijriLocalPredicate;

typedef struct {
  /* UT, not TT. The name is a contract: hijri_jd_tt_from_ut() is applied
   * internally before any ephemeris evaluation. Callers populate this from
   * UTC in practice, and UT1 minus UTC ranges over +/- 0.9 s by
   * construction, which no table in this file can remove. */
  double jd_sunset_ut;
  /* UT. hijri_find_relevant_conjunction() picks whichever of the previous
   * or next conjunction is nearer to jd_sunset_ut. NAN if sunset_status is
   * not HIJRI_EVENT_OK, since the search below never ran. */
  double jd_relevant_conjunction_ut;
  /* UT. From hijri_find_moonset() searching forward from jd_sunset_ut. NAN
   * if moonset_status is not HIJRI_EVENT_OK. */
  double jd_moonset_ut;
  /* Result of the jd_sunset_ut search. Everything below this field is only
   * meaningful when this is HIJRI_EVENT_OK, otherwise the struct is
   * returned early with the remaining fields left at NAN/0. */
  HijriEventStatus sunset_status;
  /* Initialized to sunset_status, then overwritten with the actual
   * hijri_find_moonset() result only when sunset_status is HIJRI_EVENT_OK,
   * since moonset is never searched otherwise. */
  HijriEventStatus moonset_status;
  /* GEOCENTRIC Sun, deliberately: solar parallax is omitted because the
   * conventions this library reproduces omit it, see the file header.
   * Unrefracted. Evaluated at sunset, so it sits at the selected
   * convention's own target by construction. */
  double sun_center_geometric_altitude_deg;
  /* TOPOCENTRIC and UNREFRACTED. "geometric" here means unrefracted, NOT
   * geocentric: hijri_moon_altitude() applies hijri_moon_topocentric().
   * The quantity MABIMS 2021 thresholds at 3 deg. */
  double moon_center_geometric_altitude_deg;
  /* Topocentric centre altitude plus semidiameter plus horizon refraction,
   * both taken from the HijriSunsetConvention the call selected. The
   * quantity HIJRI_PREDICATE_WUJUDUL_HILAL tests against zero. */
  double moon_upper_limb_apparent_altitude_deg;
  /* Apparent Sun against a MEAN-OF-DATE Moon, which are different frames.
   * Measured at 0.0065798 deg against DE440. Making the frames consistent
   * measures WORSE, see the file header. */
  double geocentric_elongation_deg;
  /* Apparent Sun against a TOPOCENTRIC Moon, paired the way
   * HIJRI_PREDICATE_MABIMS_2021 (Odeh-derived) needs. Measured at 0.0069925
   * deg worst case against DE440, see the file header. */
  double topocentric_elongation_deg;
  /* (jd_sunset_ut - jd_relevant_conjunction_ut) * 24, hours, signed: negative
   * when the relevant conjunction is still ahead of sunset. NAN whenever
   * jd_relevant_conjunction_ut is NAN. */
  double moon_age_hours;
  /* (jd_moonset_ut - jd_sunset_ut) * 24 * 60, minutes, signed: negative when
   * moonset precedes sunset. NAN whenever jd_moonset_ut is NAN. */
  double lag_time_minutes;
  /* jd_relevant_conjunction_ut < jd_sunset_ut, as 0 or 1. 0 whenever
   * jd_relevant_conjunction_ut is NAN, since the comparison never ran. */
  int conjunction_before_sunset;
  /* jd_moonset_ut > jd_sunset_ut, as 0 or 1. 0 whenever jd_moonset_ut is
   * NAN, since the comparison never ran. */
  int moonset_after_sunset;
} HijriEveningParameters;

HIJRIDEF HijriEveningParameters
hijri_compute_evening_parameters(int gy, int gm, int gd,
                                 const HijriLocation *loc,
                                 const HijriSunsetConvention *conv);

HIJRIDEF int
hijri_local_predicate_evaluate(HijriLocalPredicate predicate,
                               const HijriEveningParameters *p);

typedef enum {
  HIJRI_UNIT_DEGREES,
  HIJRI_UNIT_HOURS,
  HIJRI_UNIT_MINUTES
} HijriUnit;

typedef enum {
  HIJRI_TERM_MOON_CENTER_ALTITUDE,
  HIJRI_TERM_MOON_UPPER_LIMB_ALTITUDE,
  HIJRI_TERM_GEOCENTRIC_ELONGATION,
  HIJRI_TERM_TOPOCENTRIC_ELONGATION,
  HIJRI_TERM_MOON_AGE,
  HIJRI_TERM_LAG_TIME
} HijriDecisionTerm;

typedef struct {
  /* Which quantity this row reports. */
  HijriDecisionTerm term;
  /* Unit of value, threshold and margin. Terms of different units must not
   * be compared against each other, which is why no aggregate is offered. */
  HijriUnit unit;
  /* The computed quantity, copied from HijriEveningParameters. NAN when the
   * solver that produces it did not succeed. */
  double value;
  /* What the criterion compares it against, from the constants above. */
  double threshold;
  /* value minus threshold. NAN when value is NAN. */
  double margin;
  /* 1 if the criterion uses >, 0 if it uses >=. At a margin of exactly zero
   * this is what decides the term, and whether exactly zero passes is a
   * policy question no measurement settles. */
  int strict;
  /* Whether this term alone passes, under its own operator. 0 when margin
   * is NAN. */
  int passes;
} HijriDecisionTermMargin;

#define HIJRI_MAX_DECISION_TERMS 3

typedef struct {
  /* Number of populated entries in terms. Never exceeds
   * HIJRI_MAX_DECISION_TERMS. */
  int count;
  /* The criterion's terms, in the order the criterion states them. */
  HijriDecisionTermMargin terms[HIJRI_MAX_DECISION_TERMS];
} HijriDecisionMargins;

/* Reports how far each term of `predicate` sits from its own threshold, in
 * that term's own units. Reports only. It combines nothing, converts nothing
 * between units, and labels nothing near, so it cannot change a decision.
 * Compare a margin against the error bars documented in this file's header. */
HIJRIDEF HijriDecisionMargins
hijri_predicate_margins(HijriLocalPredicate predicate,
                        const HijriEveningParameters *p);

typedef struct {
  int month_starts_next_day;
  HijriEveningParameters parameters;
} HijriMonthDecision;

HIJRIDEF HijriMonthDecision
hijri_evaluate_evening(int gy, int gm, int gd, const HijriLocation *loc,
                       HijriLocalPredicate predicate);

/* Builds a calendar from one local predicate at one observer location.
 * This is not, by itself, a national or global authority policy. */
HIJRIDEF int hijri_from_gregorian_with_local_predicate(
    int gy, int gm, int gd, const HijriLocation *loc,
    HijriLocalPredicate predicate, HijriDate *out);

/* Table range: 1882-11-12 to 2174-11-25 Gregorian (Hijri 1300-1600). Inside
 * that range the answer comes from the published table. Outside it, the
 * answer comes from the astronomical reconstruction instead, the same
 * calculation hijri_from_gregorian_with_local_predicate does at Mecca with
 * the conjunction-and-moonset predicate. Inside the table range the return
 * value is 1. Outside it, the reconstruction can fail, in which case the
 * return value is 0 and out is not filled in, so a return of 1 does not by
 * itself say which algorithm answered, hijri_umm_al_qura_covers below is how
 * a caller tells the two apart. */
HIJRIDEF int hijri_umm_al_qura_from_gregorian(int gy, int gm, int gd,
                                               HijriDate *out);

/* True if the given Gregorian date falls inside the Umm al-Qura table's
 * coverage. Inside coverage, hijri_umm_al_qura_from_gregorian answers from
 * the table and returns 1. Outside it, the answer comes from the
 * astronomical reconstruction instead, which can fail and return 0, so this
 * function is what tells a caller which algorithm is in play, a return
 * value of 1 alone does not say. */
HIJRIDEF int hijri_umm_al_qura_covers(int gy, int gm, int gd);

typedef enum {
  HIJRI_YALLOP_A_EASILY_VISIBLE,
  HIJRI_YALLOP_B_VISIBLE_PERFECT_CONDITIONS,
  HIJRI_YALLOP_C_MAY_NEED_OPTICAL_AID,
  HIJRI_YALLOP_D_NEEDS_OPTICAL_AID,
  HIJRI_YALLOP_E_NOT_VISIBLE_TELESCOPE,
  HIJRI_YALLOP_F_NOT_VISIBLE_BELOW_LIMIT
} HijriYallopZone;

typedef struct {
  double jd_best_time_ut;
  double arcv_deg;
  double crescent_width_arcmin;
  double q;
  HijriYallopZone zone;
} HijriYallopResult;

HIJRIDEF double hijri_yallop_q(double arcv_deg, double crescent_width_arcmin);
HIJRIDEF HijriYallopZone hijri_yallop_classify(double arcv_deg,
                                               double crescent_width_arcmin);
HIJRIDEF HijriYallopResult
hijri_yallop_evaluate_evening(int gy, int gm, int gd,
                              const HijriLocation *loc);

typedef enum {
  HIJRI_ODEH_NOT_VISIBLE = 0,
  HIJRI_ODEH_VISIBLE_WITH_OPTICAL_AID_ONLY,
  HIJRI_ODEH_VISIBLE_WITH_OPTICAL_AID_COULD_BE_NAKED_EYE,
  HIJRI_ODEH_VISIBLE_NAKED_EYE
} HijriOdehZone;

typedef struct {
  double jd_best_time_ut;
  double arcv_deg;
  double crescent_width_arcmin;
  double v;
  HijriOdehZone zone;
} HijriOdehResult;

HIJRIDEF double hijri_odeh_v(double arcv_deg, double crescent_width_arcmin);
HIJRIDEF HijriOdehZone hijri_odeh_classify(double arcv_deg,
                                           double crescent_width_arcmin);
HIJRIDEF HijriOdehResult
hijri_odeh_evaluate_evening(int gy, int gm, int gd,
                            const HijriLocation *loc);

/* ---- Tabular / arithmetic calendar (Kuwaiti algorithm) -------------------
 * No astronomy: fixed 30-year cycle, 11 leap years of 355 days. Matches
 * Umm al-Qura to within about +/-1-2 days. Epoch: 1 Muharram 1 AH = JD
 * 1948439.5 (civil/"Friday" epoch convention). */

/* hijri_tabular_to_jd is unchecked arithmetic by design, an out-of-range
 * month or day is not rejected, it produces a plausible wrong Julian Day
 * instead of an error. Call hijri_tabular_date_valid first if the input is
 * not already known good.
 *
 * hijri_tabular_from_jd rejects non-finite and out-of-range input and
 * returns the sentinel HijriDate {0, 0, 0} rather than a garbage date.
 *
 * SUPPORTED AND TESTED RANGE: Hijri years 1 through 9999, round tripped in
 * the test suite along with years -999 through 0. The representable and
 * safe range that hijri_tabular_date_valid checks is wider, bounded by int
 * overflow rather than by what the tests cover. The two are separate
 * claims, one is not a stand-in for the other. */
HIJRIDEF double hijri_tabular_to_jd(HijriDate date);
HIJRIDEF HijriDate hijri_tabular_from_jd(double jd);

/* True if date is representable by hijri_tabular_to_jd/hijri_tabular_from_jd,
 * i.e. year in [-999999, 999999], month in [1, 12], and day a valid day of
 * that month. This is the representable range, not the tested one. */
HIJRIDEF int hijri_tabular_date_valid(HijriDate date);

#ifdef __cplusplus
}
#endif

#endif /* HIJRI_H */

/* =========================================================================
 *  IMPLEMENTATION
 * ========================================================================= */
#ifdef HIJRI_IMPLEMENTATION

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define HIJRI__DEG2RAD(x) ((x) * M_PI / 180.0)
#define HIJRI__RAD2DEG(x) ((x) * 180.0 / M_PI)

#ifndef NAN
#define NAN (0.0 / 0.0)
#endif

/* ---- shared internal helpers -------------------------------------------- */

static double hijri__norm_deg(double d) {
  d = fmod(d, 360.0);
  if (d < 0)
    d += 360.0;
  return d;
}

static double hijri__gmst_deg(double jd_ut) {
  /* Greenwich Mean Sidereal Time, Meeus ch. 12. */
  double T = (jd_ut - 2451545.0) / 36525.0;
  double gmst = 280.46061837 + 360.98564736629 * (jd_ut - 2451545.0) +
                0.000387933 * T * T - T * T * T / 38710000.0;
  return hijri__norm_deg(gmst);
}

static double hijri__nutation_longitude_deg(double T);

/* Equation of the equinoxes: GAST - GMST = delta-psi * cos(epsilon).
 *
 * GMST above is the hour angle of the MEAN equinox. hijri_sun_position()
 * returns an APPARENT right ascension, referred to the TRUE equinox (Meeus
 * ch. 25 applies nutation and aberration to the longitude and corrects the
 * obliquity). An hour angle is only meaningful when the sidereal time and the
 * right ascension are referred to the SAME equinox, so solar hour angles use
 * GAST and lunar ones keep GMST -- hijri_moon_position() is mean-of-date and
 * uses mean obliquity, so GMST is already correct for it.
 *
 * delta-psi comes from the same helper hijri_sun_position() uses, so the
 * equinox the right ascension is referred to and the equinox the sidereal time
 * is corrected to are the same one, whatever that helper's own accuracy. The
 * two calls evaluate it at T from UT here and T from TT there, differing by
 * Delta T, which leaves a residual of about 1e-5 arcsec on a 16 arcsec term.
 * Consistent to that level, not literally exact. See issue #29. */
static double hijri__eqeq_deg(double jd_ut) {
  double T = (jd_ut - 2451545.0) / 36525.0;
  return hijri__nutation_longitude_deg(T) * cos(HIJRI__DEG2RAD(23.4393));
}

static double hijri__gast_deg(double jd_ut) {
  return hijri__norm_deg(hijri__gmst_deg(jd_ut) + hijri__eqeq_deg(jd_ut));
}

static double hijri__hour_angle_deg(double jd_ut, double longitude_deg,
                                    double ra_deg) {
  double lst = hijri__norm_deg(hijri__gmst_deg(jd_ut) + longitude_deg);
  return hijri__norm_deg(lst - ra_deg);
}

/* Altitude from a sidereal time the caller chooses. The two wrappers below
 * differ ONLY in which sidereal time they pass, so the trigonometry lives here
 * once: a correction to the altitude formula cannot be applied to one body and
 * missed on the other. */
static double hijri__altitude_from_sidereal_deg(double sidereal_deg,
                                                double ra_deg, double dec_deg,
                                                const HijriLocation *loc) {
  double H = HIJRI__DEG2RAD(hijri__norm_deg(
      hijri__norm_deg(sidereal_deg + loc->longitude_deg) - ra_deg));
  double phi = HIJRI__DEG2RAD(loc->latitude_deg);
  double dec = HIJRI__DEG2RAD(dec_deg);
  double sin_alt = sin(phi) * sin(dec) + cos(phi) * cos(dec) * cos(H);
  return HIJRI__RAD2DEG(asin(sin_alt));
}

/* For the MOON, whose right ascension is mean-of-date. */
static double hijri__altitude_deg(double ra_deg, double dec_deg, double jd_ut,
                                  const HijriLocation *loc) {
  return hijri__altitude_from_sidereal_deg(hijri__gmst_deg(jd_ut), ra_deg,
                                           dec_deg, loc);
}

/* For the SUN, whose right ascension is apparent. See hijri__eqeq_deg(). */
static double hijri__altitude_gast_deg(double ra_deg, double dec_deg,
                                       double jd_ut,
                                       const HijriLocation *loc) {
  return hijri__altitude_from_sidereal_deg(hijri__gast_deg(jd_ut), ra_deg,
                                           dec_deg, loc);
}

static double hijri__angular_separation_deg(double ra1, double dec1, double ra2,
                                            double dec2) {
  double r1 = HIJRI__DEG2RAD(ra1), d1 = HIJRI__DEG2RAD(dec1);
  double r2 = HIJRI__DEG2RAD(ra2), d2 = HIJRI__DEG2RAD(dec2);
  double cos_sep = sin(d1) * sin(d2) + cos(d1) * cos(d2) * cos(r1 - r2);
  if (cos_sep > 1.0)
    cos_sep = 1.0;
  if (cos_sep < -1.0)
    cos_sep = -1.0;
  return HIJRI__RAD2DEG(acos(cos_sep));
}

/* ---- Julian Day ---------------------------------------------------------- */

HIJRIDEF double hijri_jd_from_gregorian(int year, int month, double day) {
  if (month <= 2) {
    year -= 1;
    month += 12;
  }
  int A = year / 100;
  int B = 2 - A + A / 4;
  return floor(365.25 * (year + 4716)) + floor(30.6001 * (month + 1)) + day +
         B - 1524.5;
}

HIJRIDEF void hijri_gregorian_from_jd(double jd, int *year, int *month,
                                      double *day_frac) {
  double jd_shift = jd + 0.5;
  double Z = floor(jd_shift);
  double F = jd_shift - Z;
  double A;
  if (Z < 2299161.0) {
    A = Z;
  } else {
    double alpha = floor((Z - 1867216.25) / 36524.25);
    A = Z + 1 + alpha - floor(alpha / 4);
  }
  double B = A + 1524;
  double C = floor((B - 122.1) / 365.25);
  double D = floor(365.25 * C);
  double E = floor((B - D) / 30.6001);

  *day_frac = B - D - floor(30.6001 * E) + F;
  *month = (int)((E < 14) ? (E - 1) : (E - 13));
  *year = (int)((*month > 2) ? (C - 4716) : (C - 4715));
}

HIJRIDEF int hijri_jd_weekday(double jd) {
  long day_num = (long)floor(jd + 1.5);
  int wd = (int)(day_num % 7);
  if (wd < 0)
    wd += 7;
  return wd;
}

HIJRIDEF double hijri_julian_centuries(double jd) {
  return (jd - 2451545.0) / 36525.0;
}

/* ---- Delta T ---------------------------------------------------------------
 */

HIJRIDEF double hijri_delta_t_seconds(double jd) {
  int year, month;
  double day;
  hijri_gregorian_from_jd(jd, &year, &month, &day);
  double y = year + (month - 0.5) / 12.0;

  if (y >= 1986.0 && y <= 2050.0) {
    double u = y - 2000.0;
    return 62.92 + 0.32217 * u + 0.005589 * u * u;
  }

  double u = (y - 1820.0) / 100.0;
  return -20.0 + 32.0 * u * u;
}

HIJRIDEF double hijri_jd_tt_from_ut(double jd_ut) {
  return jd_ut + hijri_delta_t_seconds(jd_ut) / 86400.0;
}

/* Nutation in longitude, leading term only (Meeus 22.1 truncated to the
 * 17.20" Omega term). This is deliberately NOT a better nutation model: the
 * same value defines the true equinox that hijri_sun_position()'s apparent
 * right ascension is referred to AND the equation-of-the-equinoxes correction
 * applied to sidereal time in hijri__eqeq_deg(). Sharing one expression ties
 * the two together, so the model's own truncation error
 * stays inside the Meeus ch. 25 budget already documented rather than leaking
 * into the hour angle as a fresh residual. Improving this term without
 * improving hijri_sun_position() in the same way would REINTRODUCE the bug
 * this helper exists to fix. */
static double hijri__nutation_longitude_deg(double T) {
  return -0.00478 * sin(HIJRI__DEG2RAD(125.04 - 1934.136 * T));
}

/* ---- Solar position (Meeus low-precision solar theory) ---------------------
 */

HIJRIDEF HijriSunPosition hijri_sun_position(double jd_tt) {
  double T = hijri_julian_centuries(jd_tt);

  double L0 = hijri__norm_deg(280.46646 + 36000.76983 * T + 0.0003032 * T * T);
  double M = hijri__norm_deg(357.52911 + 35999.05029 * T - 0.0001537 * T * T);
  double Mr = HIJRI__DEG2RAD(M);

  double C = (1.914602 - 0.004817 * T - 0.000014 * T * T) * sin(Mr) +
             (0.019993 - 0.000101 * T) * sin(2 * Mr) + 0.000289 * sin(3 * Mr);

  double true_longitude = L0 + C;

  double omega = 125.04 - 1934.136 * T;
  double apparent_longitude =
      true_longitude - 0.00569 + hijri__nutation_longitude_deg(T);

  double eps0 =
      23.439291 - 0.0130042 * T - 0.00000016 * T * T + 0.000000504 * T * T * T;
  double eps = eps0 + 0.00256 * cos(HIJRI__DEG2RAD(omega));

  double lambda_r = HIJRI__DEG2RAD(apparent_longitude);
  double eps_r = HIJRI__DEG2RAD(eps);

  double ra = atan2(cos(eps_r) * sin(lambda_r), cos(lambda_r));
  double dec = asin(sin(eps_r) * sin(lambda_r));

  double e = 0.016708634 - 0.000042037 * T - 0.0000001267 * T * T;
  double R = (1.000001018 * (1 - e * e)) / (1 + e * cos(Mr));

  HijriSunPosition pos;
  pos.apparent_longitude_deg = hijri__norm_deg(apparent_longitude);
  pos.right_ascension_deg = hijri__norm_deg(HIJRI__RAD2DEG(ra));
  pos.declination_deg = HIJRI__RAD2DEG(dec);
  pos.distance_au = R;
  pos.obliquity_deg = eps;
  return pos;
}

/* ---- Lunar position (full Meeus ch. 47 series -- Tables 47.A and 47.B) ----
 */

/* PROVENANCE OF THE TWO TABLES BELOW
 *
 * These 120 coefficients were not written from memory. They were taken from two
 * independent published transcriptions of Meeus ch. 47 and compared row by row
 * before use:
 *
 *   PyMeeus      -- architest/pymeeus, pymeeus/Moon.py
 *   astronomia   -- commenthol/astronomia, src/moonposition.js
 *
 * Be precise about what was machine-checked, because this comment is the
 * repository's tracked provenance record and overstating it would defeat its
 * purpose. Table 47.A was compared row by row under `diff`: all 60 rows matched
 * exactly between the two sources. Table 47.B was read from both sources and
 * they agreed, but only one copy was retained, so the automated checks applied
 * to it were row count, dominant term, and parity -- not a two-source diff.
 *
 * Both tables satisfy the parity invariant of the lunar theory -- every 47.A row
 * has an even F multiple, every 47.B row an odd one -- which a corrupted table
 * would be unlikely to satisfy by accident.
 *
 * The decisive check is not either of those, though: tests/test_ephemeris_oracle.c
 * asserts Meeus's own printed worked Example 47.a, and this implementation
 * reproduces lambda, beta and Delta to every digit the book prints. That is
 * reproducible from this repository alone and does not rely on trusting either
 * transcription, or this comment.
 */

/* E^|M|, applied to terms involving the Sun's mean anomaly to account for the
 * eccentricity of Earth's orbit. Meeus specifies E for |M| = 1 and E^2 for
 * |M| = 2; no row of either table has |M| > 2. */
static double hijri__moon_e_factor(double E, long m) {
  if (m == 0)
    return 1.0;
  return (m == 1 || m == -1) ? E : E * E;
}

/* Meeus, "Astronomical Algorithms" 2nd ed., Table 47.A -- periodic terms for
 * the Moon's longitude (sigma_l, unit 1e-6 deg) and distance (sigma_r, unit
 * 1e-3 km). Columns: multiples of D, M, M', F, then sigma_l, sigma_r.
 * One row per line, matching the published table 1:1 so it stays auditable.
 * Every row has an even F multiple -- longitude and distance are even in F. */
static const long hijri__moon_lr[60][6] = {
  {0, 0, 1, 0, 6288774, -20905355},
  {2, 0, -1, 0, 1274027, -3699111},
  {2, 0, 0, 0, 658314, -2955968},
  {0, 0, 2, 0, 213618, -569925},
  {0, 1, 0, 0, -185116, 48888},
  {0, 0, 0, 2, -114332, -3149},
  {2, 0, -2, 0, 58793, 246158},
  {2, -1, -1, 0, 57066, -152138},
  {2, 0, 1, 0, 53322, -170733},
  {2, -1, 0, 0, 45758, -204586},
  {0, 1, -1, 0, -40923, -129620},
  {1, 0, 0, 0, -34720, 108743},
  {0, 1, 1, 0, -30383, 104755},
  {2, 0, 0, -2, 15327, 10321},
  {0, 0, 1, 2, -12528, 0},
  {0, 0, 1, -2, 10980, 79661},
  {4, 0, -1, 0, 10675, -34782},
  {0, 0, 3, 0, 10034, -23210},
  {4, 0, -2, 0, 8548, -21636},
  {2, 1, -1, 0, -7888, 24208},
  {2, 1, 0, 0, -6766, 30824},
  {1, 0, -1, 0, -5163, -8379},
  {1, 1, 0, 0, 4987, -16675},
  {2, -1, 1, 0, 4036, -12831},
  {2, 0, 2, 0, 3994, -10445},
  {4, 0, 0, 0, 3861, -11650},
  {2, 0, -3, 0, 3665, 14403},
  {0, 1, -2, 0, -2689, -7003},
  {2, 0, -1, 2, -2602, 0},
  {2, -1, -2, 0, 2390, 10056},
  {1, 0, 1, 0, -2348, 6322},
  {2, -2, 0, 0, 2236, -9884},
  {0, 1, 2, 0, -2120, 5751},
  {0, 2, 0, 0, -2069, 0},
  {2, -2, -1, 0, 2048, -4950},
  {2, 0, 1, -2, -1773, 4130},
  {2, 0, 0, 2, -1595, 0},
  {4, -1, -1, 0, 1215, -3958},
  {0, 0, 2, 2, -1110, 0},
  {3, 0, -1, 0, -892, 3258},
  {2, 1, 1, 0, -810, 2616},
  {4, -1, -2, 0, 759, -1897},
  {0, 2, -1, 0, -713, -2117},
  {2, 2, -1, 0, -700, 2354},
  {2, 1, -2, 0, 691, 0},
  {2, -1, 0, -2, 596, 0},
  {4, 0, 1, 0, 549, -1423},
  {0, 0, 4, 0, 537, -1117},
  {4, -1, 0, 0, 520, -1571},
  {1, 0, -2, 0, -487, -1739},
  {2, 1, 0, -2, -399, 0},
  {0, 0, 2, -2, -381, -4421},
  {1, 1, 1, 0, 351, 0},
  {3, 0, -2, 0, -340, 0},
  {4, 0, -3, 0, 330, 0},
  {2, -1, 2, 0, 327, 0},
  {0, 2, 1, 0, -323, 1165},
  {1, 1, -1, 0, 299, 0},
  {2, 0, 3, 0, 294, 0},
  {2, 0, -1, -2, 0, 8752},
};

/* Table 47.B -- periodic terms for the Moon's latitude (sigma_b, unit 1e-6
 * deg). Columns: multiples of D, M, M', F, then sigma_b. Every row has an odd
 * F multiple -- latitude is odd in F. */
static const long hijri__moon_b[60][5] = {
  {0, 0, 0, 1, 5128122},
  {0, 0, 1, 1, 280602},
  {0, 0, 1, -1, 277693},
  {2, 0, 0, -1, 173237},
  {2, 0, -1, 1, 55413},
  {2, 0, -1, -1, 46271},
  {2, 0, 0, 1, 32573},
  {0, 0, 2, 1, 17198},
  {2, 0, 1, -1, 9266},
  {0, 0, 2, -1, 8822},
  {2, -1, 0, -1, 8216},
  {2, 0, -2, -1, 4324},
  {2, 0, 1, 1, 4200},
  {2, 1, 0, -1, -3359},
  {2, -1, -1, 1, 2463},
  {2, -1, 0, 1, 2211},
  {2, -1, -1, -1, 2065},
  {0, 1, -1, -1, -1870},
  {4, 0, -1, -1, 1828},
  {0, 1, 0, 1, -1794},
  {0, 0, 0, 3, -1749},
  {0, 1, -1, 1, -1565},
  {1, 0, 0, 1, -1491},
  {0, 1, 1, 1, -1475},
  {0, 1, 1, -1, -1410},
  {0, 1, 0, -1, -1344},
  {1, 0, 0, -1, -1335},
  {0, 0, 3, 1, 1107},
  {4, 0, 0, -1, 1021},
  {4, 0, -1, 1, 833},
  {0, 0, 1, -3, 777},
  {4, 0, -2, 1, 671},
  {2, 0, 0, -3, 607},
  {2, 0, 2, -1, 596},
  {2, -1, 1, -1, 491},
  {2, 0, -2, 1, -451},
  {0, 0, 3, -1, 439},
  {2, 0, 2, 1, 422},
  {2, 0, -3, -1, 421},
  {2, 1, -1, 1, -366},
  {2, 1, 0, 1, -351},
  {4, 0, 0, 1, 331},
  {2, -1, 1, 1, 315},
  {2, -2, 0, -1, 302},
  {0, 0, 1, 3, -283},
  {2, 1, 1, -1, -229},
  {1, 1, 0, -1, 223},
  {1, 1, 0, 1, 223},
  {0, 1, -2, -1, -220},
  {2, 1, -1, -1, -220},
  {1, 0, 1, 1, -185},
  {2, -1, -2, -1, 181},
  {0, 1, 2, 1, -177},
  {4, 0, -2, -1, 176},
  {4, -1, -1, -1, 166},
  {1, 0, 1, -1, -164},
  {4, 0, 1, -1, 132},
  {1, 0, -1, -1, -119},
  {4, -1, 0, -1, 115},
  {2, -2, 0, 1, 107},
};

HIJRIDEF HijriMoonPosition hijri_moon_position(double jd_tt) {
  double T = hijri_julian_centuries(jd_tt);
  double T2 = T * T, T3 = T2 * T, T4 = T3 * T;

  double Lp = hijri__norm_deg(218.3164477 + 481267.88123421 * T -
                              0.0015786 * T2 + T3 / 538841.0 - T4 / 65194000.0);
  double D = hijri__norm_deg(297.8501921 + 445267.1114034 * T - 0.0018819 * T2 +
                             T3 / 545868.0 - T4 / 113065000.0);
  double M = hijri__norm_deg(357.5291092 + 35999.0502909 * T - 0.0001536 * T2 +
                             T3 / 24490000.0);
  double Mp = hijri__norm_deg(134.9633964 + 477198.8675055 * T +
                              0.0087414 * T2 + T3 / 69699.0 - T4 / 14712000.0);
  double F = hijri__norm_deg(93.2720950 + 483202.0175233 * T - 0.0036539 * T2 -
                             T3 / 3526000.0 + T4 / 863310000.0);

  double E = 1.0 - 0.002516 * T - 0.0000074 * T2;
  double A1 = hijri__norm_deg(119.75 + 131.849 * T);
  double A2 = hijri__norm_deg(53.09 + 479264.290 * T);
  double A3 = hijri__norm_deg(313.45 + 481266.484 * T);

  double sum_l = 0.0, sum_r = 0.0, sum_b = 0.0;
  int i;

  for (i = 0; i < 60; i++) {
    double arg = HIJRI__DEG2RAD(hijri__moon_lr[i][0] * D +
                                hijri__moon_lr[i][1] * M +
                                hijri__moon_lr[i][2] * Mp +
                                hijri__moon_lr[i][3] * F);
    double e = hijri__moon_e_factor(E, hijri__moon_lr[i][1]);
    sum_l += (double)hijri__moon_lr[i][4] * e * sin(arg);
    sum_r += (double)hijri__moon_lr[i][5] * e * cos(arg);
  }

  for (i = 0; i < 60; i++) {
    double arg = HIJRI__DEG2RAD(hijri__moon_b[i][0] * D +
                                hijri__moon_b[i][1] * M +
                                hijri__moon_b[i][2] * Mp +
                                hijri__moon_b[i][3] * F);
    sum_b += (double)hijri__moon_b[i][4] *
             hijri__moon_e_factor(E, hijri__moon_b[i][1]) * sin(arg);
  }

  sum_l += 3958.0 * sin(HIJRI__DEG2RAD(A1)) +
           1962.0 * sin(HIJRI__DEG2RAD(Lp - F)) +
           318.0 * sin(HIJRI__DEG2RAD(A2));

  sum_b += -2235.0 * sin(HIJRI__DEG2RAD(Lp)) +
           382.0 * sin(HIJRI__DEG2RAD(A3)) +
           175.0 * sin(HIJRI__DEG2RAD(A1 - F)) +
           175.0 * sin(HIJRI__DEG2RAD(A1 + F)) +
           127.0 * sin(HIJRI__DEG2RAD(Lp - Mp)) -
           115.0 * sin(HIJRI__DEG2RAD(Lp + Mp));

  double longitude = hijri__norm_deg(Lp + sum_l / 1000000.0);
  double latitude = sum_b / 1000000.0;
  double distance_km = 385000.56 + sum_r / 1000.0;

  double eps0 = 23.439291 - 0.0130042 * T;
  double eps_r = HIJRI__DEG2RAD(eps0);
  double lon_r = HIJRI__DEG2RAD(longitude);
  double lat_r = HIJRI__DEG2RAD(latitude);

  double sin_dec =
      sin(lat_r) * cos(eps_r) + cos(lat_r) * sin(eps_r) * sin(lon_r);
  double dec = asin(sin_dec);
  double y = sin(lon_r) * cos(eps_r) - tan(lat_r) * sin(eps_r);
  double x = cos(lon_r);
  double ra = atan2(y, x);

  double hp = asin(6378.14 / distance_km);

  HijriMoonPosition pos;
  pos.geocentric_longitude_deg = longitude;
  pos.geocentric_latitude_deg = latitude;
  pos.distance_km = distance_km;
  pos.right_ascension_deg = hijri__norm_deg(HIJRI__RAD2DEG(ra));
  pos.declination_deg = HIJRI__RAD2DEG(dec);
  pos.horizontal_parallax_deg = HIJRI__RAD2DEG(hp);
  return pos;
}

HIJRIDEF void hijri_moon_topocentric(const HijriMoonPosition *geo, double jd_ut,
                                     double latitude_deg, double longitude_deg,
                                     double elevation_m, double *ra_topo_deg,
                                     double *dec_topo_deg) {
  double H = HIJRI__DEG2RAD(
      hijri__hour_angle_deg(jd_ut, longitude_deg, geo->right_ascension_deg));
  double phi = HIJRI__DEG2RAD(latitude_deg);
  double dec = HIJRI__DEG2RAD(geo->declination_deg);
  double pi_r = HIJRI__DEG2RAD(geo->horizontal_parallax_deg);
  (void)elevation_m; /* elevation correction to parallax omitted: sub-arcsecond
                        effect here */

  double delta_ra = atan2(-cos(phi) * sin(pi_r) * sin(H),
                          cos(dec) - cos(phi) * sin(pi_r) * cos(H));
  double ra_topo = HIJRI__DEG2RAD(geo->right_ascension_deg) + delta_ra;

  double dec_topo = atan2((sin(dec) - sin(phi) * sin(pi_r)) * cos(delta_ra),
                          cos(dec) - cos(phi) * sin(pi_r) * cos(H));

  *ra_topo_deg = hijri__norm_deg(HIJRI__RAD2DEG(ra_topo));
  *dec_topo_deg = HIJRI__RAD2DEG(dec_topo);
}

/* ---- Rise/set solver --------------------------------------------------------
 */

HIJRIDEF double hijri_sun_altitude(double jd_ut, const HijriLocation *loc) {
  double jd_tt = hijri_jd_tt_from_ut(jd_ut);
  HijriSunPosition sun = hijri_sun_position(jd_tt);
  return hijri__altitude_gast_deg(sun.right_ascension_deg,
                                  sun.declination_deg, jd_ut, loc);
}

HIJRIDEF double hijri_moon_altitude(double jd_ut, const HijriLocation *loc) {
  double jd_tt = hijri_jd_tt_from_ut(jd_ut);
  HijriMoonPosition geo = hijri_moon_position(jd_tt);
  double ra_topo, dec_topo;
  hijri_moon_topocentric(&geo, jd_ut, loc->latitude_deg, loc->longitude_deg,
                         loc->elevation_m, &ra_topo, &dec_topo);
  return hijri__altitude_deg(ra_topo, dec_topo, jd_ut, loc);
}

/* Apparent altitude of the Sun's UPPER LIMB: geocentric centre altitude plus
 * semidiameter plus horizon refraction. The semidiameter varies with distance,
 * so it cannot be a constant crossing target the way it was before issue #33.
 * Folding it in here and solving for zero is the same shape
 * hijri__moon_upper_limb_altitude() below already uses for the Moon, and it
 * spends the distance_au that hijri_sun_position() computes and this file
 * previously discarded. */
static double hijri__sun_upper_limb_altitude(double jd_ut,
                                             const HijriLocation *loc,
                                             const HijriSunsetConvention *conv) {
  double jd_tt = hijri_jd_tt_from_ut(jd_ut);
  HijriSunPosition sun = hijri_sun_position(jd_tt);
  double sd = (conv->solar_semidiameter_arcsec_at_1au / 3600.0) /
              sun.distance_au;
  return hijri__altitude_gast_deg(sun.right_ascension_deg,
                                  sun.declination_deg, jd_ut, loc) +
         sd + conv->refraction_at_horizon_deg;
}

static int hijri__bisect_crossing(double (*altitude_fn)(
                                       double, const HijriLocation *,
                                       const HijriSunsetConvention *),
                                   const HijriLocation *loc,
                                   const HijriSunsetConvention *conv,
                                   double jd_lo, double jd_hi,
                                   double target_alt_deg, double *result_jd) {
  double a_lo = altitude_fn(jd_lo, loc, conv) - target_alt_deg;
  double a_hi = altitude_fn(jd_hi, loc, conv) - target_alt_deg;
  if (a_lo * a_hi > 0.0)
    return 0;

  for (int i = 0; i < 40; i++) {
    double jd_mid = 0.5 * (jd_lo + jd_hi);
    double a_mid = altitude_fn(jd_mid, loc, conv) - target_alt_deg;
    if ((a_lo < 0 && a_mid < 0) || (a_lo > 0 && a_mid > 0)) {
      jd_lo = jd_mid;
      a_lo = a_mid;
    } else {
      jd_hi = jd_mid;
    }
  }
  *result_jd = 0.5 * (jd_lo + jd_hi);
  return 1;
}

HIJRIDEF HijriEventStatus hijri_find_sunset(double jd_local_midnight_ut,
                                            const HijriLocation *loc,
                                            const HijriSunsetConvention *conv,
                                            double *result_jd) {
  double step = 1.0 / 24.0;
  double prev_jd = jd_local_midnight_ut;
  double prev_alt = hijri__sun_upper_limb_altitude(prev_jd, loc, conv);
  for (int h = 1; h <= 24; h++) {
    double jd = jd_local_midnight_ut + h * step;
    double alt = hijri__sun_upper_limb_altitude(jd, loc, conv);
    if (prev_alt > 0 && alt <= 0) {
      if (hijri__bisect_crossing(hijri__sun_upper_limb_altitude, loc, conv,
                                 prev_jd, jd, 0.0, result_jd)) {
        return HIJRI_EVENT_OK;
      }
    }
    prev_jd = jd;
    prev_alt = alt;
  }
  double mean_alt =
      hijri__sun_upper_limb_altitude(jd_local_midnight_ut + 0.5, loc, conv);
  return (mean_alt > 0.0) ? HIJRI_EVENT_NEVER_SETS : HIJRI_EVENT_NEVER_RISES;
}

/* Apparent altitude of the Moon's UPPER LIMB: topocentric centre altitude
 * plus semidiameter plus horizon refraction. Zero here is the same instant
 * at which moon_upper_limb_apparent_altitude_deg reads zero, and the same
 * convention hijri_find_sunset uses for the Sun -- one horizon definition
 * across the header, so "the Moon has set" and "the Moon's upper limb is
 * below the horizon" cannot disagree. */
static double
hijri__moon_upper_limb_altitude(double jd_ut, const HijriLocation *loc,
                                const HijriSunsetConvention *conv) {
  double jd_tt = hijri_jd_tt_from_ut(jd_ut);
  HijriMoonPosition geo = hijri_moon_position(jd_tt);
  double sd = 0.2725076 * geo.horizontal_parallax_deg;
  double ra_topo, dec_topo;
  /* Same computation hijri_moon_altitude() performs, reusing the position
   * already evaluated for the semidiameter -- one lunar-series evaluation
   * per sample instead of two. */
  hijri_moon_topocentric(&geo, jd_ut, loc->latitude_deg, loc->longitude_deg,
                         loc->elevation_m, &ra_topo, &dec_topo);
  return hijri__altitude_deg(ra_topo, dec_topo, jd_ut, loc) + sd +
         conv->refraction_at_horizon_deg;
}

HIJRIDEF HijriEventStatus hijri_find_moonset(double jd_after,
                                             const HijriLocation *loc,
                                             const HijriSunsetConvention *conv,
                                             double *result_jd) {
  double step = 1.0 / 24.0;
  double prev_jd = jd_after;
  double prev_alt = hijri__moon_upper_limb_altitude(prev_jd, loc, conv);
  for (int h = 1; h <= 24; h++) {
    double jd = jd_after + h * step;
    double alt = hijri__moon_upper_limb_altitude(jd, loc, conv);
    if (prev_alt > 0 && alt <= 0) {
      if (hijri__bisect_crossing(hijri__moon_upper_limb_altitude, loc, conv,
                                 prev_jd, jd, 0.0, result_jd)) {
        return HIJRI_EVENT_OK;
      }
    }
    prev_jd = jd;
    prev_alt = alt;
  }
  double mean_alt = hijri__moon_upper_limb_altitude(jd_after + 0.5, loc, conv);
  return (mean_alt > 0.0) ? HIJRI_EVENT_NEVER_SETS : HIJRI_EVENT_NEVER_RISES;
}

/* ---- Conjunction finder
 * ------------------------------------------------------ */

static double hijri__moon_sun_wrapped_diff(double jd_ut) {
  double jd_tt = hijri_jd_tt_from_ut(jd_ut);
  double moon_lon = hijri_moon_position(jd_tt).geocentric_longitude_deg;
  double sun_lon = hijri_sun_position(jd_tt).apparent_longitude_deg;
  double diff = moon_lon - sun_lon;
  diff = fmod(diff + 180.0, 360.0);
  if (diff < 0)
    diff += 360.0;
  return diff - 180.0;
}

/* Ascending zero-crossing search shared by the public finders. The old
 * public hijri_find_conjunction() -- whose 40-day window made "nearest"
 * ambiguous and which echoed its input back on failure -- lives on only
 * here, with an honest status instead of the echo. */
static HijriEventStatus hijri__search_conjunction(double jd_guess,
                                                  double *result_jd) {
  double step = 0.5;
  double jd_start, prev_jd, prev_diff;

  if (!isfinite(jd_guess)) {
    return HIJRI_EVENT_NOT_FOUND;
  }
  jd_start = jd_guess - 20.0;
  prev_jd = jd_start;
  prev_diff = hijri__moon_sun_wrapped_diff(prev_jd);

  for (double jd = jd_start + step; jd <= jd_guess + 20.0; jd += step) {
    double diff = hijri__moon_sun_wrapped_diff(jd);
    if (prev_diff < 0 && diff >= 0) {
      double lo = prev_jd, hi = jd;
      double dlo = prev_diff;
      for (int i = 0; i < 40; i++) {
        double mid = 0.5 * (lo + hi);
        double dmid = hijri__moon_sun_wrapped_diff(mid);
        if ((dlo < 0 && dmid < 0) || (dlo >= 0 && dmid >= 0)) {
          lo = mid;
          dlo = dmid;
        } else {
          hi = mid;
        }
      }
      *result_jd = 0.5 * (lo + hi);
      return HIJRI_EVENT_OK;
    }
    prev_jd = jd;
    prev_diff = diff;
  }
  return HIJRI_EVENT_NOT_FOUND;
}

HIJRIDEF HijriEventStatus
hijri_find_previous_conjunction(double jd_before, double *result_jd) {
  double step = 0.5;
  double jd_start, prev_jd, prev_diff, found_jd;
  int found = 0;

  if (!isfinite(jd_before)) {
    return HIJRI_EVENT_NOT_FOUND;
  }
  jd_start = jd_before - 33.0;
  prev_jd = jd_start;
  prev_diff = hijri__moon_sun_wrapped_diff(prev_jd);

  for (double jd = jd_start + step; jd <= jd_before; jd += step) {
    double diff = hijri__moon_sun_wrapped_diff(jd);
    if (prev_diff < 0 && diff >= 0) {
      double lo = prev_jd, hi = jd;
      double dlo = prev_diff;
      for (int i = 0; i < 40; i++) {
        double mid = 0.5 * (lo + hi);
        double dmid = hijri__moon_sun_wrapped_diff(mid);
        if ((dlo < 0 && dmid < 0) || (dlo >= 0 && dmid >= 0)) {
          lo = mid;
          dlo = dmid;
        } else {
          hi = mid;
        }
      }
      found_jd = 0.5 * (lo + hi);
      found = 1;
    }
    prev_jd = jd;
    prev_diff = diff;
  }
  if (!found) {
    return HIJRI_EVENT_NOT_FOUND;
  }
  *result_jd = found_jd;
  return HIJRI_EVENT_OK;
}

HIJRIDEF HijriEventStatus
hijri_find_next_conjunction(double jd_after, double *result_jd) {
  double candidate;
  if (hijri__search_conjunction(jd_after + 15.0, &candidate) !=
      HIJRI_EVENT_OK) {
    return HIJRI_EVENT_NOT_FOUND;
  }
  if (candidate <= jd_after) {
    if (hijri__search_conjunction(jd_after + 30.0, &candidate) !=
        HIJRI_EVENT_OK) {
      return HIJRI_EVENT_NOT_FOUND;
    }
  }
  *result_jd = candidate;
  return HIJRI_EVENT_OK;
}

HIJRIDEF HijriEventStatus
hijri_find_relevant_conjunction(double jd_evening, double *result_jd) {
  double previous, next;
  if (hijri_find_previous_conjunction(jd_evening, &previous) !=
          HIJRI_EVENT_OK ||
      hijri_find_next_conjunction(jd_evening, &next) != HIJRI_EVENT_OK) {
    return HIJRI_EVENT_NOT_FOUND;
  }
  *result_jd = fabs(jd_evening - previous) <= fabs(next - jd_evening)
                   ? previous
                   : next;
  return HIJRI_EVENT_OK;
}

/* ---- Hilal parameters and criteria
 * -------------------------------------------- */

HIJRIDEF HijriEveningParameters
hijri_compute_evening_parameters(int gy, int gm, int gd,
                                 const HijriLocation *loc,
                                 const HijriSunsetConvention *conv) {
  HijriEveningParameters p;
  /* hijri_find_sunset() scans forward 24 hours from the instant it is given
   * and its parameter is named jd_local_midnight_ut, so it must be handed
   * LOCAL midnight expressed in UT -- not UT midnight.
   *
   * This previously passed hijri_jd_from_gregorian() directly, which is 0h UT.
   * East of Greenwich that is harmless, because local sunset still falls inside
   * the same UT day. West of roughly 90 deg W it is not: local sunset happens
   * after 00:00 UT the following day, outside the scan window, so the search
   * returned the PREVIOUS local evening. Every predicate, both visibility
   * models, and the calendar conversion inherited that off-by-one-day error
   * across the Americas and the Pacific.
   *
   * Local midnight is approximated as mean solar time from longitude alone.
   * That is deliberate: hijri.h does not depend on timezone.h and carries no
   * zone database. Against civil time it can differ by a couple of hours at a
   * zone's edge, which never approaches the one-day error it replaces, but it
   * does mean the "evening of date D" is the solar-day evening rather than the
   * civil-day one. Callers needing civil-day semantics should resolve the
   * offset themselves and call the JD-based entry points. */
  double jd_midnight = hijri_jd_from_gregorian(gy, gm, (double)gd) -
                       loc->longitude_deg / 360.0;

  p.jd_sunset_ut = NAN;
  p.jd_relevant_conjunction_ut = NAN;
  p.jd_moonset_ut = NAN;
  p.sunset_status = hijri_find_sunset(jd_midnight, loc, conv, &p.jd_sunset_ut);
  p.moonset_status = p.sunset_status;
  p.sun_center_geometric_altitude_deg = NAN;
  p.moon_center_geometric_altitude_deg = NAN;
  p.moon_upper_limb_apparent_altitude_deg = NAN;
  p.geocentric_elongation_deg = NAN;
  p.topocentric_elongation_deg = NAN;
  p.moon_age_hours = NAN;
  p.lag_time_minutes = NAN;
  p.conjunction_before_sunset = 0;
  p.moonset_after_sunset = 0;

  if (p.sunset_status != HIJRI_EVENT_OK) {
    p.jd_sunset_ut = NAN;
    return p;
  }

  {
    double jd_tt = hijri_jd_tt_from_ut(p.jd_sunset_ut);
    HijriSunPosition sun = hijri_sun_position(jd_tt);
    HijriMoonPosition moon = hijri_moon_position(jd_tt);
    double moon_ra_topo;
    double moon_dec_topo;
    double semidiameter_deg = 0.2725076 * moon.horizontal_parallax_deg;

    hijri_moon_topocentric(&moon, p.jd_sunset_ut, loc->latitude_deg,
                           loc->longitude_deg, loc->elevation_m,
                           &moon_ra_topo, &moon_dec_topo);

    p.sun_center_geometric_altitude_deg =
        hijri__altitude_gast_deg(sun.right_ascension_deg,
                                 sun.declination_deg, p.jd_sunset_ut, loc);
    p.moon_center_geometric_altitude_deg =
        hijri__altitude_deg(moon_ra_topo, moon_dec_topo, p.jd_sunset_ut, loc);
    p.moon_upper_limb_apparent_altitude_deg =
        p.moon_center_geometric_altitude_deg + semidiameter_deg +
        conv->refraction_at_horizon_deg;
    p.geocentric_elongation_deg = hijri__angular_separation_deg(
        moon.right_ascension_deg, moon.declination_deg,
        sun.right_ascension_deg, sun.declination_deg);
    p.topocentric_elongation_deg = hijri__angular_separation_deg(
        moon_ra_topo, moon_dec_topo, sun.right_ascension_deg,
        sun.declination_deg);
  }

  if (hijri_find_relevant_conjunction(p.jd_sunset_ut,
                                      &p.jd_relevant_conjunction_ut) ==
      HIJRI_EVENT_OK) {
    p.moon_age_hours =
        (p.jd_sunset_ut - p.jd_relevant_conjunction_ut) * 24.0;
    p.conjunction_before_sunset =
        (p.jd_relevant_conjunction_ut < p.jd_sunset_ut);
  } else {
    /* Unreachable from valid civil dates; reported rather than echoed. */
    p.jd_relevant_conjunction_ut = NAN;
    p.moon_age_hours = NAN;
    p.conjunction_before_sunset = 0;
  }

  p.moonset_status =
      hijri_find_moonset(p.jd_sunset_ut, loc, conv, &p.jd_moonset_ut);
  if (p.moonset_status == HIJRI_EVENT_OK) {
    p.lag_time_minutes =
        (p.jd_moonset_ut - p.jd_sunset_ut) * 24.0 * 60.0;
    p.moonset_after_sunset = (p.jd_moonset_ut > p.jd_sunset_ut);
  } else {
    p.jd_moonset_ut = NAN;
  }

  return p;
}

HIJRIDEF int
hijri_local_predicate_evaluate(HijriLocalPredicate predicate,
                               const HijriEveningParameters *p) {
  switch (predicate) {
  /* UNRESOLVED CONVENTION -- do not "clean up" without a primary source.
   * Two things about the 1992 "2-3-8" criterion are not settled by any
   * technical document located so far:
   *   1. Whether the three parameters combine as (altitude AND elongation)
   *      OR age -- as coded here -- or as all three ANDed together. Indonesian
   *      government summaries word it with "serta" ("and"), which reads as a
   *      conjunction, but they are press-register prose, not a specification.
   *   2. Whether the 3 deg elongation is geocentric or topocentric. Unlike the
   *      2021 criterion this one does not derive from Odeh, so the topocentric
   *      argument below does not automatically carry over.
   * Left as-is deliberately. Changing it on the strength of a news article
   * would replace a documented gap with a guess. */
  case HIJRI_PREDICATE_MABIMS_1992:
    return (p->moon_center_geometric_altitude_deg >= HIJRI_MABIMS_1992_ALTITUDE_DEG &&
            p->geocentric_elongation_deg >= HIJRI_MABIMS_1992_ELONGATION_DEG) ||
           (p->moon_age_hours >= HIJRI_MABIMS_1992_AGE_HOURS);
  /* Both parameters are topocentric. The criterion derives from Odeh (2004),
   * whose elongation is topocentric, and pairing a geocentric elongation with
   * a topocentric altitude is geometrically incoherent -- the two would be
   * measured from different origins. T. Djamaluddin, formerly head of LAPAN
   * and a principal author of Indonesia's criteria, states it directly:
   * "Kriteria Baru MABIMS adalah tinggi bulan toposentrik 3 derajat dan
   * elongasi toposentrik 6,4 derajat."
   *
   * moon_center_geometric_altitude_deg is already topocentric -- it comes from
   * hijri_moon_altitude(), which applies hijri_moon_topocentric(). "Geometric"
   * here means unrefracted, not geocentric.
   *
   * Still undocumented by any primary source: whether the altitude is to the
   * Moon's centre or upper limb, whether refraction applies, and whether
   * exactly 3.0 deg passes. Centre and no-refraction are assumed. */
  case HIJRI_PREDICATE_MABIMS_2021:
    return p->moon_center_geometric_altitude_deg >= HIJRI_MABIMS_2021_ALTITUDE_DEG &&
           p->topocentric_elongation_deg >= HIJRI_MABIMS_2021_ELONGATION_DEG;
  /* Pedoman Hisab Muhammadiyah (Majelis Tarjih dan Tajdid, 2009), pp.
   * 88-95: h'b = (hb - Pb) + R'b + SDb + Dip, upper limb, apparent,
   * topocentric; the month begins when ijtimak precedes sunset and
   * h'b > 0. No dip term appears below deliberately: the book credits the
   * altitude with +Dip but also computes SUNSET with dip (later for an
   * elevated observer), and the two effects cancel to ~1 arcminute --
   * verified against the book's own worked example, which this library
   * reproduces to 0.4' (see tests/test_hijri.c and
   * docs/research/2026-08-01-wujudul-hilal-convention.md). Adding dip on
   * the altitude side alone would deviate from the source by ~17'. */
  case HIJRI_PREDICATE_WUJUDUL_HILAL:
    return p->conjunction_before_sunset &&
           p->moon_upper_limb_apparent_altitude_deg > HIJRI_WUJUDUL_HILAL_LIMB_DEG;
  case HIJRI_PREDICATE_LAG_AT_LEAST_5_MINUTES:
    return p->moonset_status == HIJRI_EVENT_OK &&
           p->lag_time_minutes >= HIJRI_LAG_THRESHOLD_MINUTES;
  /* Neutrally named research predicate -- claims no authority, so no external
   * convention governs it. Stated explicitly for the record: the elongation
   * here is geocentric. That is a choice, not a requirement; a caller wanting
   * the topocentric form should read topocentric_elongation_deg directly. */
  case HIJRI_PREDICATE_ALTITUDE_5_ELONGATION_8:
    return p->moon_center_geometric_altitude_deg >= HIJRI_RESEARCH_ALTITUDE_DEG &&
           p->geocentric_elongation_deg >= HIJRI_RESEARCH_ELONGATION_DEG;
  case HIJRI_PREDICATE_CONJUNCTION_AND_MOONSET:
    return p->conjunction_before_sunset &&
           p->moonset_status == HIJRI_EVENT_OK &&
           p->moonset_after_sunset;
  default:
    return 0;
  }
}

/* No isnan branch anywhere below, deliberately. When value is NAN the margin
 * is NAN and both > and >= against it are already false, so passes falls out
 * as 0 without a special case. A branch here would be a second place for the
 * NAN policy to drift from the predicate's. */
static HijriDecisionTermMargin
hijri__decision_term(HijriDecisionTerm term, HijriUnit unit, double value,
                     double threshold, int strict) {
  HijriDecisionTermMargin row;
  row.term = term;
  row.unit = unit;
  row.value = value;
  row.threshold = threshold;
  row.margin = value - threshold;
  row.strict = strict;
  row.passes = strict ? (row.margin > 0.0) : (row.margin >= 0.0);
  return row;
}

/* Mirrors the switch above, term for term, reading the same constants. The
 * `moonset_status == HIJRI_EVENT_OK` guard in two of those predicates is not
 * a term, it is availability, and the NAN margin already expresses it: when
 * moonset was not found lag_time_minutes is NAN, so the lag term reports a
 * NAN margin and does not pass. Likewise conjunction_before_sunset is exactly
 * jd_relevant_conjunction_ut < jd_sunset_ut, which is moon_age_hours > 0
 * strictly, and moonset_after_sunset is lag_time_minutes > 0 on the same
 * reasoning, so both appear here as strict terms against zero.
 *
 * tests/test_hijri.c writes each predicate's boolean structure a second time
 * and recombines these per-term results with it. That duplication is the
 * point: it is what detects this function drifting from the switch above. */
HIJRIDEF HijriDecisionMargins
hijri_predicate_margins(HijriLocalPredicate predicate,
                        const HijriEveningParameters *p) {
  HijriDecisionMargins out;
  int index;

  for (index = 0; index < HIJRI_MAX_DECISION_TERMS; index++)
    out.terms[index] = hijri__decision_term(HIJRI_TERM_MOON_CENTER_ALTITUDE,
                                            HIJRI_UNIT_DEGREES, NAN, NAN, 0);
  out.count = 0;

  switch (predicate) {
  case HIJRI_PREDICATE_MABIMS_1992:
    out.count = 3;
    out.terms[0] = hijri__decision_term(
        HIJRI_TERM_MOON_CENTER_ALTITUDE, HIJRI_UNIT_DEGREES,
        p->moon_center_geometric_altitude_deg,
        HIJRI_MABIMS_1992_ALTITUDE_DEG, 0);
    out.terms[1] = hijri__decision_term(
        HIJRI_TERM_GEOCENTRIC_ELONGATION, HIJRI_UNIT_DEGREES,
        p->geocentric_elongation_deg, HIJRI_MABIMS_1992_ELONGATION_DEG, 0);
    out.terms[2] = hijri__decision_term(HIJRI_TERM_MOON_AGE, HIJRI_UNIT_HOURS,
                                        p->moon_age_hours,
                                        HIJRI_MABIMS_1992_AGE_HOURS, 0);
    break;
  case HIJRI_PREDICATE_MABIMS_2021:
    out.count = 2;
    out.terms[0] = hijri__decision_term(
        HIJRI_TERM_MOON_CENTER_ALTITUDE, HIJRI_UNIT_DEGREES,
        p->moon_center_geometric_altitude_deg,
        HIJRI_MABIMS_2021_ALTITUDE_DEG, 0);
    out.terms[1] = hijri__decision_term(
        HIJRI_TERM_TOPOCENTRIC_ELONGATION, HIJRI_UNIT_DEGREES,
        p->topocentric_elongation_deg, HIJRI_MABIMS_2021_ELONGATION_DEG, 0);
    break;
  case HIJRI_PREDICATE_WUJUDUL_HILAL:
    out.count = 2;
    out.terms[0] = hijri__decision_term(HIJRI_TERM_MOON_AGE, HIJRI_UNIT_HOURS,
                                        p->moon_age_hours, 0.0, 1);
    out.terms[1] = hijri__decision_term(
        HIJRI_TERM_MOON_UPPER_LIMB_ALTITUDE, HIJRI_UNIT_DEGREES,
        p->moon_upper_limb_apparent_altitude_deg,
        HIJRI_WUJUDUL_HILAL_LIMB_DEG, 1);
    break;
  case HIJRI_PREDICATE_LAG_AT_LEAST_5_MINUTES:
    out.count = 1;
    out.terms[0] = hijri__decision_term(HIJRI_TERM_LAG_TIME,
                                        HIJRI_UNIT_MINUTES,
                                        p->lag_time_minutes,
                                        HIJRI_LAG_THRESHOLD_MINUTES, 0);
    break;
  case HIJRI_PREDICATE_ALTITUDE_5_ELONGATION_8:
    out.count = 2;
    out.terms[0] = hijri__decision_term(
        HIJRI_TERM_MOON_CENTER_ALTITUDE, HIJRI_UNIT_DEGREES,
        p->moon_center_geometric_altitude_deg, HIJRI_RESEARCH_ALTITUDE_DEG,
        0);
    out.terms[1] = hijri__decision_term(
        HIJRI_TERM_GEOCENTRIC_ELONGATION, HIJRI_UNIT_DEGREES,
        p->geocentric_elongation_deg, HIJRI_RESEARCH_ELONGATION_DEG, 0);
    break;
  case HIJRI_PREDICATE_CONJUNCTION_AND_MOONSET:
    out.count = 2;
    out.terms[0] = hijri__decision_term(HIJRI_TERM_MOON_AGE, HIJRI_UNIT_HOURS,
                                        p->moon_age_hours, 0.0, 1);
    out.terms[1] = hijri__decision_term(HIJRI_TERM_LAG_TIME,
                                        HIJRI_UNIT_MINUTES,
                                        p->lag_time_minutes, 0.0, 1);
    break;
  default:
    break;
  }

  return out;
}

HIJRIDEF double hijri_yallop_q(double arcv_deg, double w) {
  return (arcv_deg -
          (11.8371 - 6.3226 * w + 0.7319 * w * w - 0.1018 * w * w * w)) /
         10.0;
}

HIJRIDEF HijriYallopZone hijri_yallop_classify(double arcv_deg, double w) {
  double q = hijri_yallop_q(arcv_deg, w);
  if (q > 0.216)
    return HIJRI_YALLOP_A_EASILY_VISIBLE;
  if (q > -0.014)
    return HIJRI_YALLOP_B_VISIBLE_PERFECT_CONDITIONS;
  if (q > -0.160)
    return HIJRI_YALLOP_C_MAY_NEED_OPTICAL_AID;
  if (q > -0.232)
    return HIJRI_YALLOP_D_NEEDS_OPTICAL_AID;
  if (q > -0.293)
    return HIJRI_YALLOP_E_NOT_VISIBLE_TELESCOPE;
  return HIJRI_YALLOP_F_NOT_VISIBLE_BELOW_LIMIT;
}

HIJRIDEF double hijri_odeh_v(double arcv_deg, double w) {
  return arcv_deg -
         (-0.1018 * w * w * w + 0.7319 * w * w - 6.3226 * w + 7.1651);
}

HIJRIDEF HijriOdehZone hijri_odeh_classify(double arcv_deg, double w) {
  double v = hijri_odeh_v(arcv_deg, w);
  if (v >= 5.65)
    return HIJRI_ODEH_VISIBLE_NAKED_EYE;
  if (v >= 2.0)
    return HIJRI_ODEH_VISIBLE_WITH_OPTICAL_AID_COULD_BE_NAKED_EYE;
  if (v >= -0.96)
    return HIJRI_ODEH_VISIBLE_WITH_OPTICAL_AID_ONLY;
  return HIJRI_ODEH_NOT_VISIBLE;
}

static double
hijri__topocentric_crescent_width_arcmin(const HijriMoonPosition *moon,
                                         double moon_geocentric_altitude_deg,
                                         double elongation_deg,
                                         const HijriLocation *loc) {
  double earth_radii = moon->distance_km / 6378.14;
  double observer_radii = 1.0 + loc->elevation_m / (6378.14 * 1000.0);
  double topocentric_distance =
      sqrt(earth_radii * earth_radii +
           observer_radii * observer_radii -
           2.0 * earth_radii * observer_radii *
               sin(HIJRI__DEG2RAD(moon_geocentric_altitude_deg)));
  double geocentric_semidiameter_deg =
      0.2725076 * moon->horizontal_parallax_deg;
  double topocentric_semidiameter_deg =
      geocentric_semidiameter_deg * earth_radii / topocentric_distance;
  return 60.0 * topocentric_semidiameter_deg *
         (1.0 - cos(HIJRI__DEG2RAD(elongation_deg)));
}

HIJRIDEF HijriYallopResult
hijri_yallop_evaluate_evening(int gy, int gm, int gd,
                              const HijriLocation *loc) {
  HijriEveningParameters p = hijri_compute_evening_parameters(
      gy, gm, gd, loc, &HIJRI_SUNSET_CONVENTION_ASTRONOMICAL);
  HijriYallopResult result;
  result.jd_best_time_ut = NAN;
  result.arcv_deg = NAN;
  result.crescent_width_arcmin = NAN;
  result.q = NAN;
  result.zone = HIJRI_YALLOP_F_NOT_VISIBLE_BELOW_LIMIT;

  if (p.sunset_status == HIJRI_EVENT_OK &&
      p.moonset_status == HIJRI_EVENT_OK) {
    double jd_tt;
    HijriSunPosition sun;
    HijriMoonPosition moon;
    double sun_altitude;
    double moon_geocentric_altitude;
    double moon_ra_topocentric;
    double moon_dec_topocentric;
    double elongation_topocentric;

    result.jd_best_time_ut =
        p.jd_sunset_ut + (p.lag_time_minutes * 4.0 / 9.0) / 1440.0;
    jd_tt = hijri_jd_tt_from_ut(result.jd_best_time_ut);
    sun = hijri_sun_position(jd_tt);
    moon = hijri_moon_position(jd_tt);
    sun_altitude =
        hijri__altitude_gast_deg(sun.right_ascension_deg,
                                 sun.declination_deg, result.jd_best_time_ut,
                                 loc);
    moon_geocentric_altitude =
        hijri__altitude_deg(moon.right_ascension_deg, moon.declination_deg,
                            result.jd_best_time_ut, loc);
    hijri_moon_topocentric(
        &moon, result.jd_best_time_ut, loc->latitude_deg, loc->longitude_deg,
        loc->elevation_m, &moon_ra_topocentric, &moon_dec_topocentric);
    elongation_topocentric = hijri__angular_separation_deg(
        moon_ra_topocentric, moon_dec_topocentric, sun.right_ascension_deg,
        sun.declination_deg);
    result.arcv_deg = moon_geocentric_altitude - sun_altitude;
    result.crescent_width_arcmin =
        hijri__topocentric_crescent_width_arcmin(
            &moon, moon_geocentric_altitude, elongation_topocentric, loc);
    result.q = hijri_yallop_q(result.arcv_deg,
                              result.crescent_width_arcmin);
    result.zone = hijri_yallop_classify(result.arcv_deg,
                                         result.crescent_width_arcmin);
  }
  return result;
}

HIJRIDEF HijriOdehResult
hijri_odeh_evaluate_evening(int gy, int gm, int gd,
                            const HijriLocation *loc) {
  HijriEveningParameters p = hijri_compute_evening_parameters(
      gy, gm, gd, loc, &HIJRI_SUNSET_CONVENTION_ASTRONOMICAL);
  HijriOdehResult result;
  result.jd_best_time_ut = NAN;
  result.arcv_deg = NAN;
  result.crescent_width_arcmin = NAN;
  result.v = NAN;
  result.zone = HIJRI_ODEH_NOT_VISIBLE;

  if (p.sunset_status == HIJRI_EVENT_OK &&
      p.moonset_status == HIJRI_EVENT_OK) {
    double jd_tt;
    HijriSunPosition sun;
    HijriMoonPosition moon;
    double sun_altitude;
    double moon_geocentric_altitude;
    double moon_ra_topocentric;
    double moon_dec_topocentric;
    double moon_topocentric_altitude;
    double elongation_topocentric;

    result.jd_best_time_ut =
        p.jd_sunset_ut + (p.lag_time_minutes * 4.0 / 9.0) / 1440.0;
    jd_tt = hijri_jd_tt_from_ut(result.jd_best_time_ut);
    sun = hijri_sun_position(jd_tt);
    moon = hijri_moon_position(jd_tt);
    sun_altitude =
        hijri__altitude_gast_deg(sun.right_ascension_deg,
                                 sun.declination_deg, result.jd_best_time_ut,
                                 loc);
    moon_geocentric_altitude =
        hijri__altitude_deg(moon.right_ascension_deg, moon.declination_deg,
                            result.jd_best_time_ut, loc);
    hijri_moon_topocentric(
        &moon, result.jd_best_time_ut, loc->latitude_deg, loc->longitude_deg,
        loc->elevation_m, &moon_ra_topocentric, &moon_dec_topocentric);
    moon_topocentric_altitude = hijri__altitude_deg(
        moon_ra_topocentric, moon_dec_topocentric, result.jd_best_time_ut, loc);
    elongation_topocentric = hijri__angular_separation_deg(
        moon_ra_topocentric, moon_dec_topocentric, sun.right_ascension_deg,
        sun.declination_deg);
    result.arcv_deg = moon_topocentric_altitude - sun_altitude;
    result.crescent_width_arcmin =
        hijri__topocentric_crescent_width_arcmin(
            &moon, moon_geocentric_altitude, elongation_topocentric, loc);
    result.v =
        hijri_odeh_v(result.arcv_deg, result.crescent_width_arcmin);
    result.zone =
        hijri_odeh_classify(result.arcv_deg, result.crescent_width_arcmin);
  }
  return result;
}

/* ---- Tabular calendar
 * --------------------------------------------------------- */

#define HIJRI__TABULAR_EPOCH_JD 1948439.5

static int hijri__is_leap_year(int year) {
  long y = year;
  long m = ((11 * y + 14) % 30 + 30) % 30;
  return m < 11;
}

static int hijri__year_length(int year) {
  return hijri__is_leap_year(year) ? 355 : 354;
}

static int hijri__month_length(int year, int month) {
  if (month == 12)
    return hijri__is_leap_year(year) ? 30 : 29;
  return (month % 2 == 1) ? 30 : 29;
}

/* Floor division. C's / truncates toward zero, so -6 / 30 is 0 rather than
 * the -1 the cycle index needs. Negative Hijri years are inside this
 * function's domain, so the distinction is load bearing. */
static long hijri__floor_div(long a, long b) {
  long q = a / b;
  if ((a % b != 0) && ((a < 0) != (b < 0)))
    q--;
  return q;
}

#define HIJRI__TABULAR_MIN_YEAR (-999999)
#define HIJRI__TABULAR_MAX_YEAR 999999

/* Representable and safe range: the Julian Days whose implied Hijri year fits
 * in an int with room to spare. Outside this, hijri_tabular_from_jd returns
 * the {0, 0, 0} sentinel rather than overflowing. This is wider than the
 * SUPPORTED and TESTED range, which is Hijri years 1 through 9999. The two are
 * different claims and are stated separately on purpose.
 *
 * HIJRI__TABULAR_MIN_JD is the Julian Day of the first day, month 1 day 1, of
 * HIJRI__TABULAR_MIN_YEAR, and HIJRI__TABULAR_MAX_JD is the Julian Day of the
 * last day of month 12 of HIJRI__TABULAR_MAX_YEAR. A test asserts this, so
 * the four constants must be changed together. */
#define HIJRI__TABULAR_MIN_JD (-352418227.5)
#define HIJRI__TABULAR_MAX_JD 356314750.5

HIJRIDEF int hijri_tabular_date_valid(HijriDate date) {
  if (date.year < HIJRI__TABULAR_MIN_YEAR || date.year > HIJRI__TABULAR_MAX_YEAR)
    return 0;
  if (date.month < 1 || date.month > 12)
    return 0;
  if (date.day < 1 || date.day > hijri__month_length(date.year, date.month))
    return 0;
  return 1;
}

HIJRIDEF double hijri_tabular_to_jd(HijriDate date) {
  long days = 0;

  /* The 30-year cycle is exactly 10631 days, so the walk only has to cover
   * the partial cycle. At most 29 iterations instead of one per year. */
  long cycles = hijri__floor_div((long)date.year - 1, 30);
  days = cycles * 10631;
  for (int y = (int)(cycles * 30) + 1; y < date.year; y++)
    days += hijri__year_length(y);

  for (int m = 1; m < date.month; m++)
    days += hijri__month_length(date.year, m);
  days += date.day - 1;

  return HIJRI__TABULAR_EPOCH_JD + (double)days;
}

HIJRIDEF HijriDate hijri_tabular_from_jd(double jd) {
  HijriDate invalid = {0, 0, 0};
  long days_elapsed, cycles, rem;
  int year, month;

  /* (long)floor(NAN) is undefined behaviour, and it is reachable: the
   * evening parameters set jd_sunset_ut and jd_moonset_ut to NAN whenever
   * their solver fails, so a caller who skips the status field lands here. */
  if (!(jd >= HIJRI__TABULAR_MIN_JD && jd <= HIJRI__TABULAR_MAX_JD))
    return invalid;

  days_elapsed = (long)floor(jd - HIJRI__TABULAR_EPOCH_JD + 0.5);
  cycles = hijri__floor_div(days_elapsed, 10631);
  year = (int)(cycles * 30) + 1;
  rem = days_elapsed - cycles * 10631; /* 0 <= rem < 10631 */

  while (rem >= hijri__year_length(year)) { /* at most 29 iterations */
    rem -= hijri__year_length(year);
    year++;
  }

  month = 1;
  while (rem >= hijri__month_length(year, month)) {
    rem -= hijri__month_length(year, month);
    month++;
  }

  {
    HijriDate result;
    result.year = year;
    result.month = month;
    result.day = (int)rem + 1;
    return result;
  }
}

/* ---- Top-level orchestration ----------------------------------------- */

HIJRIDEF HijriMonthDecision
hijri_evaluate_evening(int gy, int gm, int gd, const HijriLocation *loc,
                       HijriLocalPredicate predicate) {
  HijriMonthDecision result;
  const HijriSunsetConvention *conv;
  switch (predicate) {
  case HIJRI_PREDICATE_WUJUDUL_HILAL:
    conv = &HIJRI_SUNSET_CONVENTION_MUHAMMADIYAH;
    break;
  case HIJRI_PREDICATE_MABIMS_2021:
  case HIJRI_PREDICATE_MABIMS_1992:
    conv = &HIJRI_SUNSET_CONVENTION_KEMENAG;
    break;
  default:
    conv = &HIJRI_SUNSET_CONVENTION_ASTRONOMICAL;
    break;
  }
  result.month_starts_next_day = 0;
  result.parameters = hijri_compute_evening_parameters(gy, gm, gd, loc, conv);

  if (result.parameters.sunset_status == HIJRI_EVENT_OK) {
    result.month_starts_next_day =
        hijri_local_predicate_evaluate(predicate, &result.parameters);
  }
  return result;
}

static double hijri__find_month_start_after_conjunction(
    double jd_conj, const HijriLocation *loc, HijriLocalPredicate predicate) {
  const int MAX_FORWARD_DAYS = 5;
  for (int k = 0; k < MAX_FORWARD_DAYS; k++) {
    double eve_jd = floor(jd_conj) + (double)k;
    int ey, em;
    double ed_frac;
    hijri_gregorian_from_jd(eve_jd, &ey, &em, &ed_frac);
    int ed = (int)floor(ed_frac + 0.5);

    HijriMonthDecision decision =
        hijri_evaluate_evening(ey, em, ed, loc, predicate);
    if (decision.parameters.sunset_status == HIJRI_EVENT_OK &&
        decision.month_starts_next_day) {
      return eve_jd + 1.0;
    }
  }
  return NAN;
}

HIJRIDEF int hijri_from_gregorian_with_local_predicate(
    int gy, int gm, int gd, const HijriLocation *loc,
    HijriLocalPredicate predicate, HijriDate *out) {
  double target_jd =
      floor(hijri_jd_from_gregorian(gy, gm, (double)gd));
  double jd_conjunction;
  double jd_month_start;
  int day_number;

  if (hijri_find_relevant_conjunction(target_jd + 1.0, &jd_conjunction) !=
      HIJRI_EVENT_OK) {
    return 0;
  }
  jd_month_start = hijri__find_month_start_after_conjunction(
      jd_conjunction, loc, predicate);

  if (isnan(jd_month_start) || target_jd < jd_month_start) {
    if (hijri_find_previous_conjunction(jd_conjunction - 1.0,
                                        &jd_conjunction) != HIJRI_EVENT_OK) {
      return 0;
    }
    jd_month_start = hijri__find_month_start_after_conjunction(
        jd_conjunction, loc, predicate);
    if (isnan(jd_month_start) || target_jd < jd_month_start) {
      return 0;
    }
  }

  day_number = (int)(target_jd - jd_month_start) + 1;
  if (day_number < 1 || day_number > 30) {
    return 0;
  }

  /* Sample the tabular calendar a few days *into* the month, not on the
   * boundary day itself -- the tabular calendar's own boundary can sit
   * a day off from the astronomically-resolved one, which would
   * otherwise misassign the month number (see README/commit notes). */
  HijriDate approx = hijri_tabular_from_jd(jd_month_start + 5.0);

  out->year = approx.year;
  out->month = approx.month;
  out->day = day_number;
  return 1;
}

/* ---- Umm al-Qura official table -----------------------------------------
 * The Umm al-Qura calendar is published by fiat by KACST; it is a table, not
 * a computation, and for several months it departs from its own stated
 * crescent criterion -- so no astronomical reconstruction can reproduce it
 * exactly (measured ceiling 96.5%; see
 * docs/research/2026-08-01-umm-al-qura-oracle.md).
 *
 * One 12-bit value per Hijri year 1300-1600 AH: bit (11 - (month-1)) set
 * means that month has 30 days, clear means 29. Derived from the ICU/CLDR
 * `islamic-umalqura` calendar -- the table shipped in browsers and operating
 * systems -- which was validated against five independently known Saudi
 * dates. Verified byte-identical to UMALQURA_MONTHLENGTH in ICU's
 * islamcal.cpp (Unicode license). Regeneration command recorded in the
 * research note above. */
#define HIJRI__UQ_YEAR_FIRST 1300
#define HIJRI__UQ_YEAR_COUNT 301
/* floor(JD) of 1882-11-12, which is 1 Muharram 1300. */
#define HIJRI__UQ_ANCHOR_JD 2408761.0
/* Sum of all month lengths below; the day after the table's last day. */
#define HIJRI__UQ_TOTAL_DAYS 106665

static const unsigned short hijri__uq_month_length[HIJRI__UQ_YEAR_COUNT] = {
    0x0AAA, 0x0D54, 0x0EC9, 0x06D4, 0x06EA, 0x036C, 0x0AAD, 0x0555,
    0x06A9, 0x0792, 0x0BA9, 0x05D4, 0x0ADA, 0x055C, 0x0D2D, 0x0695,
    0x074A, 0x0B54, 0x0B6A, 0x05AD, 0x04AE, 0x0A4F, 0x0517, 0x068B,
    0x06A5, 0x0AD5, 0x02D6, 0x095B, 0x049D, 0x0A4D, 0x0D26, 0x0D95,
    0x05AC, 0x09B6, 0x02BA, 0x0A5B, 0x052B, 0x0A95, 0x06CA, 0x0AE9,
    0x02F4, 0x0976, 0x02B6, 0x0956, 0x0ACA, 0x0BA4, 0x0BD2, 0x05D9,
    0x02DC, 0x096D, 0x054D, 0x0AA5, 0x0B52, 0x0BA5, 0x05B4, 0x09B6,
    0x0557, 0x0297, 0x054B, 0x06A3, 0x0752, 0x0B65, 0x056A, 0x0AAB,
    0x052B, 0x0C95, 0x0D4A, 0x0DA5, 0x05CA, 0x0AD6, 0x0957, 0x04AB,
    0x094B, 0x0AA5, 0x0B52, 0x0B6A, 0x0575, 0x0276, 0x08B7, 0x045B,
    0x0555, 0x05A9, 0x05B4, 0x09DA, 0x04DD, 0x026E, 0x0936, 0x0AAA,
    0x0D54, 0x0DB2, 0x05D5, 0x02DA, 0x095B, 0x04AB, 0x0A55, 0x0B49,
    0x0B64, 0x0B71, 0x05B4, 0x0AB5, 0x0A55, 0x0D25, 0x0E92, 0x0EC9,
    0x06D4, 0x0AE9, 0x096B, 0x04AB, 0x0A93, 0x0D49, 0x0DA4, 0x0DB2,
    0x0AB9, 0x04BA, 0x0A5B, 0x052B, 0x0A95, 0x0B2A, 0x0B55, 0x055C,
    0x04BD, 0x023D, 0x091D, 0x0A95, 0x0B4A, 0x0B5A, 0x056D, 0x02B6,
    0x093B, 0x049B, 0x0655, 0x06A9, 0x0754, 0x0B6A, 0x056C, 0x0AAD,
    0x0555, 0x0B29, 0x0B92, 0x0BA9, 0x05D4, 0x0ADA, 0x055A, 0x0AAB,
    0x0595, 0x0749, 0x0764, 0x0BAA, 0x05B5, 0x02B6, 0x0A56, 0x0E4D,
    0x0B25, 0x0B52, 0x0B6A, 0x05AD, 0x02AE, 0x092F, 0x0497, 0x064B,
    0x06A5, 0x06AC, 0x0AD6, 0x055D, 0x049D, 0x0A4D, 0x0D16, 0x0D95,
    0x05AA, 0x05B5, 0x02DA, 0x095B, 0x04AD, 0x0595, 0x06CA, 0x06E4,
    0x0AEA, 0x04F5, 0x02B6, 0x0956, 0x0AAA, 0x0B54, 0x0BD2, 0x05D9,
    0x02EA, 0x096D, 0x04AD, 0x0A95, 0x0B4A, 0x0BA5, 0x05B2, 0x09B5,
    0x04D6, 0x0A97, 0x0547, 0x0693, 0x0749, 0x0B55, 0x056A, 0x0A6B,
    0x052B, 0x0A8B, 0x0D46, 0x0DA3, 0x05CA, 0x0AD6, 0x04DB, 0x026B,
    0x094B, 0x0AA5, 0x0B52, 0x0B69, 0x0575, 0x0176, 0x08B7, 0x025B,
    0x052B, 0x0565, 0x05B4, 0x09DA, 0x04ED, 0x016D, 0x08B6, 0x0AA6,
    0x0D52, 0x0DA9, 0x05D4, 0x0ADA, 0x095B, 0x04AB, 0x0653, 0x0729,
    0x0762, 0x0BA9, 0x05B2, 0x0AB5, 0x0555, 0x0B25, 0x0D92, 0x0EC9,
    0x06D2, 0x0AE9, 0x056B, 0x04AB, 0x0A55, 0x0D29, 0x0D54, 0x0DAA,
    0x09B5, 0x04BA, 0x0A3B, 0x049B, 0x0A4D, 0x0AAA, 0x0AD5, 0x02DA,
    0x095D, 0x045E, 0x0A2E, 0x0C9A, 0x0D55, 0x06B2, 0x06B9, 0x04BA,
    0x0A5D, 0x052D, 0x0A95, 0x0B52, 0x0BA8, 0x0BB4, 0x05B9, 0x02DA,
    0x095A, 0x0B4A, 0x0DA4, 0x0ED1, 0x06E8, 0x0B6A, 0x056D, 0x0535,
    0x0695, 0x0D4A, 0x0DA8, 0x0DD4, 0x06DA, 0x055B, 0x029D, 0x062B,
    0x0B15, 0x0B4A, 0x0B95, 0x05AA, 0x0AAE, 0x092E, 0x0C8F, 0x0527,
    0x0695, 0x06AA, 0x0AD6, 0x055D, 0x029D,
};

static int hijri__uq_year_days(int index) {
  int days = 348; /* twelve 29-day months */
  int m;
  for (m = 0; m < 12; m++) {
    if (hijri__uq_month_length[index] & (1 << (11 - m))) {
      days++;
    }
  }
  return days;
}

/* Exact conversion for dates inside the table; returns 0 outside it. */
static int hijri__umm_al_qura_tabular(double target_jd, HijriDate *out) {
  long days = (long)(target_jd - HIJRI__UQ_ANCHOR_JD);
  int index = 0;
  int m;

  if (days < 0 || days >= HIJRI__UQ_TOTAL_DAYS) {
    return 0;
  }
  for (;;) {
    int year_len = hijri__uq_year_days(index);
    if (days < year_len) {
      break;
    }
    days -= year_len;
    index++;
  }
  for (m = 0; m < 12; m++) {
    int month_len =
        (hijri__uq_month_length[index] & (1 << (11 - m))) ? 30 : 29;
    if (days < month_len) {
      break;
    }
    days -= month_len;
  }
  out->year = HIJRI__UQ_YEAR_FIRST + index;
  out->month = m + 1;
  out->day = (int)days + 1;
  return 1;
}

HIJRIDEF int hijri_umm_al_qura_from_gregorian(int gy, int gm, int gd,
                                               HijriDate *out) {
  double target_jd = floor(hijri_jd_from_gregorian(gy, gm, (double)gd));
  if (hijri__umm_al_qura_tabular(target_jd, out)) {
    return 1;
  }
  /* Before 1882-11-12 or after 2174-11-25: fall back to the astronomical
   * reconstruction, which is exactly what this function was before the
   * table existed. */
  return hijri_from_gregorian_with_local_predicate(
      gy, gm, gd, &HIJRI_LOCATION_MECCA,
      HIJRI_PREDICATE_CONJUNCTION_AND_MOONSET, out);
}

HIJRIDEF int hijri_umm_al_qura_covers(int gy, int gm, int gd) {
  double target_jd = floor(hijri_jd_from_gregorian(gy, gm, (double)gd));
  return target_jd >= HIJRI__UQ_ANCHOR_JD &&
         target_jd < HIJRI__UQ_ANCHOR_JD + (double)HIJRI__UQ_TOTAL_DAYS;
}

#endif /* HIJRI_IMPLEMENTATION */

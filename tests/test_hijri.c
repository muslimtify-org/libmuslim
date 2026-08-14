#define HIJRI_IMPLEMENTATION
#include "../hijri.h"

#include <math.h>
#include <stdio.h>

static int checks;
static int failures;

static void check_int(const char *name, int actual, int expected) {
  checks++;
  if (actual != expected) {
    failures++;
    printf("FAIL exact/%s actual=%d expected=%d\n", name, actual, expected);
  }
}

static void check_close(const char *name, double actual, double expected,
                        double tolerance) {
  double difference = fabs(actual - expected);
  checks++;
  if (difference > tolerance) {
    failures++;
    printf("FAIL exact/%s actual=%.12f expected=%.12f diff=%.12f tol=%.12f\n",
           name, actual, expected, difference, tolerance);
  }
}

static void check_date(const char *name, HijriDate actual, int year, int month,
                       int day) {
  checks++;
  if (actual.year != year || actual.month != month || actual.day != day) {
    failures++;
    printf("FAIL exact/%s actual=%04d-%02d-%02d expected=%04d-%02d-%02d\n",
           name, actual.year, actual.month, actual.day, year, month, day);
  }
}

static void check_true(const char *name, int condition) {
  checks++;
  if (!condition) {
    failures++;
    printf("FAIL synthetic/%s expected=true\n", name);
  }
}

static HijriEveningParameters parameters(double center_altitude,
                                          double upper_limb_altitude,
                                          double geocentric_elongation,
                                          double topocentric_elongation,
                                          double age, double lag,
                                          int conjunction_before,
                                          int moonset_after) {
  HijriEveningParameters value = {
      0.0, 0.0, 0.0, HIJRI_EVENT_OK, HIJRI_EVENT_OK, 0.0, 0.0,
      0.0, 0.0, 0.0, 0.0, 0.0, 0,              0};
  value.sunset_status = HIJRI_EVENT_OK;
  value.moonset_status = HIJRI_EVENT_OK;
  value.moon_center_geometric_altitude_deg = center_altitude;
  value.moon_upper_limb_apparent_altitude_deg = upper_limb_altitude;
  value.geocentric_elongation_deg = geocentric_elongation;
  value.topocentric_elongation_deg = topocentric_elongation;
  value.moon_age_hours = age;
  value.lag_time_minutes = lag;
  value.conjunction_before_sunset = conjunction_before;
  value.moonset_after_sunset = moonset_after;
  return value;
}

static void test_julian_day(void) {
  int year;
  int month;
  double day;

  check_close("j2000", hijri_jd_from_gregorian(2000, 1, 1.5), 2451545.0,
              1e-9);
  check_close("unix_epoch", hijri_jd_from_gregorian(1970, 1, 1.0),
              2440587.5, 1e-9);
  check_close("gregorian_reform",
              hijri_jd_from_gregorian(1582, 10, 15.0), 2299160.5, 1e-9);

  hijri_gregorian_from_jd(2451545.0, &year, &month, &day);
  check_int("j2000_year", year, 2000);
  check_int("j2000_month", month, 1);
  check_close("j2000_day", day, 1.5, 1e-9);

  check_int("weekday_2000_01_01",
            hijri_jd_weekday(hijri_jd_from_gregorian(2000, 1, 1.0)), 6);
  check_close("centuries_j2000", hijri_julian_centuries(2451545.0), 0.0,
              1e-15);
}

static void test_tabular_calendar(void) {
  HijriDate epoch = {1, 1, 1};
  HijriDate last_common = {1, 12, 29};
  HijriDate first_second_year = {2, 1, 1};
  HijriDate leap_end = {2, 12, 30};

  check_close("tabular_epoch_jd", hijri_tabular_to_jd(epoch), 1948439.5,
              1e-9);
  check_date("tabular_epoch_inverse", hijri_tabular_from_jd(1948439.5), 1, 1,
             1);
  check_close("year_1_last_day", hijri_tabular_to_jd(last_common), 1948792.5,
              1e-9);
  check_close("year_2_first_day", hijri_tabular_to_jd(first_second_year),
              1948793.5, 1e-9);
  check_close("year_2_leap_end", hijri_tabular_to_jd(leap_end), 1949147.5,
              1e-9);
  check_date("year_3_first_day", hijri_tabular_from_jd(1949148.5), 3, 1, 1);

  {
    const HijriDate samples[] = {
        {1, 1, 1}, {2, 12, 30}, {30, 12, 29}, {1442, 9, 1}, {1500, 1, 1}};
    size_t index;
    for (index = 0; index < sizeof(samples) / sizeof(samples[0]); index++) {
      HijriDate result =
          hijri_tabular_from_jd(hijri_tabular_to_jd(samples[index]));
      char name[48];
      snprintf(name, sizeof(name), "tabular_roundtrip_%lu",
               (unsigned long)index);
      check_date(name, result, samples[index].year, samples[index].month,
                 samples[index].day);
    }
  }
}

static void check_predicate(const char *name, HijriLocalPredicate predicate,
                            HijriEveningParameters value, int expected) {
  check_int(name, hijri_local_predicate_evaluate(predicate, &value), expected);
}

static void test_binary_criteria(void) {
  HijriEveningParameters unavailable_moonset =
      parameters(0, 0, 0, 0, 0, 5.0, 0, 0);
  unavailable_moonset.moonset_status = HIJRI_EVENT_NEVER_SETS;

  check_predicate("mabims_1992_below", HIJRI_PREDICATE_MABIMS_1992,
                  parameters(1.999, 0, 2.999, 99, 7.999, 0, 0, 0), 0);
  check_predicate("mabims_1992_geocentric_equal",
                  HIJRI_PREDICATE_MABIMS_1992,
                  parameters(2.0, 0, 3.0, 2.0, 0, 0, 0, 0), 1);
  check_predicate("mabims_1992_age_equal", HIJRI_PREDICATE_MABIMS_1992,
                  parameters(0, 0, 0, 0, 8.0, 0, 0, 0), 1);
  check_predicate("mabims_1992_altitude_above",
                  HIJRI_PREDICATE_MABIMS_1992,
                  parameters(2.001, 0, 3.0, 0, 0, 0, 0, 0), 1);
  check_predicate("mabims_1992_elongation_above",
                  HIJRI_PREDICATE_MABIMS_1992,
                  parameters(2.0, 0, 3.001, 0, 0, 0, 0, 0), 1);
  check_predicate("mabims_1992_age_above",
                  HIJRI_PREDICATE_MABIMS_1992,
                  parameters(0, 0, 0, 0, 8.001, 0, 0, 0), 1);
  /* MABIMS 2021 uses the TOPOCENTRIC elongation, not the geocentric one.
   * Every case below except the altitude test deliberately sets the geocentric
   * elongation to a value that contradicts the expected result, so that
   * reverting the predicate to geocentric makes these fail rather than pass
   * silently. Argument 3 is geocentric, argument 4 is topocentric. */
  check_predicate("mabims_2021_below_altitude",
                  HIJRI_PREDICATE_MABIMS_2021,
                  parameters(2.999, 0, 99.0, 6.4, 0, 0, 0, 0), 0);
  check_predicate("mabims_2021_below_topocentric",
                  HIJRI_PREDICATE_MABIMS_2021,
                  parameters(3.0, 0, 99.0, 6.399, 0, 0, 0, 0), 0);
  check_predicate("mabims_2021_topocentric_equal",
                  HIJRI_PREDICATE_MABIMS_2021,
                  parameters(3.0, 0, 0.0, 6.4, 0, 0, 0, 0), 1);
  check_predicate("mabims_2021_altitude_above",
                  HIJRI_PREDICATE_MABIMS_2021,
                  parameters(3.001, 0, 0.0, 6.4, 0, 0, 0, 0), 1);
  check_predicate("mabims_2021_elongation_above",
                  HIJRI_PREDICATE_MABIMS_2021,
                  parameters(3.0, 0, 0.0, 6.401, 0, 0, 0, 0), 1);
  check_predicate("wujud_upper_limb_zero", HIJRI_PREDICATE_WUJUDUL_HILAL,
                  parameters(1.0, 0.0, 0, 0, 0, 0, 1, 0), 0);
  check_predicate("wujud_upper_limb_positive",
                  HIJRI_PREDICATE_WUJUDUL_HILAL,
                  parameters(-0.5, 0.001, 0, 0, 0, 0, 1, 0), 1);
  check_predicate("wujud_no_conjunction", HIJRI_PREDICATE_WUJUDUL_HILAL,
                  parameters(1.0, 1.0, 0, 0, 0, 0, 0, 0), 0);
  check_predicate("lag_below", HIJRI_PREDICATE_LAG_AT_LEAST_5_MINUTES,
                  parameters(0, 0, 0, 0, 0, 4.999, 0, 0), 0);
  check_predicate("lag_equal", HIJRI_PREDICATE_LAG_AT_LEAST_5_MINUTES,
                  parameters(0, 0, 0, 0, 0, 5.0, 0, 0), 1);
  check_predicate("lag_unavailable", HIJRI_PREDICATE_LAG_AT_LEAST_5_MINUTES,
                  unavailable_moonset, 0);
  check_predicate("altitude_5_elongation_8_below",
                  HIJRI_PREDICATE_ALTITUDE_5_ELONGATION_8,
                  parameters(4.999, 0, 8.0, 99, 0, 0, 0, 0), 0);
  check_predicate("altitude_5_elongation_8_equal",
                  HIJRI_PREDICATE_ALTITUDE_5_ELONGATION_8,
                  parameters(5.0, 0, 8.0, 7.0, 0, 0, 0, 0), 1);
}

static void test_yallop(void) {
  const double base = 11.8371;
  static const struct {
    double q;
    HijriYallopZone zone;
  } yallop_cases[] = {
      {0.217, HIJRI_YALLOP_A_EASILY_VISIBLE},
      {0.000, HIJRI_YALLOP_B_VISIBLE_PERFECT_CONDITIONS},
      {-0.100, HIJRI_YALLOP_C_MAY_NEED_OPTICAL_AID},
      {-0.200, HIJRI_YALLOP_D_NEEDS_OPTICAL_AID},
      {-0.250, HIJRI_YALLOP_E_NOT_VISIBLE_TELESCOPE},
      {-0.300, HIJRI_YALLOP_F_NOT_VISIBLE_BELOW_LIMIT}};
  static const struct {
    const char *name;
    double boundary;
    HijriYallopZone above;
    HijriYallopZone below;
  } yallop_boundaries[] = {
      {"a_b", 0.216, HIJRI_YALLOP_A_EASILY_VISIBLE,
       HIJRI_YALLOP_B_VISIBLE_PERFECT_CONDITIONS},
      {"b_c", -0.014, HIJRI_YALLOP_B_VISIBLE_PERFECT_CONDITIONS,
       HIJRI_YALLOP_C_MAY_NEED_OPTICAL_AID},
      {"c_d", -0.160, HIJRI_YALLOP_C_MAY_NEED_OPTICAL_AID,
       HIJRI_YALLOP_D_NEEDS_OPTICAL_AID},
      {"d_e", -0.232, HIJRI_YALLOP_D_NEEDS_OPTICAL_AID,
       HIJRI_YALLOP_E_NOT_VISIBLE_TELESCOPE},
      {"e_f", -0.293, HIJRI_YALLOP_E_NOT_VISIBLE_TELESCOPE,
       HIJRI_YALLOP_F_NOT_VISIBLE_BELOW_LIMIT}};
  const HijriLocation jakarta = {-6.2088, 106.8456, 8.0, "Jakarta"};
  const HijriLocation north_pole = {90.0, 0.0, 0.0, "North Pole"};
  HijriEveningParameters evening = hijri_compute_evening_parameters(
      2025, 2, 28, &jakarta, &HIJRI_SUNSET_CONVENTION_ASTRONOMICAL);
  HijriYallopResult result =
      hijri_yallop_evaluate_evening(2025, 2, 28, &jakarta);
  HijriYallopResult unavailable =
      hijri_yallop_evaluate_evening(2025, 6, 21, &north_pole);
  size_t index;

  check_close("yallop_q_zero", hijri_yallop_q(base, 0.0), 0.0, 1e-12);
  for (index = 0; index < sizeof(yallop_cases) / sizeof(yallop_cases[0]);
       index++) {
    char name[48];
    snprintf(name, sizeof(name), "yallop_zone_%lu", (unsigned long)index);
    check_int(name,
              hijri_yallop_classify(base + 10.0 * yallop_cases[index].q, 0.0),
              yallop_cases[index].zone);
  }
  for (index = 0;
       index < sizeof(yallop_boundaries) / sizeof(yallop_boundaries[0]);
       index++) {
    char above_name[64];
    char below_name[64];
    snprintf(above_name, sizeof(above_name), "yallop_boundary_%s_above",
             yallop_boundaries[index].name);
    snprintf(below_name, sizeof(below_name), "yallop_boundary_%s_below",
             yallop_boundaries[index].name);
    check_int(above_name,
              hijri_yallop_classify(
                  base + 10.0 * (yallop_boundaries[index].boundary + 1e-6),
                  0.0),
              yallop_boundaries[index].above);
    check_int(below_name,
              hijri_yallop_classify(
                  base + 10.0 * (yallop_boundaries[index].boundary - 1e-6),
                  0.0),
              yallop_boundaries[index].below);
  }
  check_close("yallop_best_time_relation",
              (result.jd_best_time_ut - evening.jd_sunset_ut) * 1440.0,
              evening.lag_time_minutes * 4.0 / 9.0, 1e-5);
  check_true("yallop_unavailable_best_time_nan",
             isnan(unavailable.jd_best_time_ut));
  check_true("yallop_unavailable_arcv_nan", isnan(unavailable.arcv_deg));
  check_true("yallop_unavailable_width_nan",
             isnan(unavailable.crescent_width_arcmin));
  check_true("yallop_unavailable_q_nan", isnan(unavailable.q));
}

static void test_odeh(void) {
  const double base = 7.1651;
  static const struct {
    double v;
    HijriOdehZone zone;
  } odeh_cases[] = {
      {6.0, HIJRI_ODEH_VISIBLE_NAKED_EYE},
      {3.0, HIJRI_ODEH_VISIBLE_WITH_OPTICAL_AID_COULD_BE_NAKED_EYE},
      {0.0, HIJRI_ODEH_VISIBLE_WITH_OPTICAL_AID_ONLY},
      {-2.0, HIJRI_ODEH_NOT_VISIBLE}};
  const HijriLocation jakarta = {-6.2088, 106.8456, 8.0, "Jakarta"};
  const HijriLocation north_pole = {90.0, 0.0, 0.0, "North Pole"};
  HijriEveningParameters evening = hijri_compute_evening_parameters(
      2025, 2, 28, &jakarta, &HIJRI_SUNSET_CONVENTION_ASTRONOMICAL);
  HijriOdehResult result =
      hijri_odeh_evaluate_evening(2025, 2, 28, &jakarta);
  HijriOdehResult unavailable =
      hijri_odeh_evaluate_evening(2025, 6, 21, &north_pole);
  size_t index;

  check_close("odeh_v_zero", hijri_odeh_v(base, 0.0), 0.0, 1e-12);
  for (index = 0; index < sizeof(odeh_cases) / sizeof(odeh_cases[0]);
       index++) {
    char name[48];
    snprintf(name, sizeof(name), "odeh_zone_%lu", (unsigned long)index);
    check_int(name, hijri_odeh_classify(base + odeh_cases[index].v, 0.0),
              odeh_cases[index].zone);
  }
  check_close("odeh_best_time_relation",
              (result.jd_best_time_ut - evening.jd_sunset_ut) * 1440.0,
              evening.lag_time_minutes * 4.0 / 9.0, 1e-5);
  check_true("odeh_unavailable_best_time_nan",
             isnan(unavailable.jd_best_time_ut));
  check_true("odeh_unavailable_arcv_nan", isnan(unavailable.arcv_deg));
  check_true("odeh_unavailable_width_nan",
             isnan(unavailable.crescent_width_arcmin));
  check_true("odeh_unavailable_v_nan", isnan(unavailable.v));
}

static void test_relevant_conjunction(void) {
  const HijriLocation jakarta = {-6.2088, 106.8456, 8.0, "Jakarta"};
  HijriEveningParameters before = hijri_compute_evening_parameters(
      2026, 2, 17, &jakarta, &HIJRI_SUNSET_CONVENTION_ASTRONOMICAL);
  HijriEveningParameters after = hijri_compute_evening_parameters(
      2025, 2, 28, &jakarta, &HIJRI_SUNSET_CONVENTION_ASTRONOMICAL);

  check_true("before_conjunction_sunset_ok",
             before.sunset_status == HIJRI_EVENT_OK);
  check_true("before_conjunction_negative_age", before.moon_age_hours < 0.0);
  check_int("before_conjunction_order",
            before.conjunction_before_sunset, 0);
  check_true("before_conjunction_not_previous_lunation",
             before.moon_age_hours > -24.0);

  check_true("after_conjunction_sunset_ok",
             after.sunset_status == HIJRI_EVENT_OK);
  check_true("after_conjunction_small_positive_age",
             after.moon_age_hours >= 0.0 && after.moon_age_hours < 24.0);
  check_int("after_conjunction_order", after.conjunction_before_sunset, 1);

  {
    double jd_tt = hijri_jd_tt_from_ut(after.jd_sunset_ut);
    HijriSunPosition sun = hijri_sun_position(jd_tt);
    HijriMoonPosition moon = hijri_moon_position(jd_tt);
    double moon_ra_topo;
    double moon_dec_topo;
    double expected_geocentric;
    double expected_topocentric;
    double expected_center;
    double expected_upper_limb;

    hijri_moon_topocentric(&moon, after.jd_sunset_ut, jakarta.latitude_deg,
                           jakarta.longitude_deg, jakarta.elevation_m,
                           &moon_ra_topo, &moon_dec_topo);
    expected_geocentric = hijri__angular_separation_deg(
        moon.right_ascension_deg, moon.declination_deg,
        sun.right_ascension_deg, sun.declination_deg);
    expected_topocentric = hijri__angular_separation_deg(
        moon_ra_topo, moon_dec_topo,
        sun.right_ascension_deg, sun.declination_deg);
    expected_center = hijri__altitude_deg(
        moon_ra_topo, moon_dec_topo, after.jd_sunset_ut, &jakarta);
    expected_upper_limb =
        expected_center + 0.2725076 * moon.horizontal_parallax_deg +
        HIJRI_SUNSET_CONVENTION_ASTRONOMICAL.refraction_at_horizon_deg;

    check_close("geocentric_elongation_reproduced",
                after.geocentric_elongation_deg, expected_geocentric, 1e-12);
    check_close("topocentric_elongation_reproduced",
                after.topocentric_elongation_deg, expected_topocentric, 1e-12);
    check_close("moon_center_altitude_reproduced",
                after.moon_center_geometric_altitude_deg, expected_center,
                1e-12);
    check_close("moon_upper_limb_reproduced",
                after.moon_upper_limb_apparent_altitude_deg,
                expected_upper_limb, 1e-12);
  }
}

static void test_orchestration(void) {
  const HijriLocation jakarta = {-6.2088, 106.8456, 8.0, "Jakarta"};
  const HijriLocation north_pole = {90.0, 0.0, 0.0, "North Pole"};
  static const struct {
    const char *name;
    int year;
    int month;
    int day;
  } dates[] = {
      {"before_conjunction", 2025, 2, 27},
      {"on_conjunction", 2025, 2, 28},
      {"after_conjunction", 2025, 3, 1}};
  static const struct {
    const char *name;
    HijriLocalPredicate predicate;
  } predicates[] = {
      {"mabims_1992", HIJRI_PREDICATE_MABIMS_1992},
      {"mabims_2021", HIJRI_PREDICATE_MABIMS_2021},
      {"wujudul_hilal", HIJRI_PREDICATE_WUJUDUL_HILAL},
      {"lag_at_least_5_minutes", HIJRI_PREDICATE_LAG_AT_LEAST_5_MINUTES},
      {"altitude_5_elongation_8", HIJRI_PREDICATE_ALTITUDE_5_ELONGATION_8},
      {"conjunction_and_moonset",
       HIJRI_PREDICATE_CONJUNCTION_AND_MOONSET}};
  size_t date_index;
  size_t predicate_index;

  for (date_index = 0; date_index < sizeof(dates) / sizeof(dates[0]);
       date_index++) {
    for (predicate_index = 0;
         predicate_index < sizeof(predicates) / sizeof(predicates[0]);
         predicate_index++) {
      HijriDate out;
      char name[96];
      int ok = hijri_from_gregorian_with_local_predicate(
          dates[date_index].year, dates[date_index].month,
          dates[date_index].day, &jakarta,
          predicates[predicate_index].predicate, &out);
      if (ok) {
        snprintf(name, sizeof(name), "orchestration_%s_%s_month_in_range",
                 dates[date_index].name, predicates[predicate_index].name);
        check_true(name, out.month >= 1 && out.month <= 12);
        snprintf(name, sizeof(name), "orchestration_%s_%s_day_in_range",
                 dates[date_index].name, predicates[predicate_index].name);
        check_true(name, out.day >= 1 && out.day <= 30);
      }
    }
  }

  {
    HijriMonthDecision unavailable = hijri_evaluate_evening(
        2025, 6, 21, &north_pole, HIJRI_PREDICATE_MABIMS_1992);
    check_true("orchestration_high_latitude_sunset_unavailable",
               unavailable.parameters.sunset_status != HIJRI_EVENT_OK);
    check_int("orchestration_unavailable_sunset_cannot_start_month",
              unavailable.month_starts_next_day, 0);
  }
}

/* The evening-parameter entry point takes a civil date and must return THAT
 * date's local evening. It previously passed 0h UT into a sunset search that
 * scans forward only 24 hours, so west of about 90 deg W -- where local sunset
 * falls after 00:00 UT the next day -- the search returned the PREVIOUS local
 * evening. Every predicate, both visibility models and the calendar conversion
 * inherited a one-day error across the Americas and the Pacific, and no test
 * saw it because every fixture site was in the eastern hemisphere.
 *
 * The sweep is the real guard: it fails at 7 of 25 longitudes without the fix.
 * The named sites are there so a failure reports somewhere recognisable. */
static void test_local_evening_date(void) {
  static const int dates[][3] = {{2024, 3, 11}, {2024, 12, 21}};
  static const double lats[] = {-45.0, 0.0, 45.0};
  size_t di, li;
  int lon;

  /* Longitudes every 15 degrees, three latitudes, two dates. Latitudes are
   * kept within +/-45 so sunset always exists and the assertion is
   * unconditional -- an earlier revision skipped unavailable-sunset cases with
   * a check that could never fail, which would have let a regression making
   * sunset unavailable report success. Polar behaviour is asserted separately
   * below. */
  for (di = 0; di < sizeof(dates) / sizeof(dates[0]); di++) {
    for (li = 0; li < sizeof(lats) / sizeof(lats[0]); li++) {
      for (lon = -180; lon <= 180; lon += 15) {
        HijriLocation loc = {lats[li], (double)lon, 0.0, "sweep"};
        HijriEveningParameters p = hijri_compute_evening_parameters(
            dates[di][0], dates[di][1], dates[di][2], &loc,
            &HIJRI_SUNSET_CONVENTION_ASTRONOMICAL);
        char name[80];
        int ly, lm;
        double ld;
        snprintf(name, sizeof(name), "local_evening_%04d%02d%02d_lat%+d_lon%+d",
                 dates[di][0], dates[di][1], dates[di][2], (int)lats[li], lon);
        check_int(name, (int)p.sunset_status, (int)HIJRI_EVENT_OK);
        hijri_gregorian_from_jd(p.jd_sunset_ut + (double)lon / 360.0, &ly, &lm,
                                &ld);
        check_int(name, (int)ld, dates[di][2]);
      }
    }
  }

  /* Named sites, so a failure names somewhere recognisable. The first three are
   * west of 90 W, where the pre-fix code returned the previous local evening. */
  {
    const HijriLocation sites[] = {
        {34.05, -118.24, 71.0, "Los Angeles"},
        {21.31, -157.86, 6.0, "Honolulu"},
        {39.74, -104.98, 1609.0, "Denver"},
        {-33.87, 151.21, 58.0, "Sydney"},
    };
    size_t i;
    for (i = 0; i < sizeof(sites) / sizeof(sites[0]); i++) {
      HijriEveningParameters p = hijri_compute_evening_parameters(
          2024, 3, 11, &sites[i], &HIJRI_SUNSET_CONVENTION_ASTRONOMICAL);
      char name[80];
      int ly, lm;
      double ld;
      snprintf(name, sizeof(name), "local_evening_site_%s", sites[i].name);
      hijri_gregorian_from_jd(p.jd_sunset_ut + sites[i].longitude_deg / 360.0,
                              &ly, &lm, &ld);
      check_int(name, (int)ld, 11);
    }
  }

  /* Local midnight is derived from longitude as mean solar time, not civil
   * time, because hijri.h ships no zone database. These are the worst
   * realistic solar-vs-civil offsets -- zones far from their solar meridian --
   * checked at the June solstice when northern sunsets are latest and the two
   * are most likely to land on different civil days. They do not. This encodes
   * the evidence for that approximation rather than leaving it in prose. */
  {
    const struct {
      double lat, lon, utc_offset_hours;
      const char *name;
    } zones[] = {
        {43.83, 87.62, 8.0, "Urumqi"},   {39.47, 75.99, 8.0, "Kashgar"},
        {42.24, -8.72, 2.0, "Vigo"},     {43.36, -8.41, 2.0, "ACoruna"},
        {61.22, -149.90, -8.0, "Anchorage"}, {51.88, -176.63, -9.0, "Adak"},
    };
    size_t i;
    for (i = 0; i < sizeof(zones) / sizeof(zones[0]); i++) {
      HijriLocation loc = {zones[i].lat, zones[i].lon, 0.0, zones[i].name};
      HijriEveningParameters p = hijri_compute_evening_parameters(
          2024, 6, 21, &loc, &HIJRI_SUNSET_CONVENTION_ASTRONOMICAL);
      char name[80];
      int sy, sm, cy, cm;
      double sd, cd;
      snprintf(name, sizeof(name), "solar_vs_civil_day_%s", zones[i].name);
      check_int(name, (int)p.sunset_status, (int)HIJRI_EVENT_OK);
      hijri_gregorian_from_jd(p.jd_sunset_ut + zones[i].lon / 360.0, &sy, &sm,
                              &sd);
      hijri_gregorian_from_jd(p.jd_sunset_ut + zones[i].utc_offset_hours / 24.0,
                              &cy, &cm, &cd);
      check_int(name, (int)sd, (int)cd);
    }
  }

  /* Polar behaviour, asserted rather than skipped: above the Arctic circle at
   * the June solstice the Sun does not set, and the API must say so. */
  {
    HijriLocation polar = {78.0, 15.0, 0.0, "Svalbard"};
    HijriEveningParameters p = hijri_compute_evening_parameters(
        2024, 6, 21, &polar, &HIJRI_SUNSET_CONVENTION_ASTRONOMICAL);
    check_true("polar_midsummer_reports_no_sunset",
               p.sunset_status != HIJRI_EVENT_OK);
  }
}

static void test_umm_al_qura_policy(void) {
  HijriDate dedicated;
  HijriDate direct;
  int dedicated_ok =
      hijri_umm_al_qura_from_gregorian(2025, 3, 1, &dedicated);
  int direct_ok = hijri_from_gregorian_with_local_predicate(
      2025, 3, 1, &HIJRI_LOCATION_MECCA,
      HIJRI_PREDICATE_CONJUNCTION_AND_MOONSET, &direct);
  check_int("umm_dedicated_status", dedicated_ok, direct_ok);
  if (dedicated_ok && direct_ok)
    check_true("umm_dedicated_matches_mecca",
               dedicated.year == direct.year &&
               dedicated.month == direct.month &&
               dedicated.day == direct.day);
}

/* Official Umm al-Qura month starts, 2015-2030.
 *
 * Source: the `islamic-umalqura` calendar in ICU/CLDR -- the Umm al-Qura table
 * that ships in browsers and operating systems -- enumerated with Node's
 * Intl.DateTimeFormat. Regeneration command and the cross-check that validates
 * it are recorded in docs/research/.
 *
 * The oracle was validated against five Saudi dates known independently of any
 * calendar table before being admitted:
 *   2025-03-01 = 1 Ramadan 1446   (fasting began)
 *   2025-03-30 = 1 Shawwal 1446   (Eid al-Fitr)
 *   2024-03-11 = 1 Ramadan 1445   (fasting began)
 *   2024-04-10 = 1 Shawwal 1445   (Eid al-Fitr)
 *   2023-06-27 = 9 Dhu al-Hijjah 1444 (Day of Arafah)
 * This matters: an earlier attempt at this work used a hand-parsed table whose
 * epoch was misread by one day, and the resulting 95%-disagreement "finding"
 * was entirely an artifact of the reference. Cross-check the oracle, always.
 *
 * Columns: Gregorian year, month, day; expected Hijri year, month. Every row
 * is day 1 of the named Hijri month by construction. */
static const int HIJRI_UMM_OFFICIAL[][5] = {
  {2015, 1, 21, 1436, 4},
  {2015, 2, 20, 1436, 5},
  {2015, 3, 21, 1436, 6},
  {2015, 4, 20, 1436, 7},
  {2015, 5, 19, 1436, 8},
  {2015, 6, 18, 1436, 9},
  {2015, 7, 17, 1436, 10},
  {2015, 8, 16, 1436, 11},
  {2015, 9, 14, 1436, 12},
  {2015, 10, 14, 1437, 1},
  {2015, 11, 13, 1437, 2},
  {2015, 12, 12, 1437, 3},
  {2016, 1, 11, 1437, 4},
  {2016, 2, 10, 1437, 5},
  {2016, 3, 10, 1437, 6},
  {2016, 4, 8, 1437, 7},
  {2016, 5, 8, 1437, 8},
  {2016, 6, 6, 1437, 9},
  {2016, 7, 6, 1437, 10},
  {2016, 8, 4, 1437, 11},
  {2016, 9, 2, 1437, 12},
  {2016, 10, 2, 1438, 1},
  {2016, 11, 1, 1438, 2},
  {2016, 11, 30, 1438, 3},
  {2016, 12, 30, 1438, 4},
  {2017, 1, 29, 1438, 5},
  {2017, 2, 28, 1438, 6},
  {2017, 3, 29, 1438, 7},
  {2017, 4, 27, 1438, 8},
  {2017, 5, 27, 1438, 9},
  {2017, 6, 25, 1438, 10},
  {2017, 7, 24, 1438, 11},
  {2017, 8, 23, 1438, 12},
  {2017, 9, 21, 1439, 1},
  {2017, 10, 21, 1439, 2},
  {2017, 11, 19, 1439, 3},
  {2017, 12, 19, 1439, 4},
  {2018, 1, 18, 1439, 5},
  {2018, 2, 17, 1439, 6},
  {2018, 3, 18, 1439, 7},
  {2018, 4, 17, 1439, 8},
  {2018, 5, 16, 1439, 9},
  {2018, 6, 15, 1439, 10},
  {2018, 7, 14, 1439, 11},
  {2018, 8, 12, 1439, 12},
  {2018, 9, 11, 1440, 1},
  {2018, 10, 10, 1440, 2},
  {2018, 11, 9, 1440, 3},
  {2018, 12, 8, 1440, 4},
  {2019, 1, 7, 1440, 5},
  {2019, 2, 6, 1440, 6},
  {2019, 3, 8, 1440, 7},
  {2019, 4, 6, 1440, 8},
  {2019, 5, 6, 1440, 9},
  {2019, 6, 4, 1440, 10},
  {2019, 7, 4, 1440, 11},
  {2019, 8, 2, 1440, 12},
  {2019, 8, 31, 1441, 1},
  {2019, 9, 30, 1441, 2},
  {2019, 10, 29, 1441, 3},
  {2019, 11, 28, 1441, 4},
  {2019, 12, 27, 1441, 5},
  {2020, 1, 26, 1441, 6},
  {2020, 2, 25, 1441, 7},
  {2020, 3, 25, 1441, 8},
  {2020, 4, 24, 1441, 9},
  {2020, 5, 24, 1441, 10},
  {2020, 6, 22, 1441, 11},
  {2020, 7, 22, 1441, 12},
  {2020, 8, 20, 1442, 1},
  {2020, 9, 18, 1442, 2},
  {2020, 10, 18, 1442, 3},
  {2020, 11, 16, 1442, 4},
  {2020, 12, 16, 1442, 5},
  {2021, 1, 14, 1442, 6},
  {2021, 2, 13, 1442, 7},
  {2021, 3, 14, 1442, 8},
  {2021, 4, 13, 1442, 9},
  {2021, 5, 13, 1442, 10},
  {2021, 6, 11, 1442, 11},
  {2021, 7, 11, 1442, 12},
  {2021, 8, 9, 1443, 1},
  {2021, 9, 8, 1443, 2},
  {2021, 10, 7, 1443, 3},
  {2021, 11, 6, 1443, 4},
  {2021, 12, 5, 1443, 5},
  {2022, 1, 4, 1443, 6},
  {2022, 2, 2, 1443, 7},
  {2022, 3, 4, 1443, 8},
  {2022, 4, 2, 1443, 9},
  {2022, 5, 2, 1443, 10},
  {2022, 5, 31, 1443, 11},
  {2022, 6, 30, 1443, 12},
  {2022, 7, 30, 1444, 1},
  {2022, 8, 28, 1444, 2},
  {2022, 9, 27, 1444, 3},
  {2022, 10, 26, 1444, 4},
  {2022, 11, 25, 1444, 5},
  {2022, 12, 25, 1444, 6},
  {2023, 1, 23, 1444, 7},
  {2023, 2, 21, 1444, 8},
  {2023, 3, 23, 1444, 9},
  {2023, 4, 21, 1444, 10},
  {2023, 5, 21, 1444, 11},
  {2023, 6, 19, 1444, 12},
  {2023, 7, 19, 1445, 1},
  {2023, 8, 17, 1445, 2},
  {2023, 9, 16, 1445, 3},
  {2023, 10, 16, 1445, 4},
  {2023, 11, 15, 1445, 5},
  {2023, 12, 14, 1445, 6},
  {2024, 1, 13, 1445, 7},
  {2024, 2, 11, 1445, 8},
  {2024, 3, 11, 1445, 9},
  {2024, 4, 10, 1445, 10},
  {2024, 5, 9, 1445, 11},
  {2024, 6, 7, 1445, 12},
  {2024, 7, 7, 1446, 1},
  {2024, 8, 5, 1446, 2},
  {2024, 9, 4, 1446, 3},
  {2024, 10, 4, 1446, 4},
  {2024, 11, 3, 1446, 5},
  {2024, 12, 2, 1446, 6},
  {2025, 1, 1, 1446, 7},
  {2025, 1, 31, 1446, 8},
  {2025, 3, 1, 1446, 9},
  {2025, 3, 30, 1446, 10},
  {2025, 4, 29, 1446, 11},
  {2025, 5, 28, 1446, 12},
  {2025, 6, 26, 1447, 1},
  {2025, 7, 26, 1447, 2},
  {2025, 8, 24, 1447, 3},
  {2025, 9, 23, 1447, 4},
  {2025, 10, 23, 1447, 5},
  {2025, 11, 22, 1447, 6},
  {2025, 12, 21, 1447, 7},
  {2026, 1, 20, 1447, 8},
  {2026, 2, 18, 1447, 9},
  {2026, 3, 20, 1447, 10},
  {2026, 4, 18, 1447, 11},
  {2026, 5, 18, 1447, 12},
  {2026, 6, 16, 1448, 1},
  {2026, 7, 15, 1448, 2},
  {2026, 8, 14, 1448, 3},
  {2026, 9, 12, 1448, 4},
  {2026, 10, 12, 1448, 5},
  {2026, 11, 11, 1448, 6},
  {2026, 12, 10, 1448, 7},
  {2027, 1, 9, 1448, 8},
  {2027, 2, 8, 1448, 9},
  {2027, 3, 9, 1448, 10},
  {2027, 4, 8, 1448, 11},
  {2027, 5, 7, 1448, 12},
  {2027, 6, 6, 1449, 1},
  {2027, 7, 5, 1449, 2},
  {2027, 8, 3, 1449, 3},
  {2027, 9, 2, 1449, 4},
  {2027, 10, 1, 1449, 5},
  {2027, 10, 31, 1449, 6},
  {2027, 11, 29, 1449, 7},
  {2027, 12, 29, 1449, 8},
  {2028, 1, 28, 1449, 9},
  {2028, 2, 26, 1449, 10},
  {2028, 3, 27, 1449, 11},
  {2028, 4, 26, 1449, 12},
  {2028, 5, 25, 1450, 1},
  {2028, 6, 24, 1450, 2},
  {2028, 7, 23, 1450, 3},
  {2028, 8, 22, 1450, 4},
  {2028, 9, 20, 1450, 5},
  {2028, 10, 19, 1450, 6},
  {2028, 11, 18, 1450, 7},
  {2028, 12, 17, 1450, 8},
  {2029, 1, 16, 1450, 9},
  {2029, 2, 14, 1450, 10},
  {2029, 3, 16, 1450, 11},
  {2029, 4, 15, 1450, 12},
  {2029, 5, 14, 1451, 1},
  {2029, 6, 13, 1451, 2},
  {2029, 7, 13, 1451, 3},
  {2029, 8, 12, 1451, 4},
  {2029, 9, 10, 1451, 5},
  {2029, 10, 9, 1451, 6},
  {2029, 11, 8, 1451, 7},
  {2029, 12, 7, 1451, 8},
  {2030, 1, 5, 1451, 9},
  {2030, 2, 4, 1451, 10},
  {2030, 3, 6, 1451, 11},
  {2030, 4, 4, 1451, 12},
  {2030, 5, 4, 1452, 1},
  {2030, 6, 3, 1452, 2},
  {2030, 7, 2, 1452, 3},
  {2030, 8, 1, 1452, 4},
  {2030, 8, 31, 1452, 5},
  {2030, 9, 29, 1452, 6},
  {2030, 10, 28, 1452, 7},
  {2030, 11, 27, 1452, 8},
  {2030, 12, 26, 1452, 9},
};

/* Asserted as EQUALITY: the conversion is now a lookup into the official
 * table (1300-1600 AH), so every in-range fixture row must be exact. The
 * fixture window 2015-2030 is entirely in range.
 *
 * History, kept because it explains why the table exists at all:
 *   - astronomical reconstruction (per-date scan): 183/198, 92.4%
 *   - sequential chain: 191/198, reverted for non-monotonic days at 60-65N
 *   - 198/198 was proven unreachable from the published rule: for seven
 *     months in the window the official table departs from its own stated
 *     criterion (see docs/research/2026-08-01-umm-al-qura-oracle.md)
 * The table closes the gap by shipping the published facts instead of
 * recomputing them.
 *
 * Mutation-tested on 2026-08-01: flipping one bit of the 1447 table entry
 * (0x0BAA -> 0x0BAB) fails umm_official_exact_month_starts (actual=141
 * expected=198), umm_official_muharram_1448 (actual=1447-12-30
 * expected=1448-01-01), and umm_boundary_last_day (actual=1600-12-29
 * expected=1600-12-30); reverting restores 452 checks, 0 failures. The
 * fixture therefore detects a single-day error in a single table entry. */
static void test_umm_al_qura_official_calendar(void) {
  const size_t total = sizeof(HIJRI_UMM_OFFICIAL) / sizeof(HIJRI_UMM_OFFICIAL[0]);
  size_t index;
  int exact = 0;
  int identity_mismatches = 0;

  for (index = 0; index < total; index++) {
    const int *row = HIJRI_UMM_OFFICIAL[index];
    HijriDate got;
    if (!hijri_umm_al_qura_from_gregorian(row[0], row[1], row[2], &got)) {
      continue;
    }
    if (got.day == 1) {
      exact++;
      if (got.year != row[3] || got.month != row[4]) {
        identity_mismatches++;
      }
    }
  }

  check_int("umm_official_exact_month_starts", exact, 198);

  check_int("umm_official_month_identity_consistent", identity_mismatches, 0);

  /* Named individual dates, so a regression names a date rather than a count. */
  {
    HijriDate d;
    if (hijri_umm_al_qura_from_gregorian(2025, 3, 1, &d)) {
      check_date("umm_official_ramadan_1446", d, 1446, 9, 1);
    }
    if (hijri_umm_al_qura_from_gregorian(2025, 3, 30, &d)) {
      check_date("umm_official_shawwal_1446", d, 1446, 10, 1);
    }
    if (hijri_umm_al_qura_from_gregorian(2026, 6, 16, &d)) {
      check_date("umm_official_muharram_1448", d, 1448, 1, 1);
    }
  }

  /* Table-INDEPENDENT anchors. The fixture above derives from ICU, and so
   * does the embedded table, so the fixture alone cannot detect an
   * ICU-vs-KACST divergence. These two dates come from historically
   * documented dual-dated Saudi events, not from any calendar table:
   *   1953-11-09 = 2 Rabi al-Awwal 1373  (death of King Abdulaziz)
   *   2005-08-01 = 26 Jumada al-Akhirah 1426  (death of King Fahd)
   * Three other documented anchors (1932, 1979, 1992 -- all before the
   * modern Umm al-Qura system took force in 1423 AH) DISAGREE with the
   * retro-computed table by one day and are deliberately not asserted;
   * see docs/research/2026-08-01-umm-al-qura-oracle.md. */
  {
    HijriDate d;
    if (hijri_umm_al_qura_from_gregorian(1953, 11, 9, &d)) {
      check_date("umm_anchor_king_abdulaziz_1373", d, 1373, 3, 2);
    }
    if (hijri_umm_al_qura_from_gregorian(2005, 8, 1, &d)) {
      check_date("umm_anchor_king_fahd_1426", d, 1426, 6, 26);
    }
  }
}

/* The property whose violation killed the reverted chain design: consecutive
 * Gregorian days must map to consecutive Hijri days, or to a day-1 rollover
 * with correct month succession. With the table this is structural; the test
 * guards against any future implementation reintroducing query-dependence.
 *
 * Note: against the astronomical implementation this sweep takes ~11 s
 * (5844 conversions, each solving sunset/moonset). Against the table it is
 * effectively instant. Slow is expected only while the test is failing. */
static void test_umm_al_qura_day_sequence_coherent(void) {
  double jd = hijri_jd_from_gregorian(2015, 1, 1.0);
  double jd_end = hijri_jd_from_gregorian(2030, 12, 31.0);
  int have = 0, py = 0, pm = 0, pd = 0;
  int incoherent = 0, conversion_failures = 0;

  for (; jd <= jd_end; jd += 1.0) {
    int gy, gm;
    double gd_frac;
    HijriDate h;
    hijri_gregorian_from_jd(jd, &gy, &gm, &gd_frac);
    if (!hijri_umm_al_qura_from_gregorian(gy, gm, (int)floor(gd_frac), &h)) {
      conversion_failures++;
      have = 0;
      continue;
    }
    if (have) {
      int ok = (h.year == py && h.month == pm && h.day == pd + 1) ||
               (h.day == 1 &&
                ((h.year == py && h.month == pm + 1) ||
                 (h.year == py + 1 && pm == 12 && h.month == 1)));
      if (!ok) {
        incoherent++;
      }
    }
    have = 1;
    py = h.year;
    pm = h.month;
    pd = h.day;
  }
  check_int("umm_sequence_incoherent_transitions", incoherent, 0);
  check_int("umm_sequence_conversion_failures", conversion_failures, 0);
}

/* Table range: 1 Muharram 1300 (1882-11-12) through 30 Dhu al-Hijjah 1600
 * (2174-11-25). Inside, the answer is the official table, asserted exactly.
 * Outside, the function falls back to the astronomical reconstruction, whose
 * result is asserted only to be a sane date -- the fallback's accuracy is not
 * this test's subject. */
static void test_umm_al_qura_table_boundaries(void) {
  HijriDate h;

  check_int("umm_boundary_first_day_status",
            hijri_umm_al_qura_from_gregorian(1882, 11, 12, &h), 1);
  check_date("umm_boundary_first_day", h, 1300, 1, 1);

  {
    int ok = hijri_umm_al_qura_from_gregorian(1882, 11, 11, &h);
    check_true("umm_boundary_pre_table_sane",
               !ok || (h.day >= 1 && h.day <= 30));
  }

  check_int("umm_boundary_last_day_status",
            hijri_umm_al_qura_from_gregorian(2174, 11, 25, &h), 1);
  check_date("umm_boundary_last_day", h, 1600, 12, 30);

  {
    int ok = hijri_umm_al_qura_from_gregorian(2174, 11, 26, &h);
    check_true("umm_boundary_post_table_sane",
               !ok || (h.day >= 1 && h.day <= 30));
  }
}

/* One Moon-horizon convention: at the instant hijri_find_moonset reports,
 * the Moon's upper limb must sit on the apparent horizon -- the same
 * definition under which moon_upper_limb_apparent_altitude_deg reads zero,
 * and the same shape of convention hijri_find_sunset uses for the Sun.
 * Under the old centre-based moonset the limb reads +1 semidiameter
 * (~0.26-0.28 deg) at the reported moonset, which is what this test
 * rejects.
 *
 * Mutation-tested on 2026-08-01: reverting the solver to the centre
 * convention fails all three checks (jakarta actual=0.275054192314,
 * mecca actual=0.270914380280, london actual=0.262178151012); grafting
 * the superseded dip credit onto the predicate fails wujud_upper_limb_zero
 * (actual=1 expected=0). Reverting either restores 469 checks, 0 failures. */
static void test_moonset_crossing_convention(void) {
  static const struct {
    double lat, lon, elev;
    int y, m, d;
    const char *name;
  } cases[] = {
      {-6.2088, 106.8456, 8.0, 2025, 3, 1, "jakarta"},
      {21.4225, 39.8262, 240.0, 2025, 6, 25, "mecca"},
      {51.5074, -0.1278, 11.0, 2026, 2, 18, "london"},
  };
  size_t index;
  char name[96];

  for (index = 0; index < sizeof(cases) / sizeof(cases[0]); index++) {
    HijriLocation loc = {cases[index].lat, cases[index].lon,
                         cases[index].elev, cases[index].name};
    HijriEveningParameters p = hijri_compute_evening_parameters(
        cases[index].y, cases[index].m, cases[index].d, &loc,
        &HIJRI_SUNSET_CONVENTION_ASTRONOMICAL);
    if (p.moonset_status != HIJRI_EVENT_OK) {
      continue;
    }
    {
      double jd_tt = hijri_jd_tt_from_ut(p.jd_moonset_ut);
      HijriMoonPosition moon = hijri_moon_position(jd_tt);
      double sd = 0.2725076 * moon.horizontal_parallax_deg;
      double ra_topo, dec_topo, center, limb;
      hijri_moon_topocentric(&moon, p.jd_moonset_ut, loc.latitude_deg,
                             loc.longitude_deg, loc.elevation_m, &ra_topo,
                             &dec_topo);
      center = hijri__altitude_deg(ra_topo, dec_topo, p.jd_moonset_ut, &loc);
      limb = center + sd +
             HIJRI_SUNSET_CONVENTION_ASTRONOMICAL.refraction_at_horizon_deg;
      snprintf(name, sizeof(name), "moonset_crossing_%s", cases[index].name);
      check_close(name, limb, 0.0, 0.01);
    }
  }
}

/* On crescent-relevant evenings, "the Moon sets after the Sun" and "the
 * Moon's upper limb is above the horizon at sunset" are the same claim.
 * Scoped to Moon age 0-48 h with sunset and moonset available: on gibbous
 * evenings the day's found moonset is the PREVIOUS night's Moon setting
 * before dawn, so the ordering comparison is legitimately unrelated to the
 * sunset-time altitude there (measured: 2738 of 5844 evenings), and an
 * unscoped test would be asserting noise.
 *
 * The sweep is lunation-driven: for each conjunction 2015-2030 it evaluates
 * the five candidate civil dates around it, which contains every evening
 * that can pass the age filter. Verified to produce IDENTICAL counts,
 * mismatches, and guard-band hits to the full 5,844-day-per-city sweep
 * (379/375/374 crescent evenings; measured 2026-08-01) at about a sixth of
 * the cost. The count floor below keeps the scoping from hollowing the
 * test either way. */
static void test_crescent_equivalence_property(void) {
  static const struct {
    double lat, lon, elev;
    const char *name;
  } cities[] = {
      {-(7.0 + 48.0 / 60.0), 110.0 + 21.0 / 60.0, 90.0, "yogyakarta"},
      {-6.2088, 106.8456, 8.0, "jakarta"},
      {21.4225, 39.8262, 240.0, "mecca"},
  };
  size_t index;
  char name[96];

  for (index = 0; index < sizeof(cities) / sizeof(cities[0]); index++) {
    HijriLocation loc = {cities[index].lat, cities[index].lon,
                         cities[index].elev, cities[index].name};
    double jd_start = hijri_jd_from_gregorian(2015, 1, 1.0);
    double jd_end = hijri_jd_from_gregorian(2030, 12, 31.0);
    int crescent = 0, mismatches = 0, guard_band = 0;
    double conjunction = 0.0;
    int conj_search_failures = 0;
    if (hijri_find_next_conjunction(jd_start - 3.0, &conjunction) !=
        HIJRI_EVENT_OK) {
      conj_search_failures++;
      conjunction = jd_end + 3.0; /* skip the sweep; the counter reports it */
    }

    while (conjunction <= jd_end + 2.0) {
      int day_offset;
      for (day_offset = -1; day_offset <= 3; day_offset++) {
        double jd = floor(conjunction) + (double)day_offset + 0.5;
        int gy, gm;
        double gd_frac;
        HijriEveningParameters p;
        if (jd < jd_start || jd > jd_end) {
          continue;
        }
        hijri_gregorian_from_jd(jd, &gy, &gm, &gd_frac);
        p = hijri_compute_evening_parameters(gy, gm, (int)floor(gd_frac),
                                             &loc,
                                             &HIJRI_SUNSET_CONVENTION_ASTRONOMICAL);
        if (p.sunset_status != HIJRI_EVENT_OK ||
            p.moonset_status != HIJRI_EVENT_OK) {
          continue;
        }
        if (p.moon_age_hours < 0.0 || p.moon_age_hours > 48.0) {
          continue;
        }
        crescent++;
        if (fabs(p.moon_upper_limb_apparent_altitude_deg) < 0.02) {
          guard_band++;
        } else if (p.moonset_after_sunset !=
                   (p.moon_upper_limb_apparent_altitude_deg > 0.0)) {
          mismatches++;
        }
      }
      if (hijri_find_next_conjunction(conjunction + 2.0, &conjunction) !=
          HIJRI_EVENT_OK) {
        conj_search_failures++;
        break;
      }
    }
    snprintf(name, sizeof(name), "equiv_conj_search_failures_%s",
             cities[index].name);
    check_int(name, conj_search_failures, 0);
    snprintf(name, sizeof(name), "equiv_crescent_count_%s",
             cities[index].name);
    check_true(name, crescent >= 350);
    snprintf(name, sizeof(name), "equiv_mismatches_%s", cities[index].name);
    check_int(name, mismatches, 0);
    snprintf(name, sizeof(name), "equiv_guard_band_%s", cities[index].name);
    check_true(name, guard_band <= 2);
  }
}

/* Pedoman Hisab Muhammadiyah (Majelis Tarjih dan Tajdid, 2009), pp. 88-95,
 * worked example: 29 Ramadan 1429 H = 2008-09-29, Yogyakarta (phi -7d48',
 * lambda 110d21', elevation 90 m), printed result h'b = -0d52'03.82".
 *
 * The library's upper-limb quantity omits the book's +Dip credit AND the
 * book's dip-delayed sunset; the two dip effects cancel to ~1', so the
 * library reproduces the printed value directly (measured delta 0.4').
 * The +-3' bound is wide enough for the ephemeris floor and the book's
 * rounding, and tight enough to refute the superseded design that added
 * the dip credit on the altitude side only (that lands 16.7' off).
 * See docs/research/2026-08-01-wujudul-hilal-convention.md. */
/* Task 1 pins hijri_sun_position() against values captured from the
 * pre-refactor header on 2026-08-05, so extracting the nutation term into a
 * shared helper cannot change the solar position. These are printed with
 * %.17g, i.e. round-trip exact for IEEE754 double.
 *
 * The tolerance is 1e-9, not 1e-12, because 1e-12 is a non-portable claim.
 * hijri_sun_position computes L0 = 280.46646 + 36000.76983*T + ..., which
 * reaches 8931.887617 deg at jd_tt 2460322.4 before hijri__norm_deg reduces
 * it. One ulp of that intermediate propagates to 1.933e-12 deg in the
 * returned right ascension. arm64 has FMA in its base ISA and macOS
 * contracts by default, x86-64 does not without -mfma, and that one
 * difference moves the result by exactly that quantum, so a 1e-12 tolerance
 * sits below the smallest difference two architectures can exhibit.
 *
 * Measured on GitHub Actions run 31014887576, macos-latest:
 *   FAIL exact/solar_position_unchanged_ra_2 actual=293.947677491489
 *   expected=293.947677491487 diff=0.000000000002 tol=0.000000000001
 *
 * 1e-9 is about 500x above the 1.933e-12 architecture quantum, and about
 * 5 million times below the 0.00478 deg error a sign or algebra mistake in
 * the shared nutation term would produce, so it still fails loudly for any
 * real defect. .github/workflows/ci.yml already restricts `make baseline`
 * to ubuntu-latest for the same underlying reason, this file now follows
 * that precedent. */
static void test_solar_position_pin(void) {
  HijriSunPosition a = hijri_sun_position(2451545.0);
  HijriSunPosition b = hijri_sun_position(2460322.4);

  check_close("solar_position_unchanged_ra", a.right_ascension_deg,
              281.28235602161232, 1e-9);
  check_close("solar_position_unchanged_dec", a.declination_deg,
              -23.032515829102056, 1e-9);
  check_close("solar_position_unchanged_ra_2", b.right_ascension_deg,
              293.94767749148741, 1e-9);
  check_close("solar_position_unchanged_dec_2", b.declination_deg,
              -21.614342073587583, 1e-9);
}

static void test_pedoman_worked_example(void) {
  HijriLocation yogyakarta = {-(7.0 + 48.0 / 60.0), 110.0 + 21.0 / 60.0,
                              90.0, "Yogyakarta"};
  HijriEveningParameters p = hijri_compute_evening_parameters(
      2008, 9, 29, &yogyakarta, &HIJRI_SUNSET_CONVENTION_MUHAMMADIYAH);
  double book_hb = -(52.0 / 60.0 + 3.82 / 3600.0);

  check_int("pedoman_sunset_available", p.sunset_status, HIJRI_EVENT_OK);
  check_close("pedoman_upper_limb_matches_book",
              p.moon_upper_limb_apparent_altitude_deg, book_hb, 3.0 / 60.0);
  check_int("pedoman_conjunction_before_sunset", p.conjunction_before_sunset,
            1);
}

/* Containment bound for the topocentric elongation against Kemenag's published
 * ranges. Measured worst excursion over these rows, printed by the temporary
 * harness in the preceding commit: 0.0106 deg. Bound below is that rounded up to
 * leave roughly 2x margin.
 *
 * The single excursion is explained rather than absorbed. Sabang sits at the
 * elongation MINIMUM in Safar 1445, where in every other row it sits near the
 * maximum, so the residual is the library's own elongation error against DE440
 * rather than a convention difference. The altitude assertion below needs no
 * tolerance at all. */
#define TOL_KEMENAG_PUBLISHED_ELONG_DEG 0.022

/* Kemenag's own published hilal figures, Ephemeris Hisab Rukyat 2023, page
 * 604, table "DAFTAR WAKTU IJTIMAK TINGGI HILAL ELONGASI PENENTU AWAL BULAN
 * HIJRIAH TAHUN 2023 M". Published by Direktorat Urusan Agama Islam dan
 * Pembinaan Syariah, Kementerian Agama RI, 2022.
 *
 * The book's own footnote states the data conforms to the new MABIMS criteria
 * and comes from the Sinkronisasi Data Hisab Taqwim Standar Indonesia held
 * 20-22 October 2021. That footnote is what makes these rows an authority
 * statement rather than one team's calculation.
 *
 * WHY THIS FIXTURE EXISTS ALONGSIDE test_kemenag_official_calendar. That test
 * validates against announced month starts, which are the OUTPUT of Kemenag's
 * method, so it can only check whether the library's predicate agrees with a
 * yes or no. These rows are the INPUTS the decision is made from, so they
 * check the quantities themselves. Both are kept.
 *
 * UNITS ARE AS PRINTED: degrees and decimal arcminutes, with the sign carried
 * in its own field. Converting to decimal degrees here would hide a
 * transcription error that a reader comparing against the page would catch.
 * Row 8's altitude minimum is printed "-00 21,78" with a comma decimal
 * separator, rendered as a period below.
 *
 * DATE IS THE IJTIMAK DATE, WITH ONE EXCEPTION. The published altitude and
 * elongation describe the sunset of the ijtimak day. Sya'ban 1444 shows why
 * that matters: ijtimak Monday 20 February, published altitude below the 3 deg
 * threshold, so Kemenag applied istikmal and the month began Wednesday 22
 * February rather than Tuesday. Sampling the announced start date instead
 * would read the wrong evening and still produce plausible numbers.
 *
 * The exception is Zulqa'dah 1444, row 5, whose printed footnote reads
 * "Posisi hilal pada tanggal 20 Mei 2023 M / 29 Syawal 1444 H". Its ijtimak
 * falls at 22:53 WIB, after sunset, so its figures describe the FOLLOWING
 * evening. The `d` field below carries the evening actually sampled, which is
 * 20 May for that row and the ijtimak date for every other. */
typedef struct {
  int y, m, d;              /* the evening sampled, see note above */
  int alt_lo_sign;
  double alt_lo_deg, alt_lo_min;
  int alt_hi_sign;
  double alt_hi_deg, alt_hi_min;
  double el_lo_deg, el_lo_min;
  double el_hi_deg, el_hi_min;
  const char *name;
} KemenagPublished;

static const KemenagPublished KEMENAG_PUBLISHED_2023[] = {
  {2023,  1, 22,  1,  6, 34.11,  1,  8,  5.57,  7, 48.85,  9, 16.20, "Rajab 1444"},
  {2023,  2, 20,  1,  1, 19.73,  1,  2, 38.94,  4,  4.89,  4, 33.93, "Sya'ban 1444"},
  {2023,  3, 22,  1,  7,  0.32,  1,  8, 43.21,  7, 56.32,  9, 32.41, "Ramadan 1444"},
  {2023,  4, 20,  1,  0, 51.24,  1,  2, 21.39,  1, 28.72,  3,  5.41, "Syawal 1444"},
  {2023,  5, 20,  1,  5, 18.88,  1,  7, 51.44,  8,  3.17,  9, 31.09, "Zulqa'dah 1444"},
  {2023,  6, 18,  1,  0, 11.78,  1,  2, 21.57,  4, 23.93,  4, 56.21, "Zulhijjah 1444"},
  {2023,  7, 18,  1,  5, 21.77,  1,  7, 29.15,  7, 26.68,  8, 34.15, "Muharram 1445"},
  {2023,  8, 16, -1,  0, 21.78,  1,  1, 26.80,  4,  6.38,  4, 28.11, "Safar 1445"},
  {2023,  9, 15,  1,  2, 25.07,  1,  3, 39.75,  3, 14.72,  4, 17.36, "Rabi'ul Awal 1445"},
  {2023, 10, 15,  1,  5,  1.40,  1,  6,  0.40,  6,  5.48,  7, 34.41, "Rabi'ul Akhir 1445"},
  {2023, 11, 13, -1,  2, 21.07, -1,  0, 47.32,  2, 29.16,  2, 49.66, "Jumadal Ula 1445"},
  {2023, 12, 13,  1,  3, 16.27,  1,  4, 50.36,  6,  0.24,  7, 14.31, "Jumadal Akhirah 1445"},
};

/* MEASURED MUTATION SENSITIVITY of the fixture above and of the assertions in
 * test_kemenag_published_quantities().
 *
 * Each mutation below was applied to the source, the suite was rebuilt with
 * `make test` and run, and the FAIL line quoted is what the binary actually
 * printed. Every mutation was reverted afterwards; nothing in this file carries
 * one. The unmutated suite reports "Hijri tests: 539 checks, 0 failures".
 *
 * Coverage: K1 and K3 attack the fixture data, one per quantity, so a corrupted
 * transcription is caught. K2 attacks the altitude CONVENTION, which is the
 * mutation that shows the altitude assertion discriminates rather than merely
 * passing because the ranges are wide. K4 inverts the geocentric counterfactual,
 * proving that asserting a failure is itself an assertion. K5 mis-dates the one
 * footnoted row, proving the ijtimak-date warning above is load-bearing.
 *
 * K1  Rajab 1444 altitude minimum, 6 34.11 -> 8 34.11 (+2 deg).
 *     CAUGHT
 *       FAIL synthetic/kemenag_published_altitude_contained_Rajab 1444 expected=true
 *     Suite went to "Hijri tests: 539 checks, 1 failures", exit 1.
 *
 * K2  The altitude read by the test, p.moon_center_geometric_altitude_deg ->
 *     p.moon_upper_limb_apparent_altitude_deg, i.e. the convention swapped from
 *     centre-geometric to upper-limb apparent.
 *     CAUGHT, and caught in 10 of the 12 rows:
 *       FAIL synthetic/kemenag_published_altitude_contained_Rajab 1444 expected=true
 *       FAIL synthetic/kemenag_published_altitude_contained_Sya'ban 1444 expected=true
 *       FAIL synthetic/kemenag_published_altitude_contained_Ramadan 1444 expected=true
 *       FAIL synthetic/kemenag_published_altitude_contained_Syawal 1444 expected=true
 *       FAIL synthetic/kemenag_published_altitude_contained_Zulqa'dah 1444 expected=true
 *       FAIL synthetic/kemenag_published_altitude_contained_Zulhijjah 1444 expected=true
 *       FAIL synthetic/kemenag_published_altitude_contained_Muharram 1445 expected=true
 *       FAIL synthetic/kemenag_published_altitude_contained_Safar 1445 expected=true
 *       FAIL synthetic/kemenag_published_altitude_contained_Rabi'ul Awal 1445 expected=true
 *       FAIL synthetic/kemenag_published_altitude_contained_Rabi'ul Akhir 1445 expected=true
 *     Suite went to "Hijri tests: 539 checks, 10 failures", exit 1. Only Jumadal
 *     Ula and Jumadal Akhirah 1445 survive the swap, so the published ranges
 *     admit the upper-limb apparent altitude in 2 rows of 12 against 12 of 12
 *     for the shipped centre-geometric quantity. That 10-row margin is the
 *     evidence that the altitude assertion selects a convention.
 *
 * K3  Safar 1445 elongation minimum, 4 6.38 -> 5 6.38 (+1 deg).
 *     CAUGHT
 *       FAIL synthetic/kemenag_published_elongation_contained_Safar 1445 expected=true
 *     Suite went to "Hijri tests: 539 checks, 1 failures", exit 1.
 *
 * K4  The geocentric counterfactual condition, (g < elo || g > ehi) ->
 *     (g >= elo && g <= ehi), i.e. counting the rows that fall INSIDE the
 *     published range instead of outside it.
 *     CAUGHT
 *       FAIL synthetic/kemenag_published_elongation_is_topocentric expected=true
 *     Suite went to "Hijri tests: 539 checks, 1 failures", exit 1. The majority
 *     that holds for the geocentric form falling outside does not hold when
 *     inverted, so the check is measuring the frame and not a tautology.
 *
 * K5  Zulqa'dah 1444 `d` field, 20 -> 19, the ijtimak date rather than the
 *     evening the published figures describe.
 *     CAUGHT twice, once per quantity:
 *       FAIL synthetic/kemenag_published_altitude_contained_Zulqa'dah 1444 expected=true
 *       FAIL synthetic/kemenag_published_elongation_contained_Zulqa'dah 1444 expected=true
 *     Suite went to "Hijri tests: 539 checks, 2 failures", exit 1. The row's
 *     ijtimak falls at 22:53 WIB, after sunset, so 19 May reads an evening
 *     before conjunction. The date exception documented above is therefore
 *     verified rather than asserted. */

/* Degrees and decimal arcminutes to decimal degrees, sign applied to the whole
 * quantity rather than to the degree field, so "-00 21.78" is negative. */
static double kemenag_dm(int sign, double deg, double min) {
  return sign * (deg + min / 60.0);
}

/* Group: the library against Kemenag's own published quantities.
 *
 * The assertion is CONTAINMENT, not agreement with an endpoint, and the reason
 * is the whole design. The published ranges span Indonesia, and the location
 * producing the extreme moves from month to month, so no fixed reference point
 * reproduces the endpoints. Sabang is inside Indonesia, so if the library
 * computes the same quantity Kemenag publishes, the library's value at Sabang
 * must lie between the published minimum and maximum. That argument needs no
 * assumption about where the endpoints are evaluated.
 *
 * An earlier analysis fitted one reference location to the published maxima and
 * concluded from the RMS that the library's centre-geometric altitude fits
 * worst and that Kemenag's altitude is mar'i. Containment reverses that
 * conclusion, and containment is the sounder test because the fit assumed a
 * fixed extreme location that does not exist. The record is kept here so the
 * fit is not attempted again.
 *
 * The geocentric check asserts a FAILURE on purpose. Kemenag's published
 * elongation is topocentric, and asserting that the geocentric form falls
 * outside the published ranges pins that as a measured fact rather than a
 * citation. It is the check that would catch a future simplification of
 * HIJRI_PREDICATE_MABIMS_2021 to the geocentric elongation. */
static void test_kemenag_published_quantities(void) {
  HijriLocation sabang = {5.8926, 95.3238, 10.0, "Sabang"};
  size_t n = sizeof(KEMENAG_PUBLISHED_2023) / sizeof(KEMENAG_PUBLISHED_2023[0]);
  size_t i;
  int geo_outside = 0;

  for (i = 0; i < n; i++) {
    const KemenagPublished *r = &KEMENAG_PUBLISHED_2023[i];
    HijriEveningParameters p = hijri_compute_evening_parameters(
        r->y, r->m, r->d, &sabang, &HIJRI_SUNSET_CONVENTION_KEMENAG);
    double alo = kemenag_dm(r->alt_lo_sign, r->alt_lo_deg, r->alt_lo_min);
    double ahi = kemenag_dm(r->alt_hi_sign, r->alt_hi_deg, r->alt_hi_min);
    double elo = kemenag_dm(1, r->el_lo_deg, r->el_lo_min);
    double ehi = kemenag_dm(1, r->el_hi_deg, r->el_hi_min);
    char label[96];
    double a, e, g;

    check_int("kemenag_published_sunset_available", p.sunset_status,
              HIJRI_EVENT_OK);
    if (p.sunset_status != HIJRI_EVENT_OK) {
      continue;
    }
    a = p.moon_center_geometric_altitude_deg;
    e = p.topocentric_elongation_deg;
    g = p.geocentric_elongation_deg;

    /* Altitude, exact containment. No tolerance: this separates the candidate
     * conventions, since adding the Moon's semidiameter or refraction pushes
     * rows outside these same ranges. */
    sprintf(label, "kemenag_published_altitude_contained_%s", r->name);
    check_true(label, a >= alo && a <= ahi);

    /* Topocentric elongation, containment within the measured bound. */
    sprintf(label, "kemenag_published_elongation_contained_%s", r->name);
    check_true(label, e >= elo - TOL_KEMENAG_PUBLISHED_ELONG_DEG &&
                          e <= ehi + TOL_KEMENAG_PUBLISHED_ELONG_DEG);

    if (g < elo || g > ehi) {
      geo_outside++;
    }
  }

  /* The geocentric form must fall outside the published range in the MAJORITY
   * of rows. Stated as a majority rather than a count so this does not become a
   * transcription of one run's output. */
  check_true("kemenag_published_elongation_is_topocentric",
             (size_t)geo_outside * 2 > n);
}

/* Official Kemenag month starts, Kalender Hijriah Indonesia 2024-2026
 * (Rajab 1445 through Rajab 1448), Ditjen Bimas Islam.
 *
 * Provenance: transcriptions of the official releases (al-habib.info,
 * cross-checked against detik.com's independent transcription for the two
 * astronomically surprising months), validated BEFORE measurement against
 * seven independently documented government announcements (Ramadan and
 * Syawal 1445, Ramadan and Syawal 1446, Ramadan 1447, Dzulhijjah 1447,
 * Muharram 1448) -- all seven match. Residual risk: a never-announced month
 * mis-transcribed identically by both sources; retrieving the official PDF
 * from the simbi.kemenag.go.id flipbook would close it. Full analysis:
 * docs/research/2026-08-01-kemenag-reference.md.
 *
 * What these tests claim, and deliberately do not claim: the library's
 * single-point MABIMS 2021 predicate is NEVER one day early against the
 * official calendar (0/37, asserted exactly, under both the shipped
 * topocentric-elongation reading and the geocentric reading Kemenag's
 * operational numbers pin), and it supports at least 33/37 official starts
 * at Indonesia's westernmost point. It does NOT reproduce the calendar
 * exactly: two Rajab starts sit ~0.85 deg below the altitude criterion at
 * the westernmost point under every convention tested. Floors are >= so a
 * genuine ephemeris improvement cannot fail the suite; never-early is
 * asserted as equality because it is the claim that matters.
 *
 * Mutation-tested on 2026-08-01: shifting one fixture date by one day
 * fails both never-early checks (actual=1); raising the altitude
 * threshold to 4.0 fails both support floors (support drops to 31).
 * Reverting either restores 475 checks, 0 failures. */
static void test_kemenag_official_calendar(void) {
  static const int KEMENAG_STARTS[][3] = {
      {2024, 1, 13},  {2024, 2, 11}, {2024, 3, 12}, {2024, 4, 10},
      {2024, 5, 10},  {2024, 6, 8},  {2024, 7, 7},  {2024, 8, 6},
      {2024, 9, 5},   {2024, 10, 4}, {2024, 11, 3}, {2024, 12, 3},
      {2025, 1, 1},   {2025, 1, 31}, {2025, 3, 1},  {2025, 3, 31},
      {2025, 4, 29},  {2025, 5, 28}, {2025, 6, 27}, {2025, 7, 26},
      {2025, 8, 25},  {2025, 9, 23}, {2025, 10, 23}, {2025, 11, 22},
      {2025, 12, 21}, {2026, 1, 20}, {2026, 2, 19}, {2026, 3, 21},
      {2026, 4, 19},  {2026, 5, 18}, {2026, 6, 16}, {2026, 7, 16},
      {2026, 8, 14},  {2026, 9, 13}, {2026, 10, 12}, {2026, 11, 11},
      {2026, 12, 10}};
  HijriLocation sabang = {5.8926, 95.3238, 10.0, "Sabang"};
  const size_t total = sizeof(KEMENAG_STARTS) / sizeof(KEMENAG_STARTS[0]);
  size_t index;
  int support_topo = 0, support_geo = 0;
  int early_topo = 0, early_geo = 0;

  for (index = 0; index < total; index++) {
    double start_jd = floor(hijri_jd_from_gregorian(KEMENAG_STARTS[index][0],
                                                    KEMENAG_STARTS[index][1],
                                                    (double)KEMENAG_STARTS[index][2]));
    int offset;
    for (offset = 1; offset <= 2; offset++) {
      int gy, gm;
      double gd_frac;
      HijriEveningParameters p;
      int topo_ok, geo_ok;
      hijri_gregorian_from_jd(start_jd - (double)offset + 0.5, &gy, &gm,
                              &gd_frac);
      p = hijri_compute_evening_parameters(gy, gm, (int)floor(gd_frac),
                                           &sabang,
                                           &HIJRI_SUNSET_CONVENTION_KEMENAG);
      if (p.sunset_status != HIJRI_EVENT_OK) {
        continue;
      }
      topo_ok = (p.moon_center_geometric_altitude_deg >= 3.0 &&
                 p.topocentric_elongation_deg >= 6.4);
      geo_ok = (p.moon_center_geometric_altitude_deg >= 3.0 &&
                p.geocentric_elongation_deg >= 6.4);
      if (offset == 1) {
        support_topo += topo_ok;
        support_geo += geo_ok;
      } else {
        early_topo += topo_ok;
        early_geo += geo_ok;
      }
    }
  }

  check_int("kemenag_never_early_topocentric", early_topo, 0);
  check_int("kemenag_never_early_geocentric", early_geo, 0);
  check_true("kemenag_support_floor_topocentric", support_topo >= 33);
  check_true("kemenag_support_floor_geocentric", support_geo >= 34);

  /* The convention pin. For the 28 Feb 2025 itsbat, Kemenag announced
   * elongation across Indonesia up to 6 deg 24.14 min = 6.4023 deg. At the
   * westernmost point this library computes geocentric 6.3952 (0.43 arcmin
   * from the announcement) while topocentric is a full degree away --
   * Kemenag's operational elongation is geocentric. Kept as a living check
   * so ephemeris drift or a convention regression surfaces here. */
  {
    HijriEveningParameters pin = hijri_compute_evening_parameters(
        2025, 2, 28, &sabang, &HIJRI_SUNSET_CONVENTION_KEMENAG);
    check_close("kemenag_pin_geocentric_matches_announcement",
                pin.geocentric_elongation_deg, 6.4023, 0.05);
    check_true("kemenag_pin_topocentric_is_far_below",
               pin.topocentric_elongation_deg < 5.5);
  }
}

/* Official Muhammadiyah month starts of the wujudul-hilal era, from the
 * primary sources: the annual Maklumat PP Muhammadiyah documents
 * (muhammadiyah.or.id) setting Ramadan, Syawal, and Zulhijah for
 * 1443-1446 H. No transcription layer -- these dates ARE the official
 * announcements. Full analysis and the published hisab pins:
 * docs/research/2026-08-01-muhammadiyah-reference.md.
 *
 * SCOPE BOUNDARY, do not extend: Muhammadiyah used hisab hakiki wujudul
 * hilal through the END of 1446 H and switched to KHGT (a different,
 * global criterion) from 1447 H. Dates from 1447 H onward must never be
 * added to this fixture under HIJRI_PREDICATE_WUJUDUL_HILAL.
 *
 * Asserted as EQUALITIES (support == 12, early == 0), unlike the Kemenag
 * fixture's floors: the measured score is perfect and the tightest margin
 * in the set is ~0.9 deg (the 2024-03-10 razor case: published +0 deg
 * 56' 28", our upper-limb quantity +1.06 deg) -- any change that flips
 * these is a regression this fixture exists to catch. The set includes
 * three 30-day negatives (belum wujud) and covers both failure modes of
 * the criterion (Moon below horizon; ijtimak after the decision evening).
 *
 * Mutation-tested on 2026-08-01: shifting one start date by one day fails
 * muh_never_early (actual=1); raising the predicate's limb threshold to
 * 1.5 deg fails muh_official_starts_supported (actual=10) and the razor
 * pin muh_maklumat_2024_03_10 (actual=0). Reverting either restores
 * 485 checks, 0 failures. */
static void test_muhammadiyah_official_calendar(void) {
  static const int MUH_STARTS[][3] = {
      {2022, 4, 2},  {2022, 5, 2},  {2022, 6, 30},
      {2023, 3, 23}, {2023, 4, 21}, {2023, 6, 19},
      {2024, 3, 11}, {2024, 4, 10}, {2024, 6, 8},
      {2025, 3, 1},  {2025, 3, 31}, {2025, 5, 28}};
  /* Decision evenings whose Maklumat publishes an explicit altitude and
   * wujud/belum-wujud verdict (altitudes quoted in the research note). */
  static const struct {
    int y, m, d, wujud;
    const char *name;
  } MUH_PINS[] = {
      {2022, 4, 1, 1, "muh_maklumat_2022_04_01"},
      {2022, 5, 1, 1, "muh_maklumat_2022_05_01"},
      {2022, 6, 29, 1, "muh_maklumat_2022_06_29"},
      {2024, 3, 10, 1, "muh_maklumat_2024_03_10"},
      {2024, 6, 6, 0, "muh_maklumat_2024_06_06"},
      {2025, 2, 28, 1, "muh_maklumat_2025_02_28"},
      {2025, 3, 29, 0, "muh_maklumat_2025_03_29"},
      {2025, 5, 27, 1, "muh_maklumat_2025_05_27"}};
  HijriLocation yogyakarta = {-(7.0 + 48.0 / 60.0), 110.0 + 21.0 / 60.0,
                              90.0, "Yogyakarta"};
  size_t index;
  int support = 0, early = 0;

  for (index = 0; index < sizeof(MUH_STARTS) / sizeof(MUH_STARTS[0]);
       index++) {
    double start_jd = floor(hijri_jd_from_gregorian(
        MUH_STARTS[index][0], MUH_STARTS[index][1],
        (double)MUH_STARTS[index][2]));
    int offset;
    for (offset = 1; offset <= 2; offset++) {
      int gy, gm;
      double gd_frac;
      HijriMonthDecision decision;
      hijri_gregorian_from_jd(start_jd - (double)offset + 0.5, &gy, &gm,
                              &gd_frac);
      decision = hijri_evaluate_evening(gy, gm, (int)floor(gd_frac),
                                        &yogyakarta,
                                        HIJRI_PREDICATE_WUJUDUL_HILAL);
      if (decision.parameters.sunset_status != HIJRI_EVENT_OK) {
        continue;
      }
      if (offset == 1) {
        support += decision.month_starts_next_day;
      } else {
        early += decision.month_starts_next_day;
      }
    }
  }
  check_int("muh_official_starts_supported", support, 12);
  check_int("muh_never_early", early, 0);

  for (index = 0; index < sizeof(MUH_PINS) / sizeof(MUH_PINS[0]); index++) {
    HijriMonthDecision decision = hijri_evaluate_evening(
        MUH_PINS[index].y, MUH_PINS[index].m, MUH_PINS[index].d, &yogyakarta,
        HIJRI_PREDICATE_WUJUDUL_HILAL);
    check_int(MUH_PINS[index].name, decision.month_starts_next_day,
              MUH_PINS[index].wujud);
  }
}


/* Status-return contract for the conjunction finders (the roadmap's
 * pre-v1.0 gate). Non-finite input reports HIJRI_EVENT_NOT_FOUND with the
 * result untouched; valid input reports OK with a Julian Day bit-identical
 * to the pre-conversion implementation (values captured before the
 * signature change; tolerance 1e-9 day = 86 microseconds, far below the
 * bisection bracket of ~4e-13 day, so any solver change trips it).
 *
 * Mutation-tested on 2026-08-01: stripping the non-finite guard and
 * found-check from hijri_find_previous_conjunction fails
 * conj_status_nan_input (actual=0) and both result-untouched checks;
 * halving the search bisection to 20 iterations fails
 * conj_bitident_next_2024_06_01 (diff 4.7e-8 vs tol 1e-9). Reverting
 * either restores 498 checks, 0 failures. The infinity guard in the
 * search helper is deliberately NOT mutation-tested: removing it makes
 * the scan non-terminating (inf + 0.5 <= inf), a hang rather than a
 * failure. */
static void test_conjunction_status_returns(void) {
  double untouched = 12345.0;
  double jd;

  check_int("conj_status_nan_input",
            hijri_find_previous_conjunction(NAN, &untouched),
            HIJRI_EVENT_NOT_FOUND);
  check_close("conj_status_nan_result_untouched", untouched, 12345.0, 0.0);
  check_int("conj_status_inf_input",
            hijri_find_next_conjunction(INFINITY, &untouched),
            HIJRI_EVENT_NOT_FOUND);
  check_close("conj_status_inf_result_untouched", untouched, 12345.0, 0.0);

  check_int("conj_status_prev_ok",
            hijri_find_previous_conjunction(
                hijri_jd_from_gregorian(2025, 3, 1.0), &jd),
            HIJRI_EVENT_OK);
  check_close("conj_bitident_prev_2025_03_01", jd, 2460734.531549764, 1e-9);
  check_int("conj_status_next_ok",
            hijri_find_next_conjunction(
                hijri_jd_from_gregorian(2024, 6, 1.0), &jd),
            HIJRI_EVENT_OK);
  check_close("conj_bitident_next_2024_06_01", jd, 2460468.026246262, 1e-9);
  check_int("conj_status_relevant_ok",
            hijri_find_relevant_conjunction(
                hijri_jd_from_gregorian(2022, 4, 1.5), &jd),
            HIJRI_EVENT_OK);
  check_close("conj_bitident_relevant_2022_04_01", jd, 2459670.766591638,
              1e-9);
}

int main(void) {
  test_julian_day();
  test_tabular_calendar();
  test_binary_criteria();
  test_yallop();
  test_odeh();
  test_relevant_conjunction();
  test_orchestration();
  test_local_evening_date();
  test_umm_al_qura_policy();
  test_umm_al_qura_official_calendar();
  test_umm_al_qura_day_sequence_coherent();
  test_umm_al_qura_table_boundaries();
  test_moonset_crossing_convention();
  test_crescent_equivalence_property();
  test_solar_position_pin();
  test_pedoman_worked_example();
  test_kemenag_official_calendar();
  test_kemenag_published_quantities();
  test_muhammadiyah_official_calendar();
  test_conjunction_status_returns();
  check_true("all_predicate_enums_represented",
             HIJRI_PREDICATE_CONJUNCTION_AND_MOONSET -
                         HIJRI_PREDICATE_MABIMS_1992 +
                     1 ==
                 6);
  printf("Hijri tests: %d checks, %d failures\n", checks, failures);
  return failures == 0 ? 0 : 1;
}

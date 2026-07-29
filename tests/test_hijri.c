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
  check_predicate("mabims_2021_below_altitude",
                  HIJRI_PREDICATE_MABIMS_2021,
                  parameters(2.999, 0, 6.4, 99, 0, 0, 0, 0), 0);
  check_predicate("mabims_2021_below_geocentric",
                  HIJRI_PREDICATE_MABIMS_2021,
                  parameters(3.0, 0, 6.399, 99, 0, 0, 0, 0), 0);
  check_predicate("mabims_2021_geocentric_equal",
                  HIJRI_PREDICATE_MABIMS_2021,
                  parameters(3.0, 0, 6.4, 5.0, 0, 0, 0, 0), 1);
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
  const HijriLocation jakarta = {-6.2088, 106.8456, 8.0, "Jakarta"};
  const HijriLocation north_pole = {90.0, 0.0, 0.0, "North Pole"};
  HijriEveningParameters evening =
      hijri_compute_evening_parameters(2025, 2, 28, &jakarta);
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
  HijriEveningParameters evening =
      hijri_compute_evening_parameters(2025, 2, 28, &jakarta);
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
  HijriEveningParameters before =
      hijri_compute_evening_parameters(2026, 2, 17, &jakarta);
  HijriEveningParameters after =
      hijri_compute_evening_parameters(2025, 2, 28, &jakarta);

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
}

static void test_orchestration(void) {
  const HijriLocation jakarta = {-6.2088, 106.8456, 8.0, "Jakarta"};
  HijriLocalPredicate predicate;

  for (predicate = HIJRI_PREDICATE_MABIMS_1992;
       predicate <= HIJRI_PREDICATE_CONJUNCTION_AND_MOONSET;
       predicate = (HijriLocalPredicate)(predicate + 1)) {
    HijriDate date;
    int ok =
        hijri_from_gregorian(2025, 3, 1, &jakarta, predicate, &date);
    if (ok) {
      check_true("orchestration_day_in_range",
                 date.day >= 1 && date.day <= 30);
    }
  }
}

static void test_umm_al_qura_policy(void) {
  HijriDate dedicated;
  HijriDate direct;
  int dedicated_ok =
      hijri_umm_al_qura_from_gregorian(2025, 3, 1, &dedicated);
  int direct_ok = hijri_from_gregorian(
      2025, 3, 1, &HIJRI_LOCATION_MECCA,
      HIJRI_PREDICATE_CONJUNCTION_AND_MOONSET, &direct);
  check_int("umm_dedicated_status", dedicated_ok, direct_ok);
  if (dedicated_ok && direct_ok)
    check_true("umm_dedicated_matches_mecca",
               dedicated.year == direct.year &&
               dedicated.month == direct.month &&
               dedicated.day == direct.day);
}

int main(void) {
  test_julian_day();
  test_tabular_calendar();
  test_binary_criteria();
  test_yallop();
  test_odeh();
  test_relevant_conjunction();
  test_orchestration();
  test_umm_al_qura_policy();
  check_true("all_predicate_enums_represented",
             HIJRI_PREDICATE_CONJUNCTION_AND_MOONSET -
                         HIJRI_PREDICATE_MABIMS_1992 +
                     1 ==
                 6);
  printf("Hijri tests: %d checks, %d failures\n", checks, failures);
  return failures == 0 ? 0 : 1;
}

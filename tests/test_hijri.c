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

static HijriHilalParameters parameters(double altitude, double elongation,
                                       double age, double lag,
                                       int conjunction_before,
                                       int moonset_after) {
  HijriHilalParameters value = {0};
  value.moon_altitude_deg = altitude;
  value.elongation_deg = elongation;
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

static void check_criterion(const char *name, HijriCriterion criterion,
                            HijriHilalParameters value, int expected) {
  check_int(name, hijri_criterion_evaluate(criterion, &value), expected);
}

static void test_binary_criteria(void) {
  check_criterion("umm_both", HIJRI_CRIT_UMM_AL_QURA,
                  parameters(0, 0, 0, 0, 1, 1), 1);
  check_criterion("umm_no_conjunction", HIJRI_CRIT_UMM_AL_QURA,
                  parameters(0, 0, 0, 0, 0, 1), 0);
  check_criterion("umm_no_moonset", HIJRI_CRIT_UMM_AL_QURA,
                  parameters(0, 0, 0, 0, 1, 0), 0);

  check_criterion("mabims_1992_below", HIJRI_CRIT_MABIMS_1992,
                  parameters(1.999, 2.999, 7.999, 0, 0, 0), 0);
  check_criterion("mabims_1992_geometry_equal", HIJRI_CRIT_MABIMS_1992,
                  parameters(2.0, 3.0, 0, 0, 0, 0), 1);
  check_criterion("mabims_1992_age_equal", HIJRI_CRIT_MABIMS_1992,
                  parameters(0, 0, 8.0, 0, 0, 0), 1);

  check_criterion("mabims_2021_below_altitude", HIJRI_CRIT_MABIMS_2021,
                  parameters(2.999, 6.4, 0, 0, 0, 0), 0);
  check_criterion("mabims_2021_below_elongation", HIJRI_CRIT_MABIMS_2021,
                  parameters(3.0, 6.399, 0, 0, 0, 0), 0);
  check_criterion("mabims_2021_equal", HIJRI_CRIT_MABIMS_2021,
                  parameters(3.0, 6.4, 0, 0, 0, 0), 1);

  check_criterion("wujud_zero", HIJRI_CRIT_WUJUDUL_HILAL,
                  parameters(0.0, 0, 0, 0, 1, 0), 0);
  check_criterion("wujud_positive", HIJRI_CRIT_WUJUDUL_HILAL,
                  parameters(0.001, 0, 0, 0, 1, 0), 1);
  check_criterion("wujud_no_conjunction", HIJRI_CRIT_WUJUDUL_HILAL,
                  parameters(1.0, 0, 0, 0, 0, 0), 0);

  check_criterion("turkey_below", HIJRI_CRIT_TURKEY_ICOP,
                  parameters(4.999, 8.0, 0, 0, 0, 0), 0);
  check_criterion("turkey_equal", HIJRI_CRIT_TURKEY_ICOP,
                  parameters(5.0, 8.0, 0, 0, 0, 0), 1);
  check_criterion("ecfr_below", HIJRI_CRIT_ECFR_ISNA,
                  parameters(5.0, 7.999, 0, 0, 0, 0), 0);
  check_criterion("ecfr_equal", HIJRI_CRIT_ECFR_ISNA,
                  parameters(5.0, 8.0, 0, 0, 0, 0), 1);

  check_criterion("egypt_below", HIJRI_CRIT_EGYPT,
                  parameters(0, 0, 0, 4.999, 0, 0), 0);
  check_criterion("egypt_equal", HIJRI_CRIT_EGYPT,
                  parameters(0, 0, 0, 5.0, 0, 0), 1);
  check_criterion("egypt_nan", HIJRI_CRIT_EGYPT,
                  parameters(0, 0, 0, NAN, 0, 0), 0);

  check_criterion("odeh_not_boolean", HIJRI_CRIT_ODEH,
                  parameters(99, 99, 99, 99, 1, 1), 0);
  check_criterion("yallop_not_boolean", HIJRI_CRIT_YALLOP,
                  parameters(99, 99, 99, 99, 1, 1), 0);
}

static void test_yallop(void) {
  const double base = 11.8371;
  check_close("yallop_q_zero", hijri_yallop_q(base, 0.0), 0.0, 1e-12);
  check_int("yallop_not_visible",
            hijri_yallop_classify(base + 10.0 * -0.232, 0.0),
            HIJRI_YALLOP_NOT_VISIBLE);
  check_int("yallop_needs_optical",
            hijri_yallop_classify(base + 10.0 * -0.231, 0.0),
            HIJRI_YALLOP_NEEDS_OPTICAL_AID);
  check_int("yallop_may_need_boundary",
            hijri_yallop_classify(
                nextafter(base + 10.0 * -0.160, -INFINITY), 0.0),
            HIJRI_YALLOP_NEEDS_OPTICAL_AID);
  check_int("yallop_may_need",
            hijri_yallop_classify(base + 10.0 * -0.159, 0.0),
            HIJRI_YALLOP_MAY_NEED_OPTICAL_AID);
  check_int("yallop_perfect_boundary",
            hijri_yallop_classify(base + 10.0 * -0.014, 0.0),
            HIJRI_YALLOP_MAY_NEED_OPTICAL_AID);
  check_int("yallop_perfect",
            hijri_yallop_classify(base + 10.0 * -0.013, 0.0),
            HIJRI_YALLOP_VISIBLE_UNDER_PERFECT_CONDITIONS);
  check_int("yallop_easy_boundary",
            hijri_yallop_classify(
                nextafter(base + 10.0 * 0.216, -INFINITY), 0.0),
            HIJRI_YALLOP_VISIBLE_UNDER_PERFECT_CONDITIONS);
  check_int("yallop_easy",
            hijri_yallop_classify(base + 10.0 * 0.217, 0.0),
            HIJRI_YALLOP_EASILY_VISIBLE);
}

static void test_odeh(void) {
  const double base = 7.1651;
  check_close("odeh_v_zero", hijri_odeh_v(base, 0.0), 0.0, 1e-12);
  check_int("odeh_not_visible",
            hijri_odeh_classify(base - 0.961, 0.0),
            HIJRI_ODEH_NOT_VISIBLE);
  check_int("odeh_optical_equal",
            hijri_odeh_classify(base - 0.960, 0.0),
            HIJRI_ODEH_VISIBLE_WITH_OPTICAL_AID_ONLY);
  check_int("odeh_possible_naked_equal",
            hijri_odeh_classify(nextafter(base + 2.0, INFINITY), 0.0),
            HIJRI_ODEH_VISIBLE_WITH_OPTICAL_AID_COULD_BE_NAKED_EYE);
  check_int("odeh_naked_equal",
            hijri_odeh_classify(base + 5.65, 0.0),
            HIJRI_ODEH_VISIBLE_NAKED_EYE);
  check_int("odeh_naked_above",
            hijri_odeh_classify(base + 5.651, 0.0),
            HIJRI_ODEH_VISIBLE_NAKED_EYE);
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

int main(void) {
  test_julian_day();
  test_tabular_calendar();
  test_binary_criteria();
  test_yallop();
  test_odeh();
  test_relevant_conjunction();
  check_true("all_criterion_enums_represented",
             HIJRI_CRIT_YALLOP - HIJRI_CRIT_UMM_AL_QURA + 1 == 9);
  printf("Hijri tests: %d checks, %d failures\n", checks, failures);
  return failures == 0 ? 0 : 1;
}

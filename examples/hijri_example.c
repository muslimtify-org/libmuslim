#define HIJRI_IMPLEMENTATION
#include "../hijri.h"
#include <math.h>
#include <stdio.h>

static void print_date(const char *label, int ok, HijriDate date) {
  if (ok)
    printf("%-32s -> %04d-%02d-%02d AH\n", label, date.year, date.month,
           date.day);
  else
    printf("%-32s -> could not resolve\n", label);
}

int main(void) {
  const HijriLocation jakarta = {-6.2088, 106.8456, 8.0, "Jakarta"};
  HijriDate result;
  HijriMonthDecision wujud;
  HijriEveningParameters parameters;
  HijriYallopResult yallop;
  HijriOdehResult odeh;
  int ok;

  printf("=== Tabular (arithmetic) calendar, no astronomy ===\n");
  result = hijri_tabular_from_jd(hijri_jd_from_gregorian(2026, 7, 27));
  print_date("2026-07-27 (tabular)", 1, result);

  printf("\n=== Local calendar predicates for Jakarta ===\n");
  ok = hijri_from_gregorian_with_local_predicate(
      2026, 7, 27, &jakarta, HIJRI_PREDICATE_MABIMS_1992, &result);
  print_date("MABIMS 1992 local calendar", ok, result);
  ok = hijri_from_gregorian_with_local_predicate(
      2026, 7, 27, &jakarta, HIJRI_PREDICATE_MABIMS_2021, &result);
  print_date("MABIMS 2021 local calendar", ok, result);

  wujud = hijri_evaluate_evening(2026, 2, 17, &jakarta,
                                 HIJRI_PREDICATE_WUJUDUL_HILAL);
  printf("Wujudul Hilal local predicate: %s\n",
         wujud.month_starts_next_day ? "passes" : "does not pass");

  printf("\n=== Dedicated Mecca-based policy ===\n");
  ok = hijri_umm_al_qura_from_gregorian(2026, 7, 27, &result);
  print_date("Umm al-Qura", ok, result);

  printf("\n=== Explicit evening parameters for Jakarta ===\n");
  parameters = hijri_compute_evening_parameters(
      2026, 2, 17, &jakarta, &HIJRI_SUNSET_CONVENTION_ASTRONOMICAL);
  if (parameters.sunset_status == HIJRI_EVENT_OK) {
    printf("Moon center altitude:       %.2f deg\n",
           parameters.moon_center_geometric_altitude_deg);
    printf("Moon upper-limb altitude:   %.2f deg\n",
           parameters.moon_upper_limb_apparent_altitude_deg);
    printf("Geocentric elongation:      %.2f deg\n",
           parameters.geocentric_elongation_deg);
    printf("Topocentric elongation:     %.2f deg\n",
           parameters.topocentric_elongation_deg);
    printf("Signed Moon age:            %.2f hours\n",
           parameters.moon_age_hours);
    printf("Moonset lag:                %.2f minutes\n",
           parameters.lag_time_minutes);
  }

  printf("\n=== Dedicated visibility models for Jakarta ===\n");
  yallop = hijri_yallop_evaluate_evening(2025, 2, 28, &jakarta);
  odeh = hijri_odeh_evaluate_evening(2025, 2, 28, &jakarta);
  if (!isnan(yallop.q))
    printf("Yallop: q = %.3f, zone = %d\n", yallop.q, (int)yallop.zone);
  if (!isnan(odeh.v))
    printf("Odeh:   V = %.3f, zone = %d\n", odeh.v, (int)odeh.zone);

  return 0;
}

#define HIJRI_IMPLEMENTATION
#include "../hijri.h"
#include <math.h>
#include <stdio.h>

static const char *criterion_name(HijriCriterion c) {
  switch (c) {
  case HIJRI_CRIT_UMM_AL_QURA:
    return "Umm al-Qura (Saudi Arabia)";
  case HIJRI_CRIT_MABIMS_1992:
    return "MABIMS 1992 (2 deg / 3 deg / 8h)";
  case HIJRI_CRIT_MABIMS_2021:
    return "MABIMS 2021 (3 deg / 6.4 deg)";
  case HIJRI_CRIT_WUJUDUL_HILAL:
    return "Wujudul Hilal (Muhammadiyah)";
  case HIJRI_CRIT_TURKEY_ICOP:
    return "Turkey / ICOP unified (5 deg / 8 deg)";
  case HIJRI_CRIT_ECFR_ISNA:
    return "ECFR / ISNA (5 deg / 8 deg, global)";
  case HIJRI_CRIT_EGYPT:
    return "Egypt (moonset >=5 min after sunset)";
  default:
    return "?";
  }
}

int main(void) {
  HijriLocation jakarta = {-6.2088, 106.8456, 8.0, "Jakarta"};

  printf("=== Tabular (arithmetic) calendar, no astronomy ===\n");
  HijriDate today_tab =
      hijri_tabular_from_jd(hijri_jd_from_gregorian(2026, 7, 27));
  /* hijri_jd_from_gregorian is declared in hijri_julian.h, pulled in
   * transitively via hijri_calendar.h -> hijri_criteria.h chain isn't
   * guaranteed, so this line assumes it's visible; if you split
   * translation units differently just #include "hijri_julian.h". */
  printf("2026-07-27 (tabular) -> %04d-%02d-%02d AH\n", today_tab.year,
         today_tab.month, today_tab.day);

  printf("\n=== Astronomical criteria, evaluated for Jakarta ===\n");
  HijriCriterion criteria[] = {HIJRI_CRIT_UMM_AL_QURA, HIJRI_CRIT_MABIMS_1992,
                               HIJRI_CRIT_MABIMS_2021, HIJRI_CRIT_WUJUDUL_HILAL,
                               HIJRI_CRIT_TURKEY_ICOP, HIJRI_CRIT_ECFR_ISNA,
                               HIJRI_CRIT_EGYPT};

  for (size_t i = 0; i < sizeof(criteria) / sizeof(criteria[0]); i++) {
    HijriDate result;
    int ok = hijri_from_gregorian(2026, 7, 27, &jakarta, criteria[i], &result);
    if (ok) {
      printf("%-40s -> %04d-%02d-%02d AH\n", criterion_name(criteria[i]),
             result.year, result.month, result.day);
    } else {
      printf("%-40s -> could not resolve\n", criterion_name(criteria[i]));
    }
  }

  printf("\n=== Raw hilal parameters for a specific evening ===\n");
  HijriMonthDecision decision =
      hijri_evaluate_evening(2026, 2, 17, &jakarta, HIJRI_CRIT_MABIMS_2021);
  if (!isnan(decision.jd_sunset_ut)) {
    printf("Moon altitude:   %.2f deg\n",
           decision.parameters.moon_altitude_deg);
    printf("Elongation:      %.2f deg\n", decision.parameters.elongation_deg);
    printf("ARCV:            %.2f deg\n", decision.parameters.arcv_deg);
    printf("Crescent width:  %.2f arcmin\n",
           decision.parameters.crescent_width_arcmin);
    printf("Moon age:        %.2f hours\n", decision.parameters.moon_age_hours);
    printf("Month starts next day (MABIMS 2021)? %s\n",
           decision.month_starts_next_day ? "yes" : "no");
    printf("Yallop zone: %d, Odeh zone: %d\n",
           hijri_yallop_classify(decision.parameters.arcv_deg,
                                 decision.parameters.crescent_width_arcmin),
           hijri_odeh_classify(decision.parameters.arcv_deg,
                               decision.parameters.crescent_width_arcmin));
  }

  return 0;
}

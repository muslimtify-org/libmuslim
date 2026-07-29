#define HIJRI_IMPLEMENTATION
#include "../hijri.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  int year;
  int month;
  int day;
} CivilDate;

typedef struct {
  const char *region;
  HijriLocation location;
} ResearchLocation;

static const CivilDate dates[] = {
    {2020, 5, 22}, {2021, 4, 12}, {2022, 4, 1},
    {2023, 3, 21}, {2024, 4, 8},  {2025, 2, 28}};

static const ResearchLocation locations[] = {
    {"MABIMS", {-6.2088, 106.8456, 8.0, "Jakarta"}},
    {"MABIMS", {3.1390, 101.6869, 66.0, "Kuala Lumpur"}},
    {"MABIMS", {1.3521, 103.8198, 15.0, "Singapore"}},
    {"MABIMS", {4.9031, 114.9398, 9.0, "Bandar Seri Begawan"}},
    {"WUJUD", {-7.7956, 110.3695, 113.0, "Yogyakarta"}},
    {"WUJUD", {-5.1477, 119.4327, 8.0, "Makassar"}},
    {"WUJUD", {-2.5916, 140.6690, 20.0, "Jayapura"}},
    {"UMM_AL_QURA", {21.4225, 39.8262, 240.0, "Mecca"}},
    {"EGYPT", {30.0444, 31.2357, 23.0, "Cairo"}},
    {"EGYPT", {31.2001, 29.9187, 5.0, "Alexandria"}},
    {"EGYPT", {24.0889, 32.8998, 99.0, "Aswan"}},
    {"TURKEY", {41.0082, 28.9784, 40.0, "Istanbul"}},
    {"TURKEY", {39.9334, 32.8597, 938.0, "Ankara"}},
    {"TURKEY", {39.9043, 41.2679, 1890.0, "Erzurum"}},
    {"ECFR_ISNA", {51.5074, -0.1278, 11.0, "London"}},
    {"ECFR_ISNA", {40.7128, -74.0060, 10.0, "New York"}},
    {"ECFR_ISNA", {43.6532, -79.3832, 76.0, "Toronto"}},
    {"SOUTH", {-33.8688, 151.2093, 58.0, "Sydney"}}};

static const char *criterion_name(HijriCriterion criterion) {
  static const char *names[] = {"UMM_AL_QURA", "MABIMS_1992", "MABIMS_2021",
                                "WUJUDUL_HILAL", "TURKEY_ICOP", "ECFR_ISNA",
                                "EGYPT"};
  return names[(int)criterion];
}

static size_t applicable_criteria(const char *region,
                                  HijriCriterion output[2]) {
  if (strcmp(region, "MABIMS") == 0) {
    output[0] = HIJRI_CRIT_MABIMS_1992;
    output[1] = HIJRI_CRIT_MABIMS_2021;
    return 2;
  }
  if (strcmp(region, "WUJUD") == 0)
    output[0] = HIJRI_CRIT_WUJUDUL_HILAL;
  else if (strcmp(region, "UMM_AL_QURA") == 0)
    output[0] = HIJRI_CRIT_UMM_AL_QURA;
  else if (strcmp(region, "EGYPT") == 0)
    output[0] = HIJRI_CRIT_EGYPT;
  else if (strcmp(region, "TURKEY") == 0)
    output[0] = HIJRI_CRIT_TURKEY_ICOP;
  else
    output[0] = HIJRI_CRIT_ECFR_ISNA;
  return 1;
}

int main(void) {
  size_t date_index;
  size_t location_index;

  puts("date,region,location,latitude,longitude,elevation,criterion,"
       "sunset_jd,conjunction_jd,moonset_jd,moon_altitude,sun_altitude,"
       "arcv,elongation,crescent_width,moon_age,lag,decision,yallop,odeh");

  for (date_index = 0; date_index < sizeof(dates) / sizeof(dates[0]);
       date_index++) {
    for (location_index = 0;
         location_index < sizeof(locations) / sizeof(locations[0]);
         location_index++) {
      HijriCriterion criteria[2];
      size_t criterion_count =
          applicable_criteria(locations[location_index].region, criteria);
      size_t criterion_index;
      for (criterion_index = 0; criterion_index < criterion_count;
           criterion_index++) {
        const CivilDate date = dates[date_index];
        const ResearchLocation site = locations[location_index];
        const HijriCriterion criterion = criteria[criterion_index];
        HijriMonthDecision result = hijri_evaluate_evening(
            date.year, date.month, date.day, &site.location, criterion);

        printf("%04d-%02d-%02d,%s,%s,%.4f,%.4f,%.1f,%s,"
               "%.9f,%.9f,%.9f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,"
               "%.6f,%d,%d,%d\n",
               date.year, date.month, date.day, site.region,
               site.location.name, site.location.latitude_deg,
               site.location.longitude_deg, site.location.elevation_m,
               criterion_name(criterion), result.jd_sunset_ut,
               result.jd_conjunction_ut, result.jd_moonset_ut,
               result.parameters.moon_altitude_deg,
               result.parameters.sun_altitude_deg, result.parameters.arcv_deg,
               result.parameters.elongation_deg,
               result.parameters.crescent_width_arcmin,
               result.parameters.moon_age_hours,
               result.parameters.lag_time_minutes,
               result.month_starts_next_day,
               (int)hijri_yallop_classify(
                   result.parameters.arcv_deg,
                   result.parameters.crescent_width_arcmin),
               (int)hijri_odeh_classify(
                   result.parameters.arcv_deg,
                   result.parameters.crescent_width_arcmin));
      }
    }
  }
  return 0;
}

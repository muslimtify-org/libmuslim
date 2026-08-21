#define HIJRI_IMPLEMENTATION
#include "../hijri.h"

#include <math.h>
#include <stdio.h>

typedef struct {
  int year;
  int month;
  int day;
} CivilDate;

typedef struct {
  const char *group;
  HijriLocation location;
  HijriLocalPredicate predicates[2];
  size_t predicate_count;
} ResearchLocation;

static const CivilDate dates[] = {
    {2020, 5, 22}, {2021, 4, 12}, {2022, 4, 1},
    {2023, 3, 21}, {2024, 4, 8},  {2025, 2, 28}};

static const ResearchLocation locations[] = {
    {"SOUTHEAST_ASIA",
     {-6.2088, 106.8456, 8.0, "Jakarta"},
     {HIJRI_PREDICATE_MABIMS_1992, HIJRI_PREDICATE_MABIMS_2021},
     2},
    {"SOUTHEAST_ASIA",
     {3.1390, 101.6869, 66.0, "Kuala Lumpur"},
     {HIJRI_PREDICATE_MABIMS_1992, HIJRI_PREDICATE_MABIMS_2021},
     2},
    {"SOUTHEAST_ASIA",
     {1.3521, 103.8198, 15.0, "Singapore"},
     {HIJRI_PREDICATE_MABIMS_1992, HIJRI_PREDICATE_MABIMS_2021},
     2},
    {"SOUTHEAST_ASIA",
     {4.9031, 114.9398, 9.0, "Bandar Seri Begawan"},
     {HIJRI_PREDICATE_MABIMS_1992, HIJRI_PREDICATE_MABIMS_2021},
     2},
    {"SOUTHEAST_ASIA",
     {-7.7956, 110.3695, 113.0, "Yogyakarta"},
     {HIJRI_PREDICATE_WUJUDUL_HILAL, HIJRI_PREDICATE_WUJUDUL_HILAL},
     1},
    {"SOUTHEAST_ASIA",
     {-5.1477, 119.4327, 8.0, "Makassar"},
     {HIJRI_PREDICATE_WUJUDUL_HILAL, HIJRI_PREDICATE_WUJUDUL_HILAL},
     1},
    {"SOUTHEAST_ASIA",
     {-2.5916, 140.6690, 20.0, "Jayapura"},
     {HIJRI_PREDICATE_WUJUDUL_HILAL, HIJRI_PREDICATE_WUJUDUL_HILAL},
     1},
    {"WEST_ASIA",
     {21.4225, 39.8262, 240.0, "Mecca"},
     {HIJRI_PREDICATE_CONJUNCTION_AND_MOONSET,
      HIJRI_PREDICATE_CONJUNCTION_AND_MOONSET},
     1},
    {"NORTH_AFRICA",
     {30.0444, 31.2357, 23.0, "Cairo"},
     {HIJRI_PREDICATE_LAG_AT_LEAST_5_MINUTES,
      HIJRI_PREDICATE_LAG_AT_LEAST_5_MINUTES},
     1},
    {"NORTH_AFRICA",
     {31.2001, 29.9187, 5.0, "Alexandria"},
     {HIJRI_PREDICATE_LAG_AT_LEAST_5_MINUTES,
      HIJRI_PREDICATE_LAG_AT_LEAST_5_MINUTES},
     1},
    {"NORTH_AFRICA",
     {24.0889, 32.8998, 99.0, "Aswan"},
     {HIJRI_PREDICATE_LAG_AT_LEAST_5_MINUTES,
      HIJRI_PREDICATE_LAG_AT_LEAST_5_MINUTES},
     1},
    {"WEST_ASIA",
     {41.0082, 28.9784, 40.0, "Istanbul"},
     {HIJRI_PREDICATE_ALTITUDE_5_ELONGATION_8,
      HIJRI_PREDICATE_ALTITUDE_5_ELONGATION_8},
     1},
    {"WEST_ASIA",
     {39.9334, 32.8597, 938.0, "Ankara"},
     {HIJRI_PREDICATE_ALTITUDE_5_ELONGATION_8,
      HIJRI_PREDICATE_ALTITUDE_5_ELONGATION_8},
     1},
    {"WEST_ASIA",
     {39.9043, 41.2679, 1890.0, "Erzurum"},
     {HIJRI_PREDICATE_ALTITUDE_5_ELONGATION_8,
      HIJRI_PREDICATE_ALTITUDE_5_ELONGATION_8},
     1},
    {"EUROPE",
     {51.5074, -0.1278, 11.0, "London"},
     {HIJRI_PREDICATE_ALTITUDE_5_ELONGATION_8,
      HIJRI_PREDICATE_ALTITUDE_5_ELONGATION_8},
     1},
    {"NORTH_AMERICA",
     {40.7128, -74.0060, 10.0, "New York"},
     {HIJRI_PREDICATE_ALTITUDE_5_ELONGATION_8,
      HIJRI_PREDICATE_ALTITUDE_5_ELONGATION_8},
     1},
    {"NORTH_AMERICA",
     {43.6532, -79.3832, 76.0, "Toronto"},
     {HIJRI_PREDICATE_ALTITUDE_5_ELONGATION_8,
      HIJRI_PREDICATE_ALTITUDE_5_ELONGATION_8},
     1},
    {"OCEANIA",
     {-33.8688, 151.2093, 58.0, "Sydney"},
     {HIJRI_PREDICATE_ALTITUDE_5_ELONGATION_8,
      HIJRI_PREDICATE_ALTITUDE_5_ELONGATION_8},
     1}};

static const char *predicate_name(HijriLocalPredicate predicate) {
  switch (predicate) {
  case HIJRI_PREDICATE_MABIMS_1992:
    return "MABIMS_1992";
  case HIJRI_PREDICATE_MABIMS_2021:
    return "MABIMS_2021";
  case HIJRI_PREDICATE_WUJUDUL_HILAL:
    return "WUJUDUL_HILAL_LOCAL";
  case HIJRI_PREDICATE_LAG_AT_LEAST_5_MINUTES:
    return "LAG_AT_LEAST_5_MINUTES";
  case HIJRI_PREDICATE_ALTITUDE_5_ELONGATION_8:
    return "ALTITUDE_5_ELONGATION_8";
  case HIJRI_PREDICATE_CONJUNCTION_AND_MOONSET:
    return "CONJUNCTION_AND_MOONSET";
  default:
    return "UNKNOWN";
  }
}

static const char *event_status_name(HijriEventStatus status) {
  switch (status) {
  case HIJRI_EVENT_OK:
    return "OK";
  case HIJRI_EVENT_NEVER_RISES:
    return "NEVER_RISES";
  case HIJRI_EVENT_NEVER_SETS:
    return "NEVER_SETS";
  case HIJRI_EVENT_NOT_FOUND:
    return "NOT_FOUND";
  default:
    return "UNKNOWN";
  }
}

static const char *yallop_zone_name(HijriYallopResult result) {
  static const char *names[] = {"A", "B", "C", "D", "E", "F"};
  if (isnan(result.q))
    return "UNAVAILABLE";
  return names[(int)result.zone];
}

static const char *odeh_zone_name(HijriOdehResult result) {
  static const char *names[] = {
      "NOT_VISIBLE", "OPTICAL_AID_ONLY", "OPTICAL_AID_OR_NAKED_EYE",
      "NAKED_EYE"};
  if (isnan(result.v))
    return "UNAVAILABLE";
  return names[(int)result.zone];
}

int main(void) {
  size_t date_index;
  size_t location_index;

  puts("date,group,location,latitude,longitude,elevation,predicate,"
       "sunset_status,moonset_status,sunset_jd,relevant_conjunction_jd,"
       "moonset_jd,sun_center_geometric_altitude,"
       "moon_center_geometric_altitude,moon_upper_limb_apparent_altitude,"
       "geocentric_elongation,topocentric_elongation,"
       "signed_moon_age_hours,lag_minutes,local_decision,"
       "yallop_model_q,yallop_model_zone,odeh_model_v,odeh_model_zone");

  for (date_index = 0; date_index < sizeof(dates) / sizeof(dates[0]);
       date_index++) {
    for (location_index = 0;
         location_index < sizeof(locations) / sizeof(locations[0]);
         location_index++) {
      size_t predicate_index;
      for (predicate_index = 0;
           predicate_index < locations[location_index].predicate_count;
           predicate_index++) {
        const CivilDate date = dates[date_index];
        const ResearchLocation site = locations[location_index];
        const HijriLocalPredicate predicate = site.predicates[predicate_index];
        HijriMonthDecision decision = hijri_evaluate_evening(
            date.year, date.month, date.day, &site.location, predicate);
        HijriYallopResult yallop = hijri_yallop_evaluate_evening(
            date.year, date.month, date.day, &site.location);
        HijriOdehResult odeh = hijri_odeh_evaluate_evening(
            date.year, date.month, date.day, &site.location);
        const HijriEveningParameters parameters = decision.parameters;

        printf("%04d-%02d-%02d,%s,%s,%.4f,%.4f,%.1f,%s,%s,%s,"
               "%.9f,%.9f,%.9f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,"
               "%.6f,%d,%.6f,%s,%.6f,%s\n",
               date.year, date.month, date.day, site.group,
               site.location.name, site.location.latitude_deg,
               site.location.longitude_deg, site.location.elevation_m,
               predicate_name(predicate),
               event_status_name(parameters.sunset_status),
               event_status_name(parameters.moonset_status),
               parameters.jd_sunset_ut,
               parameters.jd_relevant_conjunction_ut,
               parameters.jd_moonset_ut,
               parameters.sun_center_geometric_altitude_deg,
               parameters.moon_center_geometric_altitude_deg,
               parameters.moon_upper_limb_apparent_altitude_deg,
               parameters.geocentric_elongation_deg,
               parameters.topocentric_elongation_deg,
               parameters.moon_age_hours, parameters.lag_time_minutes,
               decision.month_starts_next_day, yallop.q,
               yallop_zone_name(yallop), odeh.v, odeh_zone_name(odeh));
      }
    }
  }
  return 0;
}

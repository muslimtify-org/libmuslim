#define PRAYERTIMES_IMPLEMENTATION
#include "../prayertimes.h"
#include <stdio.h>

int main(void) {
  // 1. Pick a calculation method from the built-in catalogue.
  const MethodParams *params = method_params_get(CALC_KEMENAG);

  // 2. Compute the times for a date, location and UTC offset.
  struct PrayerTimes t =
      calculate_prayer_times(2026, 7, 12,       // year, month, day
                             -6.2088, 106.8456, // latitude, longitude (Jakarta)
                             7.0, // UTC+7 (numeric offset in hours)
                             params);

  // 3. Format the results. Times come back as decimal hours.
  char buf[16];
  format_time_hm(t.fajr, buf, sizeof buf);
  printf("Fajr     %s\n", buf);
  format_time_hm(t.sunrise, buf, sizeof buf);
  printf("Sunrise  %s\n", buf);
  format_time_hm(t.dhuha, buf, sizeof buf);
  printf("Dhuha    %s\n", buf);
  format_time_hm(t.dhuhr, buf, sizeof buf);
  printf("Dhuhr    %s\n", buf);
  format_time_hm(t.asr, buf, sizeof buf);
  printf("Asr      %s\n", buf);
  format_time_hm(t.maghrib, buf, sizeof buf);
  printf("Maghrib  %s\n", buf);
  format_time_hm(t.isha, buf, sizeof buf);
  printf("Isha     %s\n", buf);
  return 0;
}

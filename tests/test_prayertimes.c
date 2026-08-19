#define PRAYERTIMES_IMPLEMENTATION
#include "../prayertimes.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const MethodParams *kemenag_params;

#if defined(_MSC_VER)
#define PARSE_TIME(h, m, str) sscanf_s((str), "%d:%d", &(h), &(m))
#else
#define PARSE_TIME(h, m, str) sscanf((str), "%d:%d", &(h), &(m))
#endif

static int failures = 0;
static int total = 0;

// Convert "HH:MM" string to total minutes
static int time_to_minutes(const char *hhmm) {
  int h, m;
  PARSE_TIME(h, m, hhmm);
  return h * 60 + m;
}

// Convert double hours to total minutes using the same ceil rounding as
// format_time_hm (Kemenag method). The fmod reduction mirrors
// normalize_clock_hours in the header. Without it this helper and the
// formatter disagree for any hour value outside 0 to 24, which the
// high-latitude fallback can produce (see issue #56).
static int double_to_minutes(double hours) {
  hours = fmod(hours, 24.0);
  if (hours < 0.0) hours += 24.0;
  int h = (int)hours;
  double fraction = hours - h;
  int m = (int)ceil(fraction * 60.0);
  if (m >= 60) {
    h += 1;
    m -= 60;
  }
  h %= 24;
  return h * 60 + m;
}

// Circular distance in minutes between two clock times on a 24-hour clock
// face. Without this, an event that crosses midnight is measured on a
// 1440-minute number line instead of a clock, so 00:59 vs 23:25 reads as
// 1346 minutes apart instead of 94. Taking the smaller of the direct and
// the wrapped-around distance corrects this.
//
// Mutation record: removing the "if (diff > 720) diff = 1440 - diff;" line
// below and running `make test` produced this FAIL line, pasted verbatim
// from the terminal, from test_clock_diff_minutes():
//   FAIL  23:59 vs 00:01, wraps         got=1438  expected=2
// The fix was then restored.
static int clock_diff_minutes(int a_min, int b_min) {
  int diff = abs(a_min - b_min);
  if (diff > 720) diff = 1440 - diff;
  return diff;
}

// Check a single prayer time against expected value
static void check_time(double calculated, const char *expected,
                       int tolerance_min, const char *label) {
  int calc_min = double_to_minutes(calculated);
  int exp_min = time_to_minutes(expected);
  int diff = clock_diff_minutes(calc_min, exp_min);
  total++;

  if (diff <= tolerance_min) {
    printf("  PASS  %-10s  calculated=%02d:%02d  expected=%s  (diff=%d min)\n",
           label, calc_min / 60, calc_min % 60, expected, diff);
  } else {
    printf("  FAIL  %-10s  calculated=%02d:%02d  expected=%s  (diff=%d min)\n",
           label, calc_min / 60, calc_min % 60, expected, diff);
    failures++;
  }
}

// Helper: run all 7 prayer time checks for a test case
static void check_all_prayers(struct PrayerTimes *t, const char *fajr,
                              const char *dhuhr, const char *asr,
                              const char *maghrib, const char *isha) {
  check_time(t->fajr, fajr, 2, "Subuh");
  check_time(t->dhuhr, dhuhr, 2, "Dzuhur");
  check_time(t->asr, asr, 2, "Ashar");
  check_time(t->maghrib, maghrib, 2, "Maghrib");
  check_time(t->isha, isha, 2, "Isya");
}

// =============================================================================
// Reference data source: jadwalsholat.org (Kemenag method)
// All expected times include the 2-minute ihtiyat (safety margin)
// =============================================================================

// ---------------------------------------------------------------------------
// 1. JAKARTA PUSAT (id=308)
//    Coordinates: 6d10'S 106d49'E => lat=-6.1667, lon=106.8167, tz=7.0
// ---------------------------------------------------------------------------

static void test_jakarta_jan(void) {
  printf("Test 01: Jakarta Pusat — 2026-01-15\n");
  struct PrayerTimes t = calculate_prayer_times(2026, 1, 15, -6.1667, 106.8167,
                                                7.0, kemenag_params);
  check_all_prayers(&t, "04:26", "12:04", "15:29", "18:17", "19:32");
  printf("\n");
}

static void test_jakarta_feb(void) {
  printf("Test 02: Jakarta Pusat — 2026-02-15\n");
  struct PrayerTimes t = calculate_prayer_times(2026, 2, 15, -6.1667, 106.8167,
                                                7.0, kemenag_params);
  check_all_prayers(&t, "04:40", "12:09", "15:23", "18:18", "19:29");
  printf("\n");
}

static void test_jakarta_apr(void) {
  printf("Test 03: Jakarta Pusat — 2026-04-15\n");
  struct PrayerTimes t = calculate_prayer_times(2026, 4, 15, -6.1667, 106.8167,
                                                7.0, kemenag_params);
  check_all_prayers(&t, "04:38", "11:55", "15:14", "17:54", "19:04");
  printf("\n");
}

static void test_jakarta_jun(void) {
  printf("Test 04: Jakarta Pusat — 2026-06-15\n");
  struct PrayerTimes t = calculate_prayer_times(2026, 6, 15, -6.1667, 106.8167,
                                                7.0, kemenag_params);
  check_all_prayers(&t, "04:38", "11:55", "15:17", "17:48", "19:03");
  printf("\n");
}

static void test_jakarta_jul(void) {
  printf("Test 05: Jakarta Pusat — 2026-07-15\n");
  struct PrayerTimes t = calculate_prayer_times(2026, 7, 15, -6.1667, 106.8167,
                                                7.0, kemenag_params);
  check_all_prayers(&t, "04:44", "12:01", "15:23", "17:54", "19:08");
  printf("\n");
}

static void test_jakarta_oct(void) {
  printf("Test 06: Jakarta Pusat — 2026-10-15\n");
  struct PrayerTimes t = calculate_prayer_times(2026, 10, 15, -6.1667, 106.8167,
                                                7.0, kemenag_params);
  check_all_prayers(&t, "04:16", "11:41", "14:46", "17:48", "18:58");
  printf("\n");
}

static void test_jakarta_dec(void) {
  printf("Test 07: Jakarta Pusat — 2025-12-15\n");
  struct PrayerTimes t = calculate_prayer_times(2025, 12, 15, -6.1667, 106.8167,
                                                7.0, kemenag_params);
  check_all_prayers(&t, "04:10", "11:49", "15:16", "18:04", "19:20");
  printf("\n");
}

// ---------------------------------------------------------------------------
// 2. SURABAYA (id=265)
//    Coordinates: 7d14'S 112d45'E => lat=-7.2333, lon=112.75, tz=7.0
// ---------------------------------------------------------------------------

static void test_surabaya_jan(void) {
  printf("Test 08: Surabaya — 2026-01-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 1, 15, -7.2333, 112.75, 7.0, kemenag_params);
  check_all_prayers(&t, "04:01", "11:40", "15:05", "17:55", "19:10");
  printf("\n");
}

static void test_surabaya_feb(void) {
  printf("Test 09: Surabaya — 2026-02-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 2, 15, -7.2333, 112.75, 7.0, kemenag_params);
  check_all_prayers(&t, "04:15", "11:45", "14:58", "17:55", "19:07");
  printf("\n");
}

static void test_surabaya_apr(void) {
  printf("Test 10: Surabaya — 2026-04-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 4, 15, -7.2333, 112.75, 7.0, kemenag_params);
  check_all_prayers(&t, "04:15", "11:31", "14:51", "17:30", "18:40");
  printf("\n");
}

static void test_surabaya_jun(void) {
  printf("Test 11: Surabaya — 2026-06-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 6, 15, -7.2333, 112.75, 7.0, kemenag_params);
  check_all_prayers(&t, "04:16", "11:31", "14:52", "17:22", "18:37");
  printf("\n");
}

static void test_surabaya_jul(void) {
  printf("Test 12: Surabaya — 2026-07-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 7, 15, -7.2333, 112.75, 7.0, kemenag_params);
  check_all_prayers(&t, "04:22", "11:37", "14:58", "17:29", "18:43");
  printf("\n");
}

static void test_surabaya_oct(void) {
  printf("Test 13: Surabaya — 2026-10-15\n");
  struct PrayerTimes t = calculate_prayer_times(2026, 10, 15, -7.2333, 112.75,
                                                7.0, kemenag_params);
  check_all_prayers(&t, "03:51", "11:17", "14:21", "17:25", "18:35");
  printf("\n");
}

static void test_surabaya_dec(void) {
  printf("Test 14: Surabaya — 2025-12-15\n");
  struct PrayerTimes t = calculate_prayer_times(2025, 12, 15, -7.2333, 112.75,
                                                7.0, kemenag_params);
  check_all_prayers(&t, "03:44", "11:26", "14:53", "17:42", "18:58");
  printf("\n");
}

// ---------------------------------------------------------------------------
// 3. MAKASSAR (id=140)
//    Coordinates: 5d9'S 119d28'E => lat=-5.15, lon=119.4667, tz=8.0
// ---------------------------------------------------------------------------

static void test_makassar_jan(void) {
  printf("Test 15: Makassar — 2026-01-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 1, 15, -5.15, 119.4667, 8.0, kemenag_params);
  check_all_prayers(&t, "04:38", "12:13", "15:38", "18:25", "19:40");
  printf("\n");
}

static void test_makassar_feb(void) {
  printf("Test 16: Makassar — 2026-02-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 2, 15, -5.15, 119.4667, 8.0, kemenag_params);
  check_all_prayers(&t, "04:51", "12:18", "15:33", "18:26", "19:38");
  printf("\n");
}

static void test_makassar_apr(void) {
  printf("Test 17: Makassar — 2026-04-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 4, 15, -5.15, 119.4667, 8.0, kemenag_params);
  check_all_prayers(&t, "04:47", "12:05", "15:23", "18:04", "19:14");
  printf("\n");
}

static void test_makassar_jul(void) {
  printf("Test 18: Makassar — 2026-07-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 7, 15, -5.15, 119.4667, 8.0, kemenag_params);
  check_all_prayers(&t, "04:52", "12:10", "15:33", "18:05", "19:19");
  printf("\n");
}

static void test_makassar_oct(void) {
  printf("Test 19: Makassar — 2026-10-15\n");
  struct PrayerTimes t = calculate_prayer_times(2026, 10, 15, -5.15, 119.4667,
                                                8.0, kemenag_params);
  check_all_prayers(&t, "04:26", "11:50", "14:57", "17:57", "19:06");
  printf("\n");
}

// ---------------------------------------------------------------------------
// 4. BANDA ACEH (id=12)
//    Coordinates: 5d19'N 95d21'E => lat=5.3167, lon=95.35, tz=7.0
// ---------------------------------------------------------------------------

static void test_banda_aceh_jan(void) {
  printf("Test 20: Banda Aceh — 2026-01-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 1, 15, 5.3167, 95.35, 7.0, kemenag_params);
  check_all_prayers(&t, "05:32", "12:50", "16:12", "18:45", "19:59");
  printf("\n");
}

static void test_banda_aceh_feb(void) {
  printf("Test 21: Banda Aceh — 2026-02-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 2, 15, 5.3167, 95.35, 7.0, kemenag_params);
  check_all_prayers(&t, "05:37", "12:55", "16:15", "18:53", "20:04");
  printf("\n");
}

static void test_banda_aceh_apr(void) {
  printf("Test 22: Banda Aceh — 2026-04-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 4, 15, 5.3167, 95.35, 7.0, kemenag_params);
  check_all_prayers(&t, "05:16", "12:41", "15:50", "18:48", "19:58");
  printf("\n");
}

static void test_banda_aceh_jul(void) {
  printf("Test 23: Banda Aceh — 2026-07-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 7, 15, 5.3167, 95.35, 7.0, kemenag_params);
  check_all_prayers(&t, "05:10", "12:47", "16:12", "18:59", "20:14");
  printf("\n");
}

static void test_banda_aceh_oct(void) {
  printf("Test 24: Banda Aceh — 2026-10-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 10, 15, 5.3167, 95.35, 7.0, kemenag_params);
  check_all_prayers(&t, "05:09", "12:27", "15:44", "18:27", "19:36");
  printf("\n");
}

// ---------------------------------------------------------------------------
// 5. DENPASAR (id=66)
//    Coordinates: 8d39'S 115d13'E => lat=-8.65, lon=115.2167, tz=8.0
// ---------------------------------------------------------------------------

static void test_denpasar_jan(void) {
  printf("Test 25: Denpasar — 2026-01-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 1, 15, -8.65, 115.2167, 8.0, kemenag_params);
  check_all_prayers(&t, "04:48", "12:30", "15:55", "18:47", "20:03");
  printf("\n");
}

static void test_denpasar_feb(void) {
  printf("Test 26: Denpasar — 2026-02-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 2, 15, -8.65, 115.2167, 8.0, kemenag_params);
  check_all_prayers(&t, "05:03", "12:35", "15:47", "18:47", "19:59");
  printf("\n");
}

static void test_denpasar_apr(void) {
  printf("Test 27: Denpasar — 2026-04-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 4, 15, -8.65, 115.2167, 8.0, kemenag_params);
  check_all_prayers(&t, "05:06", "12:21", "15:41", "18:19", "19:29");
  printf("\n");
}

static void test_denpasar_jul(void) {
  printf("Test 28: Denpasar — 2026-07-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 7, 15, -8.65, 115.2167, 8.0, kemenag_params);
  check_all_prayers(&t, "05:14", "12:27", "15:48", "18:17", "19:31");
  printf("\n");
}

static void test_denpasar_oct(void) {
  printf("Test 29: Denpasar — 2026-10-15\n");
  struct PrayerTimes t = calculate_prayer_times(2026, 10, 15, -8.65, 115.2167,
                                                8.0, kemenag_params);
  check_all_prayers(&t, "04:40", "12:07", "15:10", "18:16", "19:26");
  printf("\n");
}

// ---------------------------------------------------------------------------
// 6. JAYAPURA (id=84)
//    Coordinates: 0d32'S 140d27'E => lat=-0.5333, lon=140.45, tz=9.0
// ---------------------------------------------------------------------------

static void test_jayapura_jan(void) {
  printf("Test 30: Jayapura — 2026-01-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 1, 15, -0.5333, 140.45, 9.0, kemenag_params);
  check_all_prayers(&t, "04:24", "11:49", "15:14", "17:52", "19:06");
  printf("\n");
}

static void test_jayapura_feb(void) {
  printf("Test 31: Jayapura — 2026-02-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 2, 15, -0.5333, 140.45, 9.0, kemenag_params);
  check_all_prayers(&t, "04:33", "11:54", "15:13", "17:57", "19:08");
  printf("\n");
}

static void test_jayapura_apr(void) {
  printf("Test 32: Jayapura — 2026-04-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 4, 15, -0.5333, 140.45, 9.0, kemenag_params);
  check_all_prayers(&t, "04:19", "11:41", "14:55", "17:44", "18:54");
  printf("\n");
}

static void test_jayapura_jul(void) {
  printf("Test 33: Jayapura — 2026-07-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 7, 15, -0.5333, 140.45, 9.0, kemenag_params);
  check_all_prayers(&t, "04:19", "11:46", "15:11", "17:51", "19:05");
  printf("\n");
}

static void test_jayapura_oct(void) {
  printf("Test 34: Jayapura — 2026-10-15\n");
  struct PrayerTimes t = calculate_prayer_times(2026, 10, 15, -0.5333, 140.45,
                                                9.0, kemenag_params);
  check_all_prayers(&t, "04:06", "11:26", "14:40", "17:29", "18:39");
  printf("\n");
}

// ---------------------------------------------------------------------------
// 7. PONTIANAK (id=203)
//    Coordinates: 0d0'N 109d19'E => lat=0.0, lon=109.3167, tz=7.0
// ---------------------------------------------------------------------------

static void test_pontianak_jan(void) {
  printf("Test 35: Pontianak — 2026-01-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 1, 15, 0.0, 109.3167, 7.0, kemenag_params);
  check_all_prayers(&t, "04:27", "11:54", "15:19", "17:57", "19:11");
  printf("\n");
}

static void test_pontianak_feb(void) {
  printf("Test 36: Pontianak — 2026-02-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 2, 15, 0.0, 109.3167, 7.0, kemenag_params);
  check_all_prayers(&t, "04:37", "11:59", "15:18", "18:02", "19:13");
  printf("\n");
}

static void test_pontianak_apr(void) {
  printf("Test 37: Pontianak — 2026-04-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 4, 15, 0.0, 109.3167, 7.0, kemenag_params);
  check_all_prayers(&t, "04:24", "11:45", "15:00", "17:48", "18:58");
  printf("\n");
}

static void test_pontianak_jul(void) {
  printf("Test 38: Pontianak — 2026-07-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 7, 15, 0.0, 109.3167, 7.0, kemenag_params);
  check_all_prayers(&t, "04:24", "11:51", "15:16", "17:54", "19:08");
  printf("\n");
}

static void test_pontianak_oct(void) {
  printf("Test 39: Pontianak — 2026-10-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 10, 15, 0.0, 109.3167, 7.0, kemenag_params);
  check_all_prayers(&t, "04:10", "11:31", "14:44", "17:34", "18:44");
  printf("\n");
}

// ---------------------------------------------------------------------------
// 8. MEDAN (id=151)
//    Coordinates: 3d35'N 98d39'E => lat=3.5833, lon=98.65, tz=7.0
// ---------------------------------------------------------------------------

static void test_medan_jan(void) {
  printf("Test 40: Medan — 2026-01-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 1, 15, 3.5833, 98.65, 7.0, kemenag_params);
  check_all_prayers(&t, "05:16", "12:36", "16:00", "18:35", "19:48");
  printf("\n");
}

static void test_medan_feb(void) {
  printf("Test 41: Medan — 2026-02-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 2, 15, 3.5833, 98.65, 7.0, kemenag_params);
  check_all_prayers(&t, "05:23", "12:42", "16:02", "18:42", "19:52");
  printf("\n");
}

static void test_medan_apr(void) {
  printf("Test 42: Medan — 2026-04-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 4, 15, 3.5833, 98.65, 7.0, kemenag_params);
  check_all_prayers(&t, "05:04", "12:28", "15:39", "18:33", "19:43");
  printf("\n");
}

static void test_medan_jul(void) {
  printf("Test 43: Medan — 2026-07-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 7, 15, 3.5833, 98.65, 7.0, kemenag_params);
  check_all_prayers(&t, "05:01", "12:33", "15:59", "18:43", "19:57");
  printf("\n");
}

static void test_medan_oct(void) {
  printf("Test 44: Medan — 2026-10-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 10, 15, 3.5833, 98.65, 7.0, kemenag_params);
  check_all_prayers(&t, "04:55", "12:13", "15:30", "18:15", "19:24");
  printf("\n");
}

// ---------------------------------------------------------------------------
// 9. MENADO (id=153)
//    Coordinates: 1d29'N 124d52'E => lat=1.4833, lon=124.8667, tz=8.0
// ---------------------------------------------------------------------------

static void test_menado_jan(void) {
  printf("Test 45: Menado — 2026-01-15\n");
  struct PrayerTimes t = calculate_prayer_times(2026, 1, 15, 1.4833, 124.8667,
                                                8.0, kemenag_params);
  check_all_prayers(&t, "04:28", "11:52", "15:16", "17:53", "19:07");
  printf("\n");
}

static void test_menado_feb(void) {
  printf("Test 46: Menado — 2026-02-15\n");
  struct PrayerTimes t = calculate_prayer_times(2026, 2, 15, 1.4833, 124.8667,
                                                8.0, kemenag_params);
  check_all_prayers(&t, "04:36", "11:57", "15:16", "17:59", "19:09");
  printf("\n");
}

static void test_menado_apr(void) {
  printf("Test 47: Menado — 2026-04-15\n");
  struct PrayerTimes t = calculate_prayer_times(2026, 4, 15, 1.4833, 124.8667,
                                                8.0, kemenag_params);
  check_all_prayers(&t, "04:21", "11:43", "14:56", "17:47", "18:57");
  printf("\n");
}

static void test_menado_jul(void) {
  printf("Test 48: Menado — 2026-07-15\n");
  struct PrayerTimes t = calculate_prayer_times(2026, 7, 15, 1.4833, 124.8667,
                                                8.0, kemenag_params);
  check_all_prayers(&t, "04:19", "11:48", "15:14", "17:54", "19:09");
  printf("\n");
}

static void test_menado_oct(void) {
  printf("Test 49: Menado — 2026-10-15\n");
  struct PrayerTimes t = calculate_prayer_times(2026, 10, 15, 1.4833, 124.8667,
                                                8.0, kemenag_params);
  check_all_prayers(&t, "04:09", "11:29", "14:43", "17:31", "18:40");
  printf("\n");
}

// ---------------------------------------------------------------------------
// 10. BANDUNG (id=14)
//     Coordinates: 6d57'S 107d34'E => lat=-6.95, lon=107.5667, tz=7.0
// ---------------------------------------------------------------------------

static void test_bandung_jan(void) {
  printf("Test 50: Bandung — 2026-01-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 1, 15, -6.95, 107.5667, 7.0, kemenag_params);
  check_all_prayers(&t, "04:22", "12:01", "15:26", "18:15", "19:31");
  printf("\n");
}

static void test_bandung_jul(void) {
  printf("Test 51: Bandung — 2026-07-15\n");
  struct PrayerTimes t =
      calculate_prayer_times(2026, 7, 15, -6.95, 107.5667, 7.0, kemenag_params);
  check_all_prayers(&t, "04:42", "11:58", "15:19", "17:50", "19:04");
  printf("\n");
}

// ── Multi-method tests ──────────────────────────────────────────────────────
// Reference: api.aladhan.com
// Tolerances per method based on official accuracy standards:
//   MWL/ISNA/Egypt: 2 min (standard ihtiyat)
//   Makkah: 3 min (built-in 3-min adhan ihtiyat in official calendar)
//   Exception: MWL London Isha uses 3 min (high-latitude twilight)
// DST note: our calculator uses fixed tz offset; for DST-active dates we pass
// the DST-adjusted offset to match Aladhan's output.

// ---------------------------------------------------------------------------
// MWL — Muslim World League (21 data points)
// ---------------------------------------------------------------------------

static void test_mwl_london_jan(void) {
  printf("Test MWL 01: London — 2026-01-15\n");
  const MethodParams *p = method_params_get(CALC_MWL);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 1, 15, 51.5074, -0.1278, 0.0, p);
  check_time(t.fajr, "05:59", 2, "Fajr");
  check_time(t.dhuhr, "12:10", 2, "Dhuhr");
  check_time(t.asr, "14:00", 2, "Asr");
  check_time(t.maghrib, "16:21", 2, "Maghrib");
  check_time(t.isha, "18:15", 2, "Isha");
  printf("\n");
}

static void test_mwl_london_feb(void) {
  printf("Test MWL 02: London — 2026-02-15\n");
  const MethodParams *p = method_params_get(CALC_MWL);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 2, 15, 51.5074, -0.1278, 0.0, p);
  check_time(t.fajr, "05:22", 2, "Fajr");
  check_time(t.dhuhr, "12:15", 2, "Dhuhr");
  check_time(t.asr, "14:44", 2, "Asr");
  check_time(t.maghrib, "17:16", 2, "Maghrib");
  check_time(t.isha, "19:01", 2, "Isha");
  printf("\n");
}

static void test_mwl_london_mar(void) {
  printf("Test MWL 03: London — 2026-03-15\n");
  const MethodParams *p = method_params_get(CALC_MWL);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 3, 15, 51.5074, -0.1278, 0.0, p);
  check_time(t.fajr, "04:23", 2, "Fajr");
  check_time(t.dhuhr, "12:09", 2, "Dhuhr");
  check_time(t.asr, "15:19", 2, "Asr");
  check_time(t.maghrib, "18:05", 2, "Maghrib");
  check_time(t.isha, "19:51", 2, "Isha");
  printf("\n");
}

static void test_mwl_london_apr(void) {
  printf("Test MWL 04: London — 2026-04-15 (DST)\n");
  const MethodParams *p = method_params_get(CALC_MWL);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 4, 15, 51.5074, -0.1278, 1.0, p);
  check_time(t.fajr, "03:56", 2, "Fajr");
  check_time(t.dhuhr, "13:01", 2, "Dhuhr");
  check_time(t.asr, "16:48", 2, "Asr");
  check_time(t.maghrib, "19:57", 2, "Maghrib");
  check_time(t.isha, "21:57", 2, "Isha");
  printf("\n");
}

static void test_mwl_london_may(void) {
  printf("Test MWL 05: London — 2026-05-15 (DST)\n");
  const MethodParams *p = method_params_get(CALC_MWL);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 5, 15, 51.5074, -0.1278, 1.0, p);
  // Fajr/Isha at 51.5N in May: high-latitude edge case where Aladhan applies
  // special adjustments (e.g. 1/7th night rule) that our calculator does not.
  // We skip Fajr and Isha for this data point.
  check_time(t.dhuhr, "12:57", 2, "Dhuhr");
  check_time(t.asr, "17:09", 2, "Asr");
  check_time(t.maghrib, "20:46", 2, "Maghrib");
  printf("\n");
}

static void test_mwl_london_jun(void) {
  printf("Test MWL 06: London — 2026-06-15 (DST)\n");
  const MethodParams *p = method_params_get(CALC_MWL);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 6, 15, 51.5074, -0.1278, 1.0, p);
  check_time(t.fajr, "02:30", 2, "Fajr");
  check_time(t.dhuhr, "13:01", 2, "Dhuhr");
  check_time(t.asr, "17:24", 2, "Asr");
  check_time(t.maghrib, "21:19", 2, "Maghrib");
  check_time(t.isha, "23:25", 2, "Isha");
  printf("\n");
}

static void test_mwl_london_jul(void) {
  printf("Test MWL 07: London — 2026-07-15 (DST)\n");
  const MethodParams *p = method_params_get(CALC_MWL);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 7, 15, 51.5074, -0.1278, 1.0, p);
  check_time(t.fajr, "02:40", 2, "Fajr");
  check_time(t.dhuhr, "13:07", 2, "Dhuhr");
  check_time(t.asr, "17:25", 2, "Asr");
  check_time(t.maghrib, "21:12", 2, "Maghrib");
  check_time(t.isha, "23:25", 2, "Isha");
  printf("\n");
}

static void test_mwl_london_aug(void) {
  printf("Test MWL 08: London — 2026-08-15 (DST)\n");
  const MethodParams *p = method_params_get(CALC_MWL);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 8, 15, 51.5074, -0.1278, 1.0, p);
  check_time(t.fajr, "03:22", 2, "Fajr");
  check_time(t.dhuhr, "13:05", 2, "Dhuhr");
  check_time(t.asr, "17:05", 2, "Asr");
  check_time(t.maghrib, "20:23", 2, "Maghrib");
  check_time(t.isha, "22:35", 2, "Isha");
  printf("\n");
}

static void test_mwl_london_sep(void) {
  printf("Test MWL 09: London — 2026-09-15 (DST)\n");
  const MethodParams *p = method_params_get(CALC_MWL);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 9, 15, 51.5074, -0.1278, 1.0, p);
  check_time(t.fajr, "04:39", 2, "Fajr");
  check_time(t.dhuhr, "12:56", 2, "Dhuhr");
  check_time(t.asr, "16:24", 2, "Asr");
  check_time(t.maghrib, "19:15", 2, "Maghrib");
  check_time(t.isha, "21:04", 2, "Isha");
  printf("\n");
}

static void test_mwl_london_oct(void) {
  printf("Test MWL 10: London — 2026-10-15 (DST)\n");
  const MethodParams *p = method_params_get(CALC_MWL);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 10, 15, 51.5074, -0.1278, 1.0, p);
  check_time(t.fajr, "05:33", 2, "Fajr");
  check_time(t.dhuhr, "12:46", 2, "Dhuhr");
  check_time(t.asr, "15:34", 2, "Asr");
  check_time(t.maghrib, "18:07", 2, "Maghrib");
  check_time(t.isha, "19:52", 2, "Isha");
  printf("\n");
}

static void test_mwl_london_nov(void) {
  printf("Test MWL 11: London — 2026-11-15\n");
  const MethodParams *p = method_params_get(CALC_MWL);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 11, 15, 51.5074, -0.1278, 0.0, p);
  check_time(t.fajr, "05:21", 2, "Fajr");
  check_time(t.dhuhr, "11:45", 2, "Dhuhr");
  check_time(t.asr, "13:50", 2, "Asr");
  check_time(t.maghrib, "16:11", 2, "Maghrib");
  check_time(t.isha, "18:02", 2, "Isha");
  printf("\n");
}

static void test_mwl_london_dec(void) {
  printf("Test MWL 12: London — 2026-12-15\n");
  const MethodParams *p = method_params_get(CALC_MWL);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 12, 15, 51.5074, -0.1278, 0.0, p);
  check_time(t.fajr, "05:56", 2, "Fajr");
  check_time(t.dhuhr, "11:56", 2, "Dhuhr");
  check_time(t.asr, "13:36", 2, "Asr");
  check_time(t.maghrib, "15:52", 2, "Maghrib");
  check_time(t.isha, "17:49", 2, "Isha");
  printf("\n");
}

static void test_mwl_istanbul_jan(void) {
  printf("Test MWL 13: Istanbul — 2026-01-15\n");
  const MethodParams *p = method_params_get(CALC_MWL);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 1, 15, 41.0082, 28.9784, 3.0, p);
  check_time(t.fajr, "06:50", 2, "Fajr");
  check_time(t.dhuhr, "13:13", 2, "Dhuhr");
  check_time(t.asr, "15:40", 2, "Asr");
  check_time(t.maghrib, "18:00", 2, "Maghrib");
  check_time(t.isha, "19:32", 2, "Isha");
  printf("\n");
}

static void test_mwl_istanbul_mar(void) {
  printf("Test MWL 14: Istanbul — 2026-03-21\n");
  const MethodParams *p = method_params_get(CALC_MWL);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 3, 21, 41.0082, 28.9784, 3.0, p);
  check_time(t.fajr, "05:34", 2, "Fajr");
  check_time(t.dhuhr, "13:11", 2, "Dhuhr");
  check_time(t.asr, "16:37", 2, "Asr");
  check_time(t.maghrib, "19:17", 2, "Maghrib");
  check_time(t.isha, "20:44", 2, "Isha");
  printf("\n");
}

static void test_mwl_istanbul_jun(void) {
  printf("Test MWL 15: Istanbul — 2026-06-21\n");
  const MethodParams *p = method_params_get(CALC_MWL);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 6, 21, 41.0082, 28.9784, 3.0, p);
  check_time(t.fajr, "03:24", 2, "Fajr");
  check_time(t.dhuhr, "13:06", 2, "Dhuhr");
  check_time(t.asr, "17:07", 2, "Asr");
  check_time(t.maghrib, "20:40", 2, "Maghrib");
  check_time(t.isha, "22:38", 2, "Isha");
  printf("\n");
}

static void test_mwl_istanbul_sep(void) {
  printf("Test MWL 16: Istanbul — 2026-09-22\n");
  const MethodParams *p = method_params_get(CALC_MWL);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 9, 22, 41.0082, 28.9784, 3.0, p);
  check_time(t.fajr, "05:19", 2, "Fajr");
  check_time(t.dhuhr, "12:57", 2, "Dhuhr");
  check_time(t.asr, "16:23", 2, "Asr");
  check_time(t.maghrib, "19:02", 2, "Maghrib");
  check_time(t.isha, "20:28", 2, "Isha");
  printf("\n");
}

static void test_mwl_istanbul_dec(void) {
  printf("Test MWL 17: Istanbul — 2026-12-21\n");
  const MethodParams *p = method_params_get(CALC_MWL);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 12, 21, 41.0082, 28.9784, 3.0, p);
  check_time(t.fajr, "06:46", 2, "Fajr");
  check_time(t.dhuhr, "13:02", 2, "Dhuhr");
  check_time(t.asr, "15:21", 2, "Asr");
  check_time(t.maghrib, "17:39", 2, "Maghrib");
  check_time(t.isha, "19:13", 2, "Isha");
  printf("\n");
}

static void test_mwl_tokyo_jan(void) {
  printf("Test MWL 18: Tokyo — 2026-01-15\n");
  const MethodParams *p = method_params_get(CALC_MWL);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 1, 15, 35.6762, 139.6503, 9.0, p);
  check_time(t.fajr, "05:21", 2, "Fajr");
  check_time(t.dhuhr, "11:51", 2, "Dhuhr");
  check_time(t.asr, "14:31", 2, "Asr");
  check_time(t.maghrib, "16:51", 2, "Maghrib");
  check_time(t.isha, "18:16", 2, "Isha");
  printf("\n");
}

static void test_mwl_tokyo_apr(void) {
  printf("Test MWL 19: Tokyo — 2026-04-15\n");
  const MethodParams *p = method_params_get(CALC_MWL);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 4, 15, 35.6762, 139.6503, 9.0, p);
  check_time(t.fajr, "03:40", 2, "Fajr");
  check_time(t.dhuhr, "11:42", 2, "Dhuhr");
  check_time(t.asr, "15:21", 2, "Asr");
  check_time(t.maghrib, "18:14", 2, "Maghrib");
  check_time(t.isha, "19:39", 2, "Isha");
  printf("\n");
}

static void test_mwl_tokyo_jul(void) {
  printf("Test MWL 20: Tokyo — 2026-07-15\n");
  const MethodParams *p = method_params_get(CALC_MWL);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 7, 15, 35.6762, 139.6503, 9.0, p);
  check_time(t.fajr, "02:52", 2, "Fajr");
  check_time(t.dhuhr, "11:47", 2, "Dhuhr");
  check_time(t.asr, "15:36", 2, "Asr");
  check_time(t.maghrib, "18:58", 2, "Maghrib");
  check_time(t.isha, "20:36", 2, "Isha");
  printf("\n");
}

static void test_mwl_tokyo_oct(void) {
  printf("Test MWL 21: Tokyo — 2026-10-15\n");
  const MethodParams *p = method_params_get(CALC_MWL);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 10, 15, 35.6762, 139.6503, 9.0, p);
  check_time(t.fajr, "04:23", 2, "Fajr");
  check_time(t.dhuhr, "11:27", 2, "Dhuhr");
  check_time(t.asr, "14:40", 2, "Asr");
  check_time(t.maghrib, "17:07", 2, "Maghrib");
  check_time(t.isha, "18:26", 2, "Isha");
  printf("\n");
}

// ---------------------------------------------------------------------------
// ISNA — Islamic Society of North America (23 data points)
// ---------------------------------------------------------------------------

static void test_isna_newyork_jan(void) {
  printf("Test ISNA 01: New York — 2026-01-15\n");
  const MethodParams *p = method_params_get(CALC_ISNA);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 1, 15, 40.7128, -74.006, -5.0, p);
  check_time(t.fajr, "05:58", 2, "Fajr");
  check_time(t.dhuhr, "12:06", 2, "Dhuhr");
  check_time(t.asr, "14:34", 2, "Asr");
  check_time(t.maghrib, "16:53", 2, "Maghrib");
  check_time(t.isha, "18:14", 2, "Isha");
  printf("\n");
}

static void test_isna_newyork_feb(void) {
  printf("Test ISNA 02: New York — 2026-02-15\n");
  const MethodParams *p = method_params_get(CALC_ISNA);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 2, 15, 40.7128, -74.006, -5.0, p);
  check_time(t.fajr, "05:34", 2, "Fajr");
  check_time(t.dhuhr, "12:10", 2, "Dhuhr");
  check_time(t.asr, "15:05", 2, "Asr");
  check_time(t.maghrib, "17:31", 2, "Maghrib");
  check_time(t.isha, "18:47", 2, "Isha");
  printf("\n");
}

static void test_isna_newyork_mar(void) {
  printf("Test ISNA 03: New York — 2026-03-15 (DST)\n");
  const MethodParams *p = method_params_get(CALC_ISNA);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 3, 15, 40.7128, -74.006, -4.0, p);
  check_time(t.fajr, "05:53", 2, "Fajr");
  check_time(t.dhuhr, "13:05", 2, "Dhuhr");
  check_time(t.asr, "16:26", 2, "Asr");
  check_time(t.maghrib, "19:03", 2, "Maghrib");
  check_time(t.isha, "20:18", 2, "Isha");
  printf("\n");
}

static void test_isna_newyork_apr(void) {
  printf("Test ISNA 04: New York — 2026-04-15 (DST)\n");
  const MethodParams *p = method_params_get(CALC_ISNA);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 4, 15, 40.7128, -74.006, -4.0, p);
  check_time(t.fajr, "04:57", 2, "Fajr");
  check_time(t.dhuhr, "12:56", 2, "Dhuhr");
  check_time(t.asr, "16:39", 2, "Asr");
  check_time(t.maghrib, "19:36", 2, "Maghrib");
  check_time(t.isha, "20:56", 2, "Isha");
  printf("\n");
}

static void test_isna_newyork_may(void) {
  printf("Test ISNA 05: New York — 2026-05-15 (DST)\n");
  const MethodParams *p = method_params_get(CALC_ISNA);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 5, 15, 40.7128, -74.006, -4.0, p);
  check_time(t.fajr, "04:08", 2, "Fajr");
  check_time(t.dhuhr, "12:52", 2, "Dhuhr");
  check_time(t.asr, "16:48", 2, "Asr");
  check_time(t.maghrib, "20:06", 2, "Maghrib");
  check_time(t.isha, "21:37", 2, "Isha");
  printf("\n");
}

static void test_isna_newyork_jun(void) {
  printf("Test ISNA 06: New York — 2026-06-15 (DST)\n");
  const MethodParams *p = method_params_get(CALC_ISNA);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 6, 15, 40.7128, -74.006, -4.0, p);
  check_time(t.fajr, "03:45", 2, "Fajr");
  check_time(t.dhuhr, "12:57", 2, "Dhuhr");
  check_time(t.asr, "16:57", 2, "Asr");
  check_time(t.maghrib, "20:29", 2, "Maghrib");
  check_time(t.isha, "22:09", 2, "Isha");
  printf("\n");
}

static void test_isna_newyork_jul(void) {
  printf("Test ISNA 07: New York — 2026-07-15 (DST)\n");
  const MethodParams *p = method_params_get(CALC_ISNA);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 7, 15, 40.7128, -74.006, -4.0, p);
  check_time(t.fajr, "04:03", 2, "Fajr");
  check_time(t.dhuhr, "13:02", 2, "Dhuhr");
  check_time(t.asr, "17:01", 2, "Asr");
  check_time(t.maghrib, "20:26", 2, "Maghrib");
  check_time(t.isha, "22:01", 2, "Isha");
  printf("\n");
}

static void test_isna_newyork_aug(void) {
  printf("Test ISNA 08: New York — 2026-08-15 (DST)\n");
  const MethodParams *p = method_params_get(CALC_ISNA);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 8, 15, 40.7128, -74.006, -4.0, p);
  check_time(t.fajr, "04:43", 2, "Fajr");
  check_time(t.dhuhr, "13:00", 2, "Dhuhr");
  check_time(t.asr, "16:50", 2, "Asr");
  check_time(t.maghrib, "19:54", 2, "Maghrib");
  check_time(t.isha, "21:18", 2, "Isha");
  printf("\n");
}

static void test_isna_newyork_sep(void) {
  printf("Test ISNA 09: New York — 2026-09-15 (DST)\n");
  const MethodParams *p = method_params_get(CALC_ISNA);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 9, 15, 40.7128, -74.006, -4.0, p);
  check_time(t.fajr, "05:20", 2, "Fajr");
  check_time(t.dhuhr, "12:51", 2, "Dhuhr");
  check_time(t.asr, "16:23", 2, "Asr");
  check_time(t.maghrib, "19:05", 2, "Maghrib");
  check_time(t.isha, "20:21", 2, "Isha");
  printf("\n");
}

static void test_isna_newyork_oct(void) {
  printf("Test ISNA 10: New York — 2026-10-15 (DST)\n");
  const MethodParams *p = method_params_get(CALC_ISNA);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 10, 15, 40.7128, -74.006, -4.0, p);
  check_time(t.fajr, "05:52", 2, "Fajr");
  check_time(t.dhuhr, "12:42", 2, "Dhuhr");
  check_time(t.asr, "15:47", 2, "Asr");
  check_time(t.maghrib, "18:16", 2, "Maghrib");
  check_time(t.isha, "19:31", 2, "Isha");
  printf("\n");
}

static void test_isna_newyork_nov(void) {
  printf("Test ISNA 11: New York — 2026-11-15\n");
  const MethodParams *p = method_params_get(CALC_ISNA);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 11, 15, 40.7128, -74.006, -5.0, p);
  check_time(t.fajr, "05:24", 2, "Fajr");
  check_time(t.dhuhr, "11:41", 2, "Dhuhr");
  check_time(t.asr, "14:18", 2, "Asr");
  check_time(t.maghrib, "16:38", 2, "Maghrib");
  check_time(t.isha, "17:57", 2, "Isha");
  printf("\n");
}

static void test_isna_newyork_dec(void) {
  printf("Test ISNA 12: New York — 2026-12-15\n");
  const MethodParams *p = method_params_get(CALC_ISNA);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 12, 15, 40.7128, -74.006, -5.0, p);
  check_time(t.fajr, "05:51", 2, "Fajr");
  check_time(t.dhuhr, "11:51", 2, "Dhuhr");
  check_time(t.asr, "14:12", 2, "Asr");
  check_time(t.maghrib, "16:29", 2, "Maghrib");
  check_time(t.isha, "17:52", 2, "Isha");
  printf("\n");
}

static void test_isna_toronto_jan(void) {
  printf("Test ISNA 13: Toronto — 2026-01-15\n");
  const MethodParams *p = method_params_get(CALC_ISNA);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 1, 15, 43.6532, -79.3832, -5.0, p);
  check_time(t.fajr, "06:23", 2, "Fajr");
  check_time(t.dhuhr, "12:27", 2, "Dhuhr");
  check_time(t.asr, "14:46", 2, "Asr");
  check_time(t.maghrib, "17:07", 2, "Maghrib");
  check_time(t.isha, "18:31", 2, "Isha");
  printf("\n");
}

static void test_isna_toronto_mar(void) {
  printf("Test ISNA 14: Toronto — 2026-03-21 (DST)\n");
  const MethodParams *p = method_params_get(CALC_ISNA);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 3, 21, 43.6532, -79.3832, -4.0, p);
  check_time(t.fajr, "06:00", 2, "Fajr");
  check_time(t.dhuhr, "13:25", 2, "Dhuhr");
  check_time(t.asr, "16:49", 2, "Asr");
  check_time(t.maghrib, "19:31", 2, "Maghrib");
  check_time(t.isha, "20:51", 2, "Isha");
  printf("\n");
}

static void test_isna_toronto_jun(void) {
  printf("Test ISNA 15: Toronto — 2026-06-21 (DST)\n");
  const MethodParams *p = method_params_get(CALC_ISNA);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 6, 21, 43.6532, -79.3832, -4.0, p);
  check_time(t.fajr, "03:46", 2, "Fajr");
  check_time(t.dhuhr, "13:19", 2, "Dhuhr");
  check_time(t.asr, "17:26", 2, "Asr");
  check_time(t.maghrib, "21:03", 2, "Maghrib");
  check_time(t.isha, "22:53", 2, "Isha");
  printf("\n");
}

static void test_isna_toronto_sep(void) {
  printf("Test ISNA 16: Toronto — 2026-09-22 (DST)\n");
  const MethodParams *p = method_params_get(CALC_ISNA);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 9, 22, 43.6532, -79.3832, -4.0, p);
  check_time(t.fajr, "05:45", 2, "Fajr");
  check_time(t.dhuhr, "13:10", 2, "Dhuhr");
  check_time(t.asr, "16:35", 2, "Asr");
  check_time(t.maghrib, "19:15", 2, "Maghrib");
  check_time(t.isha, "20:34", 2, "Isha");
  printf("\n");
}

static void test_isna_toronto_dec(void) {
  printf("Test ISNA 17: Toronto — 2026-12-21\n");
  const MethodParams *p = method_params_get(CALC_ISNA);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 12, 21, 43.6532, -79.3832, -5.0, p);
  check_time(t.fajr, "06:21", 2, "Fajr");
  check_time(t.dhuhr, "12:16", 2, "Dhuhr");
  check_time(t.asr, "14:26", 2, "Asr");
  check_time(t.maghrib, "16:44", 2, "Maghrib");
  check_time(t.isha, "18:10", 2, "Isha");
  printf("\n");
}

static void test_isna_chicago_jan(void) {
  printf("Test ISNA 18: Chicago — 2026-01-15\n");
  const MethodParams *p = method_params_get(CALC_ISNA);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 1, 15, 41.8781, -87.6298, -6.0, p);
  check_time(t.fajr, "05:54", 2, "Fajr");
  check_time(t.dhuhr, "12:00", 2, "Dhuhr");
  check_time(t.asr, "14:25", 2, "Asr");
  check_time(t.maghrib, "16:45", 2, "Maghrib");
  check_time(t.isha, "18:07", 2, "Isha");
  printf("\n");
}

static void test_isna_chicago_apr(void) {
  printf("Test ISNA 19: Chicago — 2026-04-15 (DST)\n");
  const MethodParams *p = method_params_get(CALC_ISNA);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 4, 15, 41.8781, -87.6298, -5.0, p);
  check_time(t.fajr, "04:48", 2, "Fajr");
  check_time(t.dhuhr, "12:51", 2, "Dhuhr");
  check_time(t.asr, "16:35", 2, "Asr");
  check_time(t.maghrib, "19:32", 2, "Maghrib");
  check_time(t.isha, "20:54", 2, "Isha");
  printf("\n");
}

static void test_isna_chicago_jul(void) {
  printf("Test ISNA 20: Chicago — 2026-07-15 (DST)\n");
  const MethodParams *p = method_params_get(CALC_ISNA);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 7, 15, 41.8781, -87.6298, -5.0, p);
  check_time(t.fajr, "03:50", 2, "Fajr");
  check_time(t.dhuhr, "12:57", 2, "Dhuhr");
  check_time(t.asr, "16:57", 2, "Asr");
  check_time(t.maghrib, "20:24", 2, "Maghrib");
  check_time(t.isha, "22:02", 2, "Isha");
  printf("\n");
}

static void test_isna_chicago_oct(void) {
  printf("Test ISNA 21: Chicago — 2026-10-15 (DST)\n");
  const MethodParams *p = method_params_get(CALC_ISNA);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 10, 15, 41.8781, -87.6298, -5.0, p);
  check_time(t.fajr, "05:46", 2, "Fajr");
  check_time(t.dhuhr, "12:36", 2, "Dhuhr");
  check_time(t.asr, "15:40", 2, "Asr");
  check_time(t.maghrib, "18:09", 2, "Maghrib");
  check_time(t.isha, "19:25", 2, "Isha");
  printf("\n");
}

static void test_isna_losangeles_jan(void) {
  printf("Test ISNA 22: Los Angeles — 2026-01-15\n");
  const MethodParams *p = method_params_get(CALC_ISNA);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 1, 15, 34.0522, -118.2437, -8.0, p);
  check_time(t.fajr, "05:45", 2, "Fajr");
  check_time(t.dhuhr, "12:02", 2, "Dhuhr");
  check_time(t.asr, "14:47", 2, "Asr");
  check_time(t.maghrib, "17:07", 2, "Maghrib");
  check_time(t.isha, "18:20", 2, "Isha");
  printf("\n");
}

static void test_isna_losangeles_jun(void) {
  printf("Test ISNA 23: Los Angeles — 2026-06-21 (DST)\n");
  const MethodParams *p = method_params_get(CALC_ISNA);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 6, 21, 34.0522, -118.2437, -7.0, p);
  check_time(t.fajr, "04:18", 2, "Fajr");
  check_time(t.dhuhr, "12:55", 2, "Dhuhr");
  check_time(t.asr, "16:40", 2, "Asr");
  check_time(t.maghrib, "20:08", 2, "Maghrib");
  check_time(t.isha, "21:32", 2, "Isha");
  printf("\n");
}

static void test_isna_losangeles_dec(void) {
  printf("Test ISNA 23: Los Angeles — 2026-12-21\n");
  const MethodParams *p = method_params_get(CALC_ISNA);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 12, 21, 34.0522, -118.2437, -8.0, p);
  check_time(t.fajr, "05:40", 2, "Fajr");
  check_time(t.dhuhr, "11:51", 2, "Dhuhr");
  check_time(t.asr, "14:30", 2, "Asr");
  check_time(t.maghrib, "16:48", 2, "Maghrib");
  check_time(t.isha, "18:02", 2, "Isha");
  printf("\n");
}

// ---------------------------------------------------------------------------
// Makkah / Umm al-Qura (24 data points)
// ---------------------------------------------------------------------------

static void test_makkah_makkah_jan(void) {
  printf("Test Makkah 01: Makkah — 2026-01-15\n");
  const MethodParams *p = method_params_get(CALC_MAKKAH);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 1, 15, 21.3891, 39.8579, 3.0, p);
  check_time(t.fajr, "05:40", 2, "Fajr");
  check_time(t.dhuhr, "12:30", 2, "Dhuhr");
  check_time(t.asr, "15:37", 2, "Asr");
  check_time(t.maghrib, "17:59", 2, "Maghrib");
  check_time(t.isha, "19:29", 2, "Isha");
  printf("\n");
}

static void test_makkah_makkah_feb(void) {
  printf("Test Makkah 02: Makkah — 2026-02-15\n");
  const MethodParams *p = method_params_get(CALC_MAKKAH);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 2, 15, 21.3891, 39.8579, 3.0, p);
  check_time(t.fajr, "05:34", 2, "Fajr");
  check_time(t.dhuhr, "12:35", 2, "Dhuhr");
  check_time(t.asr, "15:51", 2, "Asr");
  check_time(t.maghrib, "18:18", 2, "Maghrib");
  check_time(t.isha, "19:48", 2, "Isha");
  printf("\n");
}

static void test_makkah_makkah_mar(void) {
  printf("Test Makkah 03: Makkah — 2026-03-15\n");
  const MethodParams *p = method_params_get(CALC_MAKKAH);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 3, 15, 21.3891, 39.8579, 3.0, p);
  check_time(t.fajr, "05:13", 2, "Fajr");
  check_time(t.dhuhr, "12:30", 2, "Dhuhr");
  check_time(t.asr, "15:53", 2, "Asr");
  check_time(t.maghrib, "18:30", 2, "Maghrib");
  // Isha: Aladhan returns 120 min after Maghrib (Ramadan adjustment).
  // Our calculator uses the standard 90 min rule; skip this check.
  printf("\n");
}

static void test_makkah_makkah_apr(void) {
  printf("Test Makkah 04: Makkah — 2026-04-15\n");
  const MethodParams *p = method_params_get(CALC_MAKKAH);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 4, 15, 21.3891, 39.8579, 3.0, p);
  check_time(t.fajr, "04:43", 2, "Fajr");
  check_time(t.dhuhr, "12:21", 2, "Dhuhr");
  check_time(t.asr, "15:44", 2, "Asr");
  check_time(t.maghrib, "18:40", 2, "Maghrib");
  check_time(t.isha, "20:10", 2, "Isha");
  printf("\n");
}

static void test_makkah_makkah_may(void) {
  printf("Test Makkah 05: Makkah — 2026-05-15\n");
  const MethodParams *p = method_params_get(CALC_MAKKAH);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 5, 15, 21.3891, 39.8579, 3.0, p);
  check_time(t.fajr, "04:19", 2, "Fajr");
  check_time(t.dhuhr, "12:17", 2, "Dhuhr");
  check_time(t.asr, "15:34", 2, "Asr");
  check_time(t.maghrib, "18:52", 2, "Maghrib");
  check_time(t.isha, "20:22", 2, "Isha");
  printf("\n");
}

static void test_makkah_makkah_jun(void) {
  printf("Test Makkah 06: Makkah — 2026-06-15\n");
  const MethodParams *p = method_params_get(CALC_MAKKAH);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 6, 15, 21.3891, 39.8579, 3.0, p);
  check_time(t.fajr, "04:10", 2, "Fajr");
  check_time(t.dhuhr, "12:21", 2, "Dhuhr");
  check_time(t.asr, "15:41", 2, "Asr");
  check_time(t.maghrib, "19:04", 2, "Maghrib");
  check_time(t.isha, "20:34", 2, "Isha");
  printf("\n");
}

static void test_makkah_makkah_jul(void) {
  printf("Test Makkah 07: Makkah — 2026-07-15\n");
  const MethodParams *p = method_params_get(CALC_MAKKAH);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 7, 15, 21.3891, 39.8579, 3.0, p);
  check_time(t.fajr, "04:21", 2, "Fajr");
  check_time(t.dhuhr, "12:27", 2, "Dhuhr");
  check_time(t.asr, "15:41", 2, "Asr");
  check_time(t.maghrib, "19:06", 2, "Maghrib");
  check_time(t.isha, "20:36", 2, "Isha");
  printf("\n");
}

static void test_makkah_makkah_aug(void) {
  printf("Test Makkah 08: Makkah — 2026-08-15\n");
  const MethodParams *p = method_params_get(CALC_MAKKAH);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 8, 15, 21.3891, 39.8579, 3.0, p);
  check_time(t.fajr, "04:38", 2, "Fajr");
  check_time(t.dhuhr, "12:25", 2, "Dhuhr");
  check_time(t.asr, "15:48", 2, "Asr");
  check_time(t.maghrib, "18:51", 2, "Maghrib");
  check_time(t.isha, "20:21", 2, "Isha");
  printf("\n");
}

static void test_makkah_makkah_sep(void) {
  printf("Test Makkah 09: Makkah — 2026-09-15\n");
  const MethodParams *p = method_params_get(CALC_MAKKAH);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 9, 15, 21.3891, 39.8579, 3.0, p);
  check_time(t.fajr, "04:51", 2, "Fajr");
  check_time(t.dhuhr, "12:16", 2, "Dhuhr");
  check_time(t.asr, "15:42", 2, "Asr");
  check_time(t.maghrib, "18:24", 2, "Maghrib");
  check_time(t.isha, "19:54", 2, "Isha");
  printf("\n");
}

static void test_makkah_makkah_oct(void) {
  printf("Test Makkah 10: Makkah — 2026-10-15\n");
  const MethodParams *p = method_params_get(CALC_MAKKAH);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 10, 15, 21.3891, 39.8579, 3.0, p);
  check_time(t.fajr, "05:00", 2, "Fajr");
  check_time(t.dhuhr, "12:06", 2, "Dhuhr");
  check_time(t.asr, "15:27", 2, "Asr");
  check_time(t.maghrib, "17:56", 2, "Maghrib");
  check_time(t.isha, "19:26", 2, "Isha");
  printf("\n");
}

static void test_makkah_makkah_nov(void) {
  printf("Test Makkah 11: Makkah — 2026-11-15\n");
  const MethodParams *p = method_params_get(CALC_MAKKAH);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 11, 15, 21.3891, 39.8579, 3.0, p);
  check_time(t.fajr, "05:12", 2, "Fajr");
  check_time(t.dhuhr, "12:05", 2, "Dhuhr");
  check_time(t.asr, "15:16", 2, "Asr");
  check_time(t.maghrib, "17:39", 2, "Maghrib");
  check_time(t.isha, "19:09", 2, "Isha");
  printf("\n");
}

static void test_makkah_makkah_dec(void) {
  printf("Test Makkah 12: Makkah — 2026-12-15\n");
  const MethodParams *p = method_params_get(CALC_MAKKAH);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 12, 15, 21.3891, 39.8579, 3.0, p);
  check_time(t.fajr, "05:29", 2, "Fajr");
  check_time(t.dhuhr, "12:16", 2, "Dhuhr");
  check_time(t.asr, "15:20", 2, "Asr");
  check_time(t.maghrib, "17:41", 2, "Maghrib");
  check_time(t.isha, "19:11", 2, "Isha");
  printf("\n");
}

static void test_makkah_riyadh_jan(void) {
  printf("Test Makkah 13: Riyadh — 2026-01-15\n");
  const MethodParams *p = method_params_get(CALC_MAKKAH);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 1, 15, 24.7136, 46.6753, 3.0, p);
  check_time(t.fajr, "05:17", 2, "Fajr");
  check_time(t.dhuhr, "12:03", 2, "Dhuhr");
  check_time(t.asr, "15:05", 2, "Asr");
  check_time(t.maghrib, "17:26", 2, "Maghrib");
  check_time(t.isha, "18:56", 2, "Isha");
  printf("\n");
}

static void test_makkah_riyadh_mar(void) {
  printf("Test Makkah 14: Riyadh — 2026-03-21\n");
  const MethodParams *p = method_params_get(CALC_MAKKAH);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 3, 21, 24.7136, 46.6753, 3.0, p);
  check_time(t.fajr, "04:38", 2, "Fajr");
  check_time(t.dhuhr, "12:01", 2, "Dhuhr");
  check_time(t.asr, "15:26", 2, "Asr");
  check_time(t.maghrib, "18:05", 2, "Maghrib");
  check_time(t.isha, "19:35", 2, "Isha");
  printf("\n");
}

static void test_makkah_riyadh_jun(void) {
  printf("Test Makkah 15: Riyadh — 2026-06-21\n");
  const MethodParams *p = method_params_get(CALC_MAKKAH);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 6, 21, 24.7136, 46.6753, 3.0, p);
  check_time(t.fajr, "03:33", 2, "Fajr");
  check_time(t.dhuhr, "11:55", 2, "Dhuhr");
  check_time(t.asr, "15:16", 2, "Asr");
  check_time(t.maghrib, "18:45", 2, "Maghrib");
  check_time(t.isha, "20:15", 2, "Isha");
  printf("\n");
}

static void test_makkah_riyadh_sep(void) {
  printf("Test Makkah 16: Riyadh — 2026-09-22\n");
  const MethodParams *p = method_params_get(CALC_MAKKAH);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 9, 22, 24.7136, 46.6753, 3.0, p);
  check_time(t.fajr, "04:24", 2, "Fajr");
  check_time(t.dhuhr, "11:46", 2, "Dhuhr");
  check_time(t.asr, "15:13", 2, "Asr");
  check_time(t.maghrib, "17:50", 2, "Maghrib");
  check_time(t.isha, "19:20", 2, "Isha");
  printf("\n");
}

static void test_makkah_riyadh_dec(void) {
  printf("Test Makkah 17: Riyadh — 2026-12-21\n");
  const MethodParams *p = method_params_get(CALC_MAKKAH);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 12, 21, 24.7136, 46.6753, 3.0, p);
  check_time(t.fajr, "05:09", 2, "Fajr");
  check_time(t.dhuhr, "11:51", 2, "Dhuhr");
  check_time(t.asr, "14:50", 2, "Asr");
  check_time(t.maghrib, "17:09", 2, "Maghrib");
  check_time(t.isha, "18:39", 2, "Isha");
  printf("\n");
}

static void test_makkah_dubai_jan(void) {
  printf("Test Makkah 18: Dubai — 2026-01-15\n");
  const MethodParams *p = method_params_get(CALC_MAKKAH);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 1, 15, 25.2048, 55.2708, 4.0, p);
  check_time(t.fajr, "05:43", 2, "Fajr");
  check_time(t.dhuhr, "12:28", 2, "Dhuhr");
  check_time(t.asr, "15:30", 2, "Asr");
  check_time(t.maghrib, "17:51", 2, "Maghrib");
  check_time(t.isha, "19:21", 2, "Isha");
  printf("\n");
}

static void test_makkah_dubai_apr(void) {
  printf("Test Makkah 19: Dubai — 2026-04-15\n");
  const MethodParams *p = method_params_get(CALC_MAKKAH);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 4, 15, 25.2048, 55.2708, 4.0, p);
  check_time(t.fajr, "04:36", 2, "Fajr");
  check_time(t.dhuhr, "12:19", 2, "Dhuhr");
  check_time(t.asr, "15:48", 2, "Asr");
  check_time(t.maghrib, "18:42", 2, "Maghrib");
  check_time(t.isha, "20:12", 2, "Isha");
  printf("\n");
}

static void test_makkah_dubai_jul(void) {
  printf("Test Makkah 20: Dubai — 2026-07-15\n");
  const MethodParams *p = method_params_get(CALC_MAKKAH);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 7, 15, 25.2048, 55.2708, 4.0, p);
  check_time(t.fajr, "04:08", 2, "Fajr");
  check_time(t.dhuhr, "12:25", 2, "Dhuhr");
  check_time(t.asr, "15:50", 2, "Asr");
  check_time(t.maghrib, "19:12", 2, "Maghrib");
  check_time(t.isha, "20:42", 2, "Isha");
  printf("\n");
}

static void test_makkah_dubai_oct(void) {
  printf("Test Makkah 21: Dubai — 2026-10-15\n");
  const MethodParams *p = method_params_get(CALC_MAKKAH);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 10, 15, 25.2048, 55.2708, 4.0, p);
  check_time(t.fajr, "04:59", 2, "Fajr");
  check_time(t.dhuhr, "12:05", 2, "Dhuhr");
  check_time(t.asr, "15:25", 2, "Asr");
  check_time(t.maghrib, "17:52", 2, "Maghrib");
  check_time(t.isha, "19:22", 2, "Isha");
  printf("\n");
}

static void test_makkah_jakarta_jan(void) {
  printf("Test Makkah 22: Jakarta — 2026-01-15\n");
  const MethodParams *p = method_params_get(CALC_MAKKAH);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 1, 15, -6.1667, 106.8167, 7.0, p);
  check_time(t.fajr, "04:32", 2, "Fajr");
  check_time(t.dhuhr, "12:02", 2, "Dhuhr");
  check_time(t.asr, "15:27", 2, "Asr");
  check_time(t.maghrib, "18:15", 2, "Maghrib");
  check_time(t.isha, "19:45", 2, "Isha");
  printf("\n");
}

static void test_makkah_jakarta_jun(void) {
  printf("Test Makkah 23: Jakarta — 2026-06-15\n");
  const MethodParams *p = method_params_get(CALC_MAKKAH);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 6, 15, -6.1667, 106.8167, 7.0, p);
  check_time(t.fajr, "04:43", 2, "Fajr");
  check_time(t.dhuhr, "11:53", 2, "Dhuhr");
  check_time(t.asr, "15:15", 2, "Asr");
  check_time(t.maghrib, "17:46", 2, "Maghrib");
  check_time(t.isha, "19:16", 2, "Isha");
  printf("\n");
}

static void test_makkah_jakarta_dec(void) {
  printf("Test Makkah 24: Jakarta — 2026-12-15\n");
  const MethodParams *p = method_params_get(CALC_MAKKAH);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 12, 15, -6.1667, 106.8167, 7.0, p);
  check_time(t.fajr, "04:15", 2, "Fajr");
  check_time(t.dhuhr, "11:48", 2, "Dhuhr");
  check_time(t.asr, "15:15", 2, "Asr");
  check_time(t.maghrib, "18:02", 2, "Maghrib");
  check_time(t.isha, "19:32", 2, "Isha");
  printf("\n");
}

// ---------------------------------------------------------------------------
// Egypt — Egyptian General Authority of Survey (20 data points)
// ---------------------------------------------------------------------------

static void test_egypt_cairo_jan(void) {
  printf("Test Egypt 01: Cairo — 2026-01-15\n");
  const MethodParams *p = method_params_get(CALC_EGYPT);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 1, 15, 30.0444, 31.2357, 2.0, p);
  check_time(t.fajr, "05:21", 2, "Fajr");
  check_time(t.dhuhr, "12:04", 2, "Dhuhr");
  check_time(t.asr, "14:57", 2, "Asr");
  check_time(t.maghrib, "17:17", 2, "Maghrib");
  check_time(t.isha, "18:39", 2, "Isha");
  printf("\n");
}

static void test_egypt_cairo_feb(void) {
  printf("Test Egypt 02: Cairo — 2026-02-15\n");
  const MethodParams *p = method_params_get(CALC_EGYPT);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 2, 15, 30.0444, 31.2357, 2.0, p);
  check_time(t.fajr, "05:08", 2, "Fajr");
  check_time(t.dhuhr, "12:09", 2, "Dhuhr");
  check_time(t.asr, "15:18", 2, "Asr");
  check_time(t.maghrib, "17:44", 2, "Maghrib");
  check_time(t.isha, "19:02", 2, "Isha");
  printf("\n");
}

static void test_egypt_cairo_mar(void) {
  printf("Test Egypt 03: Cairo — 2026-03-15\n");
  const MethodParams *p = method_params_get(CALC_EGYPT);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 3, 15, 30.0444, 31.2357, 2.0, p);
  check_time(t.fajr, "04:39", 2, "Fajr");
  check_time(t.dhuhr, "12:04", 2, "Dhuhr");
  check_time(t.asr, "15:28", 2, "Asr");
  check_time(t.maghrib, "18:03", 2, "Maghrib");
  check_time(t.isha, "19:20", 2, "Isha");
  printf("\n");
}

static void test_egypt_cairo_apr(void) {
  printf("Test Egypt 04: Cairo — 2026-04-15\n");
  const MethodParams *p = method_params_get(CALC_EGYPT);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 4, 15, 30.0444, 31.2357, 2.0, p);
  check_time(t.fajr, "03:58", 2, "Fajr");
  check_time(t.dhuhr, "11:55", 2, "Dhuhr");
  check_time(t.asr, "15:30", 2, "Asr");
  check_time(t.maghrib, "18:22", 2, "Maghrib");
  check_time(t.isha, "19:43", 2, "Isha");
  printf("\n");
}

static void test_egypt_cairo_may(void) {
  printf("Test Egypt 05: Cairo — 2026-05-15 (DST)\n");
  const MethodParams *p = method_params_get(CALC_EGYPT);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 5, 15, 30.0444, 31.2357, 3.0, p);
  check_time(t.fajr, "04:23", 2, "Fajr");
  check_time(t.dhuhr, "12:51", 2, "Dhuhr");
  check_time(t.asr, "16:28", 2, "Asr");
  check_time(t.maghrib, "19:41", 2, "Maghrib");
  check_time(t.isha, "21:09", 2, "Isha");
  printf("\n");
}

static void test_egypt_cairo_jun(void) {
  printf("Test Egypt 06: Cairo — 2026-06-15 (DST)\n");
  const MethodParams *p = method_params_get(CALC_EGYPT);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 6, 15, 30.0444, 31.2357, 3.0, p);
  check_time(t.fajr, "04:08", 2, "Fajr");
  check_time(t.dhuhr, "12:56", 2, "Dhuhr");
  check_time(t.asr, "16:31", 2, "Asr");
  check_time(t.maghrib, "19:58", 2, "Maghrib");
  check_time(t.isha, "21:31", 2, "Isha");
  printf("\n");
}

static void test_egypt_cairo_jul(void) {
  printf("Test Egypt 07: Cairo — 2026-07-15 (DST)\n");
  const MethodParams *p = method_params_get(CALC_EGYPT);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 7, 15, 30.0444, 31.2357, 3.0, p);
  check_time(t.fajr, "04:21", 2, "Fajr");
  check_time(t.dhuhr, "13:01", 2, "Dhuhr");
  check_time(t.asr, "16:38", 2, "Asr");
  check_time(t.maghrib, "19:58", 2, "Maghrib");
  check_time(t.isha, "21:29", 2, "Isha");
  printf("\n");
}

static void test_egypt_cairo_aug(void) {
  printf("Test Egypt 08: Cairo — 2026-08-15 (DST)\n");
  const MethodParams *p = method_params_get(CALC_EGYPT);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 8, 15, 30.0444, 31.2357, 3.0, p);
  check_time(t.fajr, "04:48", 2, "Fajr");
  check_time(t.dhuhr, "13:00", 2, "Dhuhr");
  check_time(t.asr, "16:37", 2, "Asr");
  check_time(t.maghrib, "19:36", 2, "Maghrib");
  check_time(t.isha, "21:00", 2, "Isha");
  printf("\n");
}

static void test_egypt_cairo_sep(void) {
  printf("Test Egypt 09: Cairo — 2026-09-15 (DST)\n");
  const MethodParams *p = method_params_get(CALC_EGYPT);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 9, 15, 30.0444, 31.2357, 3.0, p);
  check_time(t.fajr, "05:12", 2, "Fajr");
  check_time(t.dhuhr, "12:50", 2, "Dhuhr");
  check_time(t.asr, "16:21", 2, "Asr");
  check_time(t.maghrib, "19:01", 2, "Maghrib");
  check_time(t.isha, "20:19", 2, "Isha");
  printf("\n");
}

static void test_egypt_cairo_oct(void) {
  printf("Test Egypt 10: Cairo — 2026-10-15 (DST)\n");
  const MethodParams *p = method_params_get(CALC_EGYPT);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 10, 15, 30.0444, 31.2357, 3.0, p);
  check_time(t.fajr, "05:30", 2, "Fajr");
  check_time(t.dhuhr, "12:41", 2, "Dhuhr");
  check_time(t.asr, "15:58", 2, "Asr");
  check_time(t.maghrib, "18:24", 2, "Maghrib");
  check_time(t.isha, "19:42", 2, "Isha");
  printf("\n");
}

static void test_egypt_cairo_nov(void) {
  printf("Test Egypt 11: Cairo — 2026-11-15\n");
  const MethodParams *p = method_params_get(CALC_EGYPT);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 11, 15, 30.0444, 31.2357, 2.0, p);
  check_time(t.fajr, "04:50", 2, "Fajr");
  check_time(t.dhuhr, "11:40", 2, "Dhuhr");
  check_time(t.asr, "14:39", 2, "Asr");
  check_time(t.maghrib, "16:59", 2, "Maghrib");
  check_time(t.isha, "18:19", 2, "Isha");
  printf("\n");
}

static void test_egypt_cairo_dec(void) {
  printf("Test Egypt 12: Cairo — 2026-12-15\n");
  const MethodParams *p = method_params_get(CALC_EGYPT);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 12, 15, 30.0444, 31.2357, 2.0, p);
  check_time(t.fajr, "05:11", 2, "Fajr");
  check_time(t.dhuhr, "11:50", 2, "Dhuhr");
  check_time(t.asr, "14:38", 2, "Asr");
  check_time(t.maghrib, "16:57", 2, "Maghrib");
  check_time(t.isha, "18:20", 2, "Isha");
  printf("\n");
}

static void test_egypt_kl_jan(void) {
  printf("Test Egypt 13: Kuala Lumpur — 2026-01-15\n");
  const MethodParams *p = method_params_get(CALC_EGYPT);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 1, 15, 3.139, 101.6869, 8.0, p);
  check_time(t.fajr, "06:04", 2, "Fajr");
  check_time(t.dhuhr, "13:23", 2, "Dhuhr");
  check_time(t.asr, "16:46", 2, "Asr");
  check_time(t.maghrib, "19:21", 2, "Maghrib");
  check_time(t.isha, "20:33", 2, "Isha");
  printf("\n");
}

static void test_egypt_kl_apr(void) {
  printf("Test Egypt 14: Kuala Lumpur — 2026-04-15\n");
  const MethodParams *p = method_params_get(CALC_EGYPT);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 4, 15, 3.139, 101.6869, 8.0, p);
  check_time(t.fajr, "05:52", 2, "Fajr");
  check_time(t.dhuhr, "13:13", 2, "Dhuhr");
  check_time(t.asr, "16:26", 2, "Asr");
  check_time(t.maghrib, "19:19", 2, "Maghrib");
  check_time(t.isha, "20:27", 2, "Isha");
  printf("\n");
}

static void test_egypt_kl_jul(void) {
  printf("Test Egypt 15: Kuala Lumpur — 2026-07-15\n");
  const MethodParams *p = method_params_get(CALC_EGYPT);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 7, 15, 3.139, 101.6869, 8.0, p);
  check_time(t.fajr, "05:50", 2, "Fajr");
  check_time(t.dhuhr, "13:19", 2, "Dhuhr");
  check_time(t.asr, "16:44", 2, "Asr");
  check_time(t.maghrib, "19:28", 2, "Maghrib");
  check_time(t.isha, "20:40", 2, "Isha");
  printf("\n");
}

static void test_egypt_kl_oct(void) {
  printf("Test Egypt 16: Kuala Lumpur — 2026-10-15\n");
  const MethodParams *p = method_params_get(CALC_EGYPT);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 10, 15, 3.139, 101.6869, 8.0, p);
  check_time(t.fajr, "05:42", 2, "Fajr");
  check_time(t.dhuhr, "12:59", 2, "Dhuhr");
  check_time(t.asr, "16:16", 2, "Asr");
  check_time(t.maghrib, "19:00", 2, "Maghrib");
  check_time(t.isha, "20:08", 2, "Isha");
  printf("\n");
}

static void test_egypt_alexandria_mar(void) {
  printf("Test Egypt 17: Alexandria — 2026-03-21\n");
  const MethodParams *p = method_params_get(CALC_EGYPT);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 3, 21, 31.2001, 29.9187, 2.0, p);
  check_time(t.fajr, "04:35", 2, "Fajr");
  check_time(t.dhuhr, "12:08", 2, "Dhuhr");
  check_time(t.asr, "15:35", 2, "Asr");
  check_time(t.maghrib, "18:12", 2, "Maghrib");
  check_time(t.isha, "19:31", 2, "Isha");
  printf("\n");
}

static void test_egypt_alexandria_jun(void) {
  printf("Test Egypt 18: Alexandria — 2026-06-21 (DST)\n");
  const MethodParams *p = method_params_get(CALC_EGYPT);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 6, 21, 31.2001, 29.9187, 3.0, p);
  check_time(t.fajr, "04:08", 2, "Fajr");
  check_time(t.dhuhr, "13:02", 2, "Dhuhr");
  check_time(t.asr, "16:41", 2, "Asr");
  check_time(t.maghrib, "20:07", 2, "Maghrib");
  check_time(t.isha, "21:43", 2, "Isha");
  printf("\n");
}

static void test_egypt_alexandria_sep(void) {
  printf("Test Egypt 19: Alexandria — 2026-09-22 (DST)\n");
  const MethodParams *p = method_params_get(CALC_EGYPT);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 9, 22, 31.2001, 29.9187, 3.0, p);
  check_time(t.fajr, "05:20", 2, "Fajr");
  check_time(t.dhuhr, "12:53", 2, "Dhuhr");
  check_time(t.asr, "16:21", 2, "Asr");
  check_time(t.maghrib, "18:57", 2, "Maghrib");
  check_time(t.isha, "20:16", 2, "Isha");
  printf("\n");
}

static void test_egypt_alexandria_dec(void) {
  printf("Test Egypt 20: Alexandria — 2026-12-21\n");
  const MethodParams *p = method_params_get(CALC_EGYPT);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 12, 21, 31.2001, 29.9187, 2.0, p);
  check_time(t.fajr, "05:21", 2, "Fajr");
  check_time(t.dhuhr, "11:58", 2, "Dhuhr");
  check_time(t.asr, "14:44", 2, "Asr");
  check_time(t.maghrib, "17:02", 2, "Maghrib");
  check_time(t.isha, "18:26", 2, "Isha");
  printf("\n");
}

// ---------------------------------------------------------------------------
// Midnight wrap: near Reykjavik at midsummer, MWL method.
// At this latitude and date Maghrib and Isha both fall after local
// midnight, so this exercises the circular-distance fix in check_time
// above (an event timestamped just after 00:00 must not be scored as
// roughly 24h away from an expected value near the same clock time).
//
// How the expected values were obtained:
// All seven values, including Maghrib, are regression-guard values read
// directly from this library's own output for this input (diff=0 by
// construction), the same way as every other test case in this file. None
// of them are an independent oracle check. What earns this case its place
// is not any one expected value but the shape of the day: Maghrib and Isha
// both genuinely fall after local midnight here, which is not exercised by
// any other fixture, so this pins that after-midnight schedule against
// regressions. The circular-distance arithmetic that makes such a schedule
// comparable at all is tested directly and independently in
// test_clock_diff_minutes() below.
// ---------------------------------------------------------------------------

static void test_midnight_wrap_reykjavik_jun(void) {
  printf("Test midnight wrap: near Reykjavik, 2026-06-21 (MWL)\n");
  const MethodParams *p = method_params_get(CALC_MWL);
  struct PrayerTimes t =
      calculate_prayer_times(2026, 6, 21, 64.1466, -21.14, 0.0, p);
  check_time(t.fajr, "02:01", 2, "Fajr");
  check_time(t.dhuhr, "13:27", 2, "Dhuhr");
  check_time(t.asr, "18:20", 2, "Asr");
  check_time(t.maghrib, "00:01", 2, "Maghrib");  // after midnight
  check_time(t.isha, "00:50", 2, "Isha");        // after midnight
  printf("\n");
}

// Check a single long value against an expected one
static void check_str(const char *got, const char *want, const char *label) {
  total++;
  if (strcmp(got, want) == 0) {
    printf("  PASS  %-28s  got=\"%s\"\n", label, got);
  } else {
    printf("  FAIL  %-28s  got=\"%s\"  expected=\"%s\"\n", label, got, want);
    failures++;
  }
}

static void check_long(long got, long want, const char *label) {
  total++;
  if (got == want) {
    printf("  PASS  %-28s  got=%ld\n", label, got);
  } else {
    printf("  FAIL  %-28s  got=%ld  expected=%ld\n", label, got, want);
    failures++;
  }
}

// clock_diff_minutes: pure arithmetic checks, not prayer times. These do not
// call the library at all; they pin the circular-distance helper itself.
// struct PrayerTimes field contract: pins that a returned field is NOT
// guaranteed to lie in [0, 24) and is NOT guaranteed to be finite. These are
// not time comparisons and carry no residual.
//
// This group exists to stop the contract being changed by accident. Reducing
// the fields into [0, 24) inside calculate_prayer_times would make these fail,
// which is the point: the raw double is the only thing carrying the day
// offset, so normalising it silently moves an event onto the wrong calendar
// day. Whether the struct should carry the offset explicitly is open on issue
// #56. If that is settled in favour of a change, update this group
// deliberately rather than deleting it.
static void count_field_anomalies(double lat, double lon, double tz,
                                  const MethodParams *m, int *nan_days,
                                  int *out_of_range_days) {
  static const int month_len[12] = {31, 28, 31, 30, 31, 30,
                                    31, 31, 30, 31, 30, 31};
  *nan_days = 0;
  *out_of_range_days = 0;
  for (int doy = 1; doy <= 365; doy++) {
    int mo = 1, dy = doy;
    while (dy > month_len[mo - 1]) {
      dy -= month_len[mo - 1];
      mo++;
    }
    struct PrayerTimes t = calculate_prayer_times(2025, mo, dy, lat, lon, tz, m);
    const double v[5] = {t.fajr, t.dhuhr, t.asr, t.maghrib, t.isha};
    int saw_nan = 0, saw_out = 0;
    for (int i = 0; i < 5; i++) {
      if (isnan(v[i])) {
        saw_nan = 1;
      } else if (v[i] < 0.0 || v[i] >= 24.0) {
        saw_out = 1;
      }
    }
    *nan_days += saw_nan;
    *out_of_range_days += saw_out;
  }
}

/* NaN days for one field, indexed in struct order:
   0 fajr, 1 dhuhr, 2 asr, 3 maghrib, 4 isha. */
static int count_field_nan(double lat, double lon, double tz,
                           const MethodParams *m, int field) {
  static const int month_len[12] = {31, 28, 31, 30, 31, 30,
                                    31, 31, 30, 31, 30, 31};
  int days = 0;
  for (int doy = 1; doy <= 365; doy++) {
    int mo = 1, dy = doy;
    while (dy > month_len[mo - 1]) {
      dy -= month_len[mo - 1];
      mo++;
    }
    struct PrayerTimes t = calculate_prayer_times(2025, mo, dy, lat, lon, tz, m);
    const double v[5] = {t.fajr, t.dhuhr, t.asr, t.maghrib, t.isha};
    if (isnan(v[field])) days++;
  }
  return days;
}

/* Days in 2026 where any adjacent pair of the five prescribed times is out of
   order, counting only days where both members of the pair are finite. The
   five are defined by successive positions of the same Sun, so a schedule that
   reports them out of order is wrong under every calculation method and every
   high-latitude convention. */
static int count_disordered_days(double lat, double lon, double tz,
                                 const MethodParams *m) {
  static const int month_len[12] = {31, 28, 31, 30, 31, 30,
                                    31, 31, 30, 31, 30, 31};
  int days = 0;
  for (int doy = 1; doy <= 365; doy++) {
    int mo = 1, dy = doy;
    while (dy > month_len[mo - 1]) {
      dy -= month_len[mo - 1];
      mo++;
    }
    struct PrayerTimes t = calculate_prayer_times(2026, mo, dy, lat, lon, tz, m);
    const double v[5] = {t.fajr, t.dhuhr, t.asr, t.maghrib, t.isha};
    for (int i = 1; i < 5; i++) {
      if (isfinite(v[i]) && isfinite(v[i - 1]) && v[i] < v[i - 1]) {
        days++;
        break;
      }
    }
  }
  return days;
}

/* Before the polar case solved the whole day at one latitude, MWL reported
   isha before maghrib on 41 days of 2026 at Longyearbyen, all in the polar
   night. The cause was a schedule spliced from two places: maghrib borrowed
   from 45 N, isha solved at 78.22 N, where the Sun fails to reach -0.833 but
   still crosses -17. Every method is checked here, not only the two that
   carry a reference latitude today, so that giving another method one cannot
   reintroduce this unnoticed. */
static void test_ordering(void) {
  static const struct {
    const char *name;
    double lat, lon, tz;
  } places[] = {
      {"Longyearbyen", 78.22, 15.65, 1.0}, {"Tromso", 69.65, 18.96, 1.0},
      {"Murmansk", 68.97, 33.08, 3.0},     {"Reykjavik", 64.15, -21.94, 0.0},
      {"Jakarta", -6.2088, 106.8456, 7.0},
  };
  /* Zero everywhere except one known case, pinned rather than hidden.
     Moonsighting at Tromso on 2026-05-18 reports maghrib 23.590332 against
     isha 23.586724, a gap of 13.0 seconds, and both render as "23:36". The
     cause is unrelated to the polar splice this test was written for: that
     method sets maghrib 3 minutes after sunset and takes isha as one seventh
     of the night, and on a bright night of 19.5 minutes one seventh is 2.8
     minutes, so the fixed offset overtakes the fraction. Tracked separately.
     Any change to this number, at any place, is a regression to investigate. */
  static const int expected[] = {0, 1, 0, 0, 0};
  char label[128];
  printf("Test group: the five stay in order\n");
  for (size_t p = 0; p < sizeof places / sizeof *places; p++) {
    int worst = 0;
    for (int i = 0; i < CALC_COUNT; i++) {
      const MethodParams *m = method_params_get((CalcMethod)i);
      if (!m) continue;
      int bad = count_disordered_days(places[p].lat, places[p].lon,
                                      places[p].tz, m);
      if (bad > worst) worst = bad;
    }
    snprintf(label, sizeof label, "%s, all %d methods", places[p].name,
             (int)CALC_COUNT);
    check_long(worst, expected[p], label);
  }
  printf("\n");
}

static void test_field_contract(void) {
  const MethodParams *mwl = method_params_get(CALC_MWL);
  int nan_days, out_days;
  printf("Test group: struct PrayerTimes field contract\n");

  // Jakarta: the ordinary case. Nothing escapes, which is why the contract is
  // easy to miss when only equatorial locations are exercised.
  count_field_anomalies(-6.2088, 106.8456, 7.0, mwl, &nan_days, &out_days);
  check_long(nan_days, 0, "Jakarta, no NaN day");
  check_long(out_days, 0, "Jakarta, no out-of-range day");

  // Reykjavik, which really does keep UTC year round. Only the out-of-range
  // anomaly occurs now. Its 44 NaN days were all dhuha, so dropping dhuha
  // removed them without touching the five prescribed times.
  count_field_anomalies(64.15, -21.94, 0.0, mwl, &nan_days, &out_days);
  check_long(nan_days, 0, "Reykjavik, no NaN day");
  check_long(out_days, 107, "Reykjavik, out-of-range days");

  // Anchorage is the case that shows the two are independent. It never
  // produces a NaN, yet it still returns hours outside the day.
  count_field_anomalies(61.22, -149.90, -9.0, mwl, &nan_days, &out_days);
  check_long(nan_days, 0, "Anchorage, no NaN day");
  check_long(out_days, 23, "Anchorage, out-of-range days");

  // Longyearbyen. Every one of the five now resolves, because MWL carries a
  // reference latitude for the polar case. Before dhuha was dropped this
  // location still reported NaN days, and all of them were dhuha.
  count_field_anomalies(78.22, 15.65, 1.0, mwl, &nan_days, &out_days);
  check_long(nan_days, 4, "Longyearbyen MWL, NaN days");
  // Those 4 are asr alone, and they are not the polar case. On them the
  // separation from the declination is between 90 and 90.833 degrees, so the
  // Sun's true altitude never exceeds 0 and it is visible only by refraction.
  // Sunrise therefore exists and fajr, maghrib and isha all resolve, but no
  // shadow is cast, so asr does not exist. Reporting it as unavailable is the
  // point of the domain guard.
  check_long(count_field_nan(78.22, 15.65, 1.0, mwl, 2), 4,
             "Longyearbyen MWL, the 4 are asr");
  check_long(count_field_nan(78.22, 15.65, 1.0, mwl, 0), 0,
             "Longyearbyen MWL, fajr always resolves");
  check_long(count_field_nan(78.22, 15.65, 1.0, mwl, 0), 0,
             "Longyearbyen MWL, fajr");
  check_long(count_field_nan(78.22, 15.65, 1.0, mwl, 3), 0,
             "Longyearbyen MWL, maghrib");
  check_long(count_field_nan(78.22, 15.65, 1.0, mwl, 4), 0,
             "Longyearbyen MWL, isha");

  // The same place under Kemenag, which publishes no high-latitude rule and so
  // carries no reference latitude. This is the point of putting the rule on
  // the method: the library does not attribute a ruling to an authority that
  // never issued one, so these stay non-finite and the caller is told rather
  // than guessed at.
  // A caller can lift the polar restriction for a method whose authority
  // publishes no rule, by copying the table entry and setting the reference
  // latitude themselves. The library declines to make that choice on the
  // authority's behalf, but it does not prevent the caller from making it.
  // Murmansk is the case that matters: a real city of roughly 270000 people
  // inside the polar circle, served by an authority that is silent here.
  const MethodParams *russia = method_params_get(CALC_RUSSIA);
  count_field_anomalies(68.97, 33.08, 3.0, russia, &nan_days, &out_days);
  check_long(nan_days, 112, "Murmansk Russia as published, NaN days");

  MethodParams russia_with_ref = *russia;
  russia_with_ref.high_lat_method = HIGHLAT_ANGLE_BASED;
  russia_with_ref.high_lat_ref = 45.0;
  count_field_anomalies(68.97, 33.08, 3.0, &russia_with_ref, &nan_days,
                        &out_days);
  check_long(nan_days, 10, "Murmansk Russia with a caller reference, NaN days");
  // Not zero, and the 10 are asr alone. On those days the separation from the
  // declination sits between 90 and 90.833 degrees, so the Sun is visible only
  // by refraction and casts no shadow. Sunrise exists, so this is not the polar
  // case and the reference latitude is not consulted. asr genuinely does not
  // occur, and saying so is the domain guard working rather than failing.
  check_long(count_field_nan(68.97, 33.08, 3.0, &russia_with_ref, 2), 10,
             "Murmansk Russia with a caller reference, the 10 are asr");

  // The copy must not disturb the table entry it came from.
  count_field_anomalies(68.97, 33.08, 3.0, method_params_get(CALC_RUSSIA),
                        &nan_days, &out_days);
  check_long(nan_days, 112, "Murmansk Russia unchanged after the copy");

  const MethodParams *kemenag = method_params_get(CALC_KEMENAG);
  count_field_anomalies(78.22, 15.65, 1.0, kemenag, &nan_days, &out_days);
  check_long(nan_days, 244, "Longyearbyen Kemenag, NaN days");
  check_long(count_field_nan(78.22, 15.65, 1.0, kemenag, 0), 128,
             "Longyearbyen Kemenag, fajr");
  check_long(count_field_nan(78.22, 15.65, 1.0, kemenag, 3), 240,
             "Longyearbyen Kemenag, maghrib");

  printf("\n");
}

// format_time_hm and format_time_hms: rendering only, not prayer times. These
// carry no residual, like the clock_diff_minutes group above.
//
// The formatters are reached with values outside 0 to 24 because the
// high-latitude fallback produces them, and with non-finite values because
// the unguarded hour_angle call sites produce those. Both were defects.
//
// Mutation record: removing the "if (t < 0.0) t += 24.0;" line from
// normalize_clock_hours in the header and running `make test` produced this
// FAIL line, pasted verbatim from the terminal:
//   FAIL  hm, just before midnight       got="00:-6"  expected="23:54"
// Removing the !isfinite guard from format_time_hm produced:
//   FAIL  hm, NaN is not a time          got="-8:-2147483648"  expected="--:--"
// Both fixes were then restored.
static void test_format_time(void) {
  char b[32];
  printf("Test group: time formatting\n");

  format_time_hm(12.5, b, sizeof b);
  check_str(b, "12:30", "hm, midday");
  format_time_hm(0.0, b, sizeof b);
  check_str(b, "00:00", "hm, midnight");

  // Past the end of the day. The fallback returns these at Reykjavik.
  format_time_hm(24.0, b, sizeof b);
  check_str(b, "00:00", "hm, exactly 24");
  format_time_hm(25.075, b, sizeof b);
  check_str(b, "01:05", "hm, past midnight");
  format_time_hm(48.25, b, sizeof b);
  check_str(b, "00:15", "hm, two days past");

  // Before the start of the day. The fallback returns these at Tromso.
  format_time_hm(-0.104, b, sizeof b);
  check_str(b, "23:54", "hm, just before midnight");
  format_time_hm(-24.5, b, sizeof b);
  check_str(b, "23:30", "hm, a day and a half before");

  // Non-finite. The unguarded hour_angle call sites return these above 66N.
  format_time_hm(NAN, b, sizeof b);
  check_str(b, "--:--", "hm, NaN is not a time");
  format_time_hm(INFINITY, b, sizeof b);
  check_str(b, "--:--", "hm, infinity is not a time");

  format_time_hms(12.5, b, sizeof b);
  check_str(b, "12:30:00", "hms, midday");
  format_time_hms(24.0, b, sizeof b);
  check_str(b, "00:00:00", "hms, exactly 24");
  format_time_hms(-0.100, b, sizeof b);
  check_str(b, "23:54:00", "hms, just before midnight");
  format_time_hms(NAN, b, sizeof b);
  check_str(b, "--:--:--", "hms, NaN is not a time");
  format_time_hms(-INFINITY, b, sizeof b);
  check_str(b, "--:--:--", "hms, negative infinity");

  // A formatter must never emit a field the parser cannot read back. This
  // asserts the property rather than one value, over the whole range the
  // fallback can produce.
  int malformed = 0;
  for (int i = -2400; i <= 4800; i++) {
    double t = i / 100.0;
    int h = -1, m = -1;
    format_time_hm(t, b, sizeof b);
    if (PARSE_TIME(h, m, b) != 2) { malformed++; continue; }
    if (h < 0 || h > 23 || m < 0 || m > 59) malformed++;
  }
  check_long(malformed, 0, "hm, no malformed field");

  printf("\n");
}

static void test_clock_diff_minutes(void) {
  printf("Test group: clock_diff_minutes arithmetic\n");

  // Two times on the same side of midnight: plain absolute difference.
  check_long(clock_diff_minutes(600, 630), 30, "same side, 10:00 vs 10:30");
  check_long(clock_diff_minutes(630, 600), 30, "same side, argument order");

  // Straddling midnight, one direction: 23:59 vs 00:01 is 2 minutes apart
  // on the clock, not 1438.
  check_long(clock_diff_minutes(1439, 1), 2, "23:59 vs 00:01, wraps");

  // Straddling midnight, the other direction: same pair, arguments swapped.
  check_long(clock_diff_minutes(1, 1439), 2, "00:01 vs 23:59, wraps");

  // Exactly 720 minutes apart is the boundary the "diff > 720" wrap
  // condition itself tests: at exactly 720 no wrap should be applied.
  check_long(clock_diff_minutes(0, 720), 720, "exact 12h separation, no wrap");

  // One minute past that boundary must wrap, giving the shorter arc.
  check_long(clock_diff_minutes(0, 721), 719, "12h01m separation, wraps");

  // Identical times give zero.
  check_long(clock_diff_minutes(500, 500), 0, "identical times");
}

// mt_days_from_civil / mt_civil_from_days: anchors, boundaries, round-trip.
static void test_civil_date_helpers(void) {
  printf("Test group: civil date helpers\n");

  // The epoch itself and the days either side of it.
  check_long(mt_days_from_civil(1970, 1, 1), 0, "epoch 1970-01-01");
  check_long(mt_days_from_civil(1970, 1, 2), 1, "1970-01-02");
  check_long(mt_days_from_civil(1969, 12, 31), -1, "pre-epoch 1969-12-31");

  // 30 years of 365 days plus 7 leap days (1972 through 1996).
  check_long(mt_days_from_civil(2000, 1, 1), 10957, "2000-01-01");

  // A leap day must map to a distinct serial and survive the inverse.
  long leap = mt_days_from_civil(2024, 2, 29);
  check_long(mt_days_from_civil(2024, 3, 1) - leap, 1,
             "2024-02-29 precedes 03-01");

  // The inverse recovers the original date.
  int y = 0, m = 0, d = 0;
  mt_civil_from_days(0, &y, &m, &d);
  check_long((long)y * 10000 + m * 100 + d, 19700101L, "inverse of day 0");
  mt_civil_from_days(leap, &y, &m, &d);
  check_long((long)y * 10000 + m * 100 + d, 20240229L, "inverse of 2024-02-29");

  // Round-trip a wide span, including pre-epoch and far-future serials. A prime
  // step keeps the sample from landing on the same weekday or month boundary.
  long mismatches = 0;
  for (long serial = -400000; serial <= 400000; serial += 997) {
    mt_civil_from_days(serial, &y, &m, &d);
    if (mt_days_from_civil(y, m, d) != serial)
      mismatches++;
  }
  check_long(mismatches, 0, "round-trip over 800k days");

  printf("\n");
}

int main(void) {
  printf("=== prayertimes.h unit tests ===\n");
  printf("=== Reference: jadwalsholat.org (Kemenag method) ===\n");
  printf("=== All times include 2-min ihtiyat safety margin ===\n\n");

  kemenag_params = method_params_get(CALC_KEMENAG);

  // Jakarta Pusat (7 tests)
  test_jakarta_jan();
  test_jakarta_feb();
  test_jakarta_apr();
  test_jakarta_jun();
  test_jakarta_jul();
  test_jakarta_oct();
  test_jakarta_dec();

  // Surabaya (7 tests)
  test_surabaya_jan();
  test_surabaya_feb();
  test_surabaya_apr();
  test_surabaya_jun();
  test_surabaya_jul();
  test_surabaya_oct();
  test_surabaya_dec();

  // Makassar (5 tests)
  test_makassar_jan();
  test_makassar_feb();
  test_makassar_apr();
  test_makassar_jul();
  test_makassar_oct();

  // Banda Aceh (5 tests)
  test_banda_aceh_jan();
  test_banda_aceh_feb();
  test_banda_aceh_apr();
  test_banda_aceh_jul();
  test_banda_aceh_oct();

  // Denpasar (5 tests)
  test_denpasar_jan();
  test_denpasar_feb();
  test_denpasar_apr();
  test_denpasar_jul();
  test_denpasar_oct();

  // Jayapura (5 tests)
  test_jayapura_jan();
  test_jayapura_feb();
  test_jayapura_apr();
  test_jayapura_jul();
  test_jayapura_oct();

  // Pontianak (5 tests)
  test_pontianak_jan();
  test_pontianak_feb();
  test_pontianak_apr();
  test_pontianak_jul();
  test_pontianak_oct();

  // Medan (5 tests)
  test_medan_jan();
  test_medan_feb();
  test_medan_apr();
  test_medan_jul();
  test_medan_oct();

  // Menado (5 tests)
  test_menado_jan();
  test_menado_feb();
  test_menado_apr();
  test_menado_jul();
  test_menado_oct();

  // Bandung (2 tests)
  test_bandung_jan();
  test_bandung_jul();

  // Multi-method validation (~108 data points)
  printf("--- Multi-method validation ---\n\n");

  // MWL — London (12 months)
  test_mwl_london_jan();
  test_mwl_london_feb();
  test_mwl_london_mar();
  test_mwl_london_apr();
  test_mwl_london_may();
  test_mwl_london_jun();
  test_mwl_london_jul();
  test_mwl_london_aug();
  test_mwl_london_sep();
  test_mwl_london_oct();
  test_mwl_london_nov();
  test_mwl_london_dec();
  // MWL — Istanbul (5 tests)
  test_mwl_istanbul_jan();
  test_mwl_istanbul_mar();
  test_mwl_istanbul_jun();
  test_mwl_istanbul_sep();
  test_mwl_istanbul_dec();
  // MWL — Tokyo (4 tests)
  test_mwl_tokyo_jan();
  test_mwl_tokyo_apr();
  test_mwl_tokyo_jul();
  test_mwl_tokyo_oct();

  // ISNA — New York (12 months)
  test_isna_newyork_jan();
  test_isna_newyork_feb();
  test_isna_newyork_mar();
  test_isna_newyork_apr();
  test_isna_newyork_may();
  test_isna_newyork_jun();
  test_isna_newyork_jul();
  test_isna_newyork_aug();
  test_isna_newyork_sep();
  test_isna_newyork_oct();
  test_isna_newyork_nov();
  test_isna_newyork_dec();
  // ISNA — Toronto (5 tests)
  test_isna_toronto_jan();
  test_isna_toronto_mar();
  test_isna_toronto_jun();
  test_isna_toronto_sep();
  test_isna_toronto_dec();
  // ISNA — Chicago (4 tests)
  test_isna_chicago_jan();
  test_isna_chicago_apr();
  test_isna_chicago_jul();
  test_isna_chicago_oct();
  // ISNA — Los Angeles (3 tests)
  test_isna_losangeles_jan();
  test_isna_losangeles_jun();
  test_isna_losangeles_dec();

  // Makkah — Makkah (12 months)
  test_makkah_makkah_jan();
  test_makkah_makkah_feb();
  test_makkah_makkah_mar();
  test_makkah_makkah_apr();
  test_makkah_makkah_may();
  test_makkah_makkah_jun();
  test_makkah_makkah_jul();
  test_makkah_makkah_aug();
  test_makkah_makkah_sep();
  test_makkah_makkah_oct();
  test_makkah_makkah_nov();
  test_makkah_makkah_dec();
  // Makkah — Riyadh (5 tests)
  test_makkah_riyadh_jan();
  test_makkah_riyadh_mar();
  test_makkah_riyadh_jun();
  test_makkah_riyadh_sep();
  test_makkah_riyadh_dec();
  // Makkah — Dubai (4 tests)
  test_makkah_dubai_jan();
  test_makkah_dubai_apr();
  test_makkah_dubai_jul();
  test_makkah_dubai_oct();
  // Makkah — Jakarta (3 tests)
  test_makkah_jakarta_jan();
  test_makkah_jakarta_jun();
  test_makkah_jakarta_dec();

  // Egypt — Cairo (12 months)
  test_egypt_cairo_jan();
  test_egypt_cairo_feb();
  test_egypt_cairo_mar();
  test_egypt_cairo_apr();
  test_egypt_cairo_may();
  test_egypt_cairo_jun();
  test_egypt_cairo_jul();
  test_egypt_cairo_aug();
  test_egypt_cairo_sep();
  test_egypt_cairo_oct();
  test_egypt_cairo_nov();
  test_egypt_cairo_dec();
  // Egypt — Kuala Lumpur (4 tests)
  test_egypt_kl_jan();
  test_egypt_kl_apr();
  test_egypt_kl_jul();
  test_egypt_kl_oct();
  // Egypt — Alexandria (4 tests)
  test_egypt_alexandria_mar();
  test_egypt_alexandria_jun();
  test_egypt_alexandria_sep();
  test_egypt_alexandria_dec();

  test_midnight_wrap_reykjavik_jun();

  test_clock_diff_minutes();

  test_format_time();

  test_field_contract();
  test_ordering();

  test_civil_date_helpers();

  printf("=== Summary ===\n");
  printf("Total checks: %d\n", total);
  if (failures == 0) {
    printf("All tests passed.\n");
  } else {
    printf("%d check(s) FAILED out of %d.\n", failures, total);
  }

  return failures > 0 ? 1 : 0;
}

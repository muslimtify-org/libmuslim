// Tests for the composed civil timezone offset feeding prayer time
// calculation at polar and DST-affected latitudes.
//
// This file composes timezone.h and prayertimes.h in one translation unit
// and is kept separate from tests/test_prayertimes.c so that file keeps its
// libm-only portability. It uses the host operating system's tz database,
// the same assumption tests/test_timezone.c documents. The pinned counts
// below depend on Norway's current DST rules, so a tzdb rule change is a
// legitimate cause of failure here rather than a sign the astronomy moved.

/* timezone.h first: its implementation block defines _DEFAULT_SOURCE only when
   no feature macro is set yet, so including prayertimes.h ahead of it leaves
   readlink and timegm implicitly declared and the build fails. */
#define MUSLIM_TIMEZONE_IMPLEMENTATION
#include "../timezone.h"
#define PRAYERTIMES_IMPLEMENTATION
#include "../prayertimes.h"

#include <math.h>
#include <stdio.h>
#include <time.h>

static int failures = 0;
static int total = 0;

static void check_long(long got, long expected, const char *label) {
  total++;
  if (got == expected) {
    printf("  PASS  %-46s  %ld\n", label, got);
  } else {
    printf("  FAIL  %-46s  got=%ld expected=%ld\n", label, got, expected);
    failures++;
  }
}

/* Noon UTC on the given civil date, matching the convention in
   tests/test_timezone.c of sampling far from any transition boundary. Built
   with mt_days_from_civil rather than timegm, which is not standard C and has
   no Windows equivalent under that name. */
static double civil_offset(const char *zone, int y, int m, int d) {
  time_t when = (time_t)(mt_days_from_civil(y, m, d) * 86400L + 43200L);
  double off = 0.0;
  if (parse_timezone_offset(zone, when, &off) != 0) {
    printf("  FAIL  could not resolve zone %s\n", zone);
    failures++;
    total++;
    return NAN;
  }
  return off;
}

static const int month_len[12] = {31, 28, 31, 30, 31, 30,
                                  31, 31, 30, 31, 30, 31};

/* Walks 2025 twice for one place, once at the fixed offset the existing polar
   group uses and once at the true civil offset, and reports three quantities.
   The offset enters the solar noon computation at prayertimes.h:931 rather
   than as a final addition, but it cannot change which events resolve, only
   which side of midnight they land on. nan_fixed and nan_civil are collected
   so that invariant is asserted rather than assumed. */
static void survey(const char *zone, double lat, double lon, double fixed_tz,
                   const MethodParams *m, long *out_civil, long *differing,
                   long *nan_fixed, long *nan_civil) {
  *out_civil = 0;
  *differing = 0;
  *nan_fixed = 0;
  *nan_civil = 0;
  for (int doy = 1; doy <= 365; doy++) {
    int mo = 1, dy = doy;
    while (dy > month_len[mo - 1]) {
      dy -= month_len[mo - 1];
      mo++;
    }
    double ctz = civil_offset(zone, 2025, mo, dy);
    if (!isfinite(ctz))
      return;
    struct PrayerTimes a =
        calculate_prayer_times(2025, mo, dy, lat, lon, fixed_tz, m);
    struct PrayerTimes b = calculate_prayer_times(2025, mo, dy, lat, lon, ctz, m);
    const double va[5] = {a.fajr, a.dhuhr, a.asr, a.maghrib, a.isha};
    const double vb[5] = {b.fajr, b.dhuhr, b.asr, b.maghrib, b.isha};
    int an = 0, ao = 0, bn = 0, bo = 0;
    for (int i = 0; i < 5; i++) {
      if (isnan(va[i]))
        an = 1;
      else if (va[i] < 0.0 || va[i] >= 24.0)
        ao = 1;
      if (isnan(vb[i]))
        bn = 1;
      else if (vb[i] < 0.0 || vb[i] >= 24.0)
        bo = 1;
    }
    *nan_fixed += an;
    *nan_civil += bn;
    *out_civil += bo;
    if (an != bn || ao != bo)
      *differing += 1;
  }
}

/* Mutation record: changed the Tromso out-of-range pin ("Tromso civil,
 * out-of-range days") from 15 to 16, ran `make check`. Observed FAIL line,
 * verbatim:
 * FAIL  Tromso civil, out-of-range days               got=15 expected=16
 * Value restored afterward, and `make check` passes again. */
int main(void) {
  const MethodParams *mwl = method_params_get(CALC_MWL);
  const MethodParams *ru = method_params_get(CALC_RUSSIA);
  long out_civil, differing, nan_fixed, nan_civil;

  printf("Test group: polar locations under their true civil offsets, 2025\n");

  /* The two DST rows. These are the finding: both places are exercised at +1
     by the existing polar group, and both observe CEST at +2 through exactly
     the season when the polar effects occur. */
  survey("Europe/Oslo", 69.65, 18.96, 1.0, mwl, &out_civil, &differing,
         &nan_fixed, &nan_civil);
  check_long(out_civil, 15, "Tromso civil, out-of-range days");
  check_long(differing, 14, "Tromso, days differing from fixed +1");
  check_long(nan_civil, nan_fixed, "Tromso, NaN days unmoved by the offset");

  survey("Arctic/Longyearbyen", 78.22, 15.65, 1.0, mwl, &out_civil, &differing,
         &nan_fixed, &nan_civil);
  check_long(out_civil, 10, "Longyearbyen civil, out-of-range days");
  check_long(differing, 10, "Longyearbyen, days differing from fixed +1");
  check_long(nan_civil, nan_fixed, "Longyearbyen, NaN days unmoved");

  /* The three controls. Without them a reader cannot tell whether the effect
     above is about DST or about latitude. Murmansk is the row that settles it:
     it sits higher than Tromso and shows nothing, because Russia abolished
     DST. Iceland keeps UTC year round, and Jakarta is nowhere near either
     effect. */
  survey("Atlantic/Reykjavik", 64.15, -21.94, 0.0, mwl, &out_civil, &differing,
         &nan_fixed, &nan_civil);
  check_long(out_civil, 106, "Reykjavik civil, out-of-range days");
  check_long(differing, 0, "Reykjavik, no day differs, no DST");
  check_long(nan_civil, nan_fixed, "Reykjavik, NaN days unmoved");

  survey("Europe/Moscow", 68.97, 33.08, 3.0, ru, &out_civil, &differing,
         &nan_fixed, &nan_civil);
  check_long(out_civil, 16, "Murmansk civil, out-of-range days");
  check_long(differing, 0, "Murmansk, no day differs, DST abolished");
  check_long(nan_civil, nan_fixed, "Murmansk, NaN days unmoved");

  survey("Asia/Jakarta", -6.2088, 106.8456, 7.0, mwl, &out_civil, &differing,
         &nan_fixed, &nan_civil);
  check_long(out_civil, 0, "Jakarta civil, no out-of-range day");
  check_long(differing, 0, "Jakarta, no day differs");
  check_long(nan_civil, nan_fixed, "Jakarta, NaN days unmoved");

  printf("Polar civil offset tests: %d checks, %d failures\n", total, failures);
  return failures != 0;
}

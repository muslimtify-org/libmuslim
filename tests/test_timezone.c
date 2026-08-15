// Tests for timezone.h — the optional DST-aware offset helper.
//
// These tests use the host operating system's timezone database (POSIX tzdb
// via tzset/tm_gmtoff, or the Win32 timezone APIs). They assume a standard,
// up-to-date tz database is installed — true on essentially all Linux, macOS,
// and Windows systems. The chosen instants (noon UTC) sit far from any DST
// transition boundary so the expected offsets are unambiguous.
//
// Build:  cc -std=c11 -Wall -Wextra -o test_timezone test_timezone.c
//   (-lm not required; this test uses no libm beyond fabs, which is inlined)

#define MUSLIM_TIMEZONE_IMPLEMENTATION
#include "../timezone.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures = 0;
static int total = 0;

// Fixed UTC instants (Unix epoch seconds), chosen at 12:00 UTC to stay clear
// of DST switch-over times.
#define TS_2026_JAN_15_NOON ((time_t)1768478400) // 2026-01-15 12:00:00 UTC
#define TS_2026_JUL_15_NOON ((time_t)1784116800) // 2026-07-15 12:00:00 UTC

static void check_offset(const char *zone, time_t when, double expected,
                         const char *label) {
  double got = 0.0;
  int rc = parse_timezone_offset(zone, when, &got);
  total++;
  if (rc == 0 && fabs(got - expected) < 0.01) {
    printf("  PASS  %-34s  offset=%+.2f  expected=%+.2f\n", label, got,
           expected);
  } else {
    printf("  FAIL  %-34s  rc=%d offset=%+.2f  expected=%+.2f\n", label, rc,
           got, expected);
    failures++;
  }
}

static void check_unresolvable(const char *zone, time_t when,
                               const char *label) {
  double got = 12345.0;
  int rc = parse_timezone_offset(zone, when, &got);
  total++;
  if (rc != 0 && got == 12345.0) {
    printf("  PASS  %-34s  rejected, out untouched\n", label);
  } else {
    printf("  FAIL  %-34s  rc=%d offset=%+.2f\n", label, rc, got);
    failures++;
  }
}

// ---------------------------------------------------------------------------
// Fixed-offset zones (no DST) — must be identical in January and July.
// ---------------------------------------------------------------------------

static void test_fixed_offsets(void) {
  printf("Test group: fixed-offset zones (no DST)\n");
  check_offset("UTC", TS_2026_JAN_15_NOON, 0.0, "UTC (Jan)");
  check_offset("UTC", TS_2026_JUL_15_NOON, 0.0, "UTC (Jul)");
  check_offset("Asia/Jakarta", TS_2026_JAN_15_NOON, 7.0,
               "Asia/Jakarta WIB (Jan)");
  check_offset("Asia/Jakarta", TS_2026_JUL_15_NOON, 7.0,
               "Asia/Jakarta WIB (Jul)");
  check_offset("Asia/Dubai", TS_2026_JUL_15_NOON, 4.0, "Asia/Dubai (Jul)");
  check_offset("Asia/Riyadh", TS_2026_JAN_15_NOON, 3.0, "Asia/Riyadh (Jan)");
  check_offset("Asia/Karachi", TS_2026_JUL_15_NOON, 5.0, "Asia/Karachi (Jul)");
  check_offset("Asia/Tokyo", TS_2026_JAN_15_NOON, 9.0, "Asia/Tokyo (Jan)");
  printf("\n");
}

// ---------------------------------------------------------------------------
// Fractional (half/quarter-hour) offsets — verify non-integer handling.
// ---------------------------------------------------------------------------

static void test_fractional_offsets(void) {
  printf("Test group: fractional offsets\n");
  check_offset("Asia/Kolkata", TS_2026_JAN_15_NOON, 5.5,
               "Asia/Kolkata IST (+5:30)");
  check_offset("Asia/Kathmandu", TS_2026_JAN_15_NOON, 5.75,
               "Asia/Kathmandu (+5:45)");
  printf("\n");
}

// ---------------------------------------------------------------------------
// DST zones — the whole point of this header. Same zone, two seasons.
// ---------------------------------------------------------------------------

static void test_dst_offsets(void) {
  printf("Test group: DST-aware offsets\n");
  // Europe/London: GMT in winter, BST (+1) in summer.
  check_offset("Europe/London", TS_2026_JAN_15_NOON, 0.0,
               "Europe/London winter (GMT)");
  check_offset("Europe/London", TS_2026_JUL_15_NOON, 1.0,
               "Europe/London summer (BST)");
  // America/New_York: EST (-5) in winter, EDT (-4) in summer.
  check_offset("America/New_York", TS_2026_JAN_15_NOON, -5.0,
               "America/New_York winter (EST)");
  check_offset("America/New_York", TS_2026_JUL_15_NOON, -4.0,
               "America/New_York summer (EDT)");
  printf("\n");
}

// ---------------------------------------------------------------------------
// Invalid / unresolvable input must be rejected, with *out left untouched.
// ---------------------------------------------------------------------------

static void test_invalid_zones(void) {
  printf("Test group: invalid input is rejected\n");
  check_unresolvable(NULL, TS_2026_JAN_15_NOON, "NULL zone");
  check_unresolvable("Not/AZone", TS_2026_JAN_15_NOON, "unknown zone name");
  check_unresolvable("", TS_2026_JAN_15_NOON, "empty zone name");
  printf("\n");
}

// ---------------------------------------------------------------------------
// Differential: the TZif reader vs libc's own reading of the same zone files.
// This is the check that catches a plausible-but-wrong reading of the file
// format (in particular of the trailing POSIX TZ footer), because it compares
// against libc rather than against this author's reading of the spec.
//
// POSIX only: the reference is the setenv/tzset implementation that the reader
// replaced, and the Windows branch never had it.
// ---------------------------------------------------------------------------

#if !defined(_WIN32)

// Verbatim copy of the pre-change POSIX body of parse_timezone_offset. It is
// racy under concurrency -- that is why it was replaced -- but this test is
// single-threaded, so it serves as libc's answer for the same zone and instant.
static int reference_offset_via_tz(const char *zone, time_t when, double *out) {
  if (!zone || !out)
    return -1;

  const char *old_tz = getenv("TZ");
  char *saved = old_tz ? strdup(old_tz) : NULL;

  setenv("TZ", zone, 1);
  tzset();

  struct tm lt;
  localtime_r(&when, &lt);
  *out = (double)lt.tm_gmtoff / 3600.0;

  if (saved) {
    setenv("TZ", saved, 1);
    free(saved);
  } else {
    unsetenv("TZ");
  }
  tzset();

  return 0;
}

static void check_agrees_with_libc(const char *zone, time_t when) {
  char label[48];
  double mine = 0.0, ref = 0.0;
  int rc = parse_timezone_offset(zone, when, &mine);
  int ref_rc = reference_offset_via_tz(zone, when, &ref);

  snprintf(label, sizeof label, "%s @ %ld", zone, (long)when);
  total++;
  if (rc == 0 && ref_rc == 0 && fabs(mine - ref) < 0.01) {
    printf("  PASS  %-34s  offset=%+.2f  libc=%+.2f\n", label, mine, ref);
  } else {
    printf("  FAIL  %-34s  rc=%d offset=%+.2f  libc=%+.2f\n", label, rc, mine,
           ref);
    failures++;
  }
}

static void test_differential(void) {
  // Every zone named in the three groups above, plus a southern-hemisphere
  // zone whose DST window wraps the year end.
  static const char *const zones[] = {
      "UTC",           "Asia/Jakarta",     "Asia/Dubai",
      "Asia/Riyadh",   "Asia/Karachi",     "Asia/Tokyo",
      "Asia/Kolkata",  "Asia/Kathmandu",   "Europe/London",
      "America/New_York", "Australia/Sydney"};
  // The 2026 DST transition instants straddled elsewhere in this file: US
  // spring forward and fall back, AU DST start and end.
  static const time_t transitions[] = {
      (time_t)1772953200, // 2026-03-08 07:00 UTC
      (time_t)1793512800, // 2026-11-01 06:00 UTC
      (time_t)1791043200, // 2026-10-03 16:00 UTC
      (time_t)1775318400  // 2026-04-04 16:00 UTC
  };
  static const long deltas[] = {-86400, 0, 86400};
  size_t z, t, d;

  printf("Test group: differential vs libc (setenv/tzset reference)\n");
  for (z = 0; z < sizeof zones / sizeof zones[0]; z++)
    for (t = 0; t < sizeof transitions / sizeof transitions[0]; t++)
      for (d = 0; d < sizeof deltas / sizeof deltas[0]; d++)
        check_agrees_with_libc(zones[z],
                               (time_t)((long long)transitions[t] + deltas[d]));
  printf("\n");
}

// ---------------------------------------------------------------------------
// The input forms the spec records as working: a bare zone name, the same with
// a leading colon, an absolute path to a zone file, and two bare POSIX TZ
// strings that match no file. Plus one form that must be rejected.
// ---------------------------------------------------------------------------

static void test_input_forms(void) {
  printf("Test group: accepted input forms\n");
  check_offset("Asia/Jakarta", TS_2026_JUL_15_NOON, 7.0, "bare zone name");
  check_offset(":Asia/Jakarta", TS_2026_JUL_15_NOON, 7.0, "leading colon");
  check_offset("/usr/share/zoneinfo/Asia/Jakarta", TS_2026_JUL_15_NOON, 7.0,
               "absolute path to zone file");
  check_offset("XYZ8", TS_2026_JUL_15_NOON, -8.0, "TZ string, no such file");
  check_offset("UTC-7", TS_2026_JUL_15_NOON, 7.0, "UTC-7 TZ string");
  check_unresolvable("../../etc/passwd", TS_2026_JUL_15_NOON,
                     "name containing ..");
  printf("\n");
}

// ---------------------------------------------------------------------------
// Concurrency guard: two threads hammering parse_timezone_offset on distinct
// fixed-offset zones must never see a wrong answer. This is a regression
// guard for issue #41, where the old setenv/tzset body raced across threads.
// POSIX only: pthreads, and the racy body it guards against, never existed on
// the Windows branch.
// ---------------------------------------------------------------------------

#if !defined(_WIN32)

#include <pthread.h>

// Issue #41's own reproduction used 200000 iterations/thread and produced
// 533 and 318 wrong answers (out of 200000) on the two zones respectively
// against the pre-fix code. 20000 is enough to fail reliably there (dozens
// of mismatches in local runs) while keeping this test's contribution to
// `make check` small.
#define CONCURRENT_ITERATIONS 20000

typedef struct {
  const char *zone;
  double expected;
  int mismatches;
} concurrent_arg;

static void *concurrent_resolve(void *p) {
  concurrent_arg *arg = (concurrent_arg *)p;
  int i;
  for (i = 0; i < CONCURRENT_ITERATIONS; i++) {
    double got = 0.0;
    int rc = parse_timezone_offset(arg->zone, TS_2026_JAN_15_NOON, &got);
    if (rc != 0 || fabs(got - arg->expected) > 0.01)
      arg->mismatches++;
  }
  return NULL;
}

static void test_concurrent_resolution(void) {
  printf("Test group: concurrent resolution (issue #41 guard)\n");

  // Eastern hemisphere vs western hemisphere, both fixed-offset (no DST) so
  // any mismatch can only come from cross-thread interference, not a real
  // seasonal change.
  concurrent_arg east = {"Asia/Jakarta", 7.0, 0};
  concurrent_arg west = {"America/New_York", -5.0, 0};
  pthread_t t_east, t_west;

  pthread_create(&t_east, NULL, concurrent_resolve, &east);
  pthread_create(&t_west, NULL, concurrent_resolve, &west);
  pthread_join(t_east, NULL);
  pthread_join(t_west, NULL);

  total++;
  if (east.mismatches == 0) {
    printf("  PASS  %-34s  0 mismatches / %d\n", east.zone,
           CONCURRENT_ITERATIONS);
  } else {
    printf("  FAIL  %-34s  %d mismatches / %d\n", east.zone, east.mismatches,
           CONCURRENT_ITERATIONS);
    failures++;
  }

  total++;
  if (west.mismatches == 0) {
    printf("  PASS  %-34s  0 mismatches / %d\n", west.zone,
           CONCURRENT_ITERATIONS);
  } else {
    printf("  FAIL  %-34s  %d mismatches / %d\n", west.zone, west.mismatches,
           CONCURRENT_ITERATIONS);
    failures++;
  }
  printf("\n");
}

#else /* _WIN32 */

static void test_concurrent_resolution(void) {
  printf("Test group: concurrent resolution (skipped on Windows)\n\n");
}

#endif

#else /* _WIN32 */

static void test_differential(void) {
  printf("Test group: differential vs libc (skipped on Windows)\n\n");
}

static void test_input_forms(void) {
  printf("Test group: accepted input forms (skipped on Windows)\n\n");
}

static void test_concurrent_resolution(void) {
  printf("Test group: concurrent resolution (skipped on Windows)\n\n");
}

#endif

// ---------------------------------------------------------------------------
// POSIX TZ string evaluator (POSIX build only — the Windows branch of
// timezone.h has no such parser). The header is included above with the
// implementation macro, so the static evaluator is visible right here.
// ---------------------------------------------------------------------------

#if !defined(_WIN32)

// Instants straddling the transitions of the TZ strings used below.
#define TS_2026_MAR_08_06Z ((time_t)1772949600) // 1h before US spring forward
#define TS_2026_MAR_08_08Z ((time_t)1772956800) // 1h after
#define TS_2026_NOV_01_05Z ((time_t)1793509200) // 1h before US fall back
#define TS_2026_NOV_01_07Z ((time_t)1793516400) // 1h after
#define TS_2026_OCT_03_15Z ((time_t)1791039600) // 1h before AU DST start
#define TS_2026_OCT_03_17Z ((time_t)1791046800) // 1h after
#define TS_2026_APR_04_15Z ((time_t)1775314800) // 1h before AU DST end
#define TS_2026_APR_04_17Z ((time_t)1775322000) // 1h after

static void check_tz_string(const char *tz, time_t when, double expected,
                            const char *label) {
  double got = 0.0;
  int rc = muslim_posix_tz_offset(tz, when, &got);
  total++;
  if (rc == 0 && fabs(got - expected) < 0.01) {
    printf("  PASS  %-34s  offset=%+.2f  expected=%+.2f\n", label, got,
           expected);
  } else {
    printf("  FAIL  %-34s  rc=%d offset=%+.2f  expected=%+.2f\n", label, rc,
           got, expected);
    failures++;
  }
}

static void check_tz_string_rejected(const char *tz, const char *label) {
  double got = 12345.0;
  int rc = muslim_posix_tz_offset(tz, TS_2026_JAN_15_NOON, &got);
  total++;
  if (rc != 0 && got == 12345.0) {
    printf("  PASS  %-34s  rejected, out untouched\n", label);
  } else {
    printf("  FAIL  %-34s  rc=%d offset=%+.2f\n", label, rc, got);
    failures++;
  }
}

static void test_posix_tz_strings(void) {
  printf("Test group: POSIX TZ string evaluator\n");

  // Fixed offset, no DST section.
  check_tz_string("WIB-7", TS_2026_JAN_15_NOON, 7.0, "WIB-7 (Jan)");
  check_tz_string("WIB-7", TS_2026_JUL_15_NOON, 7.0, "WIB-7 (Jul)");
  check_tz_string("EST5", TS_2026_JUL_15_NOON, -5.0, "EST5 west-positive");

  // Northern hemisphere, both sides of both transitions.
  check_tz_string("EST5EDT,M3.2.0,M11.1.0", TS_2026_JAN_15_NOON, -5.0,
                  "EST/EDT deep winter");
  check_tz_string("EST5EDT,M3.2.0,M11.1.0", TS_2026_JUL_15_NOON, -4.0,
                  "EST/EDT deep summer");
  check_tz_string("EST5EDT,M3.2.0,M11.1.0", TS_2026_MAR_08_06Z, -5.0,
                  "EST/EDT before spring forward");
  check_tz_string("EST5EDT,M3.2.0,M11.1.0", TS_2026_MAR_08_08Z, -4.0,
                  "EST/EDT after spring forward");
  check_tz_string("EST5EDT,M3.2.0,M11.1.0", TS_2026_NOV_01_05Z, -4.0,
                  "EST/EDT before fall back");
  check_tz_string("EST5EDT,M3.2.0,M11.1.0", TS_2026_NOV_01_07Z, -5.0,
                  "EST/EDT after fall back");
  // Explicit /time on the start rule (Europe/London switches at 01:00 UTC).
  check_tz_string("GMT0BST,M3.5.0/1,M10.5.0", TS_2026_JAN_15_NOON, 0.0,
                  "GMT/BST winter");
  check_tz_string("GMT0BST,M3.5.0/1,M10.5.0", TS_2026_JUL_15_NOON, 1.0,
                  "GMT/BST summer");

  // Southern hemisphere: the DST window wraps the year end.
  check_tz_string("AEST-10AEDT,M10.1.0,M4.1.0/3", TS_2026_JAN_15_NOON, 11.0,
                  "AEST/AEDT January (in DST)");
  check_tz_string("AEST-10AEDT,M10.1.0,M4.1.0/3", TS_2026_JUL_15_NOON, 10.0,
                  "AEST/AEDT July (standard)");
  check_tz_string("AEST-10AEDT,M10.1.0,M4.1.0/3", TS_2026_OCT_03_15Z, 10.0,
                  "AEST/AEDT before DST start");
  check_tz_string("AEST-10AEDT,M10.1.0,M4.1.0/3", TS_2026_OCT_03_17Z, 11.0,
                  "AEST/AEDT after DST start");
  check_tz_string("AEST-10AEDT,M10.1.0,M4.1.0/3", TS_2026_APR_04_15Z, 11.0,
                  "AEST/AEDT before DST end");
  check_tz_string("AEST-10AEDT,M10.1.0,M4.1.0/3", TS_2026_APR_04_17Z, 10.0,
                  "AEST/AEDT after DST end");

  // Quoted <...> names, including a quoted DST pair with Julian rules.
  check_tz_string("<-03>3", TS_2026_JAN_15_NOON, -3.0, "<-03>3 quoted name");
  check_tz_string("<+07>-7", TS_2026_JUL_15_NOON, 7.0, "<+07>-7 quoted name");
  check_tz_string("<-04>4<-03>,J60,J300", TS_2026_JAN_15_NOON, -4.0,
                  "quoted pair, Julian rules (Jan)");
  check_tz_string("<-04>4<-03>,J60,J300", TS_2026_JUL_15_NOON, -3.0,
                  "quoted pair, Julian rules (Jul)");

  // Fractional offsets.
  check_tz_string("IST-5:30", TS_2026_JAN_15_NOON, 5.5, "IST-5:30");
  check_tz_string("NPT-5:45", TS_2026_JAN_15_NOON, 5.75, "NPT-5:45");
  check_tz_string("XXX-5:45:30", TS_2026_JAN_15_NOON, 5.7583,
                  "XXX-5:45:30 seconds field");

  // Malformed strings must be rejected outright.
  check_tz_string_rejected(NULL, "NULL string");
  check_tz_string_rejected("", "empty string");
  check_tz_string_rejected("XY5", "name shorter than 3");
  check_tz_string_rejected("EST", "missing offset");
  check_tz_string_rejected("EST25", "hour out of range");
  check_tz_string_rejected("IST-5:75", "minute out of range");
  check_tz_string_rejected("EST5EDT", "DST name without rules");
  check_tz_string_rejected("EST5EDT,M13.2.0,M11.1.0", "month out of range");
  check_tz_string_rejected("EST5EDT,M3.6.0,M11.1.0", "week out of range");
  check_tz_string_rejected("EST5EDT,M3.2.7,M11.1.0", "weekday out of range");
  check_tz_string_rejected("EST5EDT,M3.2.0,M11.1.0,", "trailing garbage");
  check_tz_string_rejected("<-03", "unterminated quoted name");
  printf("\n");
}

// ---------------------------------------------------------------------------
// TZif footer fixtures. parse_timezone_offset only consults a zone file's
// trailing POSIX TZ string once the instant is at or past the last explicit
// transition. A fat tzdata build (this repository's development machines, most
// desktops) writes transitions out to 2499, so no real DST zone ever gets
// there; slim builds (Debian, Ubuntu, Alpine) reach it for ordinary
// present-day lookups. The fixtures in tests/fixtures/zoneinfo have a single
// 1950 transition and hand every later instant to the footer, so this group is
// what covers the footer's DST-rule evaluation end to end rather than only the
// rule-free fixed-offset footers the system zones happen to provide.
// See tests/fixtures/zoneinfo/README.md for how the files are regenerated.
//
// DELIBERATE ENVIRONMENT MUTATION: this group setenv()s TZDIR and restores it
// before returning. It must therefore run BEFORE test_concurrent_resolution,
// which resolves zones from other threads -- a setenv racing those threads is
// exactly the class of bug issue #41 was.
//
// TZDIR is relative, so the binary must be run from the repository root, which
// is how `make check` runs it. From elsewhere the fixture files are simply not
// found and every check below fails loudly.
// ---------------------------------------------------------------------------

#define FIXTURE_TZDIR "tests/fixtures/zoneinfo"

static void test_footer_fixtures(void) {
  const char *old_tzdir = getenv("TZDIR");
  char *saved = old_tzdir ? strdup(old_tzdir) : NULL;

  printf("Test group: TZif POSIX footer fixtures (TZDIR=" FIXTURE_TZDIR ")\n");
  setenv("TZDIR", FIXTURE_TZDIR, 1);

  // Footer "FIX-7": fixed offset, no rules.
  check_offset("Fixture/Fixed", TS_2026_JAN_15_NOON, 7.0,
               "footer FIX-7 (Jan)");
  check_offset("Fixture/Fixed", TS_2026_JUL_15_NOON, 7.0,
               "footer FIX-7 (Jul)");

  // Footer "FST5FDT,M3.2.0,M11.1.0": northern hemisphere.
  check_offset("Fixture/North", TS_2026_JAN_15_NOON, -5.0,
               "footer north winter");
  check_offset("Fixture/North", TS_2026_JUL_15_NOON, -4.0,
               "footer north summer");
  check_offset("Fixture/North", TS_2026_MAR_08_06Z, -5.0,
               "footer north pre-spring-forward");
  check_offset("Fixture/North", TS_2026_MAR_08_08Z, -4.0,
               "footer north post-spring-forward");
  check_offset("Fixture/North", TS_2026_NOV_01_05Z, -4.0,
               "footer north pre-fall-back");
  check_offset("Fixture/North", TS_2026_NOV_01_07Z, -5.0,
               "footer north post-fall-back");

  // Footer "GST-10GDT,M10.1.0,M4.1.0/3": southern hemisphere, the DST window
  // wraps the year end.
  check_offset("Fixture/South", TS_2026_JAN_15_NOON, 11.0,
               "footer south January (in DST)");
  check_offset("Fixture/South", TS_2026_JUL_15_NOON, 10.0,
               "footer south July (standard)");
  check_offset("Fixture/South", TS_2026_OCT_03_15Z, 10.0,
               "footer south pre-DST-start");
  check_offset("Fixture/South", TS_2026_OCT_03_17Z, 11.0,
               "footer south post-DST-start");
  check_offset("Fixture/South", TS_2026_APR_04_15Z, 11.0,
               "footer south pre-DST-end");
  check_offset("Fixture/South", TS_2026_APR_04_17Z, 10.0,
               "footer south post-DST-end");

  if (saved) {
    setenv("TZDIR", saved, 1);
    free(saved);
  } else {
    unsetenv("TZDIR");
  }
  printf("\n");
}

#else /* _WIN32 */

static void test_posix_tz_strings(void) {
  printf("Test group: POSIX TZ string evaluator (skipped on Windows)\n\n");
}

static void test_footer_fixtures(void) {
  printf("Test group: TZif POSIX footer fixtures (skipped on Windows)\n\n");
}

#endif

// ---------------------------------------------------------------------------
// get_system_timezone: value is host-dependent, so assert only the contract —
// on success the buffer is a non-empty string that round-trips back through
// parse_timezone_offset to a sane offset.
// ---------------------------------------------------------------------------

static void test_system_timezone(void) {
  printf("Test group: get_system_timezone contract\n");
  char zone[64];
  int rc = get_system_timezone(zone, sizeof(zone));
  printf("  info  detected zone=\"%s\" rc=%d\n", zone, rc);

  total++;
  if (rc == 0 && strlen(zone) > 0) {
    printf("  PASS  success returns non-empty zone\n");
  } else if (rc != 0 && strcmp(zone, "UTC") == 0) {
    // Unmapped/unavailable host zone: documented fallback is "UTC".
    printf("  PASS  failure falls back to \"UTC\"\n");
  } else {
    printf("  FAIL  unexpected rc/zone combination\n");
    failures++;
  }

  // Whatever zone we got, feeding it back must yield a plausible offset
  // (Earth's civil offsets span roughly -12..+14 hours).
  double off = 0.0;
  int off_rc = parse_timezone_offset(zone, TS_2026_JUL_15_NOON, &off);
  total++;
  if (off_rc == 0 && off >= -12.0 && off <= 14.0) {
    printf("  PASS  round-trip offset in range  offset=%+.2f\n", off);
  } else {
    printf("  FAIL  round-trip offset out of range  rc=%d offset=%+.2f\n",
           off_rc, off);
    failures++;
  }
  printf("\n");
}

int main(void) {
  printf("=== timezone.h tests ===\n\n");

  test_fixed_offsets();
  test_fractional_offsets();
  test_dst_offsets();
  test_invalid_zones();
  test_differential();
  test_input_forms();
  test_posix_tz_strings();
  test_system_timezone();
  // Mutates TZDIR; must stay ahead of the threaded group below.
  test_footer_fixtures();
  test_concurrent_resolution();

  printf("=== Summary ===\n");
  printf("Total checks: %d\n", total);
  if (failures == 0) {
    printf("All tests passed.\n");
  } else {
    printf("%d check(s) FAILED out of %d.\n", failures, total);
  }

  return failures > 0 ? 1 : 0;
}

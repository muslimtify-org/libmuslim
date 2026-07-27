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
#include <string.h>

static int failures = 0;
static int total = 0;

// Fixed UTC instants (Unix epoch seconds), chosen at 12:00 UTC to stay clear
// of DST switch-over times.
#define TS_2026_JAN_15_NOON ((time_t)1768478400) // 2026-01-15 12:00:00 UTC
#define TS_2026_JUL_15_NOON ((time_t)1784116800) // 2026-07-15 12:00:00 UTC

static void check_offset(const char *zone, time_t when, double expected,
                         const char *label) {
  double got = parse_timezone_offset(zone, when);
  total++;
  if (fabs(got - expected) < 0.01) {
    printf("  PASS  %-34s  offset=%+.2f  expected=%+.2f\n", label, got,
           expected);
  } else {
    printf("  FAIL  %-34s  offset=%+.2f  expected=%+.2f\n", label, got,
           expected);
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
// Invalid / unresolvable input must fall back to 0.0 (not crash).
// ---------------------------------------------------------------------------

static void test_invalid_zones(void) {
  printf("Test group: invalid input falls back to 0.0\n");
  check_offset(NULL, TS_2026_JAN_15_NOON, 0.0, "NULL zone");
  check_offset("Not/AZone", TS_2026_JAN_15_NOON, 0.0, "unknown zone name");
  check_offset("", TS_2026_JAN_15_NOON, 0.0, "empty zone name");
  printf("\n");
}

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
  double off = parse_timezone_offset(zone, TS_2026_JUL_15_NOON);
  total++;
  if (off >= -12.0 && off <= 14.0) {
    printf("  PASS  round-trip offset in range  offset=%+.2f\n", off);
  } else {
    printf("  FAIL  round-trip offset out of range  offset=%+.2f\n", off);
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
  test_system_timezone();

  printf("=== Summary ===\n");
  printf("Total checks: %d\n", total);
  if (failures == 0) {
    printf("All tests passed.\n");
  } else {
    printf("%d check(s) FAILED out of %d.\n", failures, total);
  }

  return failures > 0 ? 1 : 0;
}

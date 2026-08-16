// Regression test for hijri_yallop_evaluate_evening against TN69 Table 4.
//
// The fixture (tests/fixtures/yallop/tn69-observations.csv) is generated
// offline from Yallop's NAO Technical Note 69, see
// tests/fixtures/yallop/README.md for provenance and the exact regeneration
// commands. It carries only observational columns plus this library's own
// hijri_yallop_classify zone, frozen at fixture-generation time; it carries
// no HMNAO-computed value. This test re-evaluates every row through the
// live library and asserts the frozen zone still matches, so a change to
// the Yallop evaluation path that shifts a zone boundary is caught here.
//
// FIXTURE_PATH is repo-root-relative, following the convention at
// tests/test_timezone.c:459 (FIXTURE_TZDIR): `make check` runs test
// binaries from the repository root, so a relative path here is the
// portable choice. Run from elsewhere and the fixture is simply not found,
// which this test treats as a loud failure, not a skip.

#define HIJRI_IMPLEMENTATION
#include "../hijri.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define FIXTURE_PATH "tests/fixtures/yallop/tn69-observations.csv"

// Evening row count, frozen at fixture generation (Task 1/2 extraction of
// TN69 Table 4: 295 total rows, 271 with phase "evening"). If this ever
// legitimately changes, regenerate the fixture and update this constant
// deliberately; a silent drift here would mean rows are being dropped or
// duplicated somewhere upstream.
#define EXPECTED_ROWS 271

static int checks;
static int failures;

// Per-zone outcome counts: tally[zone][outcome], zone indexed A=0..F=5,
// outcome indexed seen_unaided=0, seen_optical_aid=1, not_seen=2. Populated
// while re-evaluating each fixture row (Step 1's reporting pass measured
// this table; the assertions below pin the measured values, see the
// comments beside expected_seen_unaided and expected_not_seen).
static long tally[6][3];

static char zone_letter(HijriYallopZone zone) {
  switch (zone) {
    case HIJRI_YALLOP_A_EASILY_VISIBLE: return 'A';
    case HIJRI_YALLOP_B_VISIBLE_PERFECT_CONDITIONS: return 'B';
    case HIJRI_YALLOP_C_MAY_NEED_OPTICAL_AID: return 'C';
    case HIJRI_YALLOP_D_NEEDS_OPTICAL_AID: return 'D';
    case HIJRI_YALLOP_E_NOT_VISIBLE_TELESCOPE: return 'E';
    case HIJRI_YALLOP_F_NOT_VISIBLE_BELOW_LIMIT: return 'F';
  }
  return '?';
}

int main(void) {
  FILE *f = fopen(FIXTURE_PATH, "r");
  if (!f) {
    checks++;
    failures++;
    printf("FAIL fixture/open path=%s\n", FIXTURE_PATH);
    printf("Yallop TN69 tests: %d checks, %d failures\n", checks, failures);
    return 1;
  }

  char line[256];
  if (!fgets(line, sizeof line, f)) {
    checks++;
    failures++;
    printf("FAIL fixture/header path=%s reason=empty_file\n", FIXTURE_PATH);
    fclose(f);
    printf("Yallop TN69 tests: %d checks, %d failures\n", checks, failures);
    return 1;
  }

  int line_no = 1;
  int row_count = 0;

  while (fgets(line, sizeof line, f)) {
    line_no++;

    int year, month, day;
    double lat_deg, lon_deg;
    char observed[32];
    char zone_str[8];

    int n = sscanf(line, "%d,%d,%d,%lf,%lf,%31[^,],%7[^\r\n]", &year, &month,
                   &day, &lat_deg, &lon_deg, observed, zone_str);
    checks++;
    if (n != 7) {
      failures++;
      printf("FAIL fixture/parse line=%d fields_parsed=%d text=%s", line_no,
             n, line);
      continue;
    }
    row_count++;

    HijriLocation loc = {lat_deg, lon_deg, 0.0, NULL};
    HijriYallopResult result =
        hijri_yallop_evaluate_evening(year, month, day, &loc);

    checks++;
    if (strcmp(zone_str, "NA") == 0) {
      if (!isnan(result.q)) {
        failures++;
        printf("FAIL zone/expected_nan date=%04d-%02d-%02d lat=%.4f "
               "lon=%.4f q=%.6f\n",
               year, month, day, lat_deg, lon_deg, result.q);
      }
    } else {
      char actual = zone_letter(result.zone);
      if (zone_str[0] != actual || zone_str[1] != '\0') {
        failures++;
        printf("FAIL zone/mismatch date=%04d-%02d-%02d lat=%.4f lon=%.4f "
               "expected=%s actual=%c\n",
               year, month, day, lat_deg, lon_deg, zone_str, actual);
      }

      int zidx = actual - 'A';
      if (zidx >= 0 && zidx < 6) {
        if (strcmp(observed, "seen_unaided") == 0) {
          tally[zidx][0]++;
        } else if (strcmp(observed, "seen_optical_aid") == 0) {
          tally[zidx][1]++;
        } else if (strcmp(observed, "not_seen") == 0) {
          tally[zidx][2]++;
        }
      }
    }
  }

  // Step 2 and Step 4: pinned zone-by-outcome counts, measured by the Step 1
  // reporting pass above against the frozen fixture (271 rows):
  //
  //   zone,seen_unaided,seen_optical_aid,not_seen
  //   A,132,3,6
  //   B,51,1,17
  //   C,6,10,8
  //   D,0,5,9
  //   E,1,3,3
  //   F,0,2,14
  //
  // These are counts over a frozen fixture, so exact equality is pinned
  // rather than a range.
  //
  // Zone E and F seen_unaided (Step 2): E=1, F=0. Do NOT read the E value
  // as a bug and "correct" it to 0. `docs/research/2026-07-30-findings.md:82`
  // states that zero naked-eye sightings fall in zones E or F, but that is
  // true of YALLOP'S classification, not this library's. Our zone E holds
  // exactly one seen_unaided row: TN69 observation 278, 1990-02-25, lat
  // 35.6, lon -83.5, coded V(V) (naked eye seen). Yallop places that same
  // row in zone D at q=-0.222, which is 0.0100 from the D/E boundary at
  // -0.232, well inside the measured maximum q residual of 0.053577
  // (Task 2). It is a boundary flip on a row already this close to the
  // line, not a mapping fault; halt condition 3 in the plan is written
  // against Yallop's own q for exactly this reason.
  //
  // Mutation record (Step 6): changed expected_seen_unaided[0] (zone A) from
  // 132 to 133, ran `make test`, observed:
  // FAIL zone_outcome/seen_unaided zone=A expected=133 actual=132
  // Value restored afterward.
  static const long expected_seen_unaided[6] = {132, 51, 6, 0, 1, 0};
  static const long expected_not_seen[6] = {6, 17, 8, 9, 3, 14};

  for (int z = 0; z < 6; z++) {
    checks++;
    if (tally[z][0] != expected_seen_unaided[z]) {
      failures++;
      printf("FAIL zone_outcome/seen_unaided zone=%c expected=%ld actual=%ld\n",
             (char)('A' + z), expected_seen_unaided[z], tally[z][0]);
    }
    checks++;
    if (tally[z][2] != expected_not_seen[z]) {
      failures++;
      printf("FAIL zone_outcome/not_seen zone=%c expected=%ld actual=%ld\n",
             (char)('A' + z), expected_not_seen[z], tally[z][2]);
    }
  }

  // Step 3: seen-unaided fraction per zone, computed from the Step 1 table
  // above: A 132/141=0.9362, B 51/69=0.7391, C 6/24=0.2500, D 0/14=0.0000,
  // E 1/7=0.1429, F 0/16=0.0000. This is NOT monotonically decreasing
  // A through F: zone D's fraction (0.0000) is lower than zone E's
  // (0.1429), so D and E break the trend. No monotonicity assertion is
  // pinned here, per the plan: ours is a reclassification of Yallop's data
  // rather than his own table, so monotonicity is not guaranteed, and the
  // same boundary flip discussed above (observation 278 moving from D into
  // E) is what produces the D/E inversion. Forcing the assertion here would
  // assert something the measurement does not show.

  fclose(f);

  // A zero-row or short read must fail loudly rather than pass vacuously:
  // this check runs unconditionally, so a fixture that opens but is empty
  // or truncated still fails here even though every per-row loop above ran
  // zero times.
  checks++;
  if (row_count != EXPECTED_ROWS) {
    failures++;
    printf("FAIL fixture/row_count actual=%d expected=%d\n", row_count,
           EXPECTED_ROWS);
  }

  // Mutation record 1 (Step 9): changed the frozen zone of the fixture's
  // first data row (1859-07-01, lat 38.0, lon 23.7) from A to B, ran
  // `make test`, observed:
  // FAIL zone/mismatch date=1859-07-01 lat=38.0000 lon=23.7000 expected=B actual=A
  // Row restored afterward.
  //
  // Mutation record 2 (Step 10): renamed
  // tests/fixtures/yallop/tn69-observations.csv out of the way, ran
  // `make test`, observed:
  // FAIL fixture/open path=tests/fixtures/yallop/tn69-observations.csv
  // Fixture name restored afterward.

  printf("Yallop TN69 tests: %d checks, %d failures\n", checks, failures);
  return failures == 0 ? 0 : 1;
}

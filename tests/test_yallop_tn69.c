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
    }
  }

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

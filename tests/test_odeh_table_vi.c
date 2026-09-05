// Regression test for hijri_odeh_evaluate_evening against Odeh's Table VI.
//
// The fixture (tests/fixtures/odeh/table-vi-observations.csv) is generated
// offline from M. S. Odeh's Table VI, see tests/fixtures/odeh/README.md for
// provenance and the exact regeneration commands. It carries only
// observational columns (date, phase, location, naked eye / binocular /
// telescope sighting marks) plus this library's own hijri_odeh_classify
// zone, recomputed and frozen at fixture-generation time; it carries no
// Odeh-computed value. This test re-evaluates every evening row through the
// live library and asserts the frozen zone still matches, so a change to
// the Odeh evaluation path that shifts a zone boundary is caught here.
//
// FIXTURE_PATH is repo-root-relative, following the convention
// tests/test_yallop_tn69.c documents in its own header comment: `make
// check` runs test binaries from the repository root, so a relative path
// here is the portable choice. Run from elsewhere and the fixture is simply
// not found, which this test treats as a loud failure, not a skip.

#define HIJRI_IMPLEMENTATION
#include "../hijri.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FIXTURE_PATH "tests/fixtures/odeh/table-vi-observations.csv"

// Evening row count, frozen at fixture generation (578 total rows, 522 with
// phase "evening", the rest "morning"). hijri_odeh_evaluate_evening only
// models evenings, so morning rows are read but skipped, not evaluated. If
// this ever legitimately changes, regenerate the fixture and update this
// constant deliberately; a silent drift here would mean rows are being
// dropped or duplicated somewhere upstream.
#define EXPECTED_ROWS 522

static int checks;
static int failures;

// Per-zone outcome counts: tally[zone][outcome], zone indexed by
// HijriOdehZone (0..3), outcome indexed seen_unaided=0, seen_optical_aid=1,
// not_seen=2. Populated while re-evaluating each evening fixture row.
static long tally[4][3];

static const char *zone_name(HijriOdehZone zone) {
  static const char *names[] = {"NOT_VISIBLE", "OPTICAL_AID_ONLY",
                                 "OPTICAL_AID_OR_NAKED_EYE", "NAKED_EYE"};
  if (zone < 0 || zone > 3)
    return "?";
  return names[zone];
}

// Splits a fixture data line on ',' into up to max fields, in place. A
// trailing newline/CR is stripped first. Empty fields (two consecutive
// commas, or a field before the trailing newline) come back as "", which
// matters here because naked_eye/binocular/telescope/zone are frequently
// empty. Returns the number of fields found.
static int split_csv(char *line, char *fields[], int max) {
  int n = 0;
  size_t len = strlen(line);
  while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
    line[--len] = '\0';

  char *p = line;
  fields[n++] = p;
  while (*p && n < max) {
    if (*p == ',') {
      *p = '\0';
      fields[n++] = p + 1;
    }
    p++;
  }
  return n;
}

int main(void) {
  FILE *f = fopen(FIXTURE_PATH, "r");
  if (!f) {
    checks++;
    failures++;
    printf("FAIL fixture/open path=%s\n", FIXTURE_PATH);
    printf("Odeh Table VI tests: %d checks, %d failures\n", checks, failures);
    return 1;
  }

  char line[256];
  if (!fgets(line, sizeof line, f)) {
    checks++;
    failures++;
    printf("FAIL fixture/header path=%s reason=empty_file\n", FIXTURE_PATH);
    fclose(f);
    printf("Odeh Table VI tests: %d checks, %d failures\n", checks, failures);
    return 1;
  }

  int line_no = 1;
  int evening_rows = 0;

  while (fgets(line, sizeof line, f)) {
    line_no++;

    char *fields[11];
    int n = split_csv(line, fields, 11);
    checks++;
    if (n != 11) {
      failures++;
      printf("FAIL fixture/parse line=%d fields_parsed=%d\n", line_no, n);
      continue;
    }

    const char *phase = fields[3];
    if (strcmp(phase, "evening") != 0)
      continue;
    evening_rows++;

    int year = atoi(fields[0]);
    int month = atoi(fields[1]);
    int day = atoi(fields[2]);
    double lat_deg = atof(fields[4]);
    double lon_deg = atof(fields[5]);
    double elev_m = atof(fields[6]);
    const char *naked_eye = fields[7];
    const char *binocular = fields[8];
    const char *telescope = fields[9];
    const char *zone_str = fields[10];

    HijriLocation loc = {lat_deg, lon_deg, elev_m, NULL};
    HijriOdehResult result = hijri_odeh_evaluate_evening(year, month, day, &loc);

    checks++;
    const char *actual = zone_name(result.zone);
    if (strcmp(zone_str, actual) != 0) {
      failures++;
      printf("FAIL zone/mismatch date=%04d-%02d-%02d lat=%.4f lon=%.4f "
             "expected=%s actual=%s\n",
             year, month, day, lat_deg, lon_deg, zone_str, actual);
    }

    int outcome;
    if (naked_eye[0] == 'V')
      outcome = 0;
    else if (binocular[0] == 'V' || telescope[0] == 'V')
      outcome = 1;
    else
      outcome = 2;

    if (result.zone >= 0 && result.zone <= 3)
      tally[result.zone][outcome]++;
  }

  fclose(f);

  // A zero-row or short read must fail loudly rather than pass vacuously:
  // this check runs unconditionally, so a fixture that opens but is empty
  // or truncated still fails here even though every per-row loop above ran
  // zero times.
  checks++;
  if (evening_rows != EXPECTED_ROWS) {
    failures++;
    printf("FAIL fixture/row_count actual=%d expected=%d\n", evening_rows,
           EXPECTED_ROWS);
  }

  // Mutation record 1: changed the frozen zone of the fixture's first data
  // row (1859-07-01, lat 38.0, lon 23.7) from NAKED_EYE to
  // OPTICAL_AID_OR_NAKED_EYE, ran `make test`, observed:
  // FAIL zone/mismatch date=1859-07-01 lat=38.0000 lon=23.7000 expected=OPTICAL_AID_OR_NAKED_EYE actual=NAKED_EYE
  // Row restored afterward.
  //
  // Mutation record 2: renamed
  // tests/fixtures/odeh/table-vi-observations.csv out of the way, ran
  // `make test`, observed:
  // FAIL fixture/open path=tests/fixtures/odeh/table-vi-observations.csv
  // Fixture name restored afterward.
  //
  // Mutation record 3: changed EXPECTED_ROWS from 522 to 523, ran
  // `make test`, observed:
  // FAIL fixture/row_count actual=522 expected=523
  // Value restored afterward.

  // Predicted zone against observed outcome, tallied over all 522 evening
  // rows. This is printed for a reader to see, not asserted against a
  // threshold: the observations carry real atmospheric scatter and Odeh's
  // own zone boundaries are probabilistic at the edges, so any pass
  // threshold here would be invented rather than derived from the data.
  printf("zone,seen_unaided,seen_optical_aid,not_seen\n");
  for (int z = 0; z <= 3; z++) {
    printf("%s,%ld,%ld,%ld\n", zone_name((HijriOdehZone)z), tally[z][0],
           tally[z][1], tally[z][2]);
  }

  printf("Odeh Table VI tests: %d checks, %d failures\n", checks, failures);
  return failures == 0 ? 0 : 1;
}

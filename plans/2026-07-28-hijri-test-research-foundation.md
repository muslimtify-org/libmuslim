# Hijri Test Research Foundation Implementation Plan

> **For executing agents:** implement this plan task-by-task. Each step uses checkbox (`- [ ]`) syntax. Do not skip steps. Do not batch commits across tasks.

**Goal:** Build the exact and synthetic Hijri test foundation plus an offline diagnostic probe that makes 2020–2025 reference research reproducible before authoritative real-calendar fixtures and tolerances are selected.

**Architecture:** A single table-driven C test executable validates deterministic arithmetic, criterion logic, and Yallop/Odeh score boundaries independently of astronomy. A separate research probe emits machine-readable results for a fixed matrix of dates, locations, and criteria; a provenance document records sources, conventions, and explained discrepancies. The resulting evidence is the required input to a follow-up plan for high-precision astronomical and published-calendar fixtures.

**Tech stack:** C99 single-header implementation, C++17 compatibility compilation, GCC or Clang, standard C library, `libm`, Markdown, CSV.

---

## Premortem

**Hidden assumptions:**
- The existing header is valid as both C99 and C++17 — Task 1 compiles the same test source in both modes and stops immediately if either compiler rejects it.
- The current criterion names describe the operational policies used during 2020–2025 — the research report labels those descriptions as unverified until primary authority material confirms them.
- A “previous conjunction” is always the conjunction relevant to an evening — Task 3 explicitly probes evenings immediately before conjunction, where that assumption can produce an implausible lunar age near one synodic month.

**Irreversible / risky steps:**
- none — all tasks add isolated test, probe, and documentation files; each commit can be reverted independently without a migration or external state change.

**Spec-misalignment:**
- This phase does not yet add authoritative real-evening or published-calendar assertions — it implements the user-approved research-first decision and ends with a concrete evidence gate; the follow-up fixture plan may only be written from the committed evidence.
- “Three locations per criterion” cannot literally apply to Umm al-Qura’s astronomical decision, which is evaluated at Mecca — the probe records Mecca as the decision site and treats other Saudi cities only as calendar-consumer cases.
- Yallop and Odeh are graded classifiers rather than binary calendars — Task 2 tests every published class boundary and Task 3 records classifications without inventing a month-start policy.

**Verify-clause weakness:**
- A test executable could report success while omitting required cases — Task 1 and Task 2 assert exact check totals in their verify clauses and summary output.
- A CSV file could exist but contain incomplete coverage — Task 3 verifies its header and exact row count, and checks that all named locations and every binary criterion occur.
- A research report could merely list links without diagnosing behavior — Task 4 requires explicit convention, observation, and disposition fields for every candidate fixture and rejects `unknown` dispositions.

## File structure

New:
- `tests/test_hijri.c` — table-driven exact arithmetic, synthetic criterion, and Yallop/Odeh boundary tests.
- `tests/hijri_research_probe.c` — fixed 2020–2025 diagnostic matrix that emits CSV from the current implementation.
- `tests/data/hijri/hijri-2020-2025-sources.md` — primary-source inventory, coordinate/time conventions, discrepancy classifications, and fixture admission decisions.
- `tests/data/hijri/hijri-2020-2025-baseline.csv` — generated output from the diagnostic matrix, committed as reproducible evidence.

Modified:
- `README.md` — commands for running the permanent test foundation and regenerating the research baseline.

### Task 1: Exact arithmetic test foundation → verify: C99 and C++17 binaries each report `19 checks, 0 failures`

**Files:**
- Create: `tests/test_hijri.c`

- [ ] **Step 1: Write the exact arithmetic test executable**

Create `tests/test_hijri.c` with this complete content:

```c
#define HIJRI_IMPLEMENTATION
#include "../hijri.h"

#include <math.h>
#include <stdio.h>

static int checks;
static int failures;

static void check_int(const char *name, int actual, int expected) {
  checks++;
  if (actual != expected) {
    failures++;
    printf("FAIL exact/%s actual=%d expected=%d\n", name, actual, expected);
  }
}

static void check_close(const char *name, double actual, double expected,
                        double tolerance) {
  double difference = fabs(actual - expected);
  checks++;
  if (difference > tolerance) {
    failures++;
    printf("FAIL exact/%s actual=%.12f expected=%.12f diff=%.12f tol=%.12f\n",
           name, actual, expected, difference, tolerance);
  }
}

static void check_date(const char *name, HijriDate actual, int year, int month,
                       int day) {
  checks++;
  if (actual.year != year || actual.month != month || actual.day != day) {
    failures++;
    printf("FAIL exact/%s actual=%04d-%02d-%02d expected=%04d-%02d-%02d\n",
           name, actual.year, actual.month, actual.day, year, month, day);
  }
}

static void test_julian_day(void) {
  int year;
  int month;
  double day;

  check_close("j2000", hijri_jd_from_gregorian(2000, 1, 1.5), 2451545.0,
              1e-9);
  check_close("unix_epoch", hijri_jd_from_gregorian(1970, 1, 1.0),
              2440587.5, 1e-9);
  check_close("gregorian_reform",
              hijri_jd_from_gregorian(1582, 10, 15.0), 2299160.5, 1e-9);

  hijri_gregorian_from_jd(2451545.0, &year, &month, &day);
  check_int("j2000_year", year, 2000);
  check_int("j2000_month", month, 1);
  check_close("j2000_day", day, 1.5, 1e-9);

  check_int("weekday_2000_01_01",
            hijri_jd_weekday(hijri_jd_from_gregorian(2000, 1, 1.0)), 6);
  check_close("centuries_j2000", hijri_julian_centuries(2451545.0), 0.0,
              1e-15);
}

static void test_tabular_calendar(void) {
  HijriDate epoch = {1, 1, 1};
  HijriDate last_common = {1, 12, 29};
  HijriDate first_second_year = {2, 1, 1};
  HijriDate leap_end = {2, 12, 30};

  check_close("tabular_epoch_jd", hijri_tabular_to_jd(epoch), 1948439.5,
              1e-9);
  check_date("tabular_epoch_inverse", hijri_tabular_from_jd(1948439.5), 1, 1,
             1);
  check_close("year_1_last_day", hijri_tabular_to_jd(last_common), 1948792.5,
              1e-9);
  check_close("year_2_first_day", hijri_tabular_to_jd(first_second_year),
              1948793.5, 1e-9);
  check_close("year_2_leap_end", hijri_tabular_to_jd(leap_end), 1949147.5,
              1e-9);
  check_date("year_3_first_day", hijri_tabular_from_jd(1949148.5), 3, 1, 1);

  {
    const HijriDate samples[] = {
        {1, 1, 1}, {2, 12, 30}, {30, 12, 29}, {1442, 9, 1}, {1500, 1, 1}};
    size_t index;
    for (index = 0; index < sizeof(samples) / sizeof(samples[0]); index++) {
      HijriDate result =
          hijri_tabular_from_jd(hijri_tabular_to_jd(samples[index]));
      char name[48];
      snprintf(name, sizeof(name), "tabular_roundtrip_%lu",
               (unsigned long)index);
      check_date(name, result, samples[index].year, samples[index].month,
                 samples[index].day);
    }
  }
}

int main(void) {
  test_julian_day();
  test_tabular_calendar();
  printf("Hijri tests: %d checks, %d failures\n", checks, failures);
  return failures == 0 ? 0 : 1;
}
```

- [ ] **Step 2: Compile and run the C99 test**

Run:

```bash
gcc -std=c99 -Wall -Wextra -Wpedantic -O2 tests/test_hijri.c -lm -o /tmp/libmuslim-test-hijri
/tmp/libmuslim-test-hijri
```

Expected: exit code `0` and the line `Hijri tests: 19 checks, 0 failures`.

- [ ] **Step 3: Compile and run the same source as C++17**

Run:

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -O2 -x c++ tests/test_hijri.c -lm -o /tmp/libmuslim-test-hijri-cpp
/tmp/libmuslim-test-hijri-cpp
```

Expected: exit code `0` and the line `Hijri tests: 19 checks, 0 failures`.

- [ ] **Step 4: Commit**

```bash
git add tests/test_hijri.c
git commit -m "test: add exact hijri arithmetic coverage"
```

### Task 2: Criterion and visibility-model boundaries → verify: the test binary reports `56 checks, 0 failures` and exercises all nine criterion enum values

**Files:**
- Modify: `tests/test_hijri.c`

- [ ] **Step 1: Add boolean and floating-point helpers**

Insert after `check_date`:

```c
static void check_true(const char *name, int condition) {
  checks++;
  if (!condition) {
    failures++;
    printf("FAIL synthetic/%s expected=true\n", name);
  }
}

static HijriHilalParameters parameters(double altitude, double elongation,
                                       double age, double lag,
                                       int conjunction_before,
                                       int moonset_after) {
  HijriHilalParameters value = {0};
  value.moon_altitude_deg = altitude;
  value.elongation_deg = elongation;
  value.moon_age_hours = age;
  value.lag_time_minutes = lag;
  value.conjunction_before_sunset = conjunction_before;
  value.moonset_after_sunset = moonset_after;
  return value;
}
```

- [ ] **Step 2: Add synthetic binary-criterion tests**

Insert before `main`:

```c
static void check_criterion(const char *name, HijriCriterion criterion,
                            HijriHilalParameters value, int expected) {
  check_int(name, hijri_criterion_evaluate(criterion, &value), expected);
}

static void test_binary_criteria(void) {
  check_criterion("umm_both", HIJRI_CRIT_UMM_AL_QURA,
                  parameters(0, 0, 0, 0, 1, 1), 1);
  check_criterion("umm_no_conjunction", HIJRI_CRIT_UMM_AL_QURA,
                  parameters(0, 0, 0, 0, 0, 1), 0);
  check_criterion("umm_no_moonset", HIJRI_CRIT_UMM_AL_QURA,
                  parameters(0, 0, 0, 0, 1, 0), 0);

  check_criterion("mabims_1992_below", HIJRI_CRIT_MABIMS_1992,
                  parameters(1.999, 2.999, 7.999, 0, 0, 0), 0);
  check_criterion("mabims_1992_geometry_equal", HIJRI_CRIT_MABIMS_1992,
                  parameters(2.0, 3.0, 0, 0, 0, 0), 1);
  check_criterion("mabims_1992_age_equal", HIJRI_CRIT_MABIMS_1992,
                  parameters(0, 0, 8.0, 0, 0, 0), 1);

  check_criterion("mabims_2021_below_altitude", HIJRI_CRIT_MABIMS_2021,
                  parameters(2.999, 6.4, 0, 0, 0, 0), 0);
  check_criterion("mabims_2021_below_elongation", HIJRI_CRIT_MABIMS_2021,
                  parameters(3.0, 6.399, 0, 0, 0, 0), 0);
  check_criterion("mabims_2021_equal", HIJRI_CRIT_MABIMS_2021,
                  parameters(3.0, 6.4, 0, 0, 0, 0), 1);

  check_criterion("wujud_zero", HIJRI_CRIT_WUJUDUL_HILAL,
                  parameters(0.0, 0, 0, 0, 1, 0), 0);
  check_criterion("wujud_positive", HIJRI_CRIT_WUJUDUL_HILAL,
                  parameters(0.001, 0, 0, 0, 1, 0), 1);
  check_criterion("wujud_no_conjunction", HIJRI_CRIT_WUJUDUL_HILAL,
                  parameters(1.0, 0, 0, 0, 0, 0), 0);

  check_criterion("turkey_below", HIJRI_CRIT_TURKEY_ICOP,
                  parameters(4.999, 8.0, 0, 0, 0, 0), 0);
  check_criterion("turkey_equal", HIJRI_CRIT_TURKEY_ICOP,
                  parameters(5.0, 8.0, 0, 0, 0, 0), 1);
  check_criterion("ecfr_below", HIJRI_CRIT_ECFR_ISNA,
                  parameters(5.0, 7.999, 0, 0, 0, 0), 0);
  check_criterion("ecfr_equal", HIJRI_CRIT_ECFR_ISNA,
                  parameters(5.0, 8.0, 0, 0, 0, 0), 1);

  check_criterion("egypt_below", HIJRI_CRIT_EGYPT,
                  parameters(0, 0, 0, 4.999, 0, 0), 0);
  check_criterion("egypt_equal", HIJRI_CRIT_EGYPT,
                  parameters(0, 0, 0, 5.0, 0, 0), 1);
  check_criterion("egypt_nan", HIJRI_CRIT_EGYPT,
                  parameters(0, 0, 0, NAN, 0, 0), 0);

  check_criterion("odeh_not_boolean", HIJRI_CRIT_ODEH,
                  parameters(99, 99, 99, 99, 1, 1), 0);
  check_criterion("yallop_not_boolean", HIJRI_CRIT_YALLOP,
                  parameters(99, 99, 99, 99, 1, 1), 0);
}
```

- [ ] **Step 3: Add Yallop and Odeh formula and zone-boundary tests**

Insert after `test_binary_criteria`:

```c
static void test_yallop(void) {
  const double base = 11.8371;
  check_close("yallop_q_zero", hijri_yallop_q(base, 0.0), 0.0, 1e-12);
  check_int("yallop_not_visible",
            hijri_yallop_classify(base + 10.0 * -0.232, 0.0),
            HIJRI_YALLOP_NOT_VISIBLE);
  check_int("yallop_needs_optical",
            hijri_yallop_classify(base + 10.0 * -0.231, 0.0),
            HIJRI_YALLOP_NEEDS_OPTICAL_AID);
  check_int("yallop_may_need_boundary",
            hijri_yallop_classify(base + 10.0 * -0.160, 0.0),
            HIJRI_YALLOP_NEEDS_OPTICAL_AID);
  check_int("yallop_may_need",
            hijri_yallop_classify(base + 10.0 * -0.159, 0.0),
            HIJRI_YALLOP_MAY_NEED_OPTICAL_AID);
  check_int("yallop_perfect_boundary",
            hijri_yallop_classify(base + 10.0 * -0.014, 0.0),
            HIJRI_YALLOP_MAY_NEED_OPTICAL_AID);
  check_int("yallop_perfect",
            hijri_yallop_classify(base + 10.0 * -0.013, 0.0),
            HIJRI_YALLOP_VISIBLE_UNDER_PERFECT_CONDITIONS);
  check_int("yallop_easy_boundary",
            hijri_yallop_classify(base + 10.0 * 0.216, 0.0),
            HIJRI_YALLOP_VISIBLE_UNDER_PERFECT_CONDITIONS);
  check_int("yallop_easy",
            hijri_yallop_classify(base + 10.0 * 0.217, 0.0),
            HIJRI_YALLOP_EASILY_VISIBLE);
}

static void test_odeh(void) {
  const double base = 7.1651;
  check_close("odeh_v_zero", hijri_odeh_v(base, 0.0), 0.0, 1e-12);
  check_int("odeh_not_visible",
            hijri_odeh_classify(base - 0.961, 0.0),
            HIJRI_ODEH_NOT_VISIBLE);
  check_int("odeh_optical_equal",
            hijri_odeh_classify(base - 0.960, 0.0),
            HIJRI_ODEH_VISIBLE_WITH_OPTICAL_AID_ONLY);
  check_int("odeh_possible_naked_equal",
            hijri_odeh_classify(base + 2.0, 0.0),
            HIJRI_ODEH_VISIBLE_WITH_OPTICAL_AID_COULD_BE_NAKED_EYE);
  check_int("odeh_naked_equal",
            hijri_odeh_classify(base + 5.65, 0.0),
            HIJRI_ODEH_VISIBLE_NAKED_EYE);
  check_int("odeh_naked_above",
            hijri_odeh_classify(base + 5.651, 0.0),
            HIJRI_ODEH_VISIBLE_NAKED_EYE);
}
```

- [ ] **Step 4: Call the new tests**

Replace `main` with:

```c
int main(void) {
  test_julian_day();
  test_tabular_calendar();
  test_binary_criteria();
  test_yallop();
  test_odeh();
  check_true("all_criterion_enums_represented",
             HIJRI_CRIT_YALLOP - HIJRI_CRIT_UMM_AL_QURA + 1 == 9);
  printf("Hijri tests: %d checks, %d failures\n", checks, failures);
  return failures == 0 ? 0 : 1;
}
```

- [ ] **Step 5: Run both language modes**

Run:

```bash
gcc -std=c99 -Wall -Wextra -Wpedantic -O2 tests/test_hijri.c -lm -o /tmp/libmuslim-test-hijri
/tmp/libmuslim-test-hijri
g++ -std=c++17 -Wall -Wextra -Wpedantic -O2 -x c++ tests/test_hijri.c -lm -o /tmp/libmuslim-test-hijri-cpp
/tmp/libmuslim-test-hijri-cpp
```

Expected: both commands exit `0` and both binaries report
`Hijri tests: 56 checks, 0 failures`.

- [ ] **Step 6: Commit**

```bash
git add tests/test_hijri.c
git commit -m "test: cover hijri criteria and visibility boundaries"
```

### Task 3: Reproducible 2020–2025 diagnostic matrix → verify: the probe emits a 133-line CSV containing 132 cases, all 18 named locations, and all seven binary criteria

**Files:**
- Create: `tests/hijri_research_probe.c`
- Create: `tests/data/hijri/hijri-2020-2025-baseline.csv`

- [ ] **Step 1: Create the diagnostic probe**

Create `tests/hijri_research_probe.c` with this complete content:

```c
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

static const HijriCriterion criteria[] = {
    HIJRI_CRIT_UMM_AL_QURA,    HIJRI_CRIT_MABIMS_1992,
    HIJRI_CRIT_MABIMS_2021,    HIJRI_CRIT_WUJUDUL_HILAL,
    HIJRI_CRIT_TURKEY_ICOP,    HIJRI_CRIT_ECFR_ISNA,
    HIJRI_CRIT_EGYPT};

static const char *criterion_name(HijriCriterion criterion) {
  static const char *names[] = {"UMM_AL_QURA", "MABIMS_1992", "MABIMS_2021",
                                "WUJUDUL_HILAL", "TURKEY_ICOP", "ECFR_ISNA",
                                "EGYPT"};
  return names[(int)criterion];
}

int main(void) {
  size_t date_index;
  size_t location_index;
  size_t criterion_index;

  puts("date,region,location,latitude,longitude,elevation,criterion,"
       "sunset_jd,conjunction_jd,moonset_jd,moon_altitude,sun_altitude,"
       "arcv,elongation,crescent_width,moon_age,lag,decision,yallop,odeh");

  for (date_index = 0; date_index < sizeof(dates) / sizeof(dates[0]);
       date_index++) {
    for (location_index = 0;
         location_index < sizeof(locations) / sizeof(locations[0]);
         location_index++) {
      for (criterion_index = 0;
           criterion_index < sizeof(criteria) / sizeof(criteria[0]);
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
```

- [ ] **Step 2: Reduce the matrix to one applicable criterion per region**

The complete Cartesian product would contain 756 cases and obscure the
criterion-to-region relationship. Replace the innermost criterion loop with a
single helper that maps each region to one criterion, while retaining both
MABIMS variants by emitting two rows for each MABIMS site. Keep one row for
each of the other 14 sites. This yields 22 rows per date and 132 data rows.
Use this exact helper:

```c
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
```

Also add `#include <string.h>` and use the helper’s returned count as the
criterion-loop bound.

- [ ] **Step 3: Compile, generate, and validate the baseline**

Run:

```bash
mkdir -p tests/data/hijri
gcc -std=c99 -Wall -Wextra -Wpedantic -O2 tests/hijri_research_probe.c -lm -o /tmp/libmuslim-hijri-probe
/tmp/libmuslim-hijri-probe > tests/data/hijri/hijri-2020-2025-baseline.csv
wc -l tests/data/hijri/hijri-2020-2025-baseline.csv
```

Expected: compilation exits `0`; `wc` reports exactly `133` lines.

Run:

```bash
for name in Jakarta 'Kuala Lumpur' Singapore 'Bandar Seri Begawan' Yogyakarta Makassar Jayapura Mecca Cairo Alexandria Aswan Istanbul Ankara Erzurum London 'New York' Toronto Sydney; do
  grep -q ",$name," tests/data/hijri/hijri-2020-2025-baseline.csv
done
for criterion in UMM_AL_QURA MABIMS_1992 MABIMS_2021 WUJUDUL_HILAL TURKEY_ICOP ECFR_ISNA EGYPT; do
  grep -q ",$criterion," tests/data/hijri/hijri-2020-2025-baseline.csv
done
```

Expected: exit code `0`.

- [ ] **Step 4: Commit**

```bash
git add tests/hijri_research_probe.c tests/data/hijri/hijri-2020-2025-baseline.csv
git commit -m "test: add hijri research diagnostic matrix"
```

### Task 4: Provenance and discrepancy gate → verify: every candidate case has a primary source, convention, observation, discrepancy category, and `admit` or `reject` disposition

**Files:**
- Create: `tests/data/hijri/hijri-2020-2025-sources.md`

- [ ] **Step 1: Write the research report structure and confirmed starting evidence**

Create `tests/data/hijri/hijri-2020-2025-sources.md` with:

```markdown
# Hijri 2020–2025 Reference Research

## Rules

Permanent fixtures must be offline and reproducible. Each admitted fixture
records the authority or original paper, retrieval date, coordinates,
elevation, civil date, evaluation instant, time scale, topocentric/geocentric
convention, expected value, source precision, tolerance rationale, and policy
scope. A secondary calculator can corroborate a value but cannot be its only
provenance.

Disposition is `admit` only when the expected value and its convention are
unambiguous. It is `reject` when observation, geographic aggregation, policy,
or an unavailable convention prevents deterministic reproduction.

## Confirmed sources

| ID | Subject | Primary source | Use |
|---|---|---|---|
| MABIMS-RI-2026-RULE | MABIMS rule and historical adoption | Kementerian Agama RI, “Sejarah dan Perkembangan Kriteria Hilal MABIMS dalam Penentuan Awal Bulan Hijriah,” https://kemenag.go.id/en/nasional/sejarah-dan-perkembangan-kriteria-hilal-mabims-dalam-penentuan-awal-bulan-hijriah-8kdmn | Confirms old 2–3–8 rule, new 3°/6.4° rule, and Indonesian use beginning in 2022; not itself a numerical fixture. |
| ODEH-2004 | Odeh classifier | Mohammad Sh. Odeh, “New Criterion for Lunar Crescent Visibility,” Experimental Astronomy 18, 39–64, DOI 10.1007/s10686-005-9002-5 | Formula, topocentric inputs, observation dataset, four classifications. |
| YALLOP-1998 | Yallop classifier | B. D. Yallop, “A Method for Predicting the First Sighting of the New Crescent Moon,” NAO Technical Note 69, updated April 1998 | Formula, best-time calculation, input conventions, classification boundaries. |
| DIYANET-2016 | Unified calendar policy | Diyanet İşleri Başkanlığı, International Hijri Calendar Union Congress final declaration, 30 May 2016, https://istanbul.diyanet.gov.tr/sayfalar/contentdetail.aspx?contentid=233&menucategory=kurumsal | Confirms global policy context; numerical 5°/8° implementation still requires primary technical documentation. |

Retrieval date for all web sources: 2026-07-28.

## Baseline observations

| ID | Case | Convention | Observation | Category | Disposition |
|---|---|---|---|---|---|
| OBS-CONJUNCTION-AGE | Evenings immediately before astronomical conjunction | The relevant lunation has not begun; age since that conjunction must be negative and `conjunction_before_sunset` false. | `hijri_evaluate_evening()` requests the previous conjunction, so the returned age can be approximately one synodic month and can trigger MABIMS 1992’s `age >= 8h` branch before the next conjunction. | formula/orchestration | reject until corrected |
| OBS-YALLOP-TIME | Every real Yallop classification | Yallop Technical Note 69 evaluates its quantities at the prescribed best time. | `hijri_compute_hilal_parameters()` evaluates the generic fields at sunset and passes them directly to `hijri_yallop_classify()`. | evaluation-time/convention | reject until a best-time API exists |
| OBS-UMM-LOCATION | Umm al-Qura local evaluations outside Mecca | The astronomical rule is evaluated for Mecca and the resulting calendar is consumed elsewhere. | Passing another city to `hijri_evaluate_evening()` changes the decision site. | geographic policy | reject as an Umm al-Qura decision fixture |

## Candidate fixture matrix

Research at least one clear pass, one clear failure, and one near-boundary
evening for every location group in the approved design. Add one row per
candidate using these columns:

| ID | Criterion/model | Location | Civil evening | Primary source | Evaluation instant and convention | Reference value | Current value | Difference | Category | Disposition |
|---|---|---|---|---|---|---|---|---|---|---|

Do not admit a row whose category or convention is unknown. Do not choose a
tolerance until the reference precision and implementation model error have
both been quantified.

## Required output for the fixture implementation plan

The follow-up plan may be written only after:

- every supported criterion/model has three or more relevant locations, except
  Umm al-Qura’s single Mecca decision site;
- each group contains clear-pass, clear-fail, and near-boundary cases;
- Yallop and Odeh cases identify the published classification zone;
- all admitted cases have numerical reference values and justified tolerances;
- every rejected case states why it cannot be a deterministic assertion;
- formula or orchestration defects discovered here have a minimal proposed
  correction and a regression case.
```

- [ ] **Step 2: Populate the candidate matrix from primary sources**

For each candidate, transcribe the published value and convention. Every
populated row must end in `admit` or `reject`; no ambiguous disposition is
permitted.

- [ ] **Step 3: Cross-check numerical candidates**

For every `admit` row, independently calculate the same quantity using a
high-precision ephemeris and record the tool/version and difference in the
row. Compare it with the matching row in
`tests/data/hijri/hijri-2020-2025-baseline.csv`.

Run:

```bash
git diff --check -- tests/data/hijri/hijri-2020-2025-sources.md
```

Expected: exit code `0`.

- [ ] **Step 4: Commit**

```bash
git add tests/data/hijri/hijri-2020-2025-sources.md
git commit -m "docs: record hijri reference research"
```

### Task 5: Document the workflow → verify: README commands rebuild the 56-check suite and regenerate a byte-identical 133-line baseline

**Files:**
- Modify: `README.md`

- [ ] **Step 1: Add Hijri verification commands**

Append this section to `README.md`:

```markdown
## Hijri verification

Run the deterministic arithmetic, criterion, and visibility-model tests:

```sh
gcc -std=c99 -Wall -Wextra -Wpedantic -O2 tests/test_hijri.c -lm -o /tmp/libmuslim-test-hijri
/tmp/libmuslim-test-hijri
```

The suite prints `Hijri tests: 56 checks, 0 failures`.

Regenerate the research baseline:

```sh
gcc -std=c99 -Wall -Wextra -Wpedantic -O2 tests/hijri_research_probe.c -lm -o /tmp/libmuslim-hijri-probe
/tmp/libmuslim-hijri-probe > /tmp/hijri-2020-2025-baseline.csv
cmp /tmp/hijri-2020-2025-baseline.csv tests/data/hijri/hijri-2020-2025-baseline.csv
```

The committed CSV is diagnostic evidence, not an authoritative oracle.
Reference sources, conventions, discrepancies, and fixture admission decisions
are recorded in `tests/data/hijri/hijri-2020-2025-sources.md`.
```

- [ ] **Step 2: Run the documented workflow exactly**

Run:

```bash
gcc -std=c99 -Wall -Wextra -Wpedantic -O2 tests/test_hijri.c -lm -o /tmp/libmuslim-test-hijri
/tmp/libmuslim-test-hijri
gcc -std=c99 -Wall -Wextra -Wpedantic -O2 tests/hijri_research_probe.c -lm -o /tmp/libmuslim-hijri-probe
/tmp/libmuslim-hijri-probe > /tmp/hijri-2020-2025-baseline.csv
cmp /tmp/hijri-2020-2025-baseline.csv tests/data/hijri/hijri-2020-2025-baseline.csv
wc -l /tmp/hijri-2020-2025-baseline.csv
```

Expected: the test reports `56 checks, 0 failures`; `cmp` exits `0`; `wc`
reports `133`.

- [ ] **Step 3: Run repository regression tests**

Run:

```bash
gcc -std=c99 -Wall -Wextra -Wpedantic -O2 tests/test_prayertimes.c -lm -o /tmp/libmuslim-test-prayertimes
/tmp/libmuslim-test-prayertimes
gcc -std=c99 -Wall -Wextra -Wpedantic -O2 tests/test_timezone.c -o /tmp/libmuslim-test-timezone
/tmp/libmuslim-test-timezone
```

Expected: both binaries exit `0`; prayer-time tests report `896` passing
checks and timezone tests print their passing summary.

- [ ] **Step 4: Commit**

```bash
git add README.md
git commit -m "docs: add hijri verification workflow"
```

### Task 6: Evidence gate for the authoritative fixture plan → verify: the research report satisfies every required-output bullet and a follow-up plan names only admitted fixtures

**Files:**
- Modify: `tests/data/hijri/hijri-2020-2025-sources.md`
- Create: `plans/2026-07-28-hijri-authoritative-fixtures.md`

- [ ] **Step 1: Audit the evidence gate**

Run:

```bash
rg -n '^\\| .*\\| (admit|reject) \\|$' tests/data/hijri/hijri-2020-2025-sources.md
rg -n 'formula/orchestration|evaluation-time/convention|geographic policy|ephemeris precision|observational decision' tests/data/hijri/hijri-2020-2025-sources.md
```

Expected: the first command lists every populated candidate row; the second
lists every discrepancy category actually encountered. If any approved-design
criterion, location group, pass/fail/boundary class, or Yallop/Odeh zone lacks
evidence, continue research rather than inventing a fixture.

- [ ] **Step 2: Write the authoritative-fixture implementation plan**

Use vibekit’s `plan-write` workflow with:

```text
Spec: specs/2026-07-28-hijri-comprehensive-tests-design.md
Evidence: tests/data/hijri/hijri-2020-2025-sources.md
Baseline: tests/data/hijri/hijri-2020-2025-baseline.csv
Output: plans/2026-07-28-hijri-authoritative-fixtures.md
Constraint: every numerical fixture in the plan must reference an `admit` row;
every implementation correction must reference a diagnosed regression case.
```

Expected: the new plan contains exact fixture values, per-quantity tolerances,
source IDs, failing-test commands, minimal corrections, passing-test commands,
and commit boundaries. It contains no placeholder or rejected fixture.

- [ ] **Step 3: Commit the evidence completion and follow-up plan**

```bash
git add tests/data/hijri/hijri-2020-2025-sources.md plans/2026-07-28-hijri-authoritative-fixtures.md
git commit -m "plan: define authoritative hijri fixtures"
```

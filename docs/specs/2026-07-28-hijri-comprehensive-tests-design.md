---
title: Comprehensive Hijri Test Suite
date: 2026-07-28
status: approved
---

# Comprehensive Hijri Test Suite — Design

## Problem

`hijri.h` implements calendar arithmetic, astronomical primitives, local
crescent parameters, multiple calendar criteria, graded Yallop and Odeh
visibility models, and Gregorian-to-Hijri orchestration. It currently has no
dedicated comprehensive test suite comparable to `tests/test_prayertimes.c`.

A collection of published calendar dates alone would not be sufficient
validation. A mismatch could originate in the solar or lunar ephemeris, event
timing, coordinate conventions, criterion interpretation, geographic policy,
observational declarations, or final date conversion. The test suite therefore
needs layered, traceable reference data that makes failures diagnosable.

The permanent reference cases will cover modern dates from 2020 through 2025.
Research must precede decisions about tolerances or implementation changes.

## Goals

- Cover every public API exposed by `hijri.h`.
- Exercise every supported binary calendar criterion with at least three
  representative locations from regions where the criterion is used.
- Cover clear-pass, clear-fail, and near-threshold evenings.
- Test Yallop and Odeh as graded crescent-visibility classifiers.
- Validate astronomical primitives against authoritative or high-precision
  independent references.
- Add selected end-to-end calendar checks where published authority policy is
  compatible with the library's deterministic calculation.
- Store reviewed fixtures offline with enough provenance to reproduce them.
- Produce diagnostic failures that identify the first calculation layer at
  which a discrepancy appears.
- Compile and run the permanent suite offline with strict compiler warnings,
  and under C and C++ where practical.

## Non-goals

- Predict weather or reproduce inherently weather-dependent sighting outcomes.
- Treat a human religious declaration as though it were always the output of a
  local deterministic astronomical rule.
- Claim religious authority or decide which calendar policy is correct.
- Force different national or organizational calendars to agree.
- Turn Yallop or Odeh classifications into a binary calendar decision without
  an explicitly documented application policy.
- Hide unexplained discrepancies by selecting broad tolerances.

## Constraints

- Reference dates are limited to 2020–2025.
- The permanent tests must not require network access.
- Every permanent external-data fixture must record its source or publication,
  retrieval date, location coordinates, elevation when relevant, civil date,
  evaluation instant, coordinate and time conventions, expected value, and
  justified tolerance.
- Source priority is:
  1. official authority calendars, regulations, and astronomical
     presentations;
  2. original Yallop and Odeh publications and published maps or records;
  3. reputable high-precision astronomical sources;
  4. independent libraries used only as cross-checks.
- The lunar model in `hijri.h` is documented as approximate. Its stated
  accuracy is a research subject, not advance permission to accept any
  particular discrepancy.
- Existing unrelated and untracked workspace files must remain untouched.

## Approach

Use a layered, table-driven, offline fixture suite in
`tests/test_hijri.c`, supported by provenance data and a research report.
An independent high-precision astronomy implementation may be used during
fixture research, but it will not become an unexplained runtime oracle.

The suite will have six layers:

1. **Exact mathematical behavior.** Test Julian Day conversion, Gregorian
   round trips, weekday calculation, Julian centuries, Delta T behavior,
   tabular Hijri conversion, Hijri month transitions, and tabular leap years.
   Exact expected results will be used wherever the underlying operation is
   exact.

2. **Synthetic criterion behavior.** Construct `HijriHilalParameters` directly
   at values immediately below, exactly equal to, and immediately above each
   threshold. This isolates boolean logic, equality semantics, conjunction and
   moonset flags, and AND/OR combinations from the astronomical pipeline.

3. **Astronomical primitives.** Compare solar and lunar positions,
   topocentric correction, altitude, conjunction, sunset, and moonset against
   high-precision references. Each quantity will have a separately justified
   tolerance based on algorithmic accuracy and reference uncertainty.

4. **Real-evening Hilal parameters.** For researched dates and locations,
   compare moon and Sun altitude, ARCV, elongation, crescent width, moon age,
   lag, conjunction ordering, moonset ordering, and resulting criterion
   decisions. Include obvious and near-boundary cases.

5. **Yallop and Odeh classifications.** Test score polynomials and every zone
   boundary synthetically, then compare real locations against published
   classification maps or records. Research and implement the models using
   their prescribed evaluation time and precise ARCV and crescent-width
   conventions. In particular, the current use of sunset values for Yallop
   must be investigated against Yallop's prescribed best time before real-map
   fixtures are accepted.

6. **Published-calendar integration.** Test full Gregorian-to-Hijri conversion
   only when the published calendar implements a policy compatible with the
   library. Cases influenced by later observations, national aggregation, or
   global visibility will be labeled and kept out of local deterministic
   assertions unless that policy is explicitly modeled.

The initial geographic matrix is:

- **MABIMS 1992 and MABIMS 2021:** Jakarta, Kuala Lumpur, Singapore, and
  Bandar Seri Begawan. MABIMS 1992 cases will precede adoption of the new
  criterion; MABIMS 2021 cases will use applicable 2022–2025 references.
- **Wujudul Hilal:** Indonesian locations spanning western, central, and
  eastern longitudes, checked against Muhammadiyah calendar decisions and
  published Hisab data.
- **Umm al-Qura:** evaluate the authority rule at Mecca. Riyadh, Jeddah, and
  other Saudi cities may be calendar-consumer integration cases, but will not
  be represented as independent astronomical decision sites.
- **Egypt:** Cairo, Alexandria, and Aswan, subject to verification that the
  implemented five-minute lag rule reflects the relevant 2020–2025 policy.
- **Turkey/ICOP:** Istanbul, Ankara, and an eastern Turkish location for local
  parameter coverage, with global-visibility policy tested separately when
  supported by authoritative data.
- **ECFR/ISNA:** London, New York, and Toronto or Chicago, separating local
  thresholds from global calendar policy.
- **Yallop and Odeh:** sample locations from published map zones, including at
  least one location from every available visibility class.

Research proceeds before permanent fixtures are written:

1. Build a source inventory and candidate fixture matrix.
2. Reproduce representative source values with an independent high-precision
   astronomy tool.
3. Compare `hijri.h` results using a temporary research harness.
4. Classify discrepancies as formula errors, ephemeris precision, evaluation
   time, coordinate/reference convention, geographic policy, observational
   decisions, or uncertain source data.
5. Promote only explained and reproducible cases into permanent fixtures.
6. Document defects separately and propose the smallest library correction
   before requiring affected integration tests to pass.

Test failure messages will contain the layer, criterion, location, civil date,
calculated value, reference value, difference, tolerance, and provenance ID.

Deliverables are:

- `tests/test_hijri.c`;
- offline fixture and provenance data where keeping it separate improves
  readability;
- a 2020–2025 research report describing sources and discrepancies;
- build and run instructions consistent with the repository.

## Alternatives considered

**Published-calendar golden tests only.** This closely resembles the existing
prayer-time suite and provides direct end-user coverage, but failures conflate
astronomy, geographic aggregation, observations, authority policy, and date
orchestration. It also cannot correctly express graded Yallop and Odeh output.

**Runtime comparison with an external library or API.** This provides a useful
research oracle and can expose low-level numerical differences, but it adds
dependencies or network requirements and risks treating another implementation
as ground truth without establishing its conventions. It will be used only for
research and fixture cross-checking.

The selected approach combines layered offline fixtures with a limited
published-calendar integration layer because it provides both diagnostic value
and end-user confidence.

## Testing

The suite itself will be validated by:

- compiling with strict warnings in the repository's supported C mode;
- compiling the public header and applicable suite paths as C++ where
  practical;
- running every fixture without network access;
- proving all synthetic threshold boundaries and classification zones are
  exercised;
- proving every public API has direct or integration coverage;
- checking that every external fixture has a valid provenance record;
- verifying representative fixture calculations independently;
- intentionally perturbing representative expected values during development
  to confirm diagnostic output identifies the correct layer and case.

No tolerance will be accepted merely because it makes the current
implementation pass. Numerical tolerances will be recorded per quantity with a
reason tied to the algorithm, source precision, and coordinate convention.

## Open questions

- Which authoritative datasets provide sufficiently precise, reproducible
  2020–2025 values for every criterion and location remains a research task.
- Whether the current Egypt, Turkey/ICOP, and ECFR/ISNA descriptions exactly
  match their respective 2020–2025 operational policies must be verified.
- The exact library changes required for standards-compliant Yallop best-time
  evaluation and model-specific ARCV conventions depend on comparison with the
  original publication.
- The practical C++ coverage boundary will be determined from the current
  header's compatibility and build environment.

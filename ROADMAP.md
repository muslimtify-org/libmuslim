# Hijri Calendar Roadmap

This roadmap covers the development of [`hijri.h`](hijri.h): its astronomical
calculations, local crescent predicates, visibility models, calendar policies,
tests, and validation data.

The roadmap is accuracy-first. A named calendar or authority policy should not
be added merely because its name and headline thresholds are known. It should
be added only when its geographic scope, astronomical conventions, decision
rules, and validation references are sufficiently documented.

## Scope

This roadmap covers [`hijri.h`](hijri.h) only. `prayertimes.h` and
`timezone.h` are tracked here solely as regression-isolation targets under
testing layer 9; neither has a roadmap yet.

One question must be answered before the time-scale documentation work below,
because it decides where that documentation belongs: do the headers
share an astronomy core, or does each carry its own solar code? Both currently
compute solar position independently.

## Current foundation

The library currently provides:

- Dependency-free, single-header C implementation.
- Strict C11 and C++17 compatibility.
- Gregorian, Julian Day, and tabular Hijri calendar arithmetic.
- Solar and abbreviated lunar-position calculations.
- Sunset, moonset, and conjunction event calculations with explicit event
  status.
- Candidate-crescent conjunction selection with signed Moon age.
- Explicit evening parameters:
  - geometric Moon-center altitude;
  - apparent Moon upper-limb altitude;
  - geocentric elongation;
  - topocentric elongation;
  - conjunction ordering;
  - Moon age and moonset lag.
- Local MABIMS 1992 and MABIMS 2021 predicates.
- Local Wujudul Hilal predicate.
- Neutrally named five-minute-lag and local 5°/8° research predicates.
- Dedicated six-zone Yallop visibility evaluation.
- Dedicated four-zone Odeh visibility evaluation.
- Explicit Mecca-based Umm al-Qura-style conversion policy.
- Predicate-derived local calendar conversion whose API explicitly requires
  an observer location.
- Synthetic threshold, boundary, unavailable-event, high-latitude, and
  orchestration tests.

The current lunar ephemeris is intentionally compact and approximate. Results
near a criterion boundary must not be presented as observational-grade or as
an official calendar declaration.

## Before v1.0 — blocks the tag

Items that are expensive or impossible to change after tagging.

### Full Meeus ch. 47 lunar series

Replace the body of `hijri_moon_position()`. It currently uses roughly six
longitude terms, four latitude terms, and four distance terms of ELP2000-82B.
The full series is public, introduces no dependency, and changes no public
declaration.

**Depends on:** nothing. This is the first item because every accuracy claim
below is bounded by it.

**Completion criteria:**

- The longitude, latitude, and distance series are complete, including the
  eccentricity correction applied to terms in the Sun's mean anomaly and the
  additive A1/A2/A3 terms.
- No public struct or function signature changes.
- The library still builds with no runtime dependency.
- Existing synthetic threshold tests continue to pass unchanged.

### Cross-engine numerical comparison

Compare `hijri.h` with at least two independent astronomical implementations
or datasets.

**Depends on:** nothing — runnable today against the compact approximation, and
again after the full series lands. Its output is the measured error bar that
near-boundary reporting and every fixture tolerance below require.

**Completion criteria:**

- Comparisons use identical input conventions.
- Raw astronomical parameters are compared before calendar decisions.
- Systematic offsets are investigated rather than hidden by broad tolerances.

### Status-return conventions

Replace failure-only returns with explicit status values where callers need to
distinguish invalid input, unavailable events, unsupported dates, and numerical
failure. `hijri_find_conjunction()`, `hijri_find_previous_conjunction()`,
`hijri_find_next_conjunction()`, and `hijri_find_relevant_conjunction()`
currently return a bare `double` with no failure channel.

**`hijri.h` is not tagged v1.0 until this lands.** The header comment already
reads `v1.0` while no git tag exists, and no external code consumes `hijri.h`
today, so the break is free now and expensive later.

**Completion criteria:**

- Every public function that can fail reports failure through a status value,
  not a sentinel or an unchecked out-parameter.
- The header version string matches the tag that actually exists.
- Examples and tests are updated to check status before using a result.

### Build and compilation enforcement

Testing layer 8 below claims strict C11 and C++17 compilation. The repository
contains no build file and nothing compiles either header as C++, so the
claim is currently unverified. Add a build entry point that checks it.

**Completion criteria:**

- One command builds every header, test, and example as strict C11.
- The same command compiles both headers as C++17.
- The advertised language compatibility is verified by that command rather
  than by hand-run instructions in `README.md`.

## v1.0 — completes the release

Additive items. Safe to land during the 1.0 cycle.

### Uncertainty and near-boundary results

Add a way to identify results too close to a threshold for the selected
ephemeris accuracy.

This should not silently change a criterion from pass to fail. It should expose
the uncertainty so applications can decide how to handle it.

**Depends on:** the measured error bar from cross-engine comparison. Without a
number, "near a boundary" has no definition.

**Completion criteria:**

- The backend documents an accuracy estimate or uncertainty source.
- Criterion evaluation can report that a result is near a boundary.
- Exact synthetic threshold tests remain deterministic.
- Documentation distinguishes numerical uncertainty from policy uncertainty.

### Time-scale and reference-frame documentation

Document UT, TT, Delta T, apparent/geometric altitude, refraction, parallax,
semidiameter, and geocentric/topocentric conventions next to the affected
public fields and calculations.
Where this documentation belongs depends on the shared-astronomy-core question
raised under Scope.

**Completion criteria:**

- Every public astronomical value has an explicit unit and convention.
- Tests independently reproduce each derived public parameter.
- No criterion relies on an ambiguously named altitude or elongation.

### Supported-date ranges

Document and test the reliable date range of every numerical backend and
official calendar table.

### Thread safety and deterministic builds

Audit implementation state, platform-specific math behavior, and optional
backends for deterministic and thread-safe use.

## Post-v1.0

Gated on external research, or on a second implementation existing.

### Pluggable ephemeris backend

Allow applications to select between the compact built-in ephemeris and an
external high-precision one.

**Not built until a second backend actually exists.** The
`HijriMoonPosition` struct is already a compile-time seam; the accuracy caveat
at the top of `hijri.h` states that nothing else in the file needs to change to
swap the implementation. A runtime selection layer for a single backend is
speculative abstraction, and this roadmap already applies that rule to calendar
policies.

The public evening-parameter and criterion APIs remain independent of the
selected backend.

**Completion criteria:**

- The compact backend remains the default and introduces no runtime
  dependency.
- At least one independently validated high-precision backend is available.
- Both backends implement the same documented coordinate and time-scale
  conventions.
- Tests quantify differences in conjunction time, altitude, elongation, and
  moonset for a representative global dataset.

### Modern reference dataset

Build a reproducible validation set for modern dates, initially covering
2020–2025 and multiple geographic regions.

Each accepted fixture must record:

- civil date and observer coordinates;
- elevation;
- time scale and Delta T convention;
- ephemeris source and version;
- atmospheric/refraction convention;
- Moon limb convention;
- geocentric or topocentric reference frame;
- expected value and justified tolerance;
- primary or independently reproducible source.

**Completion criteria:**

- No fixture is admitted from a published date or rounded number alone.
- Every tolerance follows from the source precision or a measured backend
  error.
- Validation includes locations in both hemispheres, high latitudes, and
  different elevations.
- Failures are classified by astronomy, convention, visibility model, or
  policy layer.

### General policy boundary

Introduce a general calendar-policy abstraction only when there are at least
two complete policies with genuinely different decision flows.

A policy may need to represent:

- one or multiple observation locations;
- regional or global geographic aggregation;
- time-bounded global searches;
- published calendar tables;
- administrative overrides;
- actual sighting declarations.

**Completion criteria:**

- Local predicates cannot be mistaken for complete authority policies.
- Graded Yallop and Odeh zones cannot become dates without an explicit
  application policy.
- Every named authority policy documents its geographic and decision scope.
- Custom application policies remain possible without modifying `hijri.h`.

### Official Umm al-Qura data mode

Keep the existing astronomical Mecca conjunction-and-moonset policy distinct
from exact reproduction of published Umm al-Qura civil dates.

Add an official-data mode only when validated KACST conventions, tables, or
another authoritative reproducible dataset are available.

**Completion criteria:**

- The astronomical policy and official-table mode have unambiguous names.
- Historical coverage and table version are documented.
- Differences between calculated and published dates are testable and
  explained.
- The library does not claim historical identity outside the available data.

### Additional named policies

Egyptian, Turkish, ECFR, FCNA/ISNA, or other authority policies remain
research candidates—not planned APIs—until primary technical specifications
are available.

**Admission criteria:**

- Primary documentation defines the actual decision rule.
- Geographic aggregation and time limits are known.
- Required astronomical conventions are known.
- Modern reference decisions or numerical examples are reproducible.
- The implementation represents the complete documented policy, not a
  similarly shaped local threshold.

### Observation integration

Provide optional data structures for applications that combine calculated
visibility with actual crescent observations.

Potential information includes:

- observation time and coordinates;
- observer or organization;
- optical aid;
- weather and horizon conditions;
- positive or negative sighting;
- verification or administrative status.

**Completion criteria:**

- Deterministic astronomy remains separate from observation records.
- The core library does not decide religious validity or witness acceptance.
- Applications can trace a calendar decision to its astronomical and
  observational inputs.

### Backend and policy metadata

Allow applications to record which ephemeris, policy, conventions, and data
version produced a result.

## Testing priorities

Maintain the following test layers:

1. Exact civil-date, Julian Day, and tabular arithmetic. — present
2. Synthetic formula and threshold boundaries. — present
3. Event ordering and unavailable-event behavior. — present
4. Independently reproduced astronomical parameters. — empty
5. Visibility-model zones and prescribed observation times. — present
6. Real-evening numerical validation with fully specified conventions. — empty
7. Calendar-policy decisions with authoritative references. — empty;
   `test_umm_al_qura_policy` checks the dedicated function against the
   equivalent predicate call at Mecca, which is self-consistency only
8. Strict C11 and C++17 compilation. — partial; C11 is hand-run from
   `README.md`, C++17 is never compiled
9. Prayer-time and timezone regression isolation. — tests exist, no runner

Every bug should be assigned to the lowest layer that can reproduce it. A
calendar-date mismatch should not be patched at the policy layer when its
cause is an altitude, conjunction, time-scale, or ephemeris error.

## Research prerequisites

The following sources or datasets would unlock future roadmap items:

- A stable original-publisher copy of Yallop Technical Note 69.
- Reproducible modern Yallop and Odeh reference maps or numerical examples.
- Primary MABIMS equality and rounding conventions.
- KACST event definitions and historical Umm al-Qura reference data.
- Primary technical specifications for Egyptian, Turkish, ECFR, and
  FCNA/ISNA calendar policies.
- High-precision ephemeris data with redistribution terms compatible with the
  project license.

Until those prerequisites are met, unknown conventions should remain
documented gaps rather than guessed defaults.

## Definition of production readiness

`hijri.h` may be described as production-ready for a specific use case only
when:

- the application selects and documents its astronomy backend;
- all required local predicates or policies match the intended use;
- near-boundary uncertainty is handled explicitly;
- the supported date and geographic range are known;
- validation data uses matching conventions;
- official or religious authority is not implied beyond what the implemented
  policy actually represents.

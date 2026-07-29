# Hijri Calendar Roadmap

This roadmap covers the development of [`hijri.h`](hijri.h): its astronomical
calculations, local crescent predicates, visibility models, calendar policies,
tests, and validation data.

The roadmap is accuracy-first. A named calendar or authority policy should not
be added merely because its name and headline thresholds are known. It should
be added only when its geographic scope, astronomical conventions, decision
rules, and validation references are sufficiently documented.

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

## Next: numerical accuracy

### Pluggable ephemeris backend

Allow applications to select between:

- the current compact, dependency-free approximation; and
- an optional high-precision lunar and solar ephemeris.

The public evening-parameter and criterion APIs should remain independent of
the selected ephemeris backend.

**Completion criteria:**

- The compact backend remains the default and introduces no runtime
  dependency.
- At least one independently validated high-precision backend is available.
- Both backends implement the same documented coordinate and time-scale
  conventions.
- Tests quantify differences in conjunction time, altitude, elongation, and
  moonset for a representative global dataset.

### Time-scale and reference-frame documentation

Document UT, TT, Delta T, apparent/geometric altitude, refraction, parallax,
semidiameter, and geocentric/topocentric conventions next to the affected
public fields and calculations.

**Completion criteria:**

- Every public astronomical value has an explicit unit and convention.
- Tests independently reproduce each derived public parameter.
- No criterion relies on an ambiguously named altitude or elongation.

### Uncertainty and near-boundary results

Add a way to identify results too close to a threshold for the selected
ephemeris accuracy.

This should not silently change a criterion from pass to fail. It should expose
the uncertainty so applications can decide how to handle it.

**Completion criteria:**

- The backend documents an accuracy estimate or uncertainty source.
- Criterion evaluation can report that a result is near a boundary.
- Exact synthetic threshold tests remain deterministic.
- Documentation distinguishes numerical uncertainty from policy uncertainty.

## Next: reproducible validation

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

### Cross-engine comparison

Compare `hijri.h` with at least two independent astronomical implementations
or datasets.

**Completion criteria:**

- Comparisons use identical input conventions.
- Raw astronomical parameters are compared before calendar decisions.
- Systematic offsets are investigated rather than hidden by broad tolerances.

## Later: calendar policies

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

## Later: observation integration

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

## Later: API and operational quality

### Stable error reporting

Replace ambiguous failure-only returns with explicit status values where
callers need to distinguish invalid input, unavailable events, unsupported
dates, and numerical failure.

### Backend and policy metadata

Allow applications to record which ephemeris, policy, conventions, and data
version produced a result.

### Thread safety and deterministic builds

Audit implementation state, platform-specific math behavior, and optional
backends for deterministic and thread-safe use.

### Supported-date ranges

Document and test the reliable date range of every numerical backend and
official calendar table.

## Testing priorities

Maintain the following test layers:

1. Exact civil-date, Julian Day, and tabular arithmetic.
2. Synthetic formula and threshold boundaries.
3. Event ordering and unavailable-event behavior.
4. Independently reproduced astronomical parameters.
5. Visibility-model zones and prescribed observation times.
6. Real-evening numerical validation with fully specified conventions.
7. Calendar-policy decisions with authoritative references.
8. Strict C11 and C++17 compilation.
9. Prayer-time and timezone regression isolation.

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


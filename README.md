# libmuslim

An stb-style collection of single-header C libraries for Muslim applications.
Drop a header in, define its `*_IMPLEMENTATION` macro in one translation unit,
and you are done. C11 and C++17, no build system, no package manager.

| Header | Provides | Depends on |
|---|---|---|
| [`prayertimes.h`](prayertimes.h) | Prayer times, 21 calculation methods | `<math.h>` |
| [`hijri.h`](hijri.h) | Hijri calendar, crescent visibility models | `<math.h>` |
| [`timezone.h`](timezone.h) | IANA zone name → UTC offset, DST applied | OS timezone database |

`prayertimes.h` and `hijri.h` are independent and dependency-free.
`timezone.h` is optional and is the only header that touches the OS.

## Quick start

```c
#define PRAYERTIMES_IMPLEMENTATION
#include "prayertimes.h"

const MethodParams *params = method_params_get(CALC_KEMENAG);

struct PrayerTimes t = calculate_prayer_times(
    2025, 11, 21,        // date
    -6.2851291,          // latitude, negative = South
    106.9814968,         // longitude, positive = East
    7.0,                 // UTC offset in hours
    params);

char buf[16];
format_time_hm(t.fajr, buf, sizeof buf);
printf("Fajr: %s\n", buf);   // Fajr: 04:05
```

```sh
gcc -std=c11 -Wall -Wextra -Wpedantic -O2 examples/prayertimes_example.c -lm -o example
```

## prayertimes.h

Each method sets a Fajr angle, an Isha angle or fixed interval, a Maghrib
offset, an Asr shadow factor (1 = Shafi'i, 2 = Hanafi), and an ihtiyat
precautionary margin. Kemenag is the default.

| Key | Method | Region |
|---|---|---|
| `mwl` | Muslim World League | Europe, Far East |
| `makkah` | Umm al-Qura, Makkah | Arabian Peninsula |
| `isna` | ISNA | North America |
| `egypt` | Egyptian General Authority | Africa, Middle East |
| `karachi` | Univ. Islamic Sciences, Karachi | Pakistan, India, Bangladesh |
| `turkey` | Diyanet | Turkey |
| `singapore` | MUIS | Singapore |
| `jakim` | JAKIM | Malaysia |
| `kemenag` | KEMENAG (default) | Indonesia |
| `france` | UOIF | France |
| `russia` | Spiritual Administration | Russia |
| `dubai` | GAIAE | UAE |
| `qatar` · `kuwait` · `jordan` · `gulf` | Ministries of Awqaf | Gulf states |
| `tunisia` · `algeria` · `morocco` | Ministries of Religious Affairs | Maghreb |
| `portugal` | Comunidade Islâmica de Lisboa | Portugal |
| `moonsighting` | Moonsighting Committee | Worldwide |

```c
const MethodParams *mwl = method_params_get(CALC_MWL);
CalcMethod m = method_from_string("isna");        // case-insensitive
```

Pass `CALC_CUSTOM` with your own angles for anything not listed.

To walk a date range without `struct tm` or `mktime`, and therefore without
local-time hazards, use the `static inline` day-number helpers:

```c
for (long s = mt_days_from_civil(2026, 7, 1); s <= mt_days_from_civil(2026, 7, 31); s++) {
    int y, m, d;
    mt_civil_from_days(s, &y, &m, &d);
    struct PrayerTimes t = calculate_prayer_times(y, m, d, lat, lon, tz, params);
}
```

Times are cross-checked against published timetables to within 1–2 minutes
depending on method. Those references are third-party calculators, not primary
authorities. See [`docs/KEMENAG_METHOD.md`](docs/KEMENAG_METHOD.md) for the
worked mathematics.

## hijri.h

The API is layered deliberately, and the layers mean different things:

- `HijriEveningParameters` — explicit astronomical quantities for one evening
  at one location: sunset, moonset, conjunction, altitudes, both geocentric and
  topocentric elongation, Moon age, lag.
- `HijriLocalPredicate` — one threshold condition at one observer location.
- Calendar functions — convert a Gregorian date to a Hijri date.

**A local predicate is not a national or global authority decision.** Real
authorities combine calculation with geographic aggregation, actual sighting
reports, and administrative rulings. Applications must add that themselves.

```c
HijriLocation jakarta = {-6.2088, 106.8456, 8.0, "Jakarta"};
HijriDate date;
hijri_from_gregorian_with_local_predicate(
    2026, 7, 27, &jakarta, HIJRI_PREDICATE_MABIMS_2021, &date);

hijri_umm_al_qura_from_gregorian(2026, 7, 27, &date);   // Mecca-based, takes no location
```

Yallop and Odeh are graded visibility classifications, not calendar policies —
`hijri_yallop_evaluate_evening()` and `hijri_odeh_evaluate_evening()` return
best-time parameters, a score, and a zone. Turning a zone into a date is an
application policy you define.

**Accuracy.** The lunar position uses the full Meeus ch. 47 series. Measured
against 24 JPL Horizons epochs spanning 1900–2100: 0.0051° in longitude,
0.0006° in latitude, 41.9 km in distance. Nutation and aberration are not
applied, so these are geometric positions referred to the mean equinox of date.
Calendar conversion is measured separately against a different reference.
`hijri_umm_al_qura_from_gregorian()` reads the published Umm al-Qura table for
1300–1600 AH (1882–2174 CE) rather than reconstructing it astronomically: 198
of 198 official month starts over 2015–2030, exact. The ~600-byte table is
derived from the ICU/CLDR `islamic-umalqura` calendar, verified byte-identical
to ICU's own Unicode-licensed data, and corroborated by independently
documented Saudi dates for the modern era. One honest caveat: for years before
~1423 AH (2002) the table everyone ships is a retro-computation, and three of
five documented historical correspondences (1932, 1979, 1992) differ from it
by one day. Outside the table's range the astronomical reconstruction (92.4%
measured) takes over. The measurements, anchors, and why reconstruction is
capped — including months where the official table departs from its own
stated rule — are recorded in
[`docs/research/2026-08-01-umm-al-qura-oracle.md`](docs/research/2026-08-01-umm-al-qura-oracle.md).

Against Indonesia's official calendar (Kemenag, 37 month starts 2024–2026,
validated via seven independent government announcements), the MABIMS 2021
predicate at the westernmost point is never one day early and supports 33 of
37 official starts — measured, fixture-enforced, and deliberately not
claimed as exact reproduction: the official calendar includes months the
criterion alone does not produce. Details and the geocentric-elongation
convention finding are in
[`docs/research/2026-08-01-kemenag-reference.md`](docs/research/2026-08-01-kemenag-reference.md).

Against Muhammadiyah's official calendar, the Wujudul Hilal predicate at
Yogyakarta reproduces all twelve Maklumat-announced month starts of the
criterion's final four years (1443–1446 H, before Muhammadiyah's switch to
the KHGT global calendar), never early, matching every published
wujud/belum-wujud verdict — fixture-enforced; see
[`docs/research/2026-08-01-muhammadiyah-reference.md`](docs/research/2026-08-01-muhammadiyah-reference.md).

A calculated result is still not an observation, and nothing here decides
religious validity.

**Numerical uncertainty and policy.** `hijri_predicate_margins()` reports how
far each term of a predicate sits from its own threshold, in that term's own
units, and nothing else. It does not combine terms, convert between units, or
label a margin as near, so it cannot change a decision. Whether a margin is
small enough to be unsafe is left to the application, and whether a term
sitting at exactly its threshold passes is a convention the criterion states,
not a measurement, which is why `HijriDecisionTermMargin` reports `strict`
per term. The error bars a margin should be read against are the ones
documented in `hijri.h` itself, under NUMERICAL UNCERTAINTY AND POLICY and
ACCURACY CAVEAT, and are not repeated here since duplicated figures are how
this file's past accuracy mistakes happened. That section also records that
the solar longitude residual is biased rather than symmetric, so a margin
should be weighed against the relevant side of that range, not against a
symmetric tolerance.

**Thread safety and determinism.** Every public function in `hijri.h` is a
pure function of its arguments, keeps no state between calls, and is safe to
call from any number of threads at once. Results are bit-identical run to
run on the same platform, same compiler, and same flags, which is exactly
what `make baseline` checks on every run. Measured between glibc 2.44 and
musl 1.2.6 on x86-64, the baseline CSV is byte-identical and Julian Day
instants never differ across a 336-row sweep, though angular quantities
occasionally differ by 1 to 2 units in the last place. Other platforms and
architectures are not measured. This determinism claim is specific to
`hijri.h`, see `hijri.h` itself for the full contract.

## timezone.h

> `prayertimes.h` takes a fixed numeric UTC offset and has no notion of zones or
> DST — deliberately, since DST is a political rule rather than an astronomical
> one. **For a DST-active date you must pass the adjusted offset yourself**
> (`1.0` for London in summer, `0.0` in winter).

`timezone.h` will resolve it for you from the host OS timezone database:

```c
#define MUSLIM_TIMEZONE_IMPLEMENTATION   // exactly one translation unit
#include "timezone.h"

char zone[64];
get_system_timezone(zone, sizeof zone);              // "Europe/London"
double tz = 0.0;
parse_timezone_offset(zone, time(NULL), &tz);        // DST applied
```

On a platform without a timezone database, keep supplying the offset yourself.

## Building and tests

Using the library needs no build system — drop the header in. The `Makefile` is
for checking the library itself, and needs **GNU make** (`gmake` on the BSDs):

```sh
make                              # everything below
make test                         # build all tests strict-C11 and run them
make cxx                          # compile every header as C++17
make baseline                     # regenerate the research CSV and compare
make check CC=clang CXX=clang++   # same checks, different toolchain
```

CI runs `make test cxx examples` on Linux and macOS with gcc and clang on every
push and pull request, and `make baseline` on Linux only — the CSV prints Julian
Days to nine decimals and libm rounding differs between platforms, so a
byte-exact comparison is reproducible per-platform rather than universally.

Each target enforces something the project asserts and which was previously
checked only by remembering to run a command — `prayertimes.h` once shipped
broken under its own documented build line because nobody ran it, and the first
CI run caught a macOS/glibc difference no local check could have.

`tests/test_ephemeris_oracle.c` validates the lunar series against a vendored JPL
Horizons fixture, Meeus's own printed worked example, and published ΔT values.
`tests/test_hijri.c` covers calendar arithmetic, predicate thresholds, and a
longitude sweep asserting each observer's own local evening is used.

The research baseline is a generated artifact, committed so changes to it are
visible in review. Any change to the ephemeris, the predicates, or the evening
calculation moves it, and `make baseline` fails until it is regenerated with
`make baseline-update`. **No value in that CSV is reference truth**: it is
produced by the library it is used to check, and its nine-decimal Julian Days
imply precision the underlying accuracy does not support.

Sources, conventions, and fixture admission decisions are recorded in
[`docs/research/`](docs/research/).

## Documentation

- [`docs/KEMENAG_METHOD.md`](docs/KEMENAG_METHOD.md) — prayer-time mathematics (Indonesian)
- [`docs/INTERNATIONAL_METHODS.md`](docs/INTERNATIONAL_METHODS.md) — method parameters
- [`docs/HIJRI_CALCULATIONS.md`](docs/HIJRI_CALCULATIONS.md) — Hijri calculations
- [`docs/METHOD_TOLERANCES.md`](docs/METHOD_TOLERANCES.md) — accuracy expectations
- [`ROADMAP.md`](ROADMAP.md) — what is planned and what is deliberately not
- [Understanding Islamic Prayer Time Calculations](https://medium.com/@rizkirakasiwi09/understanding-islamic-prayer-time-calculations-the-mathematics-behind-libmuslim-library-ee169e3e97c3)

## Contributing

Changes to calculation methods must be verified against a source, and the
source recorded. `docs/research/hijri-2020-2025-sources.md` shows the standard:
a fixture is admitted only when its value *and its conventions* are
unambiguous, and an unknown convention is recorded as a gap rather than guessed.

## License

MIT. Copyright 2025-2026 muslimtify-org.

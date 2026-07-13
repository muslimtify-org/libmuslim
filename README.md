# libmuslim

A lightweight C header-only library for calculating Islamic prayer times. Supports **21 international calculation methods** including MWL, ISNA, Umm al-Qura (Makkah), Egyptian General Authority, Kemenag (Indonesia), JAKIM (Malaysia), Diyanet (Turkey), and more. The default method is Kemenag.

## Features

- **21 calculation methods** — worldwide coverage from MWL to Moonsighting Committee
- Astronomical calculations using Jean Meeus algorithms
- Single-header library — easy to integrate
- Cross-platform support (Linux, macOS, Windows)
- Shafi'i and Hanafi Asr support
- CLI tool for quick calculations

## Supported Methods

| Key | Method | Region |
|-----|--------|--------|
| `mwl` | Muslim World League | Europe, Far East |
| `makkah` | Umm al-Qura, Makkah | Arabian Peninsula |
| `isna` | ISNA | North America |
| `egypt` | Egyptian General Authority | Africa, Middle East |
| `karachi` | Univ. Islamic Sciences, Karachi | Pakistan, India, Bangladesh |
| `turkey` | Diyanet, Turkey | Turkey |
| `singapore` | MUIS, Singapore | Singapore |
| `jakim` | JAKIM, Malaysia | Malaysia |
| `kemenag` | KEMENAG, Indonesia | Indonesia (default) |
| `france` | UOIF, France | France |
| `russia` | Spiritual Admin., Russia | Russia |
| `dubai` | GAIAE, Dubai | UAE |
| `qatar` | Min. of Awqaf, Qatar | Qatar |
| `kuwait` | Min. of Awqaf, Kuwait | Kuwait |
| `jordan` | Min. of Awqaf, Jordan | Jordan |
| `gulf` | Gulf Region | Gulf states |
| `tunisia` | Min. of Religious Affairs | Tunisia |
| `algeria` | Min. of Religious Affairs | Algeria |
| `morocco` | Min. of Habous, Morocco | Morocco |
| `portugal` | Comunidade Islamica de Lisboa | Portugal |
| `moonsighting` | Moonsighting Committee | Worldwide |

You can also use a custom method by passing `CALC_CUSTOM` with your own angles.

## How It Works

Each method defines a set of parameters:

- **Fajr angle** — sun depression angle for Fajr
- **Isha angle or interval** — angle-based or fixed minutes after Maghrib
- **Maghrib interval** — offset after sunset (0 for most methods)
- **Asr shadow factor** — 1 (Shafi'i) or 2 (Hanafi)
- **Ihtiyat** — precautionary margin in minutes

### Calculation Steps

1. Convert Gregorian date to Julian Day
2. Calculate solar position (declination and equation of time)
3. Determine solar transit time (true noon)
4. Compute hour angles for each prayer based on solar altitude
5. Convert hour angles to local time
6. Apply ihtiyat adjustments
7. Format times with ceiling rounding

For detailed mathematical formulas and worked examples, see [`docs/KEMENAG_METHOD.md`](docs/KEMENAG_METHOD.md).

## Building

This is a single-header library, so you can simply include `prayertimes.h` in your project.

### CLI Tool

```bash
# Compile the CLI tool
gcc -O3 -o libmuslim main.c -lm

# Run example (Bekasi, November 21, 2025)
./libmuslim 2025 11 21 -6.2851291 106.9814968 7.0
```

## Usage

### C API

```c
#include "prayertimes.h"

// Use the default method (Kemenag)
const MethodParams *params = method_params_get(CALC_KEMENAG);

struct PrayerTimes times = calculate_prayer_times(
    2025,           // year
    11,             // month
    21,             // day
    -6.2851291,     // latitude (negative = South)
    106.9814968,    // longitude (positive = East)
    7.0,            // timezone offset (WIB = UTC+7)
    params          // calculation method
);

char buffer[16];
format_time_hm(times.fajr, buffer, sizeof(buffer));
printf("Fajr: %s\n", buffer);
```

### Using a Different Method

```c
// Use MWL method
const MethodParams *mwl = method_params_get(CALC_MWL);
struct PrayerTimes times = calculate_prayer_times(2025, 11, 21, 51.5074, -0.1278, 0.0, mwl);

// Look up method by string key
CalcMethod method = method_from_string("isna");
const MethodParams *params = method_params_get(method);
```

## Timezones & DST

> **NOTE:** `prayertimes.h` does **not** handle Daylight Saving Time. The
> `timezone` argument is a fixed numeric UTC offset in hours, and the library
> uses it exactly as given — it has no notion of dates, zones, or DST rules.
> This is deliberate: DST is a political rule, not an astronomical one, and
> keeping it out leaves `prayertimes.h` a pure, dependency-free (only `<math.h>`)
> single header. **For a DST-active date you must pass the DST-adjusted offset**
> (e.g. `1.0` for London in summer, `0.0` in winter).

If you want libmuslim to compute the correct offset for you, use the optional
companion header [`timezone.h`](timezone.h). It resolves an IANA zone name and
date to a UTC offset with DST applied, using the host operating system's
timezone database:

```c
#define MUSLIM_TIMEZONE_IMPLEMENTATION   // in exactly ONE translation unit
#include "timezone.h"
#include "prayertimes.h"

char zone[64];
get_system_timezone(zone, sizeof(zone));            // e.g. "Europe/London"
double tz = parse_timezone_offset(zone, time(NULL)); // DST already applied

const MethodParams *mwl = method_params_get(CALC_MWL);
struct PrayerTimes times = calculate_prayer_times(2026, 7, 15, 51.5074, -0.1278, tz, mwl);
```

Unlike `prayertimes.h`, `timezone.h` touches the OS (POSIX `tzset`/`tm_gmtoff`
or the Win32 timezone APIs), so it is **optional** — include it only if you
want this resolution done for you. On a platform without a timezone database,
keep supplying the offset yourself.

### CLI Tool

```bash
./libmuslim <year> <month> <day> <latitude> <longitude> <timezone>
```

**Example output:**
```
Fajr    = 04:05
Sunrise = 05:22
Dhuha   = 05:50
Dhuhr   = 11:41
Asr     = 15:04
Maghrib = 17:54
Isha    = 19:07
```

## Verification

The calculations have been verified against official data sources and match within ±1-2 minute accuracy depending on the method. See the worked examples in `docs/KEMENAG_METHOD.md` for detailed verification.

## License

```
Copyright 2025 Rizki Rakasiwi.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```

## Documentation

- [Kemenag Calculation Method](docs/KEMENAG_METHOD.md) - Detailed mathematical documentation (Indonesian)
- [Medium - Understanding Islamic Prayer Time Calculations: The Mathematics Behind LibMuslim Library](https://medium.com/@rizkirakasiwi09/understanding-islamic-prayer-time-calculations-the-mathematics-behind-libmuslim-library-ee169e3e97c3)

## Contributing

Contributions are welcome! Please ensure any changes to calculation methods are verified against official data sources.

# TZif fixtures — the POSIX TZ footer path

`parse_timezone_offset` resolves a zone by binary-searching the TZif transition
table and only falls back to the file's trailing POSIX TZ string ("the footer")
when the instant lies at or past the last explicit transition.

A *fat* tzdata build — what this repository's development machines and most
desktop distributions ship — writes explicit transitions out to the year 2499,
so no DST zone ever reaches the footer for a present-day instant. A *slim*
build (Debian, Ubuntu and Alpine among others) writes only the transitions the
footer cannot predict, so ordinary present-day lookups land in the footer
evaluator. Without these fixtures the footer's DST-rule evaluation — the most
intricate code in the reader — is exercised only by fixed-offset footers such
as `WIB-7`, which carry no rules at all.

Each fixture below has exactly one explicit transition, in 1950, and hands
every later instant to its footer. `tests/test_timezone.c`
(`test_footer_fixtures`) points `TZDIR` here and asserts the offsets.

## Regenerating

From the repository root:

```sh
zic -b slim -d tests/fixtures/zoneinfo tests/fixtures/zoneinfo/fixtures.zi
```

`-b slim` is what produces the truncated transition table; the default
(`-b fat`) would bury the footer under transitions to 2499 and defeat the whole
point of these files.

The committed files were generated with **zic (tzcode) 2026c-dirty**, i.e. tzdb
release **2026c**. The source `fixtures.zi` is self-contained — it defines its
own `Rule` and `Zone` lines and depends on nothing from the system tzdata — so
a different zic release should reproduce byte-identical output unless the TZif
format itself changes.

## The zones

| File | Footer | Offsets asserted |
| --- | --- | --- |
| `Fixture/Fixed` | `FIX-7` | +7 year-round (no rules in the footer) |
| `Fixture/North` | `FST5FDT,M3.2.0,M11.1.0` | −5 winter, −4 summer, both sides of both transitions |
| `Fixture/South` | `GST-10GDT,M10.1.0,M4.1.0/3` | +11 January, +10 July — a DST window that wraps the year end |

The names are deliberately fictional, so they can never collide with a real
zone and a lookup that failed to see `TZDIR` fails loudly instead of quietly
answering from the system tzdb.

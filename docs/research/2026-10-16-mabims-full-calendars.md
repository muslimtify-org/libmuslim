# Full calendars for Malaysia, Singapore and Brunei

**Date:** 2026-10-16

Retrieval date for every source below: **2026-10-16**.

[2026-10-04-mabims-states-fixtures.md](2026-10-04-mabims-states-fixtures.md) checked the MABIMS 2021 predicate against seven announced month starts per country, Ramadan, Syawal and Zulhijjah only. This note records the full published calendars obtained since, how each was read, how each was checked, and what the library does against them.

## Sources

| Country | Source | Coverage | Month starts |
|---|---|---|---|
| Malaysia | JAKIM e-solat service, zone WLY01 | every day of 2024, 2025 and 2026 | 37, 2024-01-13 to 2026-12-11 |
| Singapore | MUIS "Key Islamic Dates" sheets | 2024 and 2025 | 25, 2024-01-13 to 2025-12-22 |
| Brunei | Ministry of Religious Affairs Taqwim, published by Pusat Da'wah Islamiah | 2025 and 2026 | 25, 2025-01-01 to 2026-12-11 |

**JAKIM.** `https://www.e-solat.gov.my/index.php?r=esolatApi/takwimsolat&period=duration&zone=WLY01`, posted with `datestart` and `dateend` for each year. The response holds one record per day with a `hijri` field as year, month and day. The `period=year` form ignores a `year` parameter and returns the current year only. The month names in the `date` field are Malay abbreviations, such as `Mac` and `Ogos`.

**MUIS.** `https://muis.gov.sg/-/media/Files/Corporate-Site/Key-Islamic-Dates-2024.ashx` and `...-2025.ashx`, one-page text PDFs. Each lists 1 of every Hijri month with its Gregorian date and weekday. No 2026 sheet exists at the same address. The annual `Muis-Calendar-<year>.ashx` PDFs, several hundred pages each, were downloaded but not needed.

**Brunei.** `https://www.mora.gov.bn/Muat Turun Library/PDI/TAQWIM 2025.pdf` and `.../TAQWIM 2026 ©PDI2025.pdf`. The 2024 Taqwim read for the 2026-09-25 note is no longer at any address tried.

## How the Taqwim was read

Each Taqwim has twelve monthly pages, a table of Gregorian day, Malay weekday, Hijri day and prayer times. `pdftotext -layout` drops most Hijri day numbers. They are present in the text layer, but printed in a symbol font whose digits are the private-use code points U+F030 to U+F039. Reading word coordinates with `pdftotext -tsv`, mapping those code points to the digits 0 to 9, and pairing each weekday word with the number to its left and the number to its right recovers a Hijri day for every one of the 730 days. The extractor is reproduced at the end of this note. The JAKIM and MUIS sources needed no such step: the first is JSON and the second a text PDF read with `pdftotext -layout`.

## Checks on each source

- **JAKIM.** All 1096 days present, consecutive, each Hijri day one more than the day before or 1 after 29 or 30. The seven declarations admitted in `2026-09-25-mabims-takwim-sources.md` all appear.
- **MUIS.** Every listed weekday matches its Gregorian date. The seven announcements admitted before all appear, apart from 2026, which the sheets do not cover.
- **Brunei.** Every weekday on every monthly page matches the real calendar, every month is 29 or 30 days, and the month that crosses from 2025 into 2026 agrees between the two documents. The two Taqwim also print 61 distinct dated lines, such as holidays, in the form "Hari Isnin, 1 Syawal 1446H bersamaan 31 Mac 2025M". 60 agree with the extracted calendar. The exception is "14 Ramadan 1447H bersamaan 3 Mac 2026M" for a lunar eclipse noted as "Peristiwa terjadi selepas waktu Maghrib", after Maghrib, when the Hijri date has already turned. The extracted calendar gives 13 Ramadan for the daytime of 3 March.

## The three agree

Every one of the 37 months is covered by at least two of the sources and 13 by all three. On every one, all the sources that cover it give the same start. The three are coordinated through MABIMS, so this agreement shows a single regional calendar published three times. It is not three independent derivations.

The 37 starts are: 2024-01-13, 2024-02-11, 2024-03-12, 2024-04-10, 2024-05-10, 2024-06-08, 2024-07-07, 2024-08-06, 2024-09-05, 2024-10-04, 2024-11-03, 2024-12-03, 2025-01-01, 2025-01-31, 2025-03-02, 2025-03-31, 2025-04-29, 2025-05-29, 2025-06-27, 2025-07-26, 2025-08-25, 2025-09-23, 2025-10-23, 2025-11-22, 2025-12-22, 2026-01-20, 2026-02-19, 2026-03-21, 2026-04-19, 2026-05-18, 2026-06-17, 2026-07-16, 2026-08-14, 2026-09-13, 2026-10-12, 2026-11-11, 2026-12-11. The first is 1 Rajab 1445 H.

## Measurement

`HIJRI_PREDICATE_MABIMS_2021` through `hijri_evaluate_evening`, on the evening before each of the 37 starts and the evening before that:

| Site | Supported | One day early | Misses |
|---|---|---|---|
| Kuala Lumpur | 37 | 0 | none |
| Langkawi | 37 | 0 | none |
| Singapore | 37 | 0 | none |
| Jakarta | 37 | 0 | none |
| Sabang | 37 | 1 | early on 2025-05-29 |
| Bandar Seri Begawan | 35 | 0 | 2024-10-04, altitude 3.89, elongation 6.284. 2025-09-23, altitude 3.43, elongation 6.323 |

Against Brunei's own 25 Taqwim starts, Bandar Seri Begawan supports 24, missing 2025-09-23. Both misses fail on elongation, 0.08 and 0.12 degrees under 6.4, far outside the 0.0070 degree error bar `hijri.h` documents.

Through the registry, day by day from 2024-01-13 to 2026-12-10, 1063 days, before this change:

| Entry | Reference point | Days wrong |
|---|---|---|
| JAKIM | Kuala Lumpur | 0 |
| MUIS | Singapore | 0 |
| Brunei ministry | Bandar Seri Begawan | 60 |

The 60 are the two months Bandar Seri Begawan misses, each one day late for a whole month. With the entry moved to Kuala Lumpur, as decided below, it is wrong on 0.

## Decision

Brunei's Taqwim states that it follows the regional MABIMS calendar and does not say where it is computed. The registry entry's reference point moves to Kuala Lumpur, which reproduces the calendar, and its comment says the point was chosen to match the published calendar. Kuala Lumpur was preferred over Singapore as the further west of the two. With that, all three entries reproduce their published calendars on every day of the record.

## Limits

- The record is 2024 to 2026 for Malaysia, 2024 and 2025 for Singapore, and 2025 and 2026 for Brunei, plus the announced starts admitted earlier. Nothing here says anything about other years.
- The starts of 2026-11-11 and 2026-12-11 were still predictions when retrieved. JAKIM's e-solat values for past dates match every declaration checked, but a later declaration could differ from a published prediction.
- JAKIM was read for zone WLY01 only. The Hijri date is national, and no other zone was compared.
- Agreement between the three is agreement with one regional calendar, as said above.

## Reproducing the Taqwim extraction

Run with the TSV from `pdftotext -tsv <Taqwim PDF> out.tsv` and the Gregorian year. It prints any weekday or day-set problem, the number of days without a Hijri number, and each implied month start with its length.

```python
"""Month starts from a Brunei Taqwim, read from `pdftotext -tsv` output.

Usage: python3 -I taqwim_starts.py <taqwim.tsv> <gregorian year>
"""
import collections
import csv
import datetime as dt
import sys

DAYS = ['Isnin', 'Selasa', 'Rabu', 'Khamis', 'Jumaat', 'Sabtu', 'Ahad']  # Monday first
# The Hijri day numbers are printed in a symbol font whose digits are U+F030..U+F039.
SYMBOL_DIGITS = {0xF030 + i: ord('0') + i for i in range(10)}


def load(path):
    pages = collections.defaultdict(list)
    for r in csv.DictReader(open(path, encoding='utf-8'), delimiter='\t'):
        if r['level'] == '5':
            r['text'] = r['text'].translate(SYMBOL_DIGITS)
            r['x'], r['y'], r['h'] = float(r['left']), float(r['top']), float(r['height'])
            pages[int(r['page_num'])].append(r)
    return pages


def rows_of(words):
    """Gregorian day -> (weekday word, Hijri day or None) for one page."""
    out = {}
    for w in words:
        if w['text'] not in DAYS:
            continue
        cy = w['y'] + w['h'] / 2
        same = [v for v in words if abs(v['y'] + v['h'] / 2 - cy) < 6]
        left = [v for v in same if v['text'].isdigit() and w['x'] - 70 < v['x'] < w['x']]
        if not left:
            continue
        times = [v['x'] for v in same if '.' in v['text'] and v['x'] > w['x']]
        limit = min(times) if times else w['x'] + 120
        right = [v for v in words if v['text'].isdigit() and w['x'] < v['x'] < limit
                 and abs(v['y'] + v['h'] / 2 - cy) < 10]
        gday = int(max(left, key=lambda v: v['x'])['text'])  # nearest the weekday
        out[gday] = (w['text'], int(right[0]['text']) if right else None)
    return out


def main():
    year = int(sys.argv[2])
    pages = load(sys.argv[1])
    month, implied, problems, missing = 0, collections.Counter(), [], []
    for p in sorted(pages):
        rows = rows_of(pages[p])
        if len(rows) < 28:
            continue
        month += 1
        ndays = (dt.date(year + (month == 12), month % 12 + 1, 1) - dt.date(year, month, 1)).days
        if sorted(rows) != list(range(1, ndays + 1)):
            problems.append((p, month, 'day set'))
            continue
        for g, (weekday, h) in rows.items():
            d = dt.date(year, month, g)
            if DAYS[d.weekday()] != weekday:
                problems.append((p, month, g, 'weekday', weekday))
            if h is None:
                missing.append(d)
            else:
                implied[d - dt.timedelta(days=h - 1)] += 1
    print('year', year, 'month pages', month, 'problems', problems,
          'days without a Hijri number', len(missing))
    starts = sorted(implied)
    for i, s in enumerate(starts):
        length = (starts[i + 1] - s).days if i + 1 < len(starts) else '-'
        print(s, 'days read', implied[s], 'length', length)


main()
```

# KHGT calendar

**Date:** 2026-11-03

This note records where `HIJRI_AUTHORITY_MUHAMMADIYAH_KHGT` in `hijri.h` v0.6.0 gets its calendar and how well the stated rule reproduces it. `docs/research/2026-09-07-khgt-sources.md` stays as the dated record of what was findable before the calendar was checked.

## Sources

Retrieval date for every source: 2026-11-03, except the Pedoman book, which was read on 2026-11-02.

- Yearly PDF calendars, `https://khgt.muhammadiyah.or.id/calendar-download?year=Y&mode=hijriah` for Y from 1447 to 1492, 46 files.
- Month pages, `https://khgt.muhammadiyah.or.id/kalendar-hijriah?year=Y&mode=hijriah` for the same years.
- Pedoman book, `https://tarjih.or.id/wp-content/uploads/2025/06/KHGT_Indonesia.pdf`.

## The fixture

The PDFs give 552 months, consecutive from Muharam 1447 to Zulhijah 1492, starting 2025-06-26. The last month has 30 days, so the table ends on 2070-02-11. Over the 551 gaps between starts, 293 are 30 days and 258 are 29.

A month's start is its first date cell. Its length is its count of date cells, and that count equals the gap to the next start in every month that has a next start.

### The correction to Zulhijah 1492

Both sources print Zulhijah 1492 with the year 1970: "Januari 1970 - Februari 1970". The start is taken as 2070-01-13, because Zulkaidah 1492 starts on 2069-12-15 and has 29 date cells.

The weekday column does not confirm this. It places 13 Jan under Selasa, which is the weekday of 1970-01-13, and 2070-01-13 is a Monday. In all the other 551 months the column agrees with the resolved date. So the column follows the misprinted year and is no evidence either way. The correction rests on the 29 days alone. The last month's length is the same under either year, because January and February 1970 and 2070 have the same number of days.

### Reading the data twice

Parsing the PDFs and parsing the month pages gave the same start and the same length in all 552 months once the correction is applied, with one difference: the month page's raw start for Zulhijah 1492 is 1970-01-13, the misprint both share. Both come from Muhammadiyah, so this guards against parse errors only. It is not an independent check of the calendar.

## The rule

The Pedoman book states the rule in section 3 (b, c), and the site repeats it as its parameters 2 to 4. The month starts when elongation is at least 8 degrees and altitude at sunset is at least 5 degrees, anywhere before 24:00 GMT. Footnote 31 makes both geocentric: "Elongasi dan ketinggian hilal geosentris". After that the month still starts when "kawasan zona waktu ujung timur telah mengalami ijtimak sebelum fajar, dan parameter 5⁰ + 8⁰ telah mencapai daratan benua Amerika".

The library reads this with the search `docs/research/2026-10-25-diyanet-global-rule.md` describes. The predicate is `HIJRI_PREDICATE_GEOCENTRIC_ALTITUDE_5_ELONGATION_8`. The only difference from Diyanet's entry is the altitude, which has no parallax applied.

## The score

A start counts as reproduced when `hijri__kind_global_rule` converts it to day 1 of its Hijri month and the day before to a different month. `test_khgt_published_calendar` pins the count.

The rule reproduces 496 of the 552. It starts 51 months a day late and 5 a day early.

The misses through 2035:

| Published start | Hijri month | Rule |
|---|---|---|
| 2028-11-17 | 1450-07 | late |
| 2029-05-15 | 1451-01 | early |
| 2029-12-06 | 1451-08 | late |
| 2030-05-04 | 1452-01 | early |
| 2030-07-31 | 1452-04 | late |
| 2032-05-10 | 1454-02 | late |
| 2035-10-02 | 1457-08 | late |

Every late miss follows a 29-day published month and begins a 30-day one, and every early miss follows a 30-day month and begins a 29-day one. That is the shape any isolated one-day shift has, so it does not explain them. Why the calendar departs from its stated rule on about one month in ten is not known. The Istanbul committee's working paper was not read and might say.

Variants measured with a scratch prototype on 2026-11-02, against the first 551 starts, and not rerun:

| Variant | Score of 551 |
|---|---|
| As stated | 495 |
| Muhammadiyah sunset convention | 495 |
| Topocentric altitude | 489 |
| Americas clause without the dawn condition | 489 |
| No Americas clause | 449 |
| Anywhere to the date line | 522 |
| Land including islands | 526 to 531 |

The last two score higher and were not shipped, because neither is what the book states and the 531 needed a tuned latitude limit. The topocentric predicate through the real converter over all 552 rows scores 490, as the recorded mutation in `tests/test_hijri.c` shows.

## The table edges

Output for 3 days on each side of both edges. `entry` is `hijri_authority_from_gregorian`, and `rule` is `hijri__kind_global_rule` alone.

| Date | entry | Status | rule |
|---|---|---|---|
| 2025-06-23 | 1446-12-27 | OUTSIDE | 1446-12-27 |
| 2025-06-24 | 1446-12-28 | OUTSIDE | 1446-12-28 |
| 2025-06-25 | 1446-12-29 | OUTSIDE | 1446-12-29 |
| 2025-06-26 | 1447-01-01 | IN | 1447-01-01 |
| 2025-06-27 | 1447-01-02 | IN | 1447-01-02 |
| 2025-06-28 | 1447-01-03 | IN | 1447-01-03 |
| 2070-02-09 | 1492-12-28 | IN | 1492-12-28 |
| 2070-02-10 | 1492-12-29 | IN | 1492-12-29 |
| 2070-02-11 | 1492-12-30 | IN | 1492-12-30 |
| 2070-02-12 | 1493-01-01 | IN | 1493-01-01 |
| 2070-02-13 | 1493-01-02 | IN | 1493-01-02 |
| 2070-02-14 | 1493-01-03 | IN | 1493-01-03 |

No seam exists at either edge. The rule starts Muharam 1447 and Muharam 1493 on the same days the table does. Any later month past an edge can still land a day from what Muhammadiyah would publish, at the rate the score gives.

## The parser

Run as `python3 -I parse_khgt.py PDF_DIR HTML_DIR OUT_DIR` with `pdftotext` on the path.

```python
import datetime as D
import re
import subprocess
import sys

HM = ["Muharam", "Safar", "Rabiulawal", "Rabiulakhir", "Jumadilawal",
      "Jumadilakhir", "Rajab", "Syakban", "Ramadan", "Syawal", "Zulkaidah",
      "Zulhijah"]
GM = {"Jan": 1, "Feb": 2, "Mar": 3, "Apr": 4, "Mei": 5, "Jun": 6, "Jul": 7,
      "Agu": 8, "Agt": 8, "Ags": 8, "Sep": 9, "Okt": 10, "Nov": 11, "Des": 12}
FULL = {"Januari": 1, "Februari": 2, "Maret": 3, "April": 4, "Mei": 5,
        "Juni": 6, "Juli": 7, "Agustus": 8, "September": 9, "Oktober": 10,
        "November": 11, "Desember": 12}
DAYS = ["Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu", "Ahad"]
TOKEN = re.compile(r"\b(\d{1,2}) (" + "|".join(GM) + r")\b")
PDF_HEAD = re.compile(
    r"^\s*(\w+)\s+(\d{4})\s+\((\w+)\s+(\d{4})(?:\s*-\s*(\w+)\s+(\d{4}))?\)")


def from_pdf(path, year):
    """Rows of (hijri_month, header_gy, header_gm, day, gm, length, wd_or_None)."""
    text = subprocess.run(["pdftotext", "-layout", path, "-"], check=True,
                          capture_output=True, text=True).stdout
    rows, cur, header_cols = [], None, None
    for line in text.splitlines():
        m = PDF_HEAD.match(line)
        if m and m.group(1) in HM and int(m.group(2)) == year:
            cur = {"month": HM.index(m.group(1)) + 1,
                   "gy": int(m.group(4)), "gm": FULL[m.group(3)],
                   "tokens": [], "weekday": None, "open": True}
            rows.append(cur)
            header_cols = None
            continue
        if cur is None or not cur["open"]:
            continue
        if line.strip().startswith("Hari Istimewa"):
            cur["open"] = False
            continue
        if "Ahad" in line and "Sabtu" in line:
            header_cols = {w: line.index(w) for w in DAYS}
            continue
        for t in TOKEN.finditer(line):
            if not cur["tokens"] and header_cols:
                # The date token starts its column near the weekday's name.
                centers = {w: abs((header_cols[w] + len(w) / 2) -
                                  (t.start() + 3)) for w in DAYS}
                cur["weekday"] = min(centers, key=centers.get)
            cur["tokens"].append((int(t.group(1)), t.group(2)))
    return rows


def from_html(path):
    """Rows of (hijri_year, hijri_month, header text, tokens)."""
    raw = open(path, encoding="utf8").read()
    raw = re.sub(r"<script.*?</script>|<style.*?</style>", "", raw, flags=re.S)
    raw = re.sub(r"<[^>]+>", " ", raw)
    raw = re.sub(r"\s+", " ", raw)
    head = re.compile(r"(" + "|".join(HM) + r") (\d{4}) H ([A-Za-z]+) (\d{4})"
                      r"(?: - ([A-Za-z]+) (\d{4}))? Hari Istimewa")
    rows = []
    for m in head.finditer(raw):
        end = raw.find("Hari Istimewa", m.end())
        rows.append({"month": HM.index(m.group(1)) + 1, "year": int(m.group(2)),
                     "gy": int(m.group(4)), "gm": FULL[m.group(3)],
                     "tokens": [(int(a), b) for a, b in
                                TOKEN.findall(raw[m.end():end])]})
    return rows


def resolve(r):
    day, name = r["tokens"][0]
    return D.date(r["gy"], GM[name], day)


def main():
    pdf_dir, html_dir, out = sys.argv[1:4]
    pdf, html, notes = {}, {}, []
    for y in range(1447, 1493):
        for r in from_pdf(f"{pdf_dir}/{y}.pdf", y):
            pdf[(y, r["month"])] = r
        for r in from_html(f"{html_dir}/{y}.html"):
            if r["year"] == y:
                html[(y, r["month"])] = r
    print("pdf months", len(pdf), "html months", len(html))

    starts, lens = [], {}
    for key in sorted(pdf):
        r = pdf[key]
        d = resolve(r)
        if d.year == 1970:
            notes.append(f"{key} PDF prints year 1970: {d} weekday={r['weekday']}")
        starts.append((key, d, r))
        lens[key] = len(r["tokens"])

    # Misprints: a start before its predecessor is resolved from the gap.
    fixed = []
    for i, (key, d, r) in enumerate(starts):
        if fixed and not 25 <= (d - fixed[-1][1]).days <= 35:
            prev_key, prev_d = fixed[-1]
            nd = prev_d + D.timedelta(days=lens[prev_key])
            notes.append(f"{key} printed start {d} corrected to {nd} from "
                         f"{prev_key} at {prev_d} plus {lens[prev_key]} days")
            d = nd
        fixed.append((key, d))

    bad_wd = []
    for (key, d), (_, _, r) in zip(fixed, starts):
        want = DAYS[d.weekday()]
        if r["weekday"] != want:
            bad_wd.append((key, d, r["weekday"], want))
    print("weekday mismatches", len(bad_wd), bad_wd[:5])
    print("zulhijah 1492", [(k, d, starts[-1][2]["weekday"]) for k, d in fixed[-1:]],
          "first token", starts[-1][2]["tokens"][0], "last", starts[-1][2]["tokens"][-1])

    for key, d in fixed:
        h = html.get(key)
        if h is None:
            print("HTML MISSING", key)
            continue
        hd = D.date(h["gy"], GM[h["tokens"][0][1]], h["tokens"][0][0])
        if hd != d:
            print("PDF/HTML START DIFF", key, "pdf", d, "html", hd, "html header",
                  h["gm"], h["gy"])
        if len(h["tokens"]) != lens[key]:
            print("PDF/HTML LEN DIFF", key, "pdf", lens[key], "html", len(h["tokens"]))

    bad_gap = 0
    for i in range(1, len(fixed)):
        (ky, kd), (py, pd) = fixed[i], fixed[i - 1]
        gap = (kd - pd).days
        if gap not in (29, 30) or gap != lens[py]:
            bad_gap += 1
            print("GAP/LEN", py, pd, ky, kd, "gap", gap, "len", lens[py])
    print("bad gaps", bad_gap)
    consecutive = [k for k, _ in fixed] == [(y, m) for y in range(1447, 1493)
                                            for m in range(1, 13)]
    print("consecutive 1447-01..1492-12", consecutive, "count", len(fixed))
    last_key, last_d = fixed[-1]
    print("last month len", lens[last_key],
          "first after table", last_d + D.timedelta(days=lens[last_key]))
    gaps = [(fixed[i][1] - fixed[i - 1][1]).days for i in range(1, len(fixed))]
    print("gaps 30:", gaps.count(30), "29:", gaps.count(29))
    for n in notes:
        print("NOTE", n)

    with open(f"{out}/starts.tsv", "w") as f:
        for (y, m), d in fixed:
            f.write(f"{d.year}\t{d.month}\t{d.day}\t{y}\t{m}\t{lens[(y, m)]}\n")

    words, total = [], 0
    for y in range(1447, 1493):
        w = 0
        for m in range(1, 13):
            total += lens[(y, m)]
            if lens[(y, m)] == 30:
                w |= 1 << (12 - m)
        words.append(w)
    anchor = fixed[0][1]
    jd = anchor.toordinal() + 1721424.5
    with open(f"{out}/table.txt", "w") as f:
        f.write(f"years {len(words)} anchor {anchor} floor_jd {int(jd // 1)} "
                f"total_days {total}\n")
        for i in range(0, len(words), 8):
            f.write("    " + " ".join(f"0x{w:04X}," for w in words[i:i + 8]) + "\n")
        f.write("FIXTURE\n")
        line = "   "
        for (y, m), d in fixed:
            item = f" {{{d.year}, {d.month}, {d.day}, {y}, {m}}},"
            if len(line) + len(item) > 78:
                f.write(line + "\n")
                line = "   "
            line += item
        f.write(line + "\n")
    print(open(f"{out}/table.txt").read().split("FIXTURE")[0])


main()
```

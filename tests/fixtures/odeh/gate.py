#!/usr/bin/env python3
"""Gate Odeh Table VI's scratch transcription and report what fails.

Runs a set of checks over the scratch CSV `extract.py` produces: column
alphabet validation, five astronomical relations recorded as holding on
page 6 in `docs/research/2026-08-22-odeh-table-vi-ocr-fidelity.md`, the
paper's own stated extremes for lag and elongation, and a completeness
check that counts each page's physical rows independently of the CSV.

This script fixes nothing. Tesseract's known error modes, `V` rendered as
a `\\Y` family or as `Vv`/`vV`/etc, and a leading minus in Long read as a
tilde, are reported by class so a human can adjudicate them against the
page images, not silently normalised here. Applying an unverified mapping
here would bake a guess into the fixture and destroy the evidence the
adjudication step needs.

The completeness check exists because the CSV's own row count cannot
prove itself complete: `extract.py` finds a row by matching a dd-mm-yyyy
Date cell, so a row whose date fails to transcribe is invisible to it,
and nothing before this check ever compared the count against the page
image. Instead this counts each page's physical rows by clustering the
No. column's tokens into row bands, which survives a damaged Date cell
because the row still carries a record number (or, failing that, ink) in
that column. It reads `SCRATCH_CSV`'s sibling `page-images` directory,
the same layout `extract.py` writes, and is skipped with a note if that
directory is absent.

Exit 0 means the gate ran to completion, not that every row passed. The
report lists every failure; driving that list to empty is later work.

Usage:
    gate.py SCRATCH_CSV

Writes SCRATCH_CSV's sibling `gate_report.txt` and prints pass/fail
counts to stdout.
"""

import csv
import math
import re
import sys
from collections import Counter, defaultdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import extract as _extract

R_ALPHABET = set("ABCDFI")
E_ALPHABET = set("ME")
MARK_ALPHABET = {"", "I", "V"}

NUMERIC_COLUMNS = [
    "No.", "Long", "Lat", "Ele", "JD", "Age", "Lag", "ARCV", "DAZ", "ARCL",
    "W", "V",
]

# Systematic OCR error classes, verified against page 8 record 389's pixels
# (see docs/research/2026-08-22-odeh-table-vi-ocr-fidelity.md) except where
# marked unverified.
V_MISREAD_FAMILY = {
    "\\Y", "\\Y,", "\\Y;", "\\Y4", "\\V/", "Vv", "vV", "VvV", "v", "V.",
}
MARK_T1_UNVERIFIED = {"T", "1"}
R_MANGLED_C = {"\u00a2", "C\u20ac", "\u20ac", "1T"}
R_T1_UNVERIFIED = {"T", "1"}

# Relation 2's local solar time bands, observed on page 6 (2026-08-22
# research). Widen only if adjudication confirms a legitimate reason.
#
# Measured over all 575 rows (2026-09-05, task 2b): 71 rows fail this
# band. Most of them form a continuous spread that tracks latitude and
# season, e.g. high-latitude June evenings run to 21.53h and December
# evenings at similar latitudes run down to 16.52h, with no gap bigger
# than 0.36h across that range, which is what a page-6-only band would
# miss since page 6 (group D) is one narrow slice of the table. Two rows
# (No. matching CSV rows 306 and 388, both E) sit apart from that spread
# by more than 9 hours with nothing between, which looks like damage
# rather than natural variation.
#
# Left unwidened anyway: any new bound we could state would just be the
# min and max of the 69 rows we believe are genuine, i.e. fitted exactly
# to exclude the two we believe are damaged. That is the gate trusting
# its own output the brief warns against, unlike the V tolerance below
# which comes from an independent physical derivation. Task 3 adjudicates
# all 71 by hand instead.
EVENING_BAND = (17.13, 19.65)
MORNING_BAND = (4.95, 6.63)

# The bands above are the research doc's own values, printed to 2 decimal
# places. This tolerance absorbs that rounding, not genuine spread: the
# page 6 row that sets the 19.65 upper bound recomputes to 19.652 from its
# own printed JD, and without this it would fail against its own bound.
TIME_BAND_TOLERANCE = 0.01

# Column resolution is one decimal place, so the rounding envelope on
# ARCV, DAZ and ARCL is half of that.
ARC_ROUNDING = 0.05

# Relation 6, the V checksum. hijri_odeh_v (hijri.h:2137) takes arc minutes;
# Table VI's W column is arc seconds.
#
# The tolerance is the sum of three independent rounding contributions,
# not the cubic term alone:
#   - W is printed to a half arc second, and the cubic's derivative
#     against W is about 0.105 per arc second there, giving 0.053.
#   - ARCV is printed to one decimal place, a half-unit rounding of 0.05.
#   - V itself is printed to two decimal places, a half-unit rounding of
#     0.005.
#   0.053 + 0.05 + 0.005 = 0.108, rounded up to 0.11.
#
# Measured over the 532 of 575 rows whose ARCV, W and V all parse
# (2026-09-05, task 2b): worst residual 44.1479, p99 0.0901, p95 0.0751,
# median 0.0288. Exactly 3 rows fail at a tolerance of 0.10, and the same
# 3 still fail at 0.50, so this bound and 0.109 pick the same 3 rows:
# rounding noise and real damage are separated by an empty gap from about
# 0.094 to 0.5. Page 6 falls to zero failures at this tolerance, matching
# the research doc's 39 of 39.
V_TOLERANCE = 0.11

# Section 7.2 and 7.3's stated extremes.
LAG_MIN_OPTICAL = 21
LAG_MIN_NAKED_EYE = 29
ARCL_MIN_OPTICAL = 6.4
ARCL_MIN_NAKED_EYE = 7.7

# Both the table and the paper's stated extremes are rounded, so a bound
# stated as an exact equality is stricter than the paper's own numbers
# support.
#
# Table VI's Lag column prints a whole number of minutes, a half-unit
# rounding envelope of 0.5 minutes. Section 7.2's own quoted minimums (21
# and 29) are also whole minutes, contributing another 0.5 minute
# envelope. A printed Lag one minute under the stated minimum can
# therefore still represent a true Lag that satisfies it, e.g. a printed
# 20 stands for a true value down to 19.5, and a quoted minimum of 21
# stands for a true value as low as 20.5, so the two can agree at 20.5.
# 0.5 + 0.5 = 1 minute.
LAG_ROUNDING_TOLERANCE = 1

# Table VI's ARCL column prints one decimal place, a half-unit rounding
# envelope of 0.05 degrees (see ARC_ROUNDING above, which is the same
# quantity). Section 7.3's own quoted minimums (6.4 and 7.7) are also
# printed to one decimal, contributing another 0.05 degree envelope. A
# printed 7.6 stands for a true value from 7.55 to 7.65, and the quoted
# 7.7 stands for a true value from 7.65 to 7.75, so the two can agree at
# 7.65. 0.05 + 0.05 = 0.1 degree.
#
# Section 7.3's figures are also quoted at the time of last or first
# visibility, while Table VI's ARCL column is at best time, a different
# instant in the same sighting. That is a second, independent reason the
# two need not match exactly even with perfect transcription, on top of
# the rounding envelope computed above.
ARCL_ROUNDING_TOLERANCE = 0.1


def classify_mark_illegal(value, extra_unverified):
    """Bucket an illegal N/B/T-style mark into a named systematic class."""
    if value in V_MISREAD_FAMILY:
        return "V misread as \\Y/Vv/vV family (verified against pixels)"
    if value in extra_unverified:
        return "T or 1 in place of a mark (unverified)"
    return "unclassified illegal value"


def classify_r_illegal(value):
    if value in R_MANGLED_C:
        return "C misread as currency glyph (unverified)"
    if value in R_T1_UNVERIFIED:
        return "T or 1 in place of a letter (unverified)"
    return "unclassified illegal value"


def to_float(value):
    try:
        return float(value)
    except (TypeError, ValueError):
        return None


def jd_to_ymd(jd):
    """Convert a Julian Date to a proleptic Gregorian (year, month, day),
    day as a float carrying the UT time-of-day fraction. Meeus's algorithm."""
    jd = jd + 0.5
    z = int(jd)
    f = jd - z
    if z < 2299161:
        a = z
    else:
        alpha = int((z - 1867216.25) / 36524.25)
        a = z + 1 + alpha - int(alpha / 4)
    b = a + 1524
    c = int((b - 122.1) / 365.25)
    d = int(365.25 * c)
    e = int((b - d) / 30.6001)
    day = b - d - int(30.6001 * e) + f
    month = e - 1 if e < 14 else e - 13
    year = c - 4716 if month > 2 else c - 4715
    return year, month, day


def local_date_from_jd(jd, longitude_deg):
    adjusted_jd = jd + longitude_deg / (15.0 * 24.0)
    year, month, day = jd_to_ymd(adjusted_jd)
    return year, month, int(day)


def local_solar_hours(jd, longitude_deg):
    frac = (jd + 0.5) % 1.0
    ut_hours = frac * 24.0
    return (ut_hours + longitude_deg / 15.0) % 24.0


def cos_deg(x):
    return math.cos(math.radians(x))


def relation4_holds(arcv, daz, arcl):
    lhs_lo = cos_deg(arcl + ARC_ROUNDING)
    lhs_hi = cos_deg(arcl - ARC_ROUNDING)
    rhs_lo = cos_deg(arcv + ARC_ROUNDING) * cos_deg(daz + ARC_ROUNDING)
    rhs_hi = cos_deg(arcv - ARC_ROUNDING) * cos_deg(daz - ARC_ROUNDING)
    epsilon = 1e-9
    return max(lhs_lo, rhs_lo) <= min(lhs_hi, rhs_hi) + epsilon


def odeh_v(arcv_deg, w_arcmin):
    w = w_arcmin
    return arcv_deg - (
        -0.1018 * w ** 3 + 0.7319 * w ** 2 - 6.3226 * w + 7.1651
    )


def check_row(row):
    """Return a list of (check_name, detail) failures for one CSV row."""
    failures = []
    r = row["R"].strip()
    e = row["E"].strip()

    if r not in R_ALPHABET:
        failures.append((
            "alphabet_R", classify_r_illegal(r), repr(row["R"]),
        ))
    if e not in E_ALPHABET:
        failures.append(("alphabet_E", "unclassified illegal value", repr(row["E"])))

    for col in ("N", "B", "T"):
        val = row[col].strip()
        if val not in MARK_ALPHABET:
            failures.append((
                f"alphabet_{col}", classify_mark_illegal(val, MARK_T1_UNVERIFIED),
                repr(row[col]),
            ))

    numeric = {}
    for col in NUMERIC_COLUMNS:
        v = to_float(row[col])
        if v is None:
            detail = "leading minus read as tilde" if row[col].strip().startswith("~") else "unparsable"
            failures.append((f"numeric_{col}", detail, repr(row[col])))
        else:
            numeric[col] = v

    # Relation 1: date against JD and longitude.
    if "JD" in numeric and "Long" in numeric:
        year, month, day = local_date_from_jd(numeric["JD"], numeric["Long"])
        printed = row["Date"].strip()
        try:
            dd, mm, yyyy = (int(p) for p in printed.split("-"))
        except ValueError:
            failures.append(("relation1_date_jd_long", "unparsable Date", repr(row["Date"])))
        else:
            if (year, month, day) != (yyyy, mm, dd):
                failures.append((
                    "relation1_date_jd_long",
                    f"JD+Long gives {year:04d}-{month:02d}-{day:02d}",
                    printed,
                ))

    # Relation 2: column E against local solar time band.
    if "JD" in numeric and "Long" in numeric and e in E_ALPHABET:
        hours = local_solar_hours(numeric["JD"], numeric["Long"])
        tol = TIME_BAND_TOLERANCE
        if e == "E" and not (EVENING_BAND[0] - tol <= hours <= EVENING_BAND[1] + tol):
            failures.append(("relation2_local_time_band", f"E row at {hours:.2f}h local", ""))
        if e == "M" and not (MORNING_BAND[0] - tol <= hours <= MORNING_BAND[1] + tol):
            failures.append(("relation2_local_time_band", f"M row at {hours:.2f}h local", ""))

    # Relation 3: column E against sign of Age.
    if "Age" in numeric and e in E_ALPHABET:
        age = numeric["Age"]
        if e == "E" and not age > 0:
            failures.append(("relation3_age_sign", f"E row has Age {age}", ""))
        if e == "M" and not age < 0:
            failures.append(("relation3_age_sign", f"M row has Age {age}", ""))

    # Relation 4: spherical closure.
    if all(c in numeric for c in ("ARCV", "DAZ", "ARCL")):
        if not relation4_holds(numeric["ARCV"], numeric["DAZ"], numeric["ARCL"]):
            failures.append((
                "relation4_spherical_closure",
                f"ARCV={numeric['ARCV']} DAZ={numeric['DAZ']} ARCL={numeric['ARCL']}",
                "",
            ))

    # Relation 6: the V checksum.
    if all(c in numeric for c in ("ARCV", "W", "V")):
        w_arcmin = numeric["W"] / 60.0
        computed = odeh_v(numeric["ARCV"], w_arcmin)
        residual = abs(computed - numeric["V"])
        if residual > V_TOLERANCE:
            failures.append((
                "relation6_v_checksum", f"residual {residual:.4f}", "",
            ))

    # Section 7.2/7.3 extremes apply to sightings, not attempts: N/B/T hold
    # `I` (invisible, tried and not seen), `V` (visible, seen) or blank (not
    # tried). Only a `V` outcome is a naked eye or optically aided sighting.
    naked_eye = row["N"].strip() == "V"
    optical_aid = row["B"].strip() == "V" or row["T"].strip() == "V"

    # A tiny epsilon absorbs binary floating point representation error in
    # the threshold subtraction below (e.g. 7.7 - 0.1 lands a hair above
    # 7.6 in binary), not any rounding envelope of the data itself.
    epsilon = 1e-9

    if "Lag" in numeric:
        lag = numeric["Lag"]
        if optical_aid and lag < LAG_MIN_OPTICAL - LAG_ROUNDING_TOLERANCE - epsilon:
            failures.append(("extreme_lag_optical", f"Lag={lag} < {LAG_MIN_OPTICAL}", ""))
        if naked_eye and lag < LAG_MIN_NAKED_EYE - LAG_ROUNDING_TOLERANCE - epsilon:
            failures.append(("extreme_lag_naked_eye", f"Lag={lag} < {LAG_MIN_NAKED_EYE}", ""))

    if "ARCL" in numeric:
        arcl = numeric["ARCL"]
        if optical_aid and arcl < ARCL_MIN_OPTICAL - ARCL_ROUNDING_TOLERANCE - epsilon:
            failures.append(("extreme_arcl_optical", f"ARCL={arcl} < {ARCL_MIN_OPTICAL}", ""))
        if naked_eye and arcl < ARCL_MIN_NAKED_EYE - ARCL_ROUNDING_TOLERANCE - epsilon:
            failures.append(("extreme_arcl_naked_eye", f"ARCL={arcl} < {ARCL_MIN_NAKED_EYE}", ""))

    return failures


# The header title repeats verbatim ("No." or "NO.") wherever it appears;
# a page image can carry it more than once if the scan stacks two of the
# book's printed pages into one raster (observed on page 7).
_HEADER_TITLE_RE = re.compile(r"^no\.?$", re.IGNORECASE)

# Row bands on the same physical row jitter by a handful of pixels; the
# next row down is a full pitch away (47-58px). This only needs to keep
# rows apart, not be precise about it, same as extract.py's own constant.
_ROW_BAND_GAP_PX = 20


def physical_row_count(tsv_path):
    """Count a page's physical data rows by clustering the No. column's
    tokens into row bands, independent of whether each row's Date parsed.

    Excludes the header title token (which can repeat, see _HEADER_TITLE_RE)
    and the narrow ruler digit that sits under the No. column one row below
    the header row: every genuine record number renders far wider (three
    digits, zero padded) than that single unpadded digit, so a width cutoff
    at half the column's own median width separates the two without a fixed
    pixel row to skip, which is what silently lost the first data row on
    almost every page the last time this was tried with `top > 200`.

    Returns None if the page's header row cannot be located.
    """
    words = _extract._read_words(tsv_path)
    anchors = _extract._find_header_anchors(words)
    if anchors is None:
        return None

    header_top = min(
        w["top"] for w in words if w["text"] in _extract._HEADER_ANCHOR_TOKENS
    )
    bounds = _extract._column_boundaries(anchors)
    no_col_words = [
        w for w in words
        if _extract._assign_column(w, bounds) == 0
        and w["top"] > header_top + _extract._COARSE_ROW_GAP_PX
        and not _HEADER_TITLE_RE.match(w["text"].strip())
    ]
    if not no_col_words:
        return 0

    widths = sorted(w["width"] for w in no_col_words)
    median_width = widths[len(widths) // 2]
    data_words = [w for w in no_col_words if w["width"] >= median_width * 0.5]
    if not data_words:
        return 0

    tops = sorted(w["top"] for w in data_words)
    row_count = 1
    for prev_top, top in zip(tops, tops[1:]):
        if top - prev_top > _ROW_BAND_GAP_PX:
            row_count += 1
    return row_count


def completeness_check(tsv_dir, rows):
    """Compare each page's physical row-band count against how many rows
    in `rows` carry that page number. Returns a list of
    (page, physical_count, transcribed_count) for every page that differs,
    in page order, or None if `tsv_dir` does not exist."""
    tsv_dir = Path(tsv_dir)
    if not tsv_dir.is_dir():
        return None

    transcribed_counts = Counter(row["page"] for row in rows)
    tsv_paths = sorted(tsv_dir.glob("page-*.tsv"))
    mismatches = []
    for index, tsv_path in enumerate(tsv_paths):
        page_num = str(_extract.FIRST_PAGE + index)
        physical = physical_row_count(tsv_path)
        if physical is None:
            continue
        transcribed = transcribed_counts.get(page_num, 0)
        if physical != transcribed:
            mismatches.append((page_num, physical, transcribed))
    return mismatches


def write_report(path, rows, all_failures, completeness_mismatches):
    total = len(rows)
    failing_row_indices = sorted(all_failures.keys())
    fail_count = len(failing_row_indices)
    pass_count = total - fail_count

    by_check = defaultdict(lambda: defaultdict(int))
    by_check_page = defaultdict(lambda: defaultdict(lambda: defaultdict(int)))
    for idx, failures in all_failures.items():
        page = rows[idx - 1]["page"]
        for check, detail, _ in failures:
            by_check[check][detail] += 1
            by_check_page[check][detail][page] += 1

    lines = []
    lines.append(f"Odeh Table VI transcription gate report")
    lines.append(f"total rows: {total}  pass: {pass_count}  fail: {fail_count}")
    lines.append("")
    lines.append("=== Summary by check, then by pattern, then by page ===")
    for check in sorted(by_check):
        check_total = sum(by_check[check].values())
        lines.append(f"\n{check}: {check_total} rows")
        for detail in sorted(by_check[check]):
            count = by_check[check][detail]
            lines.append(f"  {detail}: {count}")
            pages = by_check_page[check][detail]
            for page in sorted(pages, key=lambda p: int(p)):
                lines.append(f"    page {page}: {pages[page]}")

    lines.append("")
    lines.append("=== Every failing row ===")
    for idx in failing_row_indices:
        row = rows[idx - 1]
        checks = ", ".join(
            f"{check} ({detail})" for check, detail, _ in all_failures[idx]
        )
        lines.append(
            f"row {idx} (page {row['page']}, No. {row['No.']}): {checks}"
        )

    lines.append("")
    lines.append("=== Completeness: physical rows vs transcribed rows, by page ===")
    if completeness_mismatches is None:
        lines.append(
            "skipped: no page-images/ directory next to the scratch CSV"
        )
    elif not completeness_mismatches:
        lines.append("every page's physical row count matches its transcribed count")
    else:
        for page, physical, transcribed in completeness_mismatches:
            lines.append(
                f"page {page}: {physical} physical rows, "
                f"{transcribed} transcribed (missing {physical - transcribed})"
            )

    path.write_text("\n".join(lines) + "\n")
    return pass_count, fail_count


def main():
    if len(sys.argv) != 2:
        print(f"usage: {sys.argv[0]} SCRATCH_CSV", file=sys.stderr)
        return 2

    csv_path = Path(sys.argv[1])
    with open(csv_path, newline="") as f:
        rows = list(csv.DictReader(f))

    all_failures = {}
    for idx, row in enumerate(rows, start=1):
        failures = check_row(row)
        if failures:
            all_failures[idx] = failures

    completeness_mismatches = completeness_check(
        csv_path.with_name("page-images"), rows,
    )

    report_path = csv_path.with_name("gate_report.txt")
    pass_count, fail_count = write_report(
        report_path, rows, all_failures, completeness_mismatches,
    )

    print(f"{len(rows)} rows: {pass_count} pass, {fail_count} fail")
    if completeness_mismatches is None:
        print("completeness check: skipped, no page-images/ directory")
    else:
        print(f"completeness check: {len(completeness_mismatches)} page(s) with a mismatch")
    print(f"report: {report_path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

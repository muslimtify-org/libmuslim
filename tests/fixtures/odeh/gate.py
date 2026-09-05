#!/usr/bin/env python3
"""Gate Odeh Table VI's scratch transcription and report what fails.

Runs a set of checks over the scratch CSV `extract.py` produces: column
alphabet validation, five astronomical relations recorded as holding on
page 6 in `docs/research/2026-08-22-odeh-table-vi-ocr-fidelity.md`, and
the paper's own stated extremes for lag and elongation.

This script fixes nothing. Tesseract's known error modes, `V` rendered as
a `\\Y` family or as `Vv`/`vV`/etc, and a leading minus in Long read as a
tilde, are reported by class so a human can adjudicate them against the
page images, not silently normalised here. Applying an unverified mapping
here would bake a guess into the fixture and destroy the evidence the
adjudication step needs.

Exit 0 means the gate ran to completion, not that every row passed. The
report lists every failure; driving that list to empty is later work.

Usage:
    gate.py SCRATCH_CSV

Writes SCRATCH_CSV's sibling `gate_report.txt` and prints pass/fail
counts to stdout.
"""

import csv
import math
import sys
from collections import defaultdict
from pathlib import Path

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
V_TOLERANCE = 0.07

# Section 7.2 and 7.3's stated extremes.
LAG_MIN_OPTICAL = 21
LAG_MIN_NAKED_EYE = 29
ARCL_MIN_OPTICAL = 6.4
ARCL_MIN_NAKED_EYE = 7.7


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

    if "Lag" in numeric:
        lag = numeric["Lag"]
        if optical_aid and lag < LAG_MIN_OPTICAL:
            failures.append(("extreme_lag_optical", f"Lag={lag} < {LAG_MIN_OPTICAL}", ""))
        if naked_eye and lag < LAG_MIN_NAKED_EYE:
            failures.append(("extreme_lag_naked_eye", f"Lag={lag} < {LAG_MIN_NAKED_EYE}", ""))

    if "ARCL" in numeric:
        arcl = numeric["ARCL"]
        if optical_aid and arcl < ARCL_MIN_OPTICAL:
            failures.append(("extreme_arcl_optical", f"ARCL={arcl} < {ARCL_MIN_OPTICAL}", ""))
        if naked_eye and arcl < ARCL_MIN_NAKED_EYE:
            failures.append(("extreme_arcl_naked_eye", f"ARCL={arcl} < {ARCL_MIN_NAKED_EYE}", ""))

    return failures


def write_report(path, rows, all_failures):
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

    report_path = csv_path.with_name("gate_report.txt")
    pass_count, fail_count = write_report(report_path, rows, all_failures)

    print(f"{len(rows)} rows: {pass_count} pass, {fail_count} fail")
    print(f"report: {report_path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

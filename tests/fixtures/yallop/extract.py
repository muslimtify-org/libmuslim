#!/usr/bin/env python3
"""Extract TN69 Table 4 into a CSV.

Reads Yallop's NAO Technical Note 69, "A Method for Predicting the First
Sighting of the New Crescent Moon" (1997, updated 1998), Table 4, the list of
295 lunar crescent observations used to derive the q test. The ICOP/IAC copy
must be used, not the Utrecht copy: pdftotext preserves the middle-dot decimal
separator and the Unicode minus sign on the ICOP scan, and destroys both on
the Utrecht scan.

Only Yallop's underlying observational data (date, phase, location, ARCV,
width, the observer's own coded result) is extracted into the committed
column set. Yallop's own predicted code (BDY) and q value are HMNAO's
copyrighted computed columns; q is kept in the scratch CSV for the self-check
but must never reach a committed fixture, and BDY is dropped entirely.

TN69's printed date column is inconsistent between rows: on a small number of
evening rows it holds the UT date of the Moon's best time rather than the
local evening date that `hijri_yallop_evaluate_evening` expects (see
hijri.h:1493-1513). The local evening date is instead derived from column 6
(Julian Date of astronomical new moon minus 2 400 000 days) and column 12
(age of the Moon in hours at best time), never from the printed date: the
calendar date of the Julian Date `2400000 + col6 + col12/24 + lon_deg/360`,
converted to UTC. The printed date is dropped from the output entirely; only
the derived date and a `shifted` flag (1 where it differs from the printed
date) are emitted, so nothing downstream can accidentally use the printed
date.

Usage:
    extract.py [--self-check] INPUT.pdf OUTPUT.csv

--self-check asserts three checksums recorded in
docs/research/2026-07-30-findings.md and the 2026-08-16 amendment: 295
unique observation numbers, exactly 166 rows with q_yallop above +0.216, and
the derived-versus-printed date offset is 0 or -1 on every evening row.
Exits non-zero, without writing a corrected file, if any checksum fails.
"""

import argparse
import csv
import math
import re
import subprocess
import sys
from collections import OrderedDict
from datetime import date, timedelta

# Table 4 has 18 numbered columns; after the merge-repair below every data
# row token-splits into exactly this many fields.
FIELD_COUNT = 18

# BES/BDY are one or two letters, optionally followed by a parenthesised
# qualifier, e.g. "V", "I(I)", "V(F)". On some rows pdftotext -layout
# collapses the space between BES and BDY, producing one merged token; this
# pattern splits it back into its two codes.
_MERGED_CODE_RE = re.compile(r"^([VI]\([A-Z]+\)|[VI])([VI]\([A-Z]+\)|[VI])$")


def _normalise(text):
    # U+2212 (minus sign) and U+2013 (en dash) both appear in Table 4 as
    # negative signs; U+00B7 (middle dot) is the decimal separator. All three
    # must be normalised before any numeric parsing, or negative values and
    # fractional values are silently corrupted.
    return text.replace("−", "-").replace("–", "-").replace("·", ".")


def _julian_date_to_utc_date(jd):
    """Return the Gregorian calendar date (UTC) the instant `jd` falls in.

    JD 2440587.5 is the Unix epoch, 1970-01-01 00:00 UTC; any Julian Date's
    calendar day is the number of whole days since that instant.
    """
    days_since_unix_epoch = math.floor(jd - 2440587.5)
    return date(1970, 1, 1) + timedelta(days=days_since_unix_epoch)


def _derive_local_evening_date(jd_new_moon, age_hours, lon_deg):
    """TN69 columns 6 and 12 plus longitude, per hijri.h:1493-1513's
    local-mean-solar-time convention. Longitude is positive east."""
    jd_best_time_local = 2400000 + jd_new_moon + age_hours / 24.0 + lon_deg / 360.0
    return _julian_date_to_utc_date(jd_best_time_local)


def _pdftotext(pdf_path):
    result = subprocess.run(
        ["pdftotext", "-layout", pdf_path, "-"],
        capture_output=True,
        check=True,
    )
    return result.stdout.decode("utf-8")


def _parse_rows(text):
    """Yield (obs, row_dict) for each Table 4 data row, first-seen order."""
    rows = OrderedDict()
    for line in _normalise(text).splitlines():
        tokens = line.split()
        # Column 5 is the phase marker; only real data rows carry M or E
        # there. This also rejects the numbered header row ("1 2 3 4 5 ...")
        # and the "Table 4" / zone-section divider lines.
        if len(tokens) < 5 or tokens[4] not in ("M", "E"):
            continue

        if len(tokens) == FIELD_COUNT - 1:
            match = _MERGED_CODE_RE.match(tokens[-1])
            if not match:
                continue
            tokens = tokens[:-1] + [match.group(1), match.group(2)]

        if len(tokens) != FIELD_COUNT or not tokens[0].isdigit():
            continue

        obs = tokens[0]
        try:
            printed_year = int(tokens[1])
            printed_month = int(tokens[2])
            printed_day = int(tokens[3])
            lon_deg = float(tokens[7])
            # Column 6 (index 5): JD of astronomical new moon minus
            # 2 400 000 days. Column 12 (index 11): age of the Moon in
            # hours at best time. Not Modified Julian Date: MJD is JD minus
            # 2 400 000.5, half a day off from what column 6 gives.
            jd_new_moon = float(tokens[5])
            age_hours = float(tokens[11])

            printed_date = date(printed_year, printed_month, printed_day)
            derived_date = _derive_local_evening_date(
                jd_new_moon, age_hours, lon_deg
            )
            date_offset = (derived_date - printed_date).days

            row = {
                "obs": obs,
                "year": derived_date.year,
                "month": derived_date.month,
                "day": derived_date.day,
                "phase": "morning" if tokens[4] == "M" else "evening",
                # Longitude sign convention is positive east, matching
                # hijri.h. This is empirical (TN69's text does not state it):
                # positive-east reproduces Yallop's own q to within 0.035,
                # positive-west diverges by up to 0.721.
                "lat_deg": float(tokens[6]),
                "lon_deg": lon_deg,
                "arcv_deg": float(tokens[9]),
                "width_arcmin": float(tokens[14]),
                "q_yallop": float(tokens[15]),
                "observed_code": tokens[16],
                "shifted": 0 if date_offset == 0 else 1,
                # Internal-only, dropped from the CSV by extrasaction, kept
                # here for --self-check's date-offset checksum.
                "_date_offset": date_offset,
            }
        except ValueError:
            continue

        # Continuation pages repeat the header and reprint the last rows of
        # the previous page, so the same observation number can appear more
        # than once; keep the first occurrence.
        rows.setdefault(obs, row)
    return list(rows.values())


CSV_COLUMNS = [
    "obs",
    "year",
    "month",
    "day",
    "phase",
    "lat_deg",
    "lon_deg",
    "arcv_deg",
    "width_arcmin",
    "q_yallop",
    "observed_code",
    "shifted",
]


def _write_csv(rows, output_path):
    with open(output_path, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=CSV_COLUMNS, extrasaction="ignore")
        writer.writeheader()
        writer.writerows(rows)


def _self_check(rows):
    """Return a list of failure messages; empty means both checks passed."""
    failures = []

    unique_obs = len(rows)
    if unique_obs != 295:
        failures.append(
            f"expected 295 unique observation numbers, got {unique_obs}"
        )

    above = sum(1 for row in rows if row["q_yallop"] > 0.216)
    if above != 166:
        failures.append(
            f"expected 166 rows with q_yallop > +0.216, got {above}"
        )

    # A misparse of column 6 or column 12 produces a wild derived date, so
    # bounding the offset to {0, -1} on every evening row bounds both new
    # columns at once.
    bad_offsets = [
        (row["obs"], row["_date_offset"])
        for row in rows
        if row["phase"] == "evening" and row["_date_offset"] not in (0, -1)
    ]
    if bad_offsets:
        failures.append(
            "expected every evening row's derived-vs-printed date offset "
            f"to be 0 or -1, got out-of-bounds offsets for obs {bad_offsets}"
        )

    return failures


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input_pdf")
    parser.add_argument("output_csv")
    parser.add_argument(
        "--self-check",
        action="store_true",
        help="verify the three checksums documented above",
    )
    args = parser.parse_args()

    text = _pdftotext(args.input_pdf)
    rows = _parse_rows(text)
    _write_csv(rows, args.output_csv)

    if args.self_check:
        failures = _self_check(rows)
        if failures:
            for failure in failures:
                print(f"self-check FAILED: {failure}", file=sys.stderr)
            return 1
        print(f"self-check OK: {len(rows)} unique observations", file=sys.stderr)

    return 0


if __name__ == "__main__":
    sys.exit(main())

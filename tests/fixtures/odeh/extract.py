#!/usr/bin/env python3
"""Transcribe Odeh's Table VI into a scratch CSV.

Reads Odeh's "New Criterion for Lunar Crescent Visibility" and OCRs Table VI,
the list of lunar crescent observations the paper's ARCV/W criterion is
derived from. The table is a scanned image, one per page, across pages 6 to
22 of the PDF, so there is no text layer to lift: each page is rasterised
with `pdfimages` and read with `tesseract`'s TSV output, which gives every
recognised word its own bounding box.

Table VI has nineteen columns: No., R, E, Date, Observer, Long, Lat, Ele, N,
B, T, JD, Age, Lag, ARCV, DAZ, ARCL, W, V. Column boundaries are not fixed
pixel constants: two of the seventeen page images are narrower than the
rest, so every page's boundaries are derived from that page's own header
row, anchored on the four header tokens tesseract reads reliably (`Date`,
`Long`, `Lat`, `Ele`); tesseract misreads `JD` as `D`, `W` as `w`, and `V` as
various garbage, but those tokens still occupy the right column position and
are carried through as transcribed.

This script performs no correction. Two known OCR error modes recur in the
output: a leading minus in the Long column is sometimes read as a tilde,
and the V column is sometimes read as `T` where the printed alphabet is
only `I`, `V`, or blank. Both are recorded verbatim. Detecting and fixing
them is later pipeline work, not this script's.

Usage:
    extract.py INPUT.pdf OUTPUT_DIR

Writes OUTPUT_DIR/page-images (pdfimages + tesseract TSVs) and
OUTPUT_DIR/table_vi_transcription.csv, then prints the row count and the
output CSV path to stdout.

`extract.py --emit-fixture --scratch SCRATCH.csv --zones ZONES.csv
OUTPUT.csv` instead builds the committed fixture from the adjudicated
scratch CSV (ADJUDICATION.md) and `compare_arcv --emit-zones`'s output,
dropping every column Odeh's paper holds copyright on (see
tests/fixtures/odeh/README.md's licensing section for which columns those
are and why).
"""

import argparse
import csv
import re
import subprocess
import sys
from pathlib import Path

FIRST_PAGE = 6
LAST_PAGE = 22

# Table VI's nineteen columns, left to right, page image order.
CSV_COLUMNS = [
    "No.", "R", "E", "Date", "Observer", "Long", "Lat", "Ele", "N", "B", "T",
    "JD", "Age", "Lag", "ARCV", "DAZ", "ARCL", "W", "V",
]

# Header tokens tesseract reads reliably; used only to find the header row,
# never to build the column set (see module docstring).
_HEADER_ANCHOR_TOKENS = {"Date", "Long", "Lat", "Ele"}

_DATE_RE = re.compile(r"^\d{2}-\d{2}-\d{4}$")

# Tesseract TSV columns, in order (see `tesseract ... tsv` output header).
_TSV_FIELDS = [
    "level", "page_num", "block_num", "par_num", "line_num", "word_num",
    "left", "top", "width", "height", "conf", "text",
]

# Intra-row jitter in the header/number rows is a handful of pixels; row
# pitch itself is 47-58px depending on the page. This coarse pass only needs
# to separate rows from each other, not be precise about it.
_COARSE_ROW_GAP_PX = 15


def _run_pdfimages(pdf_path, images_dir):
    images_dir.mkdir(parents=True, exist_ok=True)
    subprocess.run(
        [
            "pdfimages", "-f", str(FIRST_PAGE), "-l", str(LAST_PAGE), "-png",
            str(pdf_path), str(images_dir / "page"),
        ],
        check=True,
    )
    return sorted(images_dir.glob("page-*.png"))


def _run_tesseract(png_path):
    base = png_path.with_suffix("")
    subprocess.run(
        [
            "tesseract", str(png_path), str(base), "--psm", "6",
            "-c", "preserve_interword_spaces=1", "tsv",
        ],
        check=True,
        capture_output=True,
    )
    return base.with_suffix(".tsv")


def _read_words(tsv_path):
    """Return level-5 (word) TSV rows as dicts with numeric fields cast."""
    words = []
    with open(tsv_path, newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f, delimiter="\t", quoting=csv.QUOTE_NONE)
        for row in reader:
            if row["level"] != "5":
                continue
            words.append({
                "left": int(row["left"]),
                "top": int(row["top"]),
                "width": int(row["width"]),
                "text": row["text"],
            })
    return words


def _find_header_anchors(words):
    """Locate the header row and return its 19 (center, text) anchors,
    sorted left to right. Returns None if no row carries the anchor tokens.

    The anchor is the header token's own horizontal center (left + width/2),
    not its left edge: header words are not left-aligned to their column
    (e.g. "Date" sits well right of where the dates themselves start), so a
    boundary derived from left edges alone misclassifies wide data values
    such as the date string, whose left edge can start before the previous
    column's boundary even though the value plainly belongs in this column.
    """
    candidate_tops = sorted({
        w["top"] for w in words if w["text"] in _HEADER_ANCHOR_TOKENS
    })
    if not candidate_tops:
        return None

    header_top = candidate_tops[0]
    header_row = [
        w for w in words if abs(w["top"] - header_top) <= _COARSE_ROW_GAP_PX
    ]
    anchor_texts = {w["text"] for w in header_row}
    if not _HEADER_ANCHOR_TOKENS.issubset(anchor_texts):
        return None

    header_row.sort(key=lambda w: w["left"])
    return [(w["left"] + w["width"] / 2, w["text"]) for w in header_row]


def _column_boundaries(anchors):
    """Midpoints between adjacent anchor centers; -inf/+inf cap the ends."""
    centers = [center for center, _ in anchors]
    bounds = [float("-inf")]
    bounds.extend(
        (centers[i] + centers[i + 1]) / 2 for i in range(len(centers) - 1)
    )
    bounds.append(float("inf"))
    return bounds


def _assign_column(word, bounds):
    center = word["left"] + word["width"] / 2
    for i in range(len(bounds) - 1):
        if bounds[i] <= center < bounds[i + 1]:
            return i
    return len(bounds) - 2


def _cluster_rows(tops):
    """Coarse top-coordinate clustering: a new row starts when a token's top
    is more than _COARSE_ROW_GAP_PX past the running cluster start."""
    clusters = []
    current = [tops[0]]
    for top in tops[1:]:
        if top - current[0] > _COARSE_ROW_GAP_PX:
            clusters.append(current)
            current = [top]
        else:
            current.append(top)
    clusters.append(current)
    return clusters


def _median(values):
    s = sorted(values)
    n = len(s)
    mid = n // 2
    return s[mid] if n % 2 else (s[mid - 1] + s[mid]) / 2


def _group_body_rows(body_words):
    """Group non-header tokens into table rows by top coordinate, using the
    page's own median inter-row pitch as the clustering threshold."""
    if not body_words:
        return []

    tops = sorted(w["top"] for w in body_words)
    coarse = _cluster_rows(tops)
    row_tops = [c[0] for c in coarse]
    gaps = [b - a for a, b in zip(row_tops, row_tops[1:]) if b - a > 0]
    row_pitch = _median(gaps) if gaps else _COARSE_ROW_GAP_PX * 2
    threshold = row_pitch * 0.6

    body_words = sorted(body_words, key=lambda w: w["top"])
    rows = []
    current = [body_words[0]]
    row_ref_top = body_words[0]["top"]
    for w in body_words[1:]:
        if w["top"] - row_ref_top > threshold:
            rows.append(current)
            current = [w]
            row_ref_top = w["top"]
        else:
            current.append(w)
    rows.append(current)
    return rows


def _row_to_cells(row_words, bounds, n_columns):
    by_column = [[] for _ in range(n_columns)]
    for w in row_words:
        col = _assign_column(w, bounds)
        by_column[col].append(w)
    cells = []
    for col_words in by_column:
        col_words.sort(key=lambda w: w["left"])
        cells.append(" ".join(w["text"] for w in col_words))
    return cells


def _transcribe_page(tsv_path, page_num):
    """Return a list of data-row dicts (CSV_COLUMNS + 'page') for one page,
    or [] if the header can't be located."""
    words = _read_words(tsv_path)
    anchors = _find_header_anchors(words)
    if anchors is None:
        print(f"warning: page {page_num}: no header row found, skipping",
              file=sys.stderr)
        return []

    header_top = min(
        w["top"] for w in words if w["text"] in _HEADER_ANCHOR_TOKENS
    )
    body_words = [
        w for w in words if w["top"] > header_top + _COARSE_ROW_GAP_PX
    ]

    bounds = _column_boundaries(anchors)
    date_col = next(
        i for i, (_, text) in enumerate(anchors) if text == "Date"
    )

    out_rows = []
    for row_words in _group_body_rows(body_words):
        cells = _row_to_cells(row_words, bounds, len(anchors))
        date_cell = cells[date_col].replace(" ", "")
        if not _DATE_RE.match(date_cell):
            continue  # not a data row: header repeat, column-number row, etc.
        row = dict(zip(CSV_COLUMNS, cells))
        row["page"] = page_num
        out_rows.append(row)
    return out_rows


# The committed fixture's own column set. Every column Odeh's paper holds
# copyright on (record number, source, observer, Julian date, age, lag,
# ARCV, DAZ, ARCL, W, V) is excluded; see README.md's licensing section.
FIXTURE_COLUMNS = [
    "year", "month", "day", "phase", "lat_deg", "lon_deg", "elev_m",
    "naked_eye", "binocular", "telescope", "zone",
]

_PHASE_NAME = {"E": "evening", "M": "morning"}


def _format_number(text):
    """Parse a numeric cell and drop the source's fixed-width padding
    (e.g. "018.4" -> 18.4, "3800" -> 3800), returning an int where the
    value is whole so integral columns like elevation print without a
    spurious ".0"."""
    value = float(text)
    return int(value) if value == int(value) else value


def _emit_fixture(scratch_csv_path, zones_csv_path, output_csv_path):
    """Build the committed fixture from the adjudicated scratch CSV
    (ADJUDICATION.md) and compare_arcv --emit-zones's output.

    The two files are read in their on-disk row order and zipped
    positionally: both walk the same 578-row scratch CSV in the same
    order, so index i in one is the same record as index i in the other.
    Each pair's No. field is cross-checked as a guard against the two
    files silently drifting out of alignment; No. itself is never written
    to the fixture, since the record number is one of the columns Odeh's
    paper holds copyright on.
    """
    with open(scratch_csv_path, newline="") as f:
        scratch_rows = list(csv.DictReader(f))
    with open(zones_csv_path, newline="") as f:
        zone_rows = list(csv.DictReader(f))

    if len(zone_rows) != len(scratch_rows):
        print(
            f"FATAL: {zones_csv_path} has {len(zone_rows)} rows, expected "
            f"{len(scratch_rows)} to match {scratch_csv_path}",
            file=sys.stderr,
        )
        sys.exit(1)

    fixture_rows = []
    for scratch, zone_row in zip(scratch_rows, zone_rows):
        if scratch["No."] != zone_row["No."]:
            print(
                f"FATAL: scratch/zones rows out of alignment: No. "
                f"{scratch['No.']!r} vs {zone_row['No.']!r}",
                file=sys.stderr,
            )
            sys.exit(1)

        phase = _PHASE_NAME.get(scratch["E"].strip())
        if phase is None:
            print(
                f"FATAL: No. {scratch['No.']}: E column {scratch['E']!r} "
                "is outside {E, M}",
                file=sys.stderr,
            )
            sys.exit(1)

        day, month, year = (int(p) for p in scratch["Date"].strip().split("-"))

        fixture_rows.append({
            "year": year,
            "month": month,
            "day": day,
            "phase": phase,
            "lat_deg": _format_number(scratch["Lat"]),
            "lon_deg": _format_number(scratch["Long"]),
            "elev_m": _format_number(scratch["Ele"]),
            "naked_eye": scratch["N"].strip(),
            "binocular": scratch["B"].strip(),
            "telescope": scratch["T"].strip(),
            "zone": zone_row["zone"].strip(),
        })

    fixture_rows.sort(
        key=lambda r: (r["year"], r["month"], r["day"], r["lat_deg"], r["lon_deg"])
    )

    with open(output_csv_path, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=FIXTURE_COLUMNS, lineterminator="\n")
        writer.writeheader()
        writer.writerows(fixture_rows)

    return fixture_rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input_pdf", nargs="?")
    parser.add_argument("output_csv")
    parser.add_argument(
        "--emit-fixture",
        action="store_true",
        help="build the committed fixture from --scratch and --zones "
        "instead of transcribing a PDF; output_csv is the fixture "
        "destination",
    )
    parser.add_argument(
        "--scratch",
        help="the adjudicated scratch CSV (--emit-fixture mode)",
    )
    parser.add_argument(
        "--zones",
        help="compare_arcv --emit-zones output, paired positionally "
        "against --scratch (--emit-fixture mode)",
    )
    args = parser.parse_args()

    if args.emit_fixture:
        if not args.scratch or not args.zones or not args.output_csv:
            parser.error(
                "--emit-fixture requires --scratch, --zones and output_csv"
            )
        rows = _emit_fixture(args.scratch, args.zones, args.output_csv)
        print(
            f"fixture OK: {len(rows)} rows written to {args.output_csv}",
            file=sys.stderr,
        )
        return 0

    if not args.input_pdf or not args.output_csv:
        parser.error("input_pdf and OUTPUT_DIR are required unless "
                      "--emit-fixture is given")

    pdf_path = Path(args.input_pdf)
    output_dir = Path(args.output_csv)
    output_dir.mkdir(parents=True, exist_ok=True)
    images_dir = output_dir / "page-images"

    pages = _run_pdfimages(pdf_path, images_dir)
    if len(pages) != LAST_PAGE - FIRST_PAGE + 1:
        print(
            f"warning: expected {LAST_PAGE - FIRST_PAGE + 1} page images, "
            f"got {len(pages)}", file=sys.stderr,
        )

    all_rows = []
    per_page_counts = []
    for index, png_path in enumerate(pages):
        page_num = FIRST_PAGE + index
        tsv_path = _run_tesseract(png_path)
        page_rows = _transcribe_page(tsv_path, page_num)
        per_page_counts.append((page_num, len(page_rows)))
        all_rows.extend(page_rows)

    csv_path = output_dir / "table_vi_transcription.csv"
    fieldnames = CSV_COLUMNS + ["page"]
    with open(csv_path, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(all_rows)

    for page_num, count in per_page_counts:
        print(f"page {page_num}: {count} rows", file=sys.stderr)
    print(f"transcribed {len(all_rows)} rows total")
    print(str(csv_path))
    return 0


if __name__ == "__main__":
    sys.exit(main())

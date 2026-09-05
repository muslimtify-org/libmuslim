# Odeh Table VI fixture, the 578-observation lunar crescent record

`hijri_odeh_evaluate_evening` implements M. S. Odeh's V criterion for lunar crescent visibility. This fixture regression-tests it against the transcribed observations Odeh's own Table VI lists, so a change to the evaluation path that shifts a zone boundary is caught by a real historical record, not only by synthetic cases.

## Provenance

- M. S. Odeh, "New Criterion for Lunar Crescent Visibility", Experimental Astronomy (2004) 18: 39-64, DOI 10.1007/s10686-005-9002-5. Table VI, pages 6-22 of the PDF, lists the observations the paper's V criterion is derived from.

Springer paywalls its own copy. The paper's own author publishes the same paper openly through his organisation's site, the Islamic Crescent Observation Project (ICOP): <https://astronomycenter.net/pdf/2006_cri.pdf>, sha256 `7aac71168255c9576f20613c2230ce218e268c0f089f87fe4f0b865a25f26464`. That is the copy this fixture was extracted from.

Table VI's pages are scanned images with no text layer, so there is nothing for `pdftotext` to lift. Each page is rasterised with `pdfimages` and read with `tesseract`'s TSV output, one word at a time with its own bounding box, the same technique `tests/fixtures/yallop/extract.py` uses for TN69's text layer, replaced with an image-aware column split.

## The printed row count against the paper's own claim

The paper's prose states 737 observation records, broken down by source list: 294 Schaefer, 6 Stamm, 42 SAAO, 15 Mirsaeed, 57 Mehrani and 323 ICOP, which sums to 737. The printed Table VI itself holds 578 rows, not 737.

This is not a download or extraction defect. The local PDF used for transcription is byte identical to the ICOP copy at the URL above, checked by sha256, so whatever caused the gap between 737 and 578 is intrinsic to the paper as published, not something introduced by this repository's extraction. Nothing in this fixture attempts to explain the gap further than that.

An earlier pass through this same extraction reported 575 rows and treated that as the table's true count. It was an undercount. `extract.py` finds a data row by matching a `dd-mm-yyyy` pattern in the Date cell, so a row whose date cell failed to transcribe was invisible to it, and to every check built on its output. `gate.py` now runs an independent completeness check that clusters the No. column's tokens into physical row bands on every page, counting rows by the presence of a record number rather than by whether the row's date parsed. That check found three rows the original pass had missed, all recovered by reading the source pixels directly, bringing the transcribed count from 575 to 578. `tests/fixtures/odeh/ADJUDICATION.md` records the full adjudication, including this recovery and the 79 of 578 rows that still fail at least one gate check.

## Licensing

Odeh's paper is the source of the V equation and zone thresholds this library implements, and its printed ARCV, DAZ, ARCL, W, V, Julian date, age and lag columns are Odeh's own computed results, not raw observational data. **No Odeh-computed column is reproduced in this fixture.** Only the underlying observational record crosses into the committed file: the local date, the crescent's phase, the observer's location (latitude, longitude, elevation), and the observer's own naked eye, binoculars and telescope sighting marks, plus this library's own `hijri_odeh_classify` result recomputed from that data. The record number, source list, observer name, Julian date, age, lag, ARCV, DAZ, ARCL, W and V are used only transiently in the scratch CSV `gate.py` and `compare_arcv.c` consume for adjudication and residual measurement. They are never written here, following the same policy `tests/fixtures/yallop/README.md` states for TN69.

### Elevation has no arithmetic check

Every other committed column in this fixture is either an observational outcome or was cross-checked against at least one relation during adjudication (see ADJUDICATION.md). Elevation is the exception: no relation in this repository constrains it, and `hijri.h:184` documents that `loc->elevation_m` is carried deliberately nearly inert, with horizon dip omitted from the sunset and moonset solvers. Elevation's only verification here is two independent human readings of the source pixels agreeing, recorded in ADJUDICATION.md's second-reading pass.

### The visibility marks have no arithmetic check either

The naked eye, binoculars and telescope columns record whether Odeh's observers tried and succeeded at seeing the crescent by each means. No formula in this table or in this library predicts what an observer saw. These three columns are the observational outcome the whole exercise exists to check the library's classification against, not a quantity any relation could verify independently, and adjudication treated them the same way it treated elevation: read directly from the pixels, corrected where a second reading disagreed with the first.

## Extraction recipe

1. `tests/fixtures/odeh/extract.py` rasterises pages 6-22 of the PDF, reads each page's tesseract TSV, and writes a scratch CSV carrying all nineteen of Table VI's printed columns, described in the module's own docstring.
2. `tests/fixtures/odeh/gate.py` validates that scratch CSV against five astronomical relations, the paper's own stated lag and elongation extremes, and the completeness check described above, and reports every failure without correcting anything.
3. Hand adjudication against the source page pixels resolves the OCR error classes the gate finds and recovers any row the completeness check flags. `ADJUDICATION.md` records that pass in full, including the 79 residual failures left uncorrected on purpose because their pixels match what is already transcribed.
4. `tests/fixtures/odeh/compare_arcv.c`, built to `build/compare_arcv`, evaluates every row through `hijri_odeh_evaluate_evening` using the row's own date, latitude, longitude and elevation. Its default mode measures the ARCV and crescent width residual against Odeh's printed values for gate-passing evening rows. Its `--emit-zones` mode instead writes a `No.,zone` line for every row, `zone` being this library's own `hijri_odeh_classify` result, empty for morning rows (which `hijri_odeh_evaluate_evening` does not model) and `UNAVAILABLE` for an evening row whose sunset or moonset event fails. Neither case occurred in this table: every one of the 522 evening rows evaluated to a zone.
5. `extract.py --emit-fixture` pairs the adjudicated scratch CSV with `compare_arcv --emit-zones`'s output positionally, both walking the same 578 rows in the same order, cross-checks each pair's record number as a guard against the two files drifting out of alignment, drops every column this fixture does not commit, sorts by date then latitude then longitude, and writes this file.

### Regenerating

The source PDF is not committed, it is a third-party copyrighted scan. Download it from the ICOP URL above to `$SCRATCH/2006_cri.pdf` first. From the repository root, with `$SCRATCH` any writable scratch directory:

```sh
python3 tests/fixtures/odeh/extract.py \
    "$SCRATCH/2006_cri.pdf" "$SCRATCH/odeh_extract"

python3 tests/fixtures/odeh/gate.py \
    "$SCRATCH/odeh_extract/table_vi_transcription.csv"
```

Read the gate report and the page images at `$SCRATCH/odeh_extract/page-images`, correct the scratch CSV by hand against the pixels, and record the correction in an updated `ADJUDICATION.md`. Once the corrected CSV passes adjudication:

```sh
gcc -std=c11 -Wall -Wextra -Wpedantic -O2 -I. \
    tests/fixtures/odeh/compare_arcv.c -lm -o build/compare_arcv
build/compare_arcv "$SCRATCH/table_vi-corrected.csv" \
    --emit-zones "$SCRATCH/table_vi-zones.csv"

python3 tests/fixtures/odeh/extract.py --emit-fixture \
    --scratch "$SCRATCH/table_vi-corrected.csv" \
    --zones "$SCRATCH/table_vi-zones.csv" \
    tests/fixtures/odeh/table-vi-observations.csv
```

Generated with **Python 3.14.7**, **tesseract 5.5.3** and **GCC 16.2.1**.

## The ARCV and crescent width residual

`hijri_odeh_evaluate_evening` computes its own topocentric ARCV and crescent width from date, latitude, longitude and elevation alone. Table VI prints Odeh's own values for the same quantities. Measured over the 469 of 522 evening rows that pass every gate check, library minus Odeh:

- ARCV: mean absolute 0.048 degrees, worst absolute 0.510 degrees, nothing beyond 1 degree.
- Crescent width: mean absolute 0.005 arc minutes, worst absolute 0.018 arc minutes, nothing beyond 1 arc minute.

Both magnitudes sit within Table VI's own print rounding (ARCV to one decimal degree, W to the nearest arc second), consistent with rounding noise rather than a systematic offset. The full measurement, including why the 53 gate-failing evening rows are excluded, is in `docs/research/2026-09-05-odeh-table-vi-residual.md`.

## Gate failures left in the data

79 of the 578 committed rows fail at least one of `gate.py`'s checks. Most of them, 76, fail only the local solar time band relation, which `ADJUDICATION.md` records as deliberately narrower than the real spread of observation times in this table: widening the band to fit every genuine row would mean fitting the check to the answer instead of deriving it independently, so it is left tight on purpose. The remaining 3 are individually adjudicated inconsistencies in Odeh's own printed table, not transcription errors. `ADJUDICATION.md` records which rows and why for all 79. This fixture commits them as observed, uncorrected and unflagged, because failing a deliberately tight gate is not the same thing as being wrong.

## The columns

| Column | Meaning |
| --- | --- |
| `year`, `month`, `day` | The local date exactly as Table VI prints it. Unlike the TN69 fixture, no derivation is applied: Table VI's Date column already carries the local evening or morning date `hijri_odeh_evaluate_evening` expects. |
| `phase` | `evening` or `morning`, from Table VI's E column. |
| `lat_deg`, `lon_deg` | Observer location, degrees, longitude positive east. |
| `elev_m` | Observer elevation in metres. Verified by two independent pixel readings only, see above. |
| `naked_eye`, `binocular`, `telescope` | Table VI's N, B and T columns verbatim: `V` for a sighting, `I` for a tried-and-failed attempt, empty for not attempted. |
| `zone` | This library's `hijri_odeh_classify` result for the row: `NOT_VISIBLE`, `OPTICAL_AID_ONLY`, `OPTICAL_AID_OR_NAKED_EYE` or `NAKED_EYE`. Empty for the 56 morning rows, which `hijri_odeh_evaluate_evening` does not model. |

578 data rows: 522 evening and 56 morning, all of Table VI's printed observations, including the 79 rows that fail a gate check on purpose (see above). Unlike the TN69 fixture, morning rows are kept rather than dropped: their observational data (location, elevation and sighting marks) is worth committing even though this library evaluates evenings only, so their `zone` column is simply left empty.

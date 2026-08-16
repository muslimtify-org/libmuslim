# TN69 fixture, the 295-observation Yallop crescent record

`hijri_yallop_evaluate_evening` implements B. D. Yallop's best-time q test.
This fixture regression-tests it against the 295 real observations Yallop
used to derive that test in the first place, so a change to the evaluation
path that shifts a zone boundary is caught by a real historical record, not
only by synthetic cases.

## Provenance

The 295 observations are Schaefer's, not Yallop's:

- B. E. Schaefer, "Visibility of the lunar crescent," *QJRAS* 29 (1988) 511.
  <https://ui.adsabs.harvard.edu/abs/1988QJRAS..29..511S>
- B. E. Schaefer, "Lunar Crescent Visibility," *QJRAS* 37 (1996) 759, which
  extends the list to 295 observations.
  <https://ui.adsabs.harvard.edu/abs/1996QJRAS..37..759S>

The transcription route used here is Yallop's own compilation of the
Schaefer observations into Table 4 of NAO Technical Note 69, "A Method for
Predicting the First Sighting of the New Crescent Moon" (1997, updated
1998). The ICOP/IAC mirror was used:
<https://astronomycenter.net/pdf/yallop_1997.pdf>. The ADS scans of the two
Schaefer papers are image-only PDFs, with no extractable text layer, so they
were not used directly. TN69's own transcription is the practical source.

HMNAO reserves copyright on TN69's own computed columns (Yallop's ARCL, ARCV,
DAZ, age, lag, parallax, crescent width W', q, and predicted outcome BDY) and
requires a licence for commercial use. **No HMNAO-computed column is
reproduced in this fixture.** Only the underlying observational data
(date, phase, location, and the observer's own coded naked-eye/optical-aid
result) crosses into the committed file, plus this library's own
`hijri_yallop_classify` result recomputed from that data. `q_yallop` and
Yallop's ARCV/width are used only transiently in the scratch CSV that
`compare_q`'s residual gate consumes. They are never written here.

## The date convention

TN69's printed date column is **not** consistent between rows. On most rows
it is the local evening date `hijri_yallop_evaluate_evening` expects. On a
small number of evening rows it instead holds the UT date of the Moon's best
time, which runs a day behind the local evening date at longitudes west of
Greenwich once the time-of-day crosses midnight UT.

This fixture carries the **derived local evening date** throughout, never
TN69's printed date. The derivation uses Table 4 column 6 (Julian Date of
astronomical new moon minus 2 400 000) and column 12 (age of the Moon in
hours at best time): the calendar date, in UTC, of Julian Date
`2400000 + col6 + col12/24 + lon_deg/360`. See `hijri.h`'s local-mean-solar-time
convention for evening evaluation and `extract.py`'s
`_derive_local_evening_date` for the exact computation.

Five evening rows have a derived date that differs from TN69's printed date
by one day. They are listed here by date and coordinates, not by TN69
observation number, since the numbering is the compilation's own
arrangement and is not committed:

| Derived date | lat_deg | lon_deg |
| --- | --- | --- |
| 1980-07-13 | 41.4 | -70.7 |
| 1987-05-28 | 39.2 | -105.5 |
| 1989-07-04 | 37.4 | -121.6 |
| 1989-10-02 | 36.1 | -108.8 |
| 1991-05-15 | 39.0 | -76.8 |

A reader checking this fixture against a printed copy of TN69 will see these
five rows' dates disagree with the table. That is this derivation, not a
transcription error.

## Extraction recipe

1. `tests/fixtures/yallop/extract.py` parses Table 4 out of the ICOP PDF with
   `pdftotext -layout`, normalises the Unicode minus and middle-dot decimal
   separator the ICOP scan uses, derives the local evening date per the
   convention above, and writes a scratch CSV carrying `q_yallop` and the
   other HMNAO-computed columns for the residual self-check only.
2. `tests/fixtures/yallop/compare_q.c`, built to `build/compare_q`, evaluates
   every evening row through the live library and gates on the maximum `q`
   residual against `q_yallop`. Its `--emit-zones` mode additionally writes
   `year,month,day,lat_deg,lon_deg,zone` for every evening row, `zone` being
   this library's own `hijri_yallop_classify` result, `NA` where the sunset
   or moonset event fails and `q` is unavailable.
3. `extract.py --emit-fixture` pairs the corrected CSV's evening rows with
   `compare_q --emit-zones`'s output positionally (both walk the same evening
   rows in the same order), maps `observed_code` through the fixed TN69 item
   17 table, drops every HMNAO-computed column and the TN69 observation
   number, sorts by date then latitude then longitude, and writes this file.

### Regenerating

The source PDF is not committed, it is a third-party copyrighted scan.
Download it from the ICOP/IAC mirror above to `$SCRATCH/yallop_1997.pdf`
first. From the repository root, with `$SCRATCH` any writable scratch
directory:

```sh
python3 tests/fixtures/yallop/extract.py \
    "$SCRATCH/yallop_1997.pdf" "$SCRATCH/tn69-corrected.csv" \
    --self-check

gcc -std=c11 -Wall -Wextra -Wpedantic -O2 -I. \
    tests/fixtures/yallop/compare_q.c -lm -o build/compare_q
build/compare_q "$SCRATCH/tn69-corrected.csv" \
    --emit-zones "$SCRATCH/tn69-zones.csv"

python3 tests/fixtures/yallop/extract.py --emit-fixture \
    --corrected "$SCRATCH/tn69-corrected.csv" \
    --zones "$SCRATCH/tn69-zones.csv" \
    tests/fixtures/yallop/tn69-observations.csv
```

`extract.py`'s `--self-check` asserts 295 unique observation numbers, 166
rows with `q_yallop` above +0.216, and a derived-versus-printed date offset
of 0 or -1 on every evening row, refusing to write the corrected CSV if any
fails. `compare_q` gates on a maximum `q` residual of 0.10 and exits non-zero
past it.

Generated with **Python 3.14.7** and **pdftotext (Poppler) 26.07.0**.

## The columns

| Column | Meaning |
| --- | --- |
| `year`, `month`, `day` | Derived local evening date (Gregorian, UTC calendar day), see above. |
| `lat_deg`, `lon_deg` | Observer location, degrees, longitude positive east. |
| `observed` | The observer's own naked-eye result per TN69 item 17: `seen_unaided` (codes `V`, `V(F)`, `V(B)`, `V(T)`, `V(V)`), `seen_optical_aid` (`I(B)`, `I(V)`), or `not_seen` (`I`, `I(I)`). |
| `zone` | This library's `hijri_yallop_classify` result for the row, `A`-`F`, or `NA` where sunset/moonset fails and `q` is unavailable. |

271 data rows: the 295 Table 4 observations minus the 24 morning-crescent
rows, which `hijri_yallop_evaluate_evening` does not model.

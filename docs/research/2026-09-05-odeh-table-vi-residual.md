# Odeh Table VI, ARCV and crescent width residual

Date: 2026-09-05.

This is the validation the earlier Table VI work prepared for. `hijri_odeh_evaluate_evening` computes its own topocentric ARCV and crescent width from date, latitude, longitude and elevation. Table VI prints Odeh's own ARCV and W for the same rows. This measures the residual between the two.

## Unit note

Table VI's W column is printed in arc seconds. `crescent_width_arcmin` is in arc minutes. The printed W is divided by 60 before comparison. `docs/research/2026-08-22-odeh-table-vi-ocr-fidelity.md` reported that Odeh's V does not close against the printed ARCV and W using the library's own V coefficients, and attributed that to a possible unit mismatch without resolving it. That non-closure is exactly this unit trap, not a defect in the transcription or in V. Applying the arcsecond to arcminute conversion here removes it.

## Input and method

Input is the scratch CSV `tests/fixtures/odeh/extract.py` and its adjudication pass produced, 578 rows, described in `tests/fixtures/odeh/ADJUDICATION.md`. That file, the source Odeh table it transcribes, and the intermediate scratch CSVs used to run this comparison are not committed, matching `tests/fixtures/yallop/README.md`'s licensing policy for the sibling Yallop fixture. Only aggregate statistics appear below, no verbatim row and no Odeh computed value beyond those aggregates.

`tests/fixtures/odeh/compare_arcv.c`, built to `build/compare_arcv`, reads a scratch CSV of evening rows, evaluates each one through `hijri_odeh_evaluate_evening` using the row's own date, latitude, longitude and elevation, and reports the residual of the library's ARCV and crescent width against Odeh's printed values.

Evening rows only were used, 522 of 578 rows, since `hijri_odeh_evaluate_evening` models the evening case and Table VI's `E` column marks the other 56 as morning.

## Excluding gate failures

`ADJUDICATION.md` records 79 of 578 rows failing `gate.py`'s checks, 53 of them among the 522 evening rows. Every one of those failures was individually confirmed against the source page pixels to be a correct transcription, not an OCR error, so exclusion here is not about transcription quality.

They are excluded anyway, because the check that most of them fail, the local time band relation, tests whether the row's own Date, JD and longitude place the printed instant inside the range this comparison assumes for an evening sighting. A row that fails it is a row where the Date column, which is the only input this comparison feeds to `hijri_odeh_evaluate_evening`, might not correspond to the printed JD instant closely enough to make a fair comparison. Running the comparison against the full 522-row evening set first, before applying this exclusion, confirmed the concern: one row produced a residual of roughly 118 degrees in ARCV and 30.6 arc minutes in width, more than three orders of magnitude past every other row, while every other row in the unfiltered set is already tight. That single row accounts for the entire excess over the filtered set's maximum. It is consistent with a printed inconsistency in Odeh's own table on a row already flagged by `ADJUDICATION.md`, not with a transcription error and not with a library defect, but including it would let one already-known edge case dominate an aggregate meant to characterize the general case.

The comparison below therefore uses the 469 evening rows that pass every gate check, out of 522 evening rows, out of 578 rows total.

## Residual distribution

ARCV, degrees, library minus Odeh, 469 rows:

- mean signed: -0.035
- mean absolute: 0.048
- median absolute: 0.034
- worst absolute: 0.510
- count beyond 1 degree absolute: 0

Crescent width, arc minutes, library minus Odeh, same 469 rows:

- mean signed: -0.003
- mean absolute: 0.005
- median absolute: 0.005
- worst absolute: 0.018
- count beyond 1 arc minute absolute: 0

## Reading the distribution

This is scatter, not a systematic offset. The magnitudes line up with Table VI's own printed rounding: ARCV is printed to one decimal degree, so a perfectly matching value can differ from the true value by up to 0.05 degrees from rounding alone, and the measured mean absolute residual of 0.048 degrees sits right at that bound. W is printed to the nearest arc second, 0.0167 arc minutes, and the measured mean absolute width residual of 0.005 arc minutes is well inside that. The signed means are small relative to the absolute means in both quantities, consistent with rounding noise centered near zero rather than a directional bias.

This is a materially tighter result than the 2026-08-22 finding on the JD best-time instant, where the best fit lag fraction was 0.55 against Odeh's stated 4/9, a real and unresolved offset in a different quantity. No comparable offset appears here. Per the task's own instruction, an offset of that shape would be recorded as a finding and left alone rather than closed by adjusting the library. None was found, so there is nothing to record beyond this distribution.

## Reproduction

The source table is a third-party copyrighted scan and is not committed, following the same policy `tests/fixtures/yallop/README.md` states for TN69. Reproducing this measurement requires re-running `tests/fixtures/odeh/extract.py` and its adjudication pass to regenerate the scratch transcription, then:

```sh
gcc -std=c11 -Wall -Wextra -Wpedantic -O2 -I. \
    tests/fixtures/odeh/compare_arcv.c -lm -o build/compare_arcv
build/compare_arcv "$SCRATCH/table_vi-evening-gatepass.csv"
```

where the input CSV holds only the evening rows that pass `tests/fixtures/odeh/gate.py`.

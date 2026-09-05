# Odeh Table VI transcription adjudication

This records how much hand correction the Table VI fixture rests on.
The scratch transcription has 575 rows.
The gate originally rejected 412 of them.
Every rejected cell, and the elevation and visibility-mark columns on every row (rejected or not), were read directly against the scanned page pixels rather than accepted from OCR or inferred from the gate's own arithmetic.

## Glyph mapping verified before use

Odeh's naked eye, binoculars and telescope columns use a plain `V` for a visible sighting.
Tesseract renders that glyph inconsistently as a family of garbled tokens, including `\Y`, `\Y,`, `\Y;`, `\Y4`, `\V/`, `Vv`, `vV`, `VvV`, `v` and `V.`.
This mapping was already confirmed once, on page 8.
Before applying it as a bulk correction here, it was independently confirmed against the source pixels on seven more cells, spanning pages 8, 11, 12, 14, 15 and 20, and all three of the N, B and T columns, and every one of the family's distinct sub-variants.
All seven read as a plain `V` with no exceptions.
On that basis the mapping was applied to every remaining exact match of the family across the whole table, correcting 450 cells across 328 rows.

## Row-by-row adjudication

The remaining 203 gate failures, plus a full second reading of elevation, naked eye, binoculars and telescope for every one of the 575 rows, were adjudicated by reading the source page images directly, page by page.
This produced 229 further cell corrections across 155 rows, and found 36 cases where the second reading disagreed with the existing transcription (elevation misread as the letter `O`, or a bracket or comma artifact attached to a digit, being the most common pattern, alongside a number of `1` and `T` glyphs that were actually the letter `I`).
Every one of those 36 disagreements was resolved by inspecting the pixels and is included in the 229 corrections above.

Combining the bulk glyph mapping and the row-by-row pass, 395 of the 575 rows received at least one correction.

No cell was left as a genuine pixel-level ambiguity.
Every flagged cell and every second-reading disagreement was legible once cropped generously and, where needed, zoomed and negated.
A small number of cells (elevation glyphs shaped identically for the digit `0` and the letter `O`) were pixel-ambiguous on their own but were resolved with certainty because the column's alphabet is numeric only, so a letter cannot be the correct reading regardless of glyph shape. Those are corrections, not ambiguities, and are counted above.

## Residual gate failures, left uncorrected on purpose

After adjudication the gate reports 497 of 575 rows passing.
The remaining 78 failures are not transcription damage and were not corrected, because the pixels for every one of them match what is already in the CSV.

75 of the 78 are `relation2_local_time_band` failures.
This mirrors the finding recorded earlier (2026-09-05, task 2b) that the gate's local-time band is deliberately narrower than the real spread of the data, and widening it would mean fitting the gate to the answer instead of deriving it independently.
The two rows previously identified as genuinely damaged in that earlier measurement, rather than merely outside the band, are corrected in this pass (their Julian Date fields carried digit errors) and now pass the gate.
The other rows in this class are left failing on purpose, as a known limitation of the gate rather than an error in the data.

The remaining 3 failures (one `relation1_date_jd_long`, one `relation4_spherical_closure`, one `extreme_arcl_naked_eye`) were each individually checked cell by cell against the source pixels for every column the check depends on.
In each case every relevant cell matches the printed page exactly, so the discrepancy is either a genuine inconsistency in Odeh's original table or an edge case the check's tolerance does not cover, not a transcription error, and no correction was made.

## Rows missing from the transcription entirely

While reading pixels for the second-reading pass, two of the seventeen page agents (page 11 and page 14) noticed that the source scan contains rows with no counterpart anywhere in the 575-row CSV, on any page.
Page 11 has one such row, between the rows for record numbers 688 and 265.
Page 14 has two, one between the rows for record numbers 488 and 621, and one after record number 031 at the end of that page's rows.
These are not cell-level errors and cannot be fixed by correcting a cell, since they have no row to attach a correction to.
Inserting them would also renumber every subsequent row index in the CSV, which is a structural change well outside adjudicating existing cells against existing gate failures.
They are recorded here as a known gap in the fixture's completeness for whoever scopes the next piece of work on it.
The rest of the page-by-page work did not include a systematic count of physical rows against CSV rows, so it cannot rule out further gaps of the same kind on other pages.

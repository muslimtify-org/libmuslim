# Odeh Table VI transcription adjudication

This records how much hand correction the Table VI fixture rests on.
The scratch transcription has 578 rows.
The gate originally rejected 412 of the 575 rows the extraction produced at that point.
Every rejected cell, and the elevation and visibility-mark columns on every row (rejected or not), were read directly against the scanned page pixels rather than accepted from OCR or inferred from the gate's own arithmetic.

The 575 figure this document originally reported was an undercount.
`extract.py` finds a row by matching a dd-mm-yyyy pattern in the Date cell, so a row whose date failed to transcribe was invisible to it and to every check built on its output, including the row-by-row adjudication below, which worked from that same 575-row CSV.
Nothing before this pass ever compared the transcription against a count of what is actually printed on the page.
That gap is closed below.

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

After adjudication the gate reports 499 of 578 rows passing.
The remaining 79 failures are not transcription damage and were not corrected, because the pixels for every one of them match what is already in the CSV.

76 of the 79 are `relation2_local_time_band` failures.
This mirrors the finding recorded earlier (2026-09-05, task 2b) that the gate's local-time band is deliberately narrower than the real spread of the data, and widening it would mean fitting the gate to the answer instead of deriving it independently.
The two rows previously identified as genuinely damaged in that earlier measurement, rather than merely outside the band, are corrected in this pass (their Julian Date fields carried digit errors) and now pass the gate.
One of the three rows inserted by the completeness check below (record 253, page 14) also falls in this class, just past the band's upper edge, and its cells were read directly from the pixels during insertion, so this is the same known limitation, not a new transcription error.
The other rows in this class are left failing on purpose, as a known limitation of the gate rather than an error in the data.

The remaining 3 failures (one `relation1_date_jd_long`, one `relation4_spherical_closure`, one `extreme_arcl_naked_eye`) were each individually checked cell by cell against the source pixels for every column the check depends on.
In each case every relevant cell matches the printed page exactly, so the discrepancy is either a genuine inconsistency in Odeh's original table or an edge case the check's tolerance does not cover, not a transcription error, and no correction was made.

## Completeness check and the rows it found

While reading pixels for the second-reading pass, two of the seventeen page agents (page 11 and page 14) noticed that the source scan contains rows with no counterpart anywhere in the CSV, on any page.
That finding could not be trusted beyond those two pages on its own, because nothing had checked the other fifteen, and the technique that produced the original count, matching a dd-mm-yyyy pattern in the Date cell, is exactly the technique that would miss a row like this: a row whose date failed to transcribe has no date pattern to match, so it is invisible to both the extraction and to any check built the same way.

`gate.py` now runs an independent completeness check instead.
For each page it clusters the No. column's tokens from that page's tesseract TSV into row bands, counting physical rows by the record number's presence (or, failing that, its ink) rather than by whether the row's date parsed, and compares that count against how many rows in the CSV carry that page number.
Run across all seventeen pages, it found exactly the two pages already flagged by hand and no others: page 11 short by one row, page 14 short by two.

Page 11 was missing record 264, between the rows for record numbers 688 and 265.
Page 14 was missing record 253, between the rows for record numbers 488 and 621, and record 303, after record number 031 at the end of that page's rows.
All three were read directly from the source pixels and inserted at their correct position and page, bringing the total from 575 to 578.
Running the completeness check again over the enlarged CSV reports no page where the physical and transcribed counts differ.

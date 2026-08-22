# Odeh Table VI, OCR fidelity spot-check

Date: 2026-08-22. Issue: #47.

#47 says Table VI does not extract, that building the fixture means OCR of numeric tables, and that OCR "is exactly the tool most likely to introduce silent digit errors into a fixture whose purpose is catching digit errors". This measures whether that risk is real and, more usefully, whether it is checkable.

Verdict: transcription is reliable and mechanically verifiable, but the checks that verify it cover mostly the columns licensing forces us to discard. Of the five columns #47 plans to commit, two cannot be verified by any arithmetic at all, and one of those two is the fixture's entire purpose.

## Source and rendering

`https://astronomycenter.net/pdf/2006_cri.pdf`, 27 pages, produced by Acrobat Distiller 7.0.5 from Textures on Macintosh in 2006.

Measuring the text layer page by page locates the problem precisely. Pages 1 to 5 and 23 to 25 carry 1100 to 2100 characters each. Pages 6 to 22 carry 29 to 79, which is the running head, the page number and the caption. Those seventeen pages are Table VI and they have no text layer, confirming #47.

The table is set in landscape on a portrait page. Rendered at 300 dpi and rotated 90 degrees clockwise it is fully legible: a monospaced typewriter face, well separated columns, no bleed between rows. This is close to the best case for transcription, and nothing like the scanned-fax case the phrase "OCR of numeric tables" usually implies.

Page 6, the first table page, was transcribed in full. 39 rows, all 19 columns.

## What can be checked, and what it proves

Five relations hold between columns that were read independently of one another. A misread digit in any participating column breaks its relation, so these are genuine checksums rather than restatements.

**1. Date, Long and JD agree, 39 of 39.** Converting JD to a UT calendar date matches the Date column on 36 rows and lands one day later on 3. Those three are at longitudes -123.1, -110.8 and -111.0, where local sunset falls after 00:00 UT. Applying the longitude makes all 39 agree exactly, which also establishes that Date is local and JD is UT. That is not stated in the caption.

**2. Column 3 partitions the local solar time of the JD instant, 39 of 39.** Rows marked E fall between 17.13 h and 19.65 h local. Rows marked M fall between 4.95 h and 6.63 h. No overlap, so column 3 is evening against morning and both classes land where they should.

**3. Column 3 agrees with the sign of Age, 39 of 39.** Every E row has positive Age, every M row negative.

**4. The spherical relation closes, 39 of 39.** `cos(ARCL) = cos(ARCV) cos(DAZ)` holds for every row once the plus or minus 0.05 rounding envelope on all three printed values is propagated. Checked against the naive plus or minus 0.05 bound, 6 rows appear to fail at up to 0.091 degrees, which is an artifact of the wrong bound rather than a transcription error.

**5. JD agrees with computed sunset plus a fraction of the lag.** Using libmuslim's own `hijri_find_sunset` over the 35 evening rows, the residual against `sunset + k * lag` has a clear minimum in k. At Odeh's stated best-time fraction of 4/9 the mean absolute residual is 3.39 minutes, and the best fit is k = 0.55 at 2.32 minutes. The minimum is real, so the relation genuinely constrains the data, but the offset from 4/9 means this check carries a modelling assumption the other four do not.

## What does not check out

**Odeh's V does not close against the printed ARCV and W.** Solving `V = ARCV - (-0.1018 W^3 + 0.7319 W^2 - 6.3226 W + 7.1651)` for W, using the library's own coefficients, gives a W that is 1.29 to 2.10 times the printed W column, with no constant ratio. Either the W column's unit is not what it appears, or V is computed from quantities other than the printed ARCV and W, for instance at a different instant.

This matters for planning. #47 names exactly this relation as the parse checksum, "recompute V from the row's own ARCV and W and compare against the row's own printed V", and proposes keeping the computed columns in an untracked scratch file to run it. That gate does not currently close on correctly transcribed data, so it would reject good rows. It has to be resolved before it can gate anything.

## The coverage problem

The five relations above validate Date, Long, JD, Age, Lag, column 3, ARCV, DAZ and ARCL.

#47's licensing analysis says the committed fixture keeps date, latitude, longitude, elevation and the three visibility columns, and drops everything else. Set those two lists against each other.

| Committed column | Covered by a check |
|---|---|
| Date | yes, relation 1, exactly |
| Long | yes, relation 1, exactly |
| Lat | weakly, see below |
| Ele | no |
| N, B, T | no |

Latitude enters only through relation 5, via sunset time. Perturbing it and re-measuring gives the sensitivity directly, over the 35 evening rows:

```
as transcribed        3.49 min mean residual
Lat  off by  1 deg    4.22
Lat  off by 10 deg   18.07
Long off by  1 deg    7.48
Ele  off by 1000 m    3.49
```

A wrong leading digit in latitude moves the residual by a factor of five and would be caught. A wrong final digit moves it by a fifth of the baseline scatter and would not. The 0.1 degree decimal is entirely invisible.

Elevation does not move the residual at all, and that is by design rather than by accident: `hijri.h` documents that horizon dip is omitted from the set solvers and that `elevation_m` is deliberately nearly inert. No other column in Table VI constrains elevation either, so nothing in this repository can verify that column.

The visibility columns cannot be verified by any arithmetic, because they are observational outcomes rather than derived quantities. They are also the entire reason to build the fixture.

## What this means for the cycle

Proceed, but not on the protocol #47 currently describes.

The arithmetic gate is worth building and is cheap, and it will catch transcription damage across nine columns at once. It is not sufficient, and the columns it misses are precisely the ones that get committed. Those need a second independent transcription pass, diffed against the first, for latitude, elevation and the three visibility columns. Seventeen pages, two passes on part of each.

The V checksum #47 planned needs the W unit question resolved first, otherwise it rejects correct rows.

## Reproduction

Scripts are in the session scratchpad rather than committed, since the transcription includes Odeh's computed columns and those are not redistributable under the policy `tests/fixtures/yallop/README.md` sets. Rendering is `pdftoppm -f 6 -l 6 -r 300`, then a 90 degree clockwise rotation. No OCR engine was involved.

Only aggregate statistics from the transcription appear above, and no verbatim rows, for the same reason.

# Yallop TN69 validation, and the Odeh gap

## Question

`hijri_yallop_evaluate_evening` had never been checked against a real
observational record. This note records what a one-time check against
Technical Note 69 Table 4 found, what the committed fixture at
`tests/fixtures/yallop/tn69-observations.csv` freezes from that check, and
what remains true of Odeh, which has no equivalent check at all.

## Sources, retrieved 2026-08-16

- ICOP/IAC mirror of TN69, used for extraction.
  <https://astronomycenter.net/pdf/yallop_1997.pdf>, HTTP 200, 214790 bytes.
- Utrecht mirror of TN69, used as a cross-check on the transcription.
  <https://webspace.science.uu.nl/~gent0113/islam/downloads/naotn_69.pdf>,
  HTTP 200, 229467 bytes.
- Schaefer 1996, *QJRAS* 37, 759, the extended 295-observation source TN69
  Table 4 compiles.
  <https://articles.adsabs.harvard.edu/pdf/1996QJRAS..37..759S>, HTTP 200.
- Schaefer 1988, *QJRAS* 29, 511, the earlier source Schaefer 1996 extends.
  <https://articles.adsabs.harvard.edu/pdf/1988QJRAS..29..511S>, HTTP 200.

Both Schaefer PDFs are ADS scans with no text layer. The 1996 paper's PDF
is 184666 bytes over 10 pages, and running `pdftotext -layout` on it
extracts 210 bytes total, which is the ADS bibcode stamp repeated once per
page, not the paper's own text. Neither Schaefer PDF was usable directly.
TN69's own transcription of the 295 observations into its Table 4, crediting
Schaefer, is the practical source, and it is what this fixture is built
from.

## Extraction recipe and parse checksums

`tests/fixtures/yallop/extract.py` parses Table 4 from the ICOP PDF with
`pdftotext -layout`, normalises the Unicode minus and the middle-dot decimal
separator the ICOP scan uses, and deduplicates on TN69's own observation
number, since continuation pages repeat the table header. Its `--self-check`
mode asserts two parse checksums before writing anything: 295 unique
observation numbers, and 166 rows with `q_yallop` above +0.216, which
matches the count TN69 itself prints in its own zone-A section header. A
third checksum, added once the date-convention defect below was found,
asserts the offset between the derived and printed date is 0 or -1 on every
evening row and never any other value, since a misparse of either source
column would produce a wild offset. All three checksums passed on the run
this fixture was built from.

`tests/fixtures/yallop/compare_q.c`, built to `build/compare_q`, evaluates
every evening row through the live library and reports the residual against
`q_yallop`. `extract.py --emit-fixture` then pairs the corrected extraction
with `compare_q --emit-zones`'s output, maps the observed code through TN69
item 17, drops every HMNAO-computed column and the TN69 observation number,
and writes the committed fixture. The full commands are in
`tests/fixtures/yallop/README.md`.

## Licensing

HMNAO holds copyright on TN69's own computed columns: ARCL, ARCV, DAZ, age,
lag, parallax, crescent width W', q, and the predicted outcome code BDY.
Commercial use of those columns needs an HMNAO licence. libmuslim is MIT
and must stay usable commercially without anyone seeking a further licence,
so no HMNAO-computed column is committed anywhere in this repository. The
committed fixture carries only observed facts, the date, latitude,
longitude, and the observer's own coded naked-eye or optical-aid result,
plus this library's own `hijri_yallop_classify` output recomputed from that
data. Yallop's own `q` and ARCV/width values exist only transiently in
scratch CSVs consumed by `compare_q`'s residual gate, and are never written
to a tracked file.

## The Yallop comparison

271 evening observations were checked. TN69 Table 4 holds 295 observations
in total, and the 24 morning observations were excluded because this
library exposes only `hijri_yallop_evaluate_evening`, so this validation
covers evening first-sighting only, not TN69's full table and not morning
sightings.

Every one of the 271 evening rows reproduces its own printed `q` from its
own printed ARCV and W' to within 0.027, which shows the extraction itself
is not the source of any disagreement measured below.

The maximum absolute `q` residual between this library's computed `q` and
Yallop's own published `q` is 0.053577, on the worst row, 1984-08-28,
latitude 15.6, longitude +35.6. No row produced NAN. The narrowest gap
between adjacent zone boundaries is 0.072, between -0.232 and -0.160, so a
residual below that gap does not by itself account for every observed zone
disagreement, and each disagreement is checked individually below rather
than assumed explained.

Zone classification disagreed with Yallop's own zone on 36 of the 271 rows.
All 36 have Yallop's own `q` within that row's residual of a zone boundary,
and 0 are unexplained by boundary proximity. Read alongside the residual
above, a raw count of 36 disagreements out of 271 rows can look like the
coefficients were mistranscribed. They were not: every one of the 36 is a
row sitting close enough to a boundary that the measured residual alone is
enough to cross it, not a row where the two `q` values diverge by anything
close to a full zone's width.

One consequence of these boundary crossings is worth naming directly, since
it is the one a reader checking the fixture is most likely to trip over.
This library's zone E contains one naked-eye sighting, and Yallop's zone E
contains none. The row is TN69 observation 278, 1990-02-25, latitude 35.6,
longitude -83.5, observed code `V(V)`. Yallop's own `q` for it is -0.222,
which he classifies as zone D, and which sits 0.0100 from the D and E
boundary at -0.232, well inside the measured residual. Under this library's
computed `q` the row falls just past that boundary, into zone E. It is a
boundary flip caused by evaluation-instant differences, not a defect in
the outcome mapping, and Yallop's own classification already disagreed
with the observer on this row before this library's residual moved it
again: zone D means the crescent needs optical aid, and the observer saw
it unaided.

### Frozen baseline, not a live comparison

The comparison above ran once, as a local scratch process against a
downloaded copy of the ICOP PDF, which is a third-party copyrighted scan
and is not committed to this repository. What is committed is its output:
`tests/fixtures/yallop/tn69-observations.csv`, carrying only observed facts
and this library's own recomputed zone, and `tests/test_yallop_tn69.c`,
which asserts each row's computed zone and the aggregate counts below
against that frozen fixture. `make check` runs that test on every build,
and it will catch a code change that shifts a zone boundary against the
frozen fixture. It does not re-run the comparison against Yallop's
published `q` values, and it does not re-download or re-parse TN69. A
reader who assumes CI is continuously checking this library against
Yallop's real observations would be wrong. CI checks this library against
its own past output, recorded once, on this date, by this process.

## The date-convention defect in TN69 itself

TN69's own printed date column is not consistent between rows. On most
evening rows it is the local evening date `hijri_yallop_evaluate_evening`
expects. On a small number of evening rows it instead holds the UT date of
the Moon's best time, which runs a day behind the local evening date at
longitudes west of Greenwich once the time-of-day crosses midnight UT. The
same observing site, two months apart, is seen using both conventions in
different rows, so the inconsistency is not tied to site or observer, only
to the individual row.

This is a defect in TN69 as a source table, not in this library or in the
extraction. The fixture therefore carries a local evening date derived from
TN69's own column 6, the Julian Date of astronomical new moon minus
2 400 000, and column 12, the age of the Moon in hours at best time, rather
than TN69's printed date. `hijri.h:1493-1513` documents this library's own
solar-day evening convention, which is what the derivation targets. The
derivation shifted 5 of the 271 evening rows relative to TN69's printed
date, and they are listed by date and coordinates in
`tests/fixtures/yallop/README.md`, since TN69's own observation numbering
is the compilation's arrangement and is not committed.

## Fixture zone distribution and outcome table

Measured over the 271 committed evening rows, using this library's own
`hijri_yallop_classify` result:

| zone | total | seen_unaided | seen_optical_aid | not_seen |
| --- | --- | --- | --- | --- |
| A | 141 | 132 | 3 | 6 |
| B | 69 | 51 | 1 | 17 |
| C | 24 | 6 | 10 | 8 |
| D | 14 | 0 | 5 | 9 |
| E | 7 | 1 | 3 | 3 |
| F | 16 | 0 | 2 | 14 |

Seen-unaided fraction per zone: A 0.9362, B 0.7391, C 0.2500, D 0.0000,
E 0.1429, F 0.0000. This is not monotonically decreasing from A through F,
since zone D's fraction sits below zone E's. `tests/test_yallop_tn69.c` records
the measured counts and the non-monotonicity rather than asserting a
monotonic relationship the data does not show.

The zone E naked-eye row is the same row discussed above under the Yallop
comparison, TN69 observation 278, 1990-02-25, latitude 35.6, longitude
-83.5, code `V(V)`.

### The zone table at `docs/research/2026-07-30-findings.md:71-82` is wrong

That table's totals are 166, 68, 45, 27, 8, 31, which sum to 345. That is
the raw row count before deduplication on TN69's observation number, not
the 295 observations TN69's own prose states its criteria were calibrated
against. Only that table's zone A figure of 166 is corroborated, since it
matches the parse checksum against TN69's own printed zone-A count. The
rest of that table should not be relied on, and the table in this note, over
the deduplicated 271-row evening fixture, supersedes it for evening rows.

## The observed-outcome code legend, quoted from TN69 item 17

The `observed` column's three values, `seen_unaided`, `seen_optical_aid`
and `not_seen`, are not an inference drawn from the single-letter codes.
TN69 item 17 states the legend directly:

> If the only character is a "V", then the Moon was visible to the unaided
> eye. An "I" means it was not seen with the unaided eye. If the first
> character is followed by (F) then optical aid was used to find the Moon,
> which was then spotted with the unaided eye. If the first character is
> followed by (B) or (T) it was visible with binoculars or a telescope,
> respectively. In the second and third papers, the rules were changed as
> follows: If the first character is followed by (I) it was invisible with
> either binoculars or a telescope. If the first character is followed by
> (V) it was visible with either binoculars or a telescope.

`seen_unaided` covers `V`, `V(F)`, `V(B)`, `V(T)` and `V(V)`, since `V(F)`
and the visible-with-aid codes still record the unaided eye as having seen
it or, for `V(F)`, spotted it once optical aid found it. `seen_optical_aid`
covers `I(B)` and `I(V)`, visible only with aid. `not_seen` covers `I` and
`I(I)`.

## Fixture key structure

The plan this cycle was built from assumed date, latitude and longitude
together form a unique key over the fixture. Measurement shows they do not:
the 271 evening rows carry 248 distinct date-latitude-longitude keys, 17 of
those keys cover 40 rows between them, and the largest single group holds 5
rows. This is corrected here since no other tracked document states it.

It is harmless to this fixture's own use in `tests/test_yallop_tn69.c`
for two reasons. First, no duplicated key's rows disagree on zone, so the
per-row zone assertion's diagnostics stay unambiguous even where a key
repeats. Second, each of the 5 rows listed above under the date-convention
defect does have a unique key, so that correction is not confused by the
duplication. Seven of the duplicated keys do differ on the observed
outcome, and that difference is genuine rather than a parse artifact:
several observers reported different results for the same event at the
same recorded position.

## Odeh remains unvalidated

Odeh's coefficients in this library have no external validation of any
kind, unlike Yallop's, which this note's comparison now checks against a
real observational record. Odeh's coefficients rest on secondary
transcription. The original source, *Experimental Astronomy* 18 (2004)
39-64, is paywalled by Springer, and every apparently open link to it
redirects to an authentication page rather than the paper. See issue #20,
which tracks obtaining that paper. Until it is obtained and a comparison
like the one in this note is run against it, nothing in this repository
checks Odeh's coefficients against an independent source.

**SUPERSEDED (2026-08-16):** This note was accurate when written earlier today.
The Odeh paper was subsequently obtained from
<https://astronomycenter.net/pdf/2006_cri.pdf>, the website of the Islamic
Crescent Observation Project, the author's own organisation. The library's Odeh
coefficients and zone thresholds match the source document verbatim. See
[`docs/research/2026-08-16-odeh-primary-source.md`](2026-08-16-odeh-primary-source.md).

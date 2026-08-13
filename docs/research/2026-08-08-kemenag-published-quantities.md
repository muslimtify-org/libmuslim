# Kemenag's published hilal quantities, 2023

**Date:** 2026-08-08
**Source:** *Ephemeris Hisab Rukyat 2023*, page 604
**Fixture:** `tests/test_hijri.c`, `KEMENAG_PUBLISHED_2023`, `test_kemenag_published_quantities`

## The source

*Ephemeris Hisab Rukyat 2023*, Direktorat Urusan Agama Islam dan Pembinaan
Syariah, Direktorat Jenderal Bimbingan Masyarakat Islam, Kementerian Agama RI,
2022. Page 604 carries the table "DAFTAR WAKTU IJTIMAK TINGGI HILAL ELONGASI
PENENTU AWAL BULAN HIJRIAH TAHUN 2023 M", which lists, for each of the twelve
Hijri month boundaries falling in 2023, the ijtimak time and a RANGE of hilal
altitude and a RANGE of elongation across Indonesia.

The book's own footnote states that the data conforms to the new MABIMS
criteria and comes from the Sinkronisasi Data Hisab Taqwim Standar Indonesia
held 20 to 22 October 2021. That footnote is what makes these rows an authority
statement about Kemenag's own computation rather than one team's private
calculation. Everything else in this project's Kemenag notes works from the
OUTPUT of that computation, the announced month starts. These rows are the
INPUT quantities the decision is made from, which is why they can separate
conventions that a yes or no answer cannot.

The published figures describe the sunset of the ijtimak day, not the sunset of
the announced month start. Sya'ban 1444 is the clearest case: ijtimak falls
Monday 20 February, the published altitude is below the 3 deg threshold, so
Kemenag applied istikmal and the month began Wednesday 22 February. Sampling
the announced start would read the wrong evening and still produce plausible
numbers.

One row is an exception. Zulqa'dah 1444 carries a printed footnote, "Posisi
hilal pada tanggal 20 Mei 2023 M / 29 Syawal 1444 H". Its ijtimak falls at
22:53 WIB, after sunset, so its published figures describe the FOLLOWING
evening. The fixture samples 20 May for that row and the ijtimak date for every
other. Mutation K5 in `tests/test_hijri.c` mis-dates that row back to 19 May
and breaks both its altitude and its elongation assertion, so the exception is
load-bearing rather than decorative.

## Why containment

The published quantities are ranges over Indonesian territory, and the location
that produces the extreme moves from month to month, because "di Indonesia"
means the extreme falls wherever in the territory it happens to fall. No fixed
reference point reproduces the endpoints.

Containment needs no knowledge of where the endpoints are evaluated. Sabang is
inside Indonesia. If the library computes the same quantity Kemenag publishes,
then the library's value at Sabang must lie between the published minimum and
the published maximum. That is the entire argument, and it survives not knowing
the endpoint locations.

**An earlier analysis did something else and got the opposite answer.** It
fitted a single reference location to the twelve published maxima and compared
RMS across candidate altitude conventions, concluding that the shipped centre
geometric altitude fits WORST and that Kemenag's altitude is mar'i. That
conclusion is wrong. The fit assumed that one fixed location produces the
maximum in every month, and that assumption is false for an
anywhere-in-Indonesia rule. The procedure looked rigorous, produced a clean
ranking, and was unsound at its first step. It is recorded here as an error
rather than deleted, because the same shape of mistake is available to anyone
who reads a published range as if it were a published point.

## Results

Containment at Sabang over the twelve published rows, printed by the temporary
measurement harness before any tolerance was pinned:

```
kemenag_published altitude    contained 12 of 12, worst 0.0000 deg
kemenag_published elong_topo  contained 11 of 12, worst 0.0106 deg
kemenag_published elong_geo   contained 1 of 12, worst 0.9637 deg
```

The altitude convention sweep, measured separately over the same rows:

```
  centre geometric (shipped)          contained 12 of 12
  upper limb geometric                contained  8 of 12
  mar'i, upper limb plus refraction   contained  2 of 12
```

The altitude assertion in the fixture carries no tolerance at all. It does not
need one.

The elongation assertion carries `TOL_KEMENAG_PUBLISHED_ELONG_DEG`, pinned at
0.022 deg, which is the measured 0.0106 rounded up to a 2.08x margin. The
single excursion is Safar 1445, where the library reads 4.0958 deg against a
published minimum of 4.1063 deg. Sabang sits at the elongation MINIMUM in that
row, where in every other row it sits near the maximum, so the residual is the
library's own elongation error rather than a convention difference. Scanning
the region rectangularly for that row gives a library span of 4.094 to 4.624
deg against the published 4.106 to 4.468.

Adding the fixture moved the suite from 502 checks to 539.

Mutations K1 through K5 were all executed and all caught, and their verbatim
FAIL lines are recorded beside the fixture in `tests/test_hijri.c`. K2 is the
one worth naming here: it swaps the asserted altitude to
`moon_upper_limb_apparent_altitude_deg` and breaks 10 of 12 rows. That quantity
is the mar'i one, upper limb plus refraction, which is why 10 of 12 broken
matches the 2 of 12 containment figure above and not the 8 of 12 figure for
upper limb geometric.

## What this closes

**The mar'i hypothesis is refuted.** `2026-08-01-kemenag-reference.md` recorded
that Kemenag's altitude is "likely mar'i (apparent)", inferred from a roughly
10.6 arcminute excess over the library's geometric value at one announced
maximum, and recorded that inference as unconfirmed by a computation manual and
as the open item blocking any future Kemenag-operational predicate. The manual
is now read. The hypothesis does not survive it. The shipped centre geometric
quantity is contained in 12 of 12 published ranges, mar'i in 2 of 12.

**The elongation frame is resolved to topocentric.** The same file recorded the
topocentric versus geocentric question for Kemenag's own implementation as
open. Against the published ranges the topocentric elongation is contained in
11 of 12 rows and the geocentric elongation in 1 of 12, worst excursion 0.9637
deg. The fixture asserts the geocentric FAILURE deliberately, as a majority
condition rather than a transcribed count, so that a future simplification of
`HIJRI_PREDICATE_MABIMS_2021` to the geocentric elongation is caught.

Both closures come from the same correction. The earlier readings compared a
library value at Sabang against a published ENDPOINT, which is only meaningful
if Sabang is where that endpoint is evaluated. Containment drops that
assumption and reverses both answers.

## What remains open

Where Kemenag evaluates the range endpoints is still unknown. The book prints
the range without naming the locations that produce its minimum and its
maximum, and nothing measured here identifies them.

This is a note and not a blocker. The containment design does not use the
endpoint locations, which is precisely why it is the test that could be built
from what the source actually states.

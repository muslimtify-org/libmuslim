# Odeh primary source, and the observation-level gap that remains

## Question

`hijri.h` carried a claim that Odeh's paper was unreachable and that its
coefficients rested on secondary transcription. This note records what
happened when the primary source was actually retrieved, what matched, and
what still does not exist, which is any check against Odeh's own 737
observation records.

## Source, retrieved 2026-08-16

- Odeh, M.S., "New Criterion for Lunar Crescent Visibility", Experimental
  Astronomy (2004) 18: 39-64, DOI 10.1007/s10686-005-9002-5.
  <https://astronomycenter.net/pdf/2006_cri.pdf>, the site of the Islamic
  Crescent Observation Project, the author's own organisation.

Springer paywalls its own copy of the paper. That paywall is real, but it
is irrelevant here, because the paper's own author publishes the same
paper openly through his own organisation's site. The earlier "no external
validation, paywalled, every link redirects to authentication" claim in
`hijri.h` was wrong on every clause after the first: the source is not
inaccessible, and it was never actually checked against.

## Equation and thresholds, as the paper states them

Equation (2):

> V = ARCV - (-0.1018 W^3 + 0.7319 W^2 - 6.3226 W + 7.1651)

Zones:

> V >= 5.65: Zone A
> 2 <= V < 5.65: Zone B
> -0.96 <= V < 2: Zone C
> V < -0.96: Zone D

The paper defines ARCV as the airless, topocentric arc of vision in
degrees, and W as the topocentric crescent width in arc minutes.

## Verbatim match against this library

`hijri.h:1783` (`hijri_odeh_v`) carries the same four coefficients, -0.1018,
0.7319, -6.3226, 7.1651, and `hijri.h:1863` (`hijri_odeh_classify`) carries
the same three thresholds, 5.65, 2.0, -0.96. All seven values match the
paper verbatim. No coefficient or threshold was changed by this check, none
needed to be.

## The topocentric frame, confirmed by a published anchor

The paper's ARCV and W are both topocentric, not geocentric, and the
library's inputs are topocentric to match. That is not just a reading of
the paper's definitions, it is confirmed against a published anchor from
Odeh's own record 274 (Pierce), 1990-02-25 23:55 UT, latitude 35.6,
longitude -83.5, elevation 0.0.

Measured against the library:

- Topocentric elongation: 7.646658966843 degrees.
- Geocentric elongation: 8.634165964619 degrees.
- Odeh's published value: 7.7 degrees.

The topocentric figure lands close to Odeh's published 7.7 degrees, and the
geocentric figure does not, which is the anchor that confirms the frame the
library uses is the frame the paper uses.

The library's geocentric new moon for this event falls at JD
2447947.871761254035, which is 80.17 seconds from Odeh's published
1990-02-25 08:54 UT.

## What still has no validation

Odeh's Table VI holds 737 observation records. It does not extract as text.
Running `pdftotext -layout` over the 27 page PDF yields zero rows in the
table's own date format, because those pages are images, not text. This is
why no observation-level validation exists for Odeh's coefficients, unlike
the row-by-row check this repository ran against Yallop's TN69 fixture. See
issue #47 for that gap.

## The asymmetry

Yallop and Odeh are validated in opposite senses, and getting this backward
is the natural mistake.

Yallop's coefficients were checked row by row against 271 evening
first-sighting observations from TN69 Table 4, and the residuals and
disagreements from that check are recorded in
`docs/research/2026-08-16-yallop-tn69-validation.md`. But Yallop's own
coefficients, as this library carries them, were never checked against
HMNAO's own statement of them, only against TN69's transcription.

Odeh is the reverse. Its coefficients and thresholds are now verified
verbatim against the paper itself, obtained from the author's own
organisation. But there is no observation-level validation at all, because
the 737-record table that would provide it does not extract as text.

Neither model is complete in the other's sense. Yallop has observation
coverage without a primary-source coefficient check. Odeh has a
primary-source coefficient check without observation coverage.

## Two traps

First, the paper's equation (2) states W in arc minutes, while its Table VI
column 18 legend states arc seconds. This library uses arc minutes,
matching the equation, not the table legend. A future reader pulling
numbers from Table VI directly, rather than from the equation, could get
this backward.

Second, Odeh's record 274 (Pierce), 1990-02-25, latitude 35.6, longitude
-83.5, is the same observation this repository's TN69 fixture carries,
committed in PR #45, which is how the record's coordinates are known here.
Odeh identifies record 274 as holding two distinct records at once: the
youngest crescent detected with the naked eye, at a topocentric age of 15
hours 33 minutes, and the minimum naked eye elongation, at 7.7 degrees.
This double identity, an extreme on two different axes at once, is why the
row sits outside both Yallop's zone prediction and this library's own,
rather than being an ordinary boundary case.

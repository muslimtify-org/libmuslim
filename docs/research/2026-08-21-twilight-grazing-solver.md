# The twilight grazing band was never a convergence problem

## Question

Issue #79 recorded that `prayertimes.h` left up to 30 arcmin of residual, roughly 6 minutes of time, where the Sun only grazes the target depression, and that iterating `refine_event` to convergence changed it by nothing at all. It proposed solving for the instant of closest approach, which exists even when no crossing does.

That target turned out to be wrong, and the reason is worth recording.

## The split that reframed it

Splitting the 186 grazing points the twilight oracle reports, by whether a root actually exists:

```
grazing points                              186
no root exists, library reported one anyway  45  (worst 30.1552 arcmin)
root exists, solver missed it               141  (worst  6.6283 arcmin)
```

The worst case, latitude 70 on 2025-03-27 under MWL:

```
isha reported      23.974770 h
altitude there     -16.989774 deg (target -17.0)
solar midnight     24.090710 h, altitude -16.9965
```

The Sun's deepest depression that night is -16.9965 degrees. It never reaches 17. There is no isha and the library printed one.

So the headline 30.1964 arcmin was not a precision figure. It was the distance to an event that was not there. Closest approach was the wrong thing to solve for, because when no root exists the correct answer is the high-latitude substitution, which the header already implements and simply was not reaching.

## Root cause

`prayertimes.h` answered two different questions from two different instants.

`hour_angle_safe` decided whether an event exists using the declination at 0h UT. `refine_event` decided when it happens by iterating with the Sun evaluated at the event. Near the seasonal boundary they disagree, and the existence test won by default because nothing rechecked it.

`solve_event` brackets local noon to solar midnight and bisects the true solar altitude. Altitude falls monotonically between them, so a sign change means exactly one root and the absence of one means the event does not occur. Existence and instant come from one function at the same instants. It is the technique `hijri.h` already uses, which is why `hijri.h` is trustworthy enough to be the oracle here.

## Why existence is gated on both tests, not one

Replacing the 0h UT test outright broke the published-table gate.

At London on 2026-07-15 the deepest solar altitude is -17.1033 degrees, so the Sun does reach MWL's 17 degrees, at 00:49. The published table gives 23:25, the angle-based substitution. The reference implementation the fixtures come from uses the coarser test and falls back.

```
decl at 0h UT              +21.549496
hour_angle_safe at 0h UT   FAILED (no event)
solve_event at own instant 24.8177 h  (alt there -17.00000)
```

An event now counts as occurring only when both tests agree. Whichever declines wins.

The cost is real and stated rather than hidden: on grazing days the header follows a published convention rather than the criterion's own definition. That is the right trade for a library whose purpose is reproducing published calendars, and the wrong one for a library whose purpose was astronomical truth.

## Result

```
twilight grazing   186 points, max 30.1964 arcmin  ->  160 points, max 0.1970
polar grazing      172 points, max  9.3160 arcmin  ->  169 points, max 0.4556
```

Both bands now sit below their own comfortably-solved maxima, 0.2312 and 0.4680. The grazing split is kept because the physics is still real, not because the numbers still differ.

All 702 published-table comparisons are byte-identical, diffed line by line against a capture taken before any of the work.

## What the horizon events had too

The same defect, at the horizon rather than at twilight.

At Longyearbyen on 2025-04-18 the 0h UT hour angle says the Sun sets. Its lowest point that day is -0.6082 degrees, above the -0.833 that defines sunset, so it does not. That is the first day of the midnight sun and the header reported a maghrib for it. Murmansk has the same thing on 2025-05-21 at -0.6572.

The polar decision now reads whichever test declined. Deciding it from the 0h UT hour angle alone left those days with maghrib unavailable while fajr and isha took substitutions, which is the split schedule issue #68 exists to prevent: the reference latitude is borrowed for the whole day or not at all.

## Two things the work found that were not the subject

**The oracle was measuring nothing on some days.** Both oracle checks decided a day was polar by re-deriving the 0h UT test. Once the header had a second route to that branch, they compared a schedule borrowed from 45 degrees against the true location's angles and read up to 1893 arcmin. They now mirror the header's own condition through one shared helper. A check that re-derives what it is testing rather than asking it drifts silently.

**Seven pinned counts encoded the bug.** Reykjavik out-of-range days 107 to 106, Anchorage 23 to 22, Murmansk under Russia 112 to 113, Longyearbyen under Kemenag 244 to 245 with fajr 128 to 129 and maghrib 240 to 241. Every one of them counted a day where the Sun never reaches the angle, or never sets at all. A number pinned from observation records whatever the code did at the time, including the parts that were wrong.

## And one about the test that pins this

The first version of the regression test asserted that the Sun falls short, that no root exists, that a time is still reported and still follows maghrib. All true, and all of it passes with the fix removed.

Mutation found it. Removing only the `solve_event` guard, so existence is decided at 0h UT alone while the instant is still bisected, left all five green. Two checks were added that discriminate: the reported isha must equal what `high_lat_substitute` produces and must not equal the crossing the 0h UT hour angle still offers. Under the same mutant both fail.

A test written from the facts of a case tends to assert the case rather than the fix.

# What bounds hijri.h geographically, measured

## Question

Issue #27 asks for the supported date and geographic range as a contract rather than a description of whatever the oracle happened to cover. The date half was answered in #78. Every measurement in it is at Mecca, so it says nothing about latitude.

This note answers the other half. The short version is that the geographic answer has a different shape from the date answer: for dates the binding limit is margin, and for geography it is availability.

## What was measured

Ten years of evenings, 2020-2029, longitude 0, elevation 0, MABIMS 2021, at fourteen latitudes. For each evening, whether the caller gets a verdict at all, and if so how close the closest criterion term sits to its threshold.

The tool is `hijri_compute_evening_parameters` followed by `hijri_predicate_margins`, which is the path a caller takes.

## Latitude

```
  lat  evenings   verdict no sunset no moonset in error bar
    0      3653    96.58%     0.00%      3.42%           0
   10      3653    96.63%     0.00%      3.37%           1
   20      3653    96.63%     0.00%      3.37%           0
   30      3653    96.66%     0.00%      3.34%           0
   40      3653    96.69%     0.00%      3.31%           0
   50      3653    96.69%     0.00%      3.31%           0
   60      3653    96.82%     0.00%      3.18%           2
   63      3653    88.20%     0.00%     11.80%           0
   66      3653    67.12%     4.76%     28.11%           0
   70      3653    34.08%    33.97%     31.95%           1
   75      3653    16.18%    54.91%     28.91%           1
   80      3653     6.71%    71.20%     22.09%           0
   85      3653     1.51%    85.93%     12.57%           0
   89      3653     0.05%    97.24%      2.71%           0
```

Flat to latitude 60, then a collapse. Nothing in that collapse is a defect. Above the polar circles the Sun does not set for part of the year, and a criterion that thresholds the Moon's altitude at sunset has nothing to threshold. The library reports that rather than computing something.

Worth stating plainly because it is easy to misread the table as degradation: the arithmetic does not get worse with latitude. The events stop existing.

## The 3.4 percent at the equator

The first surprise. At latitude 0, one evening in thirty yields no verdict, and the reason has nothing to do with geography.

`hijri_find_moonset` scans 24 hours forward from sunset. The Moon sets once per 24h 50m. When it sets just before sunset, the next moonset falls 24h 50m later, outside the window. That happens everywhere on Earth at roughly the same rate, Jakarta and Mecca included.

Before #82 this was reported as `HIJRI_EVENT_NEVER_RISES` or `HIJRI_EVENT_NEVER_SETS`, one of which was always false. The committed research baseline had 60 such rows frozen into it, including `NEVER_RISES` for the Moon at Jakarta on an evening when it spanned -77.46 to +64.41 degrees. It is now `HIJRI_EVENT_NOT_FOUND`.

## Margin does not vary with latitude

The second surprise, and the one that makes this answer different from #78's.

Counting evenings where a MABIMS 2021 term sits within 0.0070 degrees of its threshold, the worst topocentric elongation error measured against DE440: between 0 and 2 per 3653 evenings, at every latitude from 0 to 89, with no trend.

That rate matches what #78 found at Mecca across eight centuries, 4 to 10 per 33600 evenings. So the coin-toss band is a property of the criterion and the ephemeris, not of where the observer stands.

For dates, margin was the binding constraint and the epoch was not. For geography it is the reverse. Margin is flat and availability is the constraint.

## Longitude

`hijri.h` carries no zone database and derives local midnight from longitude as mean solar time, so the evening of date D is the solar-day evening rather than the civil-day one. The question is whether that ever lands on a different civil day.

Every evening of 2025 at the zones furthest from their solar meridian:

```
zone         solar-civil gap   evenings   landing on another civil day
Kashgar           -2.93 h          365          0
Adak              -2.78 h          365          0
Vigo              -2.58 h          365          0
A Coruna          -2.56 h          365          0
Urumqi            -2.16 h          365          0
Anchorage         -1.99 h          365          0
Mecca             -0.34 h          365          0
Jakarta           +0.12 h          365          0
```

Never. Sunset sits far enough from midnight that a three hour offset does not reach a date boundary. The largest offset any inhabited zone carries is under 3.5 hours, so the margin is real rather than lucky.

This extends what `test_local_evening_date` already asserts. That test checks six zones on one date, the June solstice, chosen as the worst case for northern sunsets. This checks eight zones on every date.

The approximation is still worth documenting, because a caller who needs civil-day semantics has to resolve the offset itself and use the JD-based entry points.

## Elevation

Not measured here, because the answer is already in the header and is deliberate. `loc->elevation_m` does not lower the sunset target and does not correct parallax.

Both omissions have been measured twice, and both times the correction was the error. For Wujudul Hilal the dip cancels, because the Pedoman applies it to sunset and to the altitude, so adding it on one side lands 17 arcmin out. For MABIMS 2021 it does not cancel, and applying it drops official Kemenag month starts supported from 33 of 37 to 32. See `2026-08-01-wujudul-hilal-convention.md` and `2026-08-05-solar-hour-angle-frame.md`.

## The contract

Use `hijri.h` freely to latitude 60. Better than 96 percent of evenings yield a verdict, and the rest are the Moon's own period rather than anything geographic.

Between 60 and the polar circles, expect a growing share of evenings with no answer, 88 percent yielding a verdict at 63 and 67 percent at 66.

Above the polar circles, expect most evenings to have none. Check the status before reading the value.

What a calendar should do when its criterion cannot be evaluated is not an astronomy question and this library does not answer it. Issue #51 is the same question for prayer times and is still open.

## What this depended on

Every number here was measured after #82. Before it, the statuses were unreliable often enough that the availability table would have been measuring the bug rather than the geography, and the two columns "no sunset" and "no moonset" could not have been separated at all.

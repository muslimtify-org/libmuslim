# High-latitude conventions for fajr and isha, and why the current fallback cannot work

## Question

`prayertimes.h` returns NaN for several fields above 66 degrees latitude, and its fajr and isha high-latitude fallback returns NaN on exactly the days the fallback exists to cover. Issue #51 asks what an event should be when the Sun never reaches its altitude. That question cannot be answered from the code. It is a convention question with published authority behind more than one answer.

This note records what the sources actually say, which of them were cross-checked, what the library currently implements and where that came from, and what each candidate produces when measured.

## What the library implements today, and its provenance

`prayertimes.h:591` and `prayertimes.h:612` guard fajr and isha with `hour_angle_safe`, and on failure fall back to

```c
fajr = sunrise - (params->fajr_angle / 60.0) * night;
isha = sunset  + (params->isha_angle / 60.0) * night;
```

where `night` is defined at `prayertimes.h:585` as

```c
double night = (24.0 - sunset) + sunrise;
```

This is the angle-based method as published at <http://praytimes.org/calculation>, which defines it as dividing the period between sunset and sunrise into `t` parts with `t` equal to the twilight angle divided by 60. The correspondence is exact, including the divisor.

That page is the primary source for its own method, since the method originates there. It names no author and it cites no fiqh authority for any of its three high-latitude methods. Its reference list covers Monzur Ahmed, the US Naval Observatory and Tariq Muneer, none of which is cited in support of the high-latitude section. So the rule the library currently follows above 66 degrees has a computational provenance and not a jurisprudential one. That is worth stating plainly, because the alternatives below do have one.

## Why all three praytimes.org methods fail in the polar case

The same page defines its other two options as follows. Middle of the night divides the period from sunset to sunrise into two halves. One seventh of the night divides the period between sunset and sunrise into seven parts.

Every one of the three, including the angle-based method the library uses, defines its unit of measure as the interval between sunset and sunrise. Under a true midnight sun there is no sunset and no sunrise, so that interval does not exist. None of the three is defined in the case where the library invokes it.

This is the root cause of the defect measured in #51. The guard on fajr and isha converts an `acos` domain error into a fallback whose own input is NaN, one line later. Switching between the three praytimes.org methods would not fix it, because all three share the undefined quantity.

## The Islamic Fiqh Council decree, ninth session, 1406 AH

The Islamic Fiqh Council of the Muslim World League addressed exactly this question. The decree divides the world into three zones.

- Between 45 and 48 degrees north and south, the visible signs of all prayer times occur within 24 hours, and the prescribed times are to be observed directly.
- Between 48 and 66 degrees north and south, where the signs for isha and fajr are seasonally absent, those two times are set by proportional measurement by analogy with the corresponding times in the night of the nearest region where the signs are distinguishable. The Council proposes latitude 45 degrees as that region.
- Beyond 66 degrees to the poles, all the times are set by proportional measurement by analogy with the corresponding times at latitude 45 degrees, by dividing the 24 hours in the same way the times are divided at 45 degrees.

The third clause is the one that matters here. It supplies a rule for the case where sunrise and sunset do not exist, by replacing the missing night with a division of the full 24 hours taken from the reference latitude. This is precisely the quantity the praytimes.org family lacks.

### Cross-check status of this decree

The substance above is attested by three renderings that are independent of one another and agree.

- The Muslim World League's own site, <https://ar.themwl.org/node/48>, in Arabic, which carries the 48 to 66 degree ruling and a later clarification from the twenty-first session, 24 to 28 Jumada al-Awwal 1434 AH, corresponding to 8 to 12 December 2012.
- IslamOnline's fiqh section, <https://fiqh.islamonline.net/en/praying-and-fasting-at-high-latitudes/>, in English, which gives both the 48 to 66 and the beyond 66 clauses in the proportional-measurement form quoted above.
- The collected resolutions volume indexed at <https://ketabonline.com/ar/books/24407>, covering the first through nineteenth sessions, where the item appears as Resolution No. 46, titled "بشأن مواقيت الصلاة والصيام في البلاد ذات خطوط العرض العالية".

A fourth rendering, an anonymous English translation at <https://birka.nur.nu/prayertimes/prayertimes-references/quoted/Rabita1406-transl.html>, agrees on the three zones and the 45 degree reference. It is not relied on here, for two reasons. It names no translator, and it cites as its own source a now-defunct GeoCities page. It also labels the item "The Eighth Decree" where the Arabic sources call it the sixth decree of the ninth session and the collected volume numbers it 46. That numbering conflict is unresolved and does not affect the substance.

### The limitation to record honestly

No verbatim full text of the ninth-session decree was obtained from an official source in this pass. The Muslim World League page that was retrieved carries the twenty-first session clarification rather than the original decree, and the collected-resolutions site exposed its index rather than the body. What is established here is the substance of the three zones and the 45 degree reference, agreed across independent renderings. What is not established is exact wording. Any implementation should not depend on a clause read more finely than that.

## Naming, and the second convention

The literature names two distinct substitutions.

- Ittiba' aqrab al-bilad, following the nearest locality, sets the missing time by reference to the nearest place where the sign does occur. The Fiqh Council decree is a form of this, with the reference fixed at 45 degrees rather than left to the reader.
- Ittiba' aqrab al-ayyam, following the nearest day, sets the missing time by reference to the last day on which the sign did occur at the same place.

The European Council for Fatwa and Research has treated the question for European latitudes and has indicated that it sees no harm in relying on estimations made by other fatwa bodies, including a 12 degree solar depression for fajr and isha. Its own session pages at <https://www.e-cfr.org> returned HTTP 403 to automated retrieval in this pass, so the ECFR position is recorded here as reported rather than as verified, and it should not be implemented on the strength of this note alone.

## An independent computational treatment

Andriyani, M., Nurwilda, S., Sofiyah, S., Wulandari, A., Mergianti, W.N., and Ulinnuha, N., "Islamic Prayer Time Calculation in High-Latitude Summer: A Computational Approach Using the Ittiba' Aqrab al-Bilad Method in Iceland", Al-Hilal: Journal of Islamic Astronomy, 2026, vol. 8 no. 1, DOI 10.21580/al-hilal.2026.8.1.29337.

The abstract states the defect in the same terms this note reached independently from the code.

> This condition causes conventional solar-angle-based methods to produce
> undefined (NaN) values for Fajr and Isha.

The paper applies aqrab al-bilad for Reykjavik using London as the reference region, and reports mean deviations of 14 to 15 minutes against established applications, with the larger discrepancies on isha. Note that its reference choice, London at about 51.5 degrees, is not the Fiqh Council's proposed 45 degrees. The choice of reference latitude is therefore a live parameter and not something the sources settle for us.

## Measured behaviour of the current code

MWL parameters, longitude 0, timezone 0, all 365 days of 2025, counting days on which a public field of `struct PrayerTimes` is NaN.

```
lat    fajr sunrise dhuha dhuhr asr maghrib isha
62.0      0       0     0     0   0       0    0
63.0      0       0    28     0   0       0    0
66.0     17      17    62     0   0      17   17
68.0     52      76    78     0   0      76   52
72.0     88     158   142     0   0     158   88
```

Re-deriving the internal `fajr_failed` flag alongside the NaN status of `ha_sunrise` isolates the cause with no residual.

```
lat   took fallback   fallback & sunrise NaN   fajr NaN
 64            144                        0          0
 66            156                       17         17
 68            166                       52         52
 72            186                       88         88
```

The last two columns are equal in every row. Between 60 and 65 degrees the fallback fires on up to 144 days a year and works, because bright nights still have a real sunrise and `night` is finite. Above 66 degrees it is NaN.

## Measured behaviour of the Fiqh Council rule

Prototyped against the same code, with latitude 45 on the same meridian and the same day as the reference. Two branches, matching the decree. Where a local night exists, fajr and isha take the same fraction of the local night that they take of the night at 45 degrees. Where it does not, the 45 degree schedule is transplanted about local solar noon, which is the decree's division of the 24 hours and is well defined because dhuhr never fails.

```
city           days needing a substitute   zone2   zone3   current NaN   rule NaN
Reykjavik  64.15N              146           146       0             0          0
Tromso     69.65N              174           105      69            69          0
Longyearbyen 78.22N            218            90     128           128          0
```

The zone 3 count equals the current NaN count exactly at both cities where it is non-zero, which is the same identity the fallback trace showed. The rule produces a finite value on every day at all three cities.

This is a measurement of behaviour, not an endorsement. It establishes that the decree's rule is implementable and total. It does not establish that it is the right convention for this library's users.

## What the published tables said when the rule was actually implemented

Added 2026-08-18, after implementing the Council rule rather than only prototyping it.

The section above establishes that the Council rule is implementable and total, and that is still true. It does not establish that it is what a published MWL timetable does, and implementing it showed that it is not.

Assigning `CALC_MWL` the proportional rule for the night-exists case broke three checks in the published-table suite, all London under MWL.

```
London 2026-06-15  isha   computed 23:22   published 23:25    3 min
London 2026-07-15  fajr   computed 02:55   published 02:40   15 min
London 2026-07-15  isha   computed 23:08   published 23:25   17 min
```

London is 51.5 degrees north, so in midsummer the Sun does not reach 18 degrees of depression and the substitution fires. The angle-based rule reproduces the published table to within 1 minute at these points. The Council rule is 15 to 17 minutes away.

The conclusion is not that the decree is wrong. It is that the decree and the timetables published under the MWL name are two different things, and the library validates against the timetables. Whoever computes those tables uses the angle-based convention, so that is what the MWL method means in practice wherever a night exists.

This is a halt condition in the sense the project uses the term, and it was resolved by changing the implementation rather than by widening the 2 minute tolerance that exposed it.

What survives is narrower and better supported. The decree is used only where the published tables have nothing to say, which is inside the polar circle, where there is no night to take a fraction of and every angle-based rule is undefined. `CALC_MWL` therefore carries `HIGHLAT_ANGLE_BASED` with a reference latitude of 45, and the reference is consulted only when sunrise and sunset do not exist.

Two lessons worth keeping. A rule being sourced does not make it the rule a given method's users receive. And a prototype that only shows a rule produces finite numbers has not been checked against anything, which is why the 15 minute disagreement appeared at implementation rather than at research time.

## A separate defect found while measuring

Independent of NaN, the current fallback returns hour values outside the range 0 to 24, and these reach the public struct. Measured at real cities with their standard-time offsets, over 2025.

```
city            NaN days   out-of-range days   min value   max value
Reykjavik            44                 107      24.000      25.075
Tromso              159                   2      -0.104      -0.100
Stockholm             0                   0
Anchorage             0                  23      24.002      24.626
Longyearbyen        268                   0
```

Anchorage matters most here. It has zero NaN days, so it is untouched by #51, yet it returns an out-of-range hour on 23 days a year. This defect reaches a wider set of locations than the NaN does.

`format_time_hm` masks the positive case and corrupts the negative one.

```
   25.075 -> "01:05"
   24.626 -> "00:38"
   24.000 -> "00:00"
   -0.104 -> "00:-6"
```

The first three are the next day's time printed without any indication that the day rolled over. The last is a malformed string. Bindings that convert the raw double into a date-time value, which both the Rust and Dart bindings do, will land on the wrong calendar day for the positive cases rather than merely printing oddly.

This should be tracked separately from the convention question, because it needs no fiqh decision at all.

## Whether the rule belongs to the method

The rule should follow the authority being used. If a caller selects Kemenag, the Kemenag position should apply, and if a caller selects MWL, the MWL position should apply. That is the right shape. What the sources show is that most authorities have no position to follow.

`prayertimes.h:176` already declares the enum for this.

```c
typedef enum {
  HIGHLAT_NONE,
  HIGHLAT_MIDDLE_OF_NIGHT,
  HIGHLAT_ONE_SEVENTH,
  HIGHLAT_ANGLE_BASED,
} HighLatMethod;
```

It is dead. Grepping the whole repository for `HighLatMethod` or `HIGHLAT_` returns only these five lines. There is no field of that type in `MethodParams`, no function takes one, and nothing reads one. The angle-based rule is hardcoded in `calculate_prayer_times` and applied to all 22 methods alike, so today Kemenag silently receives a rule that Kemenag never published.

### What each authority actually publishes

Researched in this pass.

- **MWL.** The Islamic Fiqh Council is a body of the Muslim World League itself, so for `CALC_MWL` the decree recorded above is not an outside analogy, it is the authority's own position. Three zones, reference latitude 45 degrees proposed.
- **Moonsighting Committee Worldwide.** <https://www.moonsighting.com/how-we.html> states that its formulae "are good up to the 55degrees latitude", that between 55 and 60 degrees "the rule of Sab'u Lail (1/7th of the night), is used because other methods give times that become hardship for those areas", and that above 60 degrees it slides the calculation down to 60 degrees and applies the one-seventh rule in summer. So `CALC_MOONSIGHTING` has a real, specified rule, with a threshold at 55 and a nearest-latitude fallback anchored at 60.
- **Fiqh Council of North America.** Its current published recommendation is 15 degrees for both fajr and isha in the USA and 13 degrees in Canada, year round. Its own page does not address the case where the angle is never reached. A 2011 FCNA position deferring to the moonsighting.com functions is reported by secondary sources but was not confirmed on an FCNA page in this pass.
- **Kemenag.** No high-latitude provision was found. The published Indonesian guidance, going back to the 1994 Pedoman Penentuan Jadwal Waktu Shalat Sepanjang Masa, is organised around sun altitude for isha and subuh, ihtiyat, and elevation. This is unsurprising rather than an oversight, because Indonesia spans roughly 6 degrees north to 11 degrees south, where the case never arises. Recording it as absent is the honest entry, not a gap to be filled by borrowing another authority's rule without saying so.

Not researched in this pass, and therefore not to be assigned a value on the strength of this note: Egypt, Karachi, Umm al-Qura, Turkey, Singapore, JAKIM, France, Russia, Dubai, Qatar, Kuwait, Jordan, Gulf, Tunisia, Algeria, Morocco, Portugal. Several of these serve jurisdictions that never reach the relevant latitudes. Others, notably France, Russia and Turkey, already carry lowered angles that may themselves be the adaptation, which is a question about the angle rather than about the never-reached case and was not investigated here.

### Independent support for the shape

The most widely used implementation of these methods, `adhan-js`, reached the same conclusion independently. Its `METHODS.md` lists a high-latitude rule for exactly one method, Moonsighting, and marks every other method as not specifying one. It keeps `HighLatitudeRule` and `PolarCircleResolution` as caller-set options rather than method defaults.

That library also separates the two axes the same way this note does. A high-latitude rule answers what to do when the angle is not reached but a night still exists. A polar circle resolution answers what to do when there is no sunrise or sunset at all. They are different questions and one does not imply the other, which is why the one-seventh rule and the angle-based rule are both insufficient on their own. Both are defined in terms of sunset to sunrise, so both are undefined inside the polar circle. Moonsighting is the only researched authority that patches this, and it does so with a nearest-latitude fallback anchored at 60 degrees.

### The reference latitude is contested

Three researched sources pick three different anchors for the same nearest-locality idea.

```
source                          anchor latitude
Islamic Fiqh Council decree     45
Al-Hilal paper, Reykjavik       London, about 51.5
Moonsighting Committee          60
```

No source settles this for the others. A per-method field therefore needs to carry the anchor as well as the rule, or the same nominal rule will mean three different things.

## What remains a decision

The sources settle less than they appear to.

1. Whether `MethodParams` gains a `HighLatMethod` field plus a reference latitude, so the rule follows the authority. The enum already exists and is unused, so this is wiring rather than new API surface. Two of the researched authorities have a position, MWL and Moonsighting. The rest do not, and `HIGHLAT_NONE` is the honest entry for them.
2. What `HIGHLAT_NONE` should then do, since it will be the value on most of the table. Returning a documented not-available signal is defensible. Silently applying the angle-based rule, which is what happens today, is the one option this note argues against, because it attributes a rule to an authority that never published it.
3. A separate axis for the polar case, since no researched rule except Moonsighting's is defined there, and Moonsighting's answer is a nearest-latitude anchor rather than a rule of the same kind.
4. What sunrise, sunset, maghrib and dhuha should be when they do not occur. The decree's third zone speaks to the prescribed prayers. Dhuha is not among them, and no source retrieved here addresses it. A documented signal that the event does not occur is the honest option for dhuha, and inventing a value for it is not supported by anything in this note.

## Sources

- <http://praytimes.org/calculation>
- <https://ar.themwl.org/node/48>
- <https://fiqh.islamonline.net/en/praying-and-fasting-at-high-latitudes/>
- <https://ketabonline.com/ar/books/24407>
- <https://birka.nur.nu/prayertimes/prayertimes-references/quoted/Rabita1406-transl.html>
- <https://journal.walisongo.ac.id/index.php/al-hilal/article/view/29337>
- <https://www.e-cfr.org/>
- <https://www.moonsighting.com/how-we.html>
- <https://fiqhcouncil.org/the-suggested-calculation-method-for-fajr-and-isha/>
- <https://github.com/batoulapps/adhan-js/blob/master/METHODS.md>

Reproduction programs for every measured table in this note are local and not committed. Each is a few dozen lines against the public API plus the two internal hour-angle helpers.

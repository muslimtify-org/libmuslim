# Egypt, Dar al-Ifta al-Misriyya: the rule and the record

Research date: 2026-11-18. This note admits `HIJRI_AUTHORITY_EGYPT_DAR_AL_IFTA` and records why its predicate is the one it is.

## What Egypt does

Dar al-Ifta al-Misriyya announces each month after sighting sessions held by committees across Egypt. Its fatwas state numeric conditions under which calculation rules a claimed sighting out. Two sets appear, on different pages, and are quoted in full under "Thresholds" below:

- Fatwa 13817 (10 December 2011): the Moon at least 8 degrees from the conjunction line after sunset, and at least 10 minutes between sunset and moonset, all together.
- Fatwa 12825 (16 June 2016): the Moon sets after the Sun, and that moonset falls 15 hours 40 minutes or more after the conjunction, all together, plus an actual sighting.

The most recent dated fatwa, 20051 (21 November 2023), states no number.

## The rule the entry computes

The selection rule in the approved spec takes the threshold set of the most recent dated fatwa that states one, which is 12825. `HIJRI_PREDICATE_EGYPT_DAR_AL_IFTA` passes when moonset is after sunset and the time from the relevant conjunction to moonset is at least 15 h 40 min. The sighting the fatwa also requires cannot be computed, so the predicate is the astronomical half of the rule only.

- Reference point: Cairo, 30.0444 N, 31.2357 E, 23 m. No Dar al-Ifta page names a reference location, so this is the library's choice.
- Sunset: the astronomical convention, the Sun's upper limb at the horizon with standard refraction. Fatwa 13817 defines sunset as the upper limb disappearing below the horizon, and no page states a refraction value.
- rule_from: 2016-06-16, the date of fatwa 12825. No page says when it stopped or started to govern, and rule_until is open.
- 15 h 40 min is inclusive ("أو أكثر", or more). Moonset after sunset is strict.

## Scoring

The fixture is the 94 announced starts listed under "Announced starts" below. A start counts as supported when the rule passes at Cairo on the evening before it, and as early when the rule also passes on the evening two days before it. This is `count_start_support` in `tests/test_hijri.c`, the measure the registry uses for every local-predicate entry. Late is total minus supported. Every row was run with `hijri.h` v0.7.0 and the astronomical sunset convention.

| Rule | Supported | Early | Late |
|---|---|---|---|
| 12825: moonset after sunset AND conjunction to moonset at least 15 h 40 min | 60 of 94 | 0 | 34 |
| 13817: geocentric elongation at least 8 degrees AND lag at least 10 min | 67 of 94 | 0 | 27 |
| 13817: topocentric elongation at least 8 degrees AND lag at least 10 min | 62 of 94 | 0 | 32 |
| moonset after sunset alone | 94 of 94 | 22 | 0 |
| conjunction to moonset at least 15 h 40 min alone | 60 of 94 | 0 | 34 |
| lag at least 10 min alone | 87 of 94 | 8 | 7 |
| geocentric elongation at least 8 degrees alone | 68 of 94 | 17 | 26 |
| topocentric elongation at least 8 degrees alone | 63 of 94 | 22 | 31 |
| `HIJRI_PREDICATE_MABIMS_1992` | 78 of 94 | 0 | 16 |
| `HIJRI_PREDICATE_MABIMS_2021` | 67 of 94 | 0 | 27 |
| `HIJRI_PREDICATE_WUJUDUL_HILAL` | 93 of 94 | 13 | 1 |
| `HIJRI_PREDICATE_LAG_AT_LEAST_5_MINUTES` | 92 of 94 | 16 | 2 |
| `HIJRI_PREDICATE_ALTITUDE_5_ELONGATION_8` | 58 of 94 | 0 | 36 |
| `HIJRI_PREDICATE_CONJUNCTION_AND_MOONSET` | 93 of 94 | 13 | 1 |
| `HIJRI_PREDICATE_GEOCENTRIC_ALTITUDE_5_ELONGATION_8` | 60 of 94 | 0 | 34 |
| 12825, passing at any of Alexandria, Cairo, Asyut, Aswan, Marsa Matruh | 60 of 94 | 0 | 34 |

## What the score says

The 12825 condition never starts a month early, and it starts 34 of the 94 a day late. In 28 of those 34, Dar al-Ifta's statement reports the crescent as sighted on an evening the condition fails at Cairo, some with the Moon only a few hours old. Safar 1438 (day 1 on 2016-10-31) was announced on a sighting the evening of 2016-10-30, when the conjunction fell after Cairo sunset. Dar al-Ifta therefore accepts sightings its own 2016 condition would reject, and the entry is that condition, not a model of Dar al-Ifta's decisions. The rule closest to the announcements is moonset after sunset alone, which supports all 94 and is early on 22.

Evaluating at five sites across Egypt changes nothing, so a multi-site basis would not help.

For dates where the announcement is known, pass it to `hijri_authority_from_gregorian_announced`, which takes precedence over the computation.

## Thresholds

Fetched 2026-11-18. Every quote is copied from the fetched page text. Translations are the researcher's.

### Summary

Two different threshold sets appear, on two different pages, and neither page states the other's set.

| Page | Date shown | Thresholds | How they combine |
|---|---|---|---|
| 13817 (Ali Gomaa) | 10 Dec 2011 | moon at least 8 degrees from the conjunction line, the moment falls after sunset, weather favourable, lag (مكث) at least 10 minutes | all together, joined by و ("and"), under one فإذا ... أمكن ("if ... then sighting is possible") |
| 13536 (Amanat al-Fatwa) and 13817 | 9 Jan 2012 / 10 Dec 2011 | 10 minutes lag after sunset, but only as a quotation of Majma' al-Buhuth decision no. 42 (1991), and only for accepting a sighting from another country sharing part of the night | conditions joined by و inside a quoted decision |
| 12825 (Shawki Allam) | 16 Jun 2016 | moon sets after the sun, and moonset is at least 15 h 40 min after conjunction | one compound condition (الحالة الخامسة, "بشرط"), and the page lists a 16 or 20 minute lag rule among views Dar al-Ifta does not adopt |
| 20051 (Shawki Allam) | 21 Nov 2023 | none | (no numbers. calculation accepted where it gives certainty the crescent is above the horizon) |

The most recent dated fatwa found is 20051 (21 Nov 2023) and it has no numeric threshold. The most recent dated page with a numeric threshold is 12825 (16 Jun 2016), whose rule is the 15 h 40 min conjunction-to-moonset condition, combined with moonset after sunset, plus a real sighting. Newer news pages (2026) mention criteria variables but give no numbers for crescent visibility.

No page found evaluates a threshold at Cairo specifically. No page found states a numeric threshold for moon altitude or illuminated fraction as a ruling criterion (13817 gives a 0.01 illuminated fraction as an astronomical fact tied to the 8 degree figure, see below).

---

### 1. Fatwa 13817

- URL: https://www.dar-alifta.org/ar/fatwa/details/13817/
- Title: الحساب الفلكي والوسائل العلمية الحديثة في تحديد بدايات الشهور القمرية
- Date: "تاريخ الفتوى: 10 ديسمبر 2011 م" (in the fatwa header block under the title). "رقم الفتوى: 2345".
- Issuer: "من فتاوى: فضيلة الأستاذ الدكتور علي جمعة محمد" (Ali Gomaa).
- NRIAG: yes. "بعد القراءة المتأنية للقضايا المطروحة في البحث المشار إليه، واستشارة المتخصصين في علم الفلك في المعهد القومي للبحوث الفلكية والجيوفيزيقية، نفيد بما يأتي:" (After careful reading of the issues raised in the paper, and consulting astronomy specialists at the National Research Institute of Astronomy and Geophysics, we state the following). The same sentence recurs in the detailed section. The committee section says each committee has "أحد الفلكيين بالمعهد القومي للبحوث الفلكية".

#### Threshold sentences

1. Summary section:
   > ورؤية الهلال هي اللحظة التي يُرى فيها ضوءُ القمر من سطح الأرض بعد افتراقه وابتعاده 8 درجات على الأقل عن خط الاقتران، فإذا جاءت هذه اللحظة بعد غروب الشمس، وكانت الظروف الجوية مواتيةً، وفترة المكث لا تقل عن عشر دقائق، أمكن رؤية الهلال، وهي الرؤية التي يعول عليها في الحكم بدخول الشهر من عدمه، ولا تعتمد على موعد ميلاد الهلال.

   Translation: "Sighting of the crescent is the moment at which the moon's light is seen from the earth's surface after it has separated and moved at least 8 degrees away from the conjunction line. If this moment comes after sunset, and the weather conditions are favourable, and the lag period (مكث) is not less than ten minutes, sighting of the crescent is possible. This is the sighting relied on in ruling whether the month has begun or not, and it does not depend on the time of the crescent's birth."

2. Detailed section ("الفرق بين ميلاد الهلال ورؤية الهلال"), nearly identical:
   > أما رؤية الهلال: فهي اللحظة التي يرى فيها ضوء القمر من سطح الأرض بعد افتراقه وابتعاده 8 درجات على الأقل عن خط الاقتران، فإذا جاءت هذه اللحظة بعد غروب الشمس، وكانت الظروف الجوية مواتيةً وفترة المكث لا تقل عن عشر دقائق، أمكن رؤية الهلال، وهي الرؤية التي يعول عليها في الحكم بدخول الشهر من عدمه، ولا تعتمد على موعد ميلاد الهلال كما سيأتي تفصيله فيما بعد.

   Combination: AND. Connecting words: "فإذا جاءت هذه اللحظة بعد غروب الشمس، **و**كانت الظروف الجوية مواتيةً، **و**فترة المكث لا تقل عن عشر دقائق، أمكن رؤية الهلال". The 8 degree separation is part of the definition of the moment of sighting, then three conditions follow, all joined by و, with the result "أمكن رؤية الهلال" (sighting is possible). The wording is "possible", not "established". The 8 degrees is measured from "خط الاقتران", which the page defines as "الخط الواصل بين مركزي الأرض والشمس" (the line joining the centres of the earth and sun), so it is an angular distance from the sun direction (elongation-like). It does not say topocentric or geocentric.

3. Illuminated fraction tied to the 8 degrees (astronomical statement, not phrased as a ruling condition):
   > الجزء المضاء من سطح القمر والذي يسمح برؤيته من سطح الأرض بعد افتراقه 8 درجات عن خط الاقتران هو 0.01 إذا كان القمر يبعد عن الأرض بقدر المسافة المتوسطة بين الأرض والقمر وهي 384400 كم، وتَقِلُّ هذه النسبة إذا قَلَّت المسافة، وتزداد بزيادتها، وذلك في وجود ظروفٍ جوية مناسبة.

   Translation: "The illuminated part of the moon's surface that allows it to be seen from the earth's surface after it has separated 8 degrees from the conjunction line is 0.01 when the moon is at the mean earth-moon distance of 384400 km. this ratio decreases if the distance decreases and increases with it, given suitable weather."

4. Majma' al-Buhuth decision 42 (quoted, 10 minutes):
   > وتأكد هذا أيضًا في قراره رقم (42) في جلسته الثانية في دورته الثامنة والعشرين التي عُقدت، بتاريخ (23 ربيع الآخر لسنة 1412هـ الموافق 31 من أكتوبر لسنة 1991م) بشأن رؤية الهلال، ونصه: أنه إذا تعذر دخول الشهر القمري، وثبتت رؤيته في بلد آخر تشترك مع المنطقة في جزء من الليل، وقال الحسابيون الفلكيون: إنه يمكث لمدة عشر دقائق فأكثر بعد الغروب، فإن دخول الشهر العربي يثبت. اهـ.

   Translation: "This was also confirmed in its decision no. (42), second session of its twenty-eighth round, held on 23 Rabi' al-Akhir 1412 AH / 31 October 1991, on crescent sighting, whose text is: if the entry of the lunar month is not possible [locally], and its sighting is established in another country that shares part of the night with the region, and the astronomers say that it remains ten minutes or more after sunset, then the entry of the Arab month is established."

   Combination: AND, three conditions: "إذا تعذر ... **و**ثبتت رؤيته في بلد آخر ... **و**قال الحسابيون الفلكيون: إنه يمكث لمدة عشر دقائق فأكثر بعد الغروب، فإن ...". This is a quotation of the Islamic Research Academy (مجمع البحوث الإسلامية) decision about accepting another country's sighting. The page does not say which location the 10 minutes is evaluated at.

5. Other numbers on the page that are astronomy background, not ruling thresholds (listed for completeness):
   - Moonset lag grows by 40 to 50 minutes per day: "يتأخر غروب القمر عن غروب الشمس من 40 إلى 50 دقيقة عن اليوم السابق".
   - Search the sky 5 degrees either side of the sunset point: "ويلتمس الهلال حول منطقة غروب الشمس بخمس درجات إلى يمينها وخمس درجات إلى يسارها."
   - An end-of-month (downward horns) crescent can appear after sunset when conjunction follows sunset "بفترة كبيرة لا تقل عن سبع ساعات" (by a long period of no less than seven hours). such a sighting is not counted.
   - Eclipse on the 29th: "إذا كان وقوع الكسوف يوم التاسع والعشرين فإن ذلك يدل دلالةً قاطعةً على عدم إمكانية رؤية الهلال في ذلك اليوم، ويُحْكَم بإتمام الشهر ثلاثين يومًا" (decisive that the crescent cannot be seen. the month is completed to 30).

#### Five cases (no numbers, but they are the page's ruling logic)

- Case 1: "أن يغرب القمر قبل غروب الشمس -أي أن المُكْثَ سالبٌ- في جميع البلاد العربية والإسلامية، وبهذا تستحيل الرؤية، ويُحكَم فيها بإكمال عدة الشهر ثلاثين يومًا، وتُرَدُّ شهادة أي شاهد" (moon sets before the sun, negative lag, in all Arab and Islamic countries: sighting impossible, month completed to 30, any testimony rejected).
- Case 2: "أن يغرب القمر بعد غروب الشمس -أي أن المُكْثَ موجب- في جميع البلاد العربية والإسلامية، وتكون احتمالات رؤيته قائمةً تبعًا لمدة المكث في كل بلد، وفي هذه الحالة يحكم بأن يكون اليوم التالي هو غرة الشهر الجديد" (moon sets after the sun, positive lag, in all Arab and Islamic countries: the next day is ruled the first of the new month). Note: no minimum lag is stated in this case.
- Case 3: conjunction after sunset: crescent not seen, month completed to 30.
- Case 4: the sun sets eclipsed: crescent cannot be seen.
- Case 5: mixed, moon sets after the sun in some countries and before in others. "يكون لكل بلد مطلعه" (each country has its own horizon) or per the rulers' decision.

#### Definitions relevant to implementation

- Lag (مكث): "وفي اليوم التاسع والعشرين من الشهر العربي، قد يأتي غروب القمر قبل غروب الشمس فلا يُرَى الهلالُ، وقد يأتي بعد غروبها فيحتمل رؤيته، ويقال: إن مُكْثَ الهلال سالب، أو يقال: إن مُكْثَ الهلال موجب" (moonset minus sunset, on the 29th).
- Sunset definition: "والمغرب: عند اختفاء الحافة العليا لقرص الشمس تحت حافة الأفق" (Maghrib: when the upper limb of the sun's disc disappears below the horizon). Also: "باستخدام معادلات غروب الجسم السماوي يتم حساب زمن غروب الشمس وزمن غروب القمر في التاسع والعشرين من كل شهر عربي، وهي نفس معادلات مواقيت صلاة المغرب" (sun and moon set times on the 29th are computed with the same equations as the Maghrib prayer times).
- Moon age is measured to sunset: "والمقصود بعمر الهلال في لحظة معينة: هو الزمن الذي ينقضي منذ ولادته حتى لحظة معينة، وعادةً ما تكون هذه اللحظة هي غروب شمس يوم الرؤية" (age = time from birth to a given moment, usually sunset of the sighting day). The page says age is not a function of lag and sets no age threshold.
- Location: no Cairo evaluation point. Committees observe "في عشرة أماكن من جمهورية مصر العربية: في حلوان، والقطامية، و6 أكتوبر، والسلوم، وقنا، وأسوان، والواحات، وتوشكى، ومَرْسَى علم، وسانت كاترين". The five cases are evaluated over "جميع البلاد العربية والإسلامية".

---

### 2. Fatwa 13536

- URL: https://www.dar-alifta.org/ar/fatwa/details/13536/
- Title: مسائل في رؤية الهلال وتعلق العبادات بالتقويم القمري
- Date: "تاريخ الفتوى: 09 يناير 2012 م" (fatwa header). "رقم الفتوى: 4389".
- Issuer: "من فتاوى: أمانة الفتوى" (the Fatwa Secretariat).
- NRIAG: not mentioned.

#### Threshold sentences

Only one, the same Majma' al-Buhuth decision 42 quotation (answer to question 5, on differing horizons):

> وتأكد هذا أيضًا في قراره رقم (42) وفي جلسته الثامنة والعشرين التي عقدت بتاريخ 23 ربيع الآخر لسنة 1412هـ، الموافق 31 من أكتوبر لسنة 1991م بشأن رؤية الهلال، ونصه: أنه إذا تعذر دخول الشهر القمري، وثبتت رؤيته في بلد آخر يشترك مع المنطقة في جزء من الليل، وقال الحسابيون الفلكيون: إنه يمكث لمدة عشر دقائق فأكثر بعد الغروب، فإن دخول الشهر القمري يثبت. اهـ.

Translation: "This was also confirmed in its decision no. (42) at its twenty-eighth session held on 23 Rabi' al-Akhir 1412 AH / 31 October 1991 on crescent sighting, whose text is: if the entry of the lunar month is not possible, and its sighting is established in another country that shares part of the night with the region, and the astronomers say it remains ten minutes or more after sunset, then the entry of the lunar month is established."

Combination: AND (و ... و ... فإن). Scope: accepting a sighting from another country. Location of evaluation not stated.

General rule on the page (no number): "إذ المختار للفتوى أن الحساب الفلكي ينفي ولا يُثبت، فيؤخذ به في نفي إمكانية طلوع الهلال ولا عبرة بدعوى الرؤية على خلافه، ولا يعتمد عليه استقلالا في الإثبات" (the chosen view is that calculation negates but does not establish).

Other numbers on the page (solar year lengths 365.24189814 days, 365 d 5 h 48 m 46 s) are not crescent thresholds.

---

### 3. Fatwa 12825

- URL: https://www.dar-alifta.org/ar/fatwa/details/12825/
- Title: القواعد والإجراءات المتبعة لرؤية الهلال في مصر
- Date: "تاريخ الفتوى: 16 يونيو 2016 م" (fatwa header). "رقم الفتوى: 3420".
- Issuer: "من فتاوى: الأستاذ الدكتور / شوقي إبراهيم علام" (Shawki Allam).
- NRIAG: not named. Sites are "في أماكن مختارة من هيئة المساحة المصريَّة ومن معهد الأرصاد" (chosen by the Egyptian Survey Authority and the Meteorological Institute). Seven committees: "وعددها سبع لجان، مبثوثة في أنحاء جمهورية مصر العربية".

#### Threshold sentences

1. Case 4 (rejection):
   > الحالة الرابعة: هي أن ينزل القمر بعد الشمس ولكن قبل مضي 15 ساعة و40 دقيقة من الاجتماع والافتراق بين الشمس والقمر.
   > وفي هذه الحالة لا تنعكس أضواء الشمس على جرم القمر، مما يستحيل معه رؤية الهلال في هذه الحالة.

   Translation: "Case four: the moon sets after the sun but before 15 hours and 40 minutes have passed since the conjunction and separation of the sun and moon. In this case the sun's light is not reflected off the moon's body, so sighting the crescent is impossible."

2. Case 5 (acceptance):
   > الحالة الخامسة: أن يغيب القمر بعد غروب الشمس، بشرط أن يكون هذا المغيب بعد 15 ساعة و40 دقيقة أو أكثر من افتراق الشمس مع القمر، فإننا نثبت -إذا ما رؤي الهلال حينئذٍ- ولادة الشهر.

   Translation: "Case five: the moon sets after sunset, on condition that this setting is 15 hours and 40 minutes or more after the separation of the sun and moon. then we establish, if the crescent is sighted at that time, the birth of the month."

   Combination: AND. Connecting words: "أن يغيب القمر بعد غروب الشمس، **بشرط** أن يكون هذا المغيب بعد 15 ساعة و40 دقيقة أو أكثر ... فإننا نثبت -**إذا ما رؤي الهلال حينئذٍ**- ولادة الشهر". So: moonset after sunset AND moonset at least 15 h 40 min after conjunction AND an actual sighting. The age is measured from conjunction to **moonset** ("هذا المغيب", "this setting", refers to the moon's setting), not to sunset.

3. Justification:
   > وهذه المدة أتت من أن الأرصاد الفلكية في العالم كله والمراصد العظمى الجبارة لم تستطع أن ترصد الهلال في أقل من 15 ساعة و40 دقيقة بعد الافتراق؛ فهذا إجماع بين الفلكيين.

   Translation: "This period comes from the fact that astronomical observation worldwide and the greatest observatories have not been able to observe the crescent at less than 15 hours and 40 minutes after separation. this is a consensus among astronomers."

4. Rejection summary (the negations are alternatives, any one rejects):
   > فإذا كانت هناك دعوى للرؤية وقد فُقِدَت شروط الاعتماد -كأن غرب القمر قبل الشمس، أو معها، أو بعدها وقبل الاجتماع والافتراق، أو بعدها وبعد الاجتماع والافتراق ولكن قبل مضي 15 ساعة و40 دقيقة- ففي كل هذه الأحوال لو ادعى أحدهم أنه رأى الهلال فإن شهادته ترد

   Translation: "If there is a claim of sighting while the conditions for acceptance are missing, such as the moon setting before the sun, or with it, or after it but before the conjunction and separation, or after it and after the conjunction and separation but before 15 hours and 40 minutes have passed, then in all these cases, if someone claims to have seen the crescent, his testimony is rejected." Connecting words: "أو ... أو ... أو" (OR between the failure cases), i.e. any failure rejects.

   Then: "أمَّا إذا وافقت الرؤية الحساب -على النحو المتقدم- فإن الشهر يثبت بتلك الرؤية." (If the sighting agrees with the calculation as described, the month is established by that sighting.)

5. Rejected lag rules:
   > هناك آراء لم تأخذ بها دار الإفتاء المصرية، ومنها:
   > - القول باشتراط بقاء الهلال بعد غروب الشمس أكثر من 16 أو 20 دقيقة أو نحو ذلك.

   Translation: "There are views Dar al-Ifta has not adopted, among them: the view requiring the crescent to remain after sunset more than 16 or 20 minutes or the like." The same list also rejects calculation alone from conjunction, relying entirely on a fixed calculation pattern, and absolute reliance on sighting regardless of calculation.

Cases 1 to 3 (moon sets before the sun, with it, or after it but before conjunction) are ruled out: "وفي هذه الأحوال لا يمكن اعتماد الشهر؛ لأن الشهر لم يولد شرعًا."

Location: not Cairo. nationwide committees. No explicit evaluation point for the 15 h 40 min check. No sunset definition on this page.

---

### 4. English fatwa 24176

- URL: https://www.dar-alifta.org/en/fatwa/details/24176/manner-of-determining-the-beginning-of-lunar-month-in-the-hijri-calendar
- Title: Manner of determining the beginning of lunar month in the Hijri calendar
- Date: "09 January 2012" (under the title).
- Issuer: "Dr. Ali Goma Mohammed".
- Numeric thresholds: none. Rule stated: "The adopted view for fatwa is that astronomical calculations can be used to rule out, but not to establish, the sighting of the crescent. Therefore, if the calculations show that the crescent cannot be sighted, no consideration is given to any claim of sighting." Sighting "takes place at sunset on the 29th day of the preceding month."
- NRIAG: not mentioned.

### 5. English article 516

- URL: https://www.dar-alifta.org/en/article/details/516/relying-on-physical-sightings-of-the-new-moon-is-the-principle-in-determining-the
- Title: Relying on physical sightings of the new moon is the principle in determining the beginning of Ramadan
- Date: none shown on the page (only the site header's current date).
- Issuer: "Dar Al-Ifta` Al-Misriyyah said ...". no named mufti.
- Numeric thresholds: none. "The opinion chosen for fatwa is that astronomical calculations negate [erroneous] physical sightings but do not confirm them."

---

### 6. Other pages searched and fetched

All fetched with HTTP 200. None contains a numeric crescent threshold.

- https://www.dar-alifta.org/ar/fatwa/details/20051/ "حكم الصيام والإفطار اعتمادًا على الحساب الفلكي". "تاريخ الفتوى: 21 نوفمبر 2023 م", "رقم الفتوى: 8123", "من فتاوى: الأستاذ الدكتور / شوقي إبراهيم علام". The newest dated fatwa found. No numbers. It allows Muslims in European countries without local sighting to rely on calculation "إذا أفاد يقينًا وجودَ الهلال في الأفق" (if it gives certainty the crescent is present on the horizon), and says the calculation should come from "المعاهد الفلكية المخصصة بذلك". It also says the competent body may choose either method and its choice binds: "فمتى اختارت الهيئةُ المختصةُ ... أيًّا مِن هذين الرأيَيْن طريقًا لإثبات الهلال وَجَب على المسلمين في تلك البلاد اتِّبَاعُهُ".
- https://www.dar-alifta.org/ar/fatwa/details/13963/ "ضوابط رؤية الهلال", 17 Mar 2015, Shawki Allam. No numbers.
- https://www.dar-alifta.org/ar/fatwa/details/18347/ "المراد برؤية الهلال في الشرع، وكيفيتها وأهميتها", 10 Dec 2011, Ali Gomaa. No numbers.
- https://www.dar-alifta.org/ar/articles/details/12437/ (news, 21 Sep 2026) "انطلاق دورة الفلك الشرعي ...". An NRIAG researcher (أحمد شكري أبو الفتوح, "الأستاذ الباحث بالمعهد القومي للبحوث الفلكية والجيوفيزيقية") lectured. Only number: Egypt's Fajr angle, "مصر تعتمد انخفاض مركز الشمس بمقدار 19.5 درجة تحت الأفق في تحديد وقت الفجر الصادق". For crescents it lists variables only: "عمر الهلال، ومدة مكثه بعد غروب الشمس، وارتفاعه عن الأفق، والاستطالة أو البعد الزاوي بين الشمس والقمر، وسمك الهلال", and says there is no agreement on one criterion.
- https://www.dar-alifta.org/ar/articles/details/12442/ (news, 23 Sep 2026). Lists model variables "الاستطالة، المكث، الارتفاع عند الغروب، عرض الجزء المضاء، والفرق الرأسي والأفقي بين الشمس والقمر" for world visibility maps. No numbers.
- https://www.dar-alifta.org/ar/articles/details/12443/ (news, 24 Sep 2026). The Dar al-Ifta crescents department studies "وقت ولادة الهلال وموقعه وإمكان رؤيته ومدة مُكثه" then field observation. No numbers.
- https://www.dar-alifta.org/ar/articles/details/11039/ Ramadan 1447 statement (17 Feb 2026), signed "أ.د/ نظير محمد محمد النظير عياد، مفتي جمهورية مصر العربية". Result only, no calculation figures.
- https://www.dar-alifta.org/ar/articles/details/11079/ (Shawwal 1447, 19 Mar 2026), https://www.dar-alifta.org/ar/articles/details/10915/ (Sha'ban 1447, 18 Jan 2026), https://www.dar-alifta.org/ar/Articles/5766/ (Sha'ban 1439), https://www.dar-alifta.org/ar/articles/details/5160/ (Shawwal 1438): no numeric thresholds.
- English: https://www.dar-alifta.org/en/fatwa/details/8050/ (03 Mar 2015), /en/fatwa/details/6204/ (01 Aug 2013), /en/fatwa/details/6203/ (01 Aug 2013), /en/fatwa/details/6603/ (22 Oct 2013), /en/fatwa/details/2555/ (20 Jan 2004), /en/article/details/535/: no numeric thresholds.

### Not verified

- Whether Dar al-Ifta currently applies the 13817 set (8 degrees, 10 minutes) or the 12825 set (15 h 40 min to moonset) in its monthly calculations. No 2017 to 2026 page found states which numbers are used now.
- The geographic point at which any threshold is evaluated. No page names Cairo or any single reference location.
- Whether the 8 degrees is topocentric or geocentric, and whether the 10 minute lag uses upper-limb sunset with refraction. 13817 only defines Maghrib by the sun's upper limb disappearing below the horizon.

## Announced starts

Research date: 2026-11-18. Every date below was read from a page fetched during this research. Nothing was filled from memory, a calendar site or Umm al-Qura.

### How the sources were found and read

- Dar al-Ifta publishes a statement for every month, titled "بيان دار الإفتاء المصرية حول رؤية هلال شهر ... لعام ...". These pages exist on the current site from Rajab 1437 (April 2016, article 3940) onward. Article IDs were found by web search and by probing ID ranges, and each page was fetched with curl (the site answers slowly, about 20 s per page, but without a challenge).
- Each statement names the weekday and the Gregorian date of day 1 in words. A script converted the words to ISO dates and checked that the stated weekday matches the computed weekday. All matched. The Gregorian year was resolved from the page's publication date, or from the signed date line where the publication date is a placeholder.
- For 1436 H the new site has no statements, so press reports were used. Ahram Online and Masrawy sit behind a Cloudflare challenge, so their text was read from Wayback Machine copies of the original URLs.
- Web search did not surface the Dar al-Ifta pages for Dhu al-Hijjah 1440, 1442 and 1447. A probe of nearby article IDs found them (6628, 7773, 11149). Youm7 reports of the same three announcements are kept as second sources, and they agree.
- Publication dates: dar-alifta.org shows "01 يناير 2017" for every article migrated from the old site (Rajab 1437 to Sha'ban 1438). That is a migration placeholder, not the real date. Those entries also give the date line signed at the foot of the statement, which is the real issue date (the evening of the sighting session).
- Quotes are copied from the page text with HTML removed. Arabic diacritics are kept as published.

### Coverage

- Ramadan, Shawwal and Dhu al-Hijjah: found for every year 1436 to 1447 H, 36 of 36, all stated directly (none derived).
- 1448 H: Ramadan, Shawwal and Dhu al-Hijjah 1448 fall in February to May 2027 and have not been announced yet. Muharram to Jumada al-Akhira 1448 are included.
- Other months: every Dar al-Ifta monthly statement that web search or the ID probe surfaced is included. Months not listed (for example Muharram 1437 to 1439 and 1441 to 1444, or several Rabi and Jumada months) were simply not looked for page by page. Their absence does not mean Dar al-Ifta made no statement.
- Only one month has two Dar al-Ifta copies (Ramadan 1438, articles 5065 and 5069, identical). No source disagreed with another anywhere.

### Day 1 semantics used

- Ramadan: first fasting day. Shawwal: Eid al-Fitr. Dhu al-Hijjah: 1 Dhu al-Hijjah as stated (not Eid al-Adha). Every Dhu al-Hijjah entry here states 1 Dhu al-Hijjah directly, so none is derived from the Eid date.
- Method field: "sighted" means the statement says ثبوت رؤية. "not sighted" means it says عدم ثبوت رؤية and names the 30th day as المتمم of the previous month (completion of 30 days). For Dhu al-Hijjah, 1436 to 1441 simply adopt the Saudi Supreme Court decision. 1442 adopts it and adds that Egyptian committees also saw the crescent. 1443 cites only Egypt's own sighting. 1444 to 1447 cite both the committees and the Saudi court.
- NRIAG predictions were excluded. One Ahram Online item for Shawwal 1436 (english.ahram.org.eg/News/135434.aspx, "Eid Al-Fitr starts in Egypt on Friday") quotes the Astronomy and Geophysics Institute, not Dar al-Ifta, so it is not used as a source.


### 1436 H

#### Ramadan 1436 H: day 1 = 2015-06-18 (Thursday)

- Source: https://english.ahram.org.eg/News/132951.aspx
- Source type: Ahram Online (english.ahram.org.eg). live site is behind a Cloudflare challenge, text read from the Wayback Machine copy https://web.archive.org/web/2016id_/http://english.ahram.org.eg/News/132951.aspx
- Title: Muslim Holy Month of Ramadan starts on Thursday: Dar Al-Ifta
- Publication date (as shown by the site): Tuesday 16 Jun 2015
- Method: not sighted on Tuesday 16 Jun 2015 (article says only 'not spotted in the sky on Tuesday'). Ramadan began Thursday, so Sha'ban was completed to 30 days
- Note: The article gives only the weekday. Publication date Tuesday 16 Jun 2015 plus 'will start on Thursday' resolves to 2015-06-18.
- Quote: "Egypt's Dar Al-Ifta, the main authority responsible for issuing religious edicts, announced that the Islamic holy month of Ramadan will start on Thursday. A religious committee is formed by Dar El-Ifta to observe whether the crescent has appeared. Ramadan crescent was not spotted in the sky on Tuesday, marking the begining of Ramadan on Thursday."

#### Shawwal 1436 H: day 1 = 2015-07-17 (Friday)

- Source: https://www.masrawy.com/News/News_PublicAffairs/details/2015/7/16/620802/
- Source type: Masrawy (DPA wire roundup). live site is behind a Cloudflare challenge, text read from the Wayback Machine copy (web.archive.org/web/2016id_/http://www.masrawy.com/News/News_PublicAffairs/details/2015/7/16/620802/...)
- Title: 10 دول عربية تعلن أن غدًا الجمعة أول أيام عيد الفطر
- Publication date (as shown by the site): الخميس, 16 يوليه, 2015, 08:22 م
- Method: not stated (the quote implies Ramadan ended on its 29th day, Thursday, so Shawwal was sighted)
- Note: 'غدا الجمعة' in an article dated Thursday 16 Jul 2015 resolves to 2015-07-17.
- Quote: "عواصم عربية - (د ب أ): أعلنت دار الافتاء المصرية أن غدا الجمعة هو غرة شهر شوال وأول أيام عيد الفطر المبارك وأن اليوم الخميس هو آخر أيام شهر رمضان للعام الهجرى 1436 ."

#### Dhu al-Hijjah 1436 H: day 1 = 2015-09-15 (Tuesday)

- Source: https://www.almasdar.com/26408
- Source type: Al-Masdar (almasdar.com, Egyptian economic news site, not on the preferred list) reproducing the Dar al-Ifta statement
- Title: "الإفتاء": غداً الثلاثاء غرة ذي الحجة والخميس أول أيام عيد الأضحى
- Publication date (as shown by the site): الإثنين 14 سبتمبر 2015 - 12:17 م
- Method: adopted the Saudi Supreme Court decision no. 75 (Dhu al-Qa'dah completed to 30)
- Quote: "أعلنت دار الإفتاء المصرية أن يومَ الثلاثاء 15 سبتمبر 2015 هو أول أيام شهر ذي الحجة لعام 1436، وأن يومَ الأربعاء الموافقَ 23 سبتمبر 2015 هو التاسعُ من ذي الحجة وهو يومُ عرفة، وأن يومَ الخميس الموافقَ24 سبتمبر لعام 2015 هو العاشرُ من ذي الحجة وهو أول أيام عيد الأضحى المبارك. يأتي ذلك وفقاً لما اعلنته المحكمة العليا السعودية بقرار رقم (75/هـ) الصادر اليوم أن يومَ الإثنين الموافق 30 ذي القَعْدة 1436 الموافقَ 14 سبتمبر2015 هو آخر أيام ذي القعدة، وأن يومَ الثلاثاء15 سبتمبر 2015 هو أول أيام ذي الحجة."


### 1437 H

#### Rajab 1437 H: day 1 = 2016-04-08 (Friday)

- Source: https://www.dar-alifta.org/ar/articles/details/3940
- Source type: dar-alifta.org article 3940
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر رجب لسنة 1437هـ
- Publication date (as shown by the site): 01 يناير 2017
- Date line signed in the statement: الخميس التاسع والعشرين من شهر جمادى الآخرة1437هـ السابع من شهر إبريل2016م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الجمعة الموافق الثامن من شهر إبريل لعام ألفين وستة عشر ميلادية هو أول أيام شهر رجب لعام ألفٍ وأربعمائةٍ وسبعة وثلاثين هجرية"

#### Sha'ban 1437 H: day 1 = 2016-05-08 (Sunday)

- Source: https://www.dar-alifta.org/ar/articles/details/3954
- Source type: dar-alifta.org article 3954
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر شعبان لسنة 1437هـ
- Publication date (as shown by the site): 01 يناير 2017
- Date line signed in the statement: الجمعة التاسع والعشرين من شهر رجب1437هـ السادس من شهر مايو2016م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ السبت الموافق السابع من شهر مايو لعام ألفين وستة عشر ميلادية هو المتمم لشهر رجب لعام ألفٍ وأربعمائةٍ وسبعة وثلاثين هجرية، وأن يوم الأحد الموافق الثامن من شهر مايو هو أول أيام شهر شعبان لعام ألفٍ وأربعمائةٍ وسبعة وثلاثين هجرية"

#### Ramadan 1437 H: day 1 = 2016-06-06 (Monday)

- Source: https://www.dar-alifta.org/ar/articles/details/4010
- Source type: dar-alifta.org article 4010
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر رمضان لسنة 1437هـ
- Publication date (as shown by the site): 01 يناير 2017
- Date line signed in the statement: الأحد التاسع والعشرين من شهر شعبان1437هـ الخامس من شهر يونيو2016م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الإثنين الموافق السادس من شهر يونيو لعام ألفين وستة عشر ميلادية هو أول أيام شهر رمضان لعام ألفٍ وأربعمائةٍ وسبعة وثلاثين هجرية"

#### Shawwal 1437 H: day 1 = 2016-07-06 (Wednesday)

- Source: https://www.dar-alifta.org/ar/articles/details/4041
- Source type: dar-alifta.org article 4041
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر شوال لسنة 1437هـ
- Publication date (as shown by the site): 01 يناير 2017
- Date line signed in the statement: دار الإفتاء المصرية الإثنين التاسع والعشرين من شهر رمضان سنة 1437ه الرابع من شهر يوليو سنة 2016م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دار الإفتاء المصرية أن يوم الثلاثاء الموافق الخامس من شهر يوليو لعامِ ألفين وستة عشر ميلادية هو المتمم لشهر رمضان لعامِ ألفٍ وأربعمائةٍ وسبعةٍ وثلاثين هجرية، وأن يوم الأربعاء الموافق السادس من شهر يوليو لعامِ ألفين وستة عشر ميلادية هو أول أيام عيد الفطر المبارك لعام ألفٍ وأربعمائةٍ وسبعةٍ وثلاثين هجرية"

#### Dhu al-Hijjah 1437 H: day 1 = 2016-09-03 (Saturday)

- Source: https://www.dar-alifta.org/ar/articles/details/4248
- Source type: dar-alifta.org article 4248
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر ذي الحجة لسنة 1437هـ
- Publication date (as shown by the site): 01 يناير 2017
- Date line signed in the statement: الجمعة الثلاثين من ذي القعدة لعام 1437هـ الثاني من سبتمبر لعام 2016م
- Method: adopted the Saudi Supreme Court decision (no own sighting result stated)
- Quote: "وعلى ذلك تُعلن دار الإفتاء المصرية أن يومَ السبت الثالث من شهر سبتمبر لعام ألفينِ وستة عشر من الميلاد هو أول أيام شهر ذي الحجة لعام ألفٍ وأَربَعِمِائةٍ وسبعة وثلاثين من الهجرة، وأن يومَ الأحد الموافقَ الحادي عشر من شهر سبتمبر لعام ألفينِ وستة عشر من الميلاد هو التاسعُ من ذي الحجة وهو يومُ عرفة، وأن يومَ الإثنين الموافقَ الثاني عشر من شهر سبتمبر لعام ألفينِ وستة عشر من الميلاد هو العاشرُ من ذي الحجة وهو أول أيام عيد الأضحى المبارك"


### 1438 H

#### Safar 1438 H: day 1 = 2016-10-31 (Monday)

- Source: https://www.dar-alifta.org/ar/articles/details/4349
- Source type: dar-alifta.org article 4349
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر صفر لسنة 1438هـ
- Publication date (as shown by the site): 01 يناير 2017
- Date line signed in the statement: الأحد التاسع والعشرين من شهر المحرم 1438ه الثلاثين من شهر أكتوبر 2016م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الإثنين الموافق الحادي والثلاثين من شهر أكتوبر لعام ألفين وستة عشر ميلادية هو أول أيام شهر صفر لعام ألفٍ وأربعمائةٍ وثمانية وثلاثين هجرية"

#### Rabi al-Awwal 1438 H: day 1 = 2016-11-30 (Wednesday)

- Source: https://www.dar-alifta.org/ar/articles/details/4460
- Source type: dar-alifta.org article 4460
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر ربيعٍ الأول لسنة 1438هـ
- Publication date (as shown by the site): 01 يناير 2017
- Date line signed in the statement: الثلاثاء التاسع والعشرين من شهر صفر 1438هـ التاسع والعشرين من شهر نوفمبر 2016م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الأربعاء الموافق الثلاثين من شهر نوفمبر لعام ألفين وستة عشر ميلادية هو أول أيام شهر ربيعٍ الأول لعام ألفٍ وأربعمائةٍ وثمانية وثلاثين هجرية"

#### Rajab 1438 H: day 1 = 2017-03-29 (Wednesday)

- Source: https://www.dar-alifta.org/ar/articles/details/4888
- Source type: dar-alifta.org article 4888
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر رجب لسنة 1438هـ
- Publication date (as shown by the site): 01 يناير 2017
- Date line signed in the statement: الثلاثاء التاسع والعشرين من شهر جمادى الثانية 1438هـ الثامن والعشرين من شهر مارس 2017م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الأربعاء الموافق التاسع والعشرين من شهر مارس لعام ألفين وسبعة عشر ميلاديًّا هو أول أيام شهر رجب لعام ألفٍ وأربعمائةٍ وثمانية وثلاثين هجريًّا"

#### Sha'ban 1438 H: day 1 = 2017-04-27 (Thursday)

- Source: https://www.dar-alifta.org/ar/articles/details/4985
- Source type: dar-alifta.org article 4985
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر شعبان لسنة 1438هـ
- Publication date (as shown by the site): 01 يناير 2017
- Date line signed in the statement: الأربعاء التاسع والعشرين من شهر رجب 1438هـ السادس والعشرين من شهر إبريل 2017م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الخميس الموافق السابع والعشرين من شهر إبريل لعام ألفين وسبعة عشر ميلاديًّا هو أول أيام شهر شعبان لعام ألفٍ وأربعمائةٍ وثمانية وثلاثين هجريًّا"

#### Ramadan 1438 H: day 1 = 2017-05-27 (Saturday)

- Source: https://www.dar-alifta.org/ar/articles/details/5065
- Source type: dar-alifta.org article 5065
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر رمضان لسنة 1438ه
- Publication date (as shown by the site): 25 مايو 2017
- Date line signed in the statement: الجمعة التاسع والعشرين من شهر شعبان 1438ه السادس والعشرين من شهر مايو 2017م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ السبت الموافق السابع والعشرين من شهر مايو لعام ألفين وسبعة عشر ميلاديًّا هو أول أيام شهر رمضان لعام ألفٍ وأربعمائةٍ وثمانية وثلاثين هجريًّا"

#### Ramadan 1438 H: day 1 = 2017-05-27 (Saturday)

- Source: https://www.dar-alifta.org/ar/articles/details/5069
- Source type: dar-alifta.org article 5069
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر رمضان لسنة 1438ه
- Publication date (as shown by the site): 26 مايو 2017
- Date line signed in the statement: الجمعة التاسع والعشرين من شهر شعبان 1438ه السادس والعشرين من شهر مايو 2017م
- Method: sighted (ثبوت رؤية)
- Note: Second copy of the same statement (5065), published a day later. Same date.
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ السبت الموافق السابع والعشرين من شهر مايو لعام ألفين وسبعة عشر ميلاديًّا هو أول أيام شهر رمضان لعام ألفٍ وأربعمائةٍ وثمانية وثلاثين هجريًّا"

#### Shawwal 1438 H: day 1 = 2017-06-25 (Sunday)

- Source: https://www.dar-alifta.org/ar/articles/details/5160
- Source type: dar-alifta.org article 5160
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر شوال لسنة 1438ه
- Publication date (as shown by the site): 24 يونيو 2017
- Date line signed in the statement: السبت التاسع والعشرين من شهر رمضان 1438ه الرابع والعشرين من شهر يونيو 2017م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الأحد الموافق الخامس والعشرين من شهر يونيو لعام ألفين وسبعة عشر ميلاديًّا هو أول أيام عيد الفطر المبارك لعام ألفٍ وأربعمائةٍ وثمانية وثلاثين هجريًّا"

#### Dhu al-Qa'dah 1438 H: day 1 = 2017-07-24 (Monday)

- Source: https://www.dar-alifta.org/ar/articles/details/5219
- Source type: dar-alifta.org article 5219
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر ذي القعدة لسنة 1438ه
- Publication date (as shown by the site): 23 يوليو 2017
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الاثنين الموافق الرابع والعشرين من شهر يوليو لعام ألفين وسبعة عشر ميلاديًّا هو أول أيام شهر ذي القعدة لعام ألفٍ وأربعمائةٍ وثمانية وثلاثين هجريًّا"

#### Dhu al-Hijjah 1438 H: day 1 = 2017-08-23 (Wednesday)

- Source: https://www.dar-alifta.org/ar/articles/details/5339
- Source type: dar-alifta.org article 5339
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر ذي الحجة لسنة 1438ه
- Publication date (as shown by the site): 22 أغسطس 2017
- Date line signed in the statement: الثلاثاء الثلاثين من شهر ذي القعدة 1438ه الثاني والعشرين من شهر أغسطس 2017م
- Method: adopted the Saudi Supreme Court decision (no own sighting result stated)
- Quote: "وعلى ذلك تُعلن دار الإفتاء المصرية أن يومَ الأربعاء الثالث والعشرين من شهر أغسطس لعام ألفينِ وسبعة عشر ميلاديًّا هو أول أيام شهر ذي الحجة لعام ألفٍ وأَربَعِمِائةٍ وثمانية وثلاثين من الهجرة، وأن يومَ الخميس الموافق الحادي والثلاثين من شهر أغسطس لعام ألفينِ وسبعة عشر ميلاديًّا هو التاسعُ من ذي الحجة وهو يومُ عرفة، وأن يومَ الجمعة الموافقَ الأول من شهر سبتمبر لعام ألفينِ وسبعة عشر ميلاديًّا هو العاشرُ من ذي الحجة وهو أول أيام عيد الأضحى المبارك"


### 1439 H

#### Safar 1439 H: day 1 = 2017-10-21 (Saturday)

- Source: https://www.dar-alifta.org/ar/articles/details/5460
- Source type: dar-alifta.org article 5460
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر صفر لعام 1439هـ
- Publication date (as shown by the site): 20 أكتوبر 2017
- Date line signed in the statement: الجمعة التاسع والعشرين من شهر المحرم 1439هـ العشرين من شهر أكتوبر 2017م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ السبت الموافق الحادي والعشرين من شهر أكتوبر لعام ألفين وسبعة عشر ميلاديًّا هو أول أيام شهر صفر لعام ألفٍ وأربعمائةٍ وتسعةٍ وثلاثين هجريًّا"

#### Rabi al-Awwal 1439 H: day 1 = 2017-11-20 (Monday)

- Source: https://www.dar-alifta.org/ar/articles/details/5540
- Source type: dar-alifta.org article 5540
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر ربيع الأول لعام 1439هـ
- Publication date (as shown by the site): 18 نوفمبر 2017
- Date line signed in the statement: السبت التاسع والعشرين من شهر صفر 1439هـ الثامن عشر من شهر نوفمبر 2017م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الأحد الموافق التاسع عشر من شهر نوفمبر لعام ألفين وسبعة عشر ميلاديًّا هو المتمم لشهر صفر لعام ألفٍ وأربعمائةٍ وتسعةٍ وثلاثين هجريًّا، وأن يوم الاثنين الموافق العشرين من شهر نوفمبر هو أول أيام شهر ربيع الأول لعام ألفٍ وأربعمائةٍ وتسعةٍ وثلاثين هجريًّا"

#### Sha'ban 1439 H: day 1 = 2018-04-17 (Tuesday)

- Source: https://www.dar-alifta.org/ar/articles/details/5766
- Source type: dar-alifta.org article 5766
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر شعبان لعام 1439هـ
- Publication date (as shown by the site): 16 أبريل 2018
- Date line signed in the statement: الإثنين التاسع والعشرين من شهر رجب 1439هـ السادس عشر من شهر إبريل 2018م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الثلاثاء الموافق السابع عشر من شهر إبريل لعام ألفين وثمانية عشر ميلاديًّا هو أول أيام شهر شعبان لعام ألفٍ وأربعمائةٍ وتسعةٍ وثلاثين هجريًّا"

#### Ramadan 1439 H: day 1 = 2018-05-17 (Thursday)

- Source: https://www.dar-alifta.org/ar/articles/details/5802
- Source type: dar-alifta.org article 5802
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر رمضان لسنة 1439هـ
- Publication date (as shown by the site): 15 مايو 2018
- Date line signed in the statement: الثلاثاء التاسع والعشرين من شهر شعبان 1439ه الخامس عشر من شهر مايو 2018م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الأربعاء الموافق السادس عشر من شهر مايو لعام ألفين وثمانية عشر ميلاديًّا هو المتمم لشهر شعبان لعام ألفٍ وأربعمائةٍ وتسعةٍ وثلاثين هجريًّا، وأن يوم الخميس الموافق السابع عشر من شهر مايو هو أول أيام شهر رمضان لعام ألفٍ وأربعمائةٍ وتسعةٍ وثلاثين هجريًّا"

#### Shawwal 1439 H: day 1 = 2018-06-15 (Friday)

- Source: https://www.dar-alifta.org/ar/articles/details/5842
- Source type: dar-alifta.org article 5842
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر شوال لعام 1439هـ
- Publication date (as shown by the site): 14 يونيو 2018
- Date line signed in the statement: الخميس التاسع والعشرين من شهر رمضان 1439هـ الرابع عشر من شهر يونيو 2018م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الجمعة الموافق الخامس عشر من شهر يونيو لعام ألفين وثمانية عشر ميلاديًّا هو أول أيام عيد الفطر المبارك لعام ألفٍ وأربعمائةٍ وتسعةٍ وثلاثين هجريًّا"

#### Dhu al-Hijjah 1439 H: day 1 = 2018-08-12 (Sunday)

- Source: https://www.dar-alifta.org/ar/articles/details/5963
- Source type: dar-alifta.org article 5963
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر ذي الحجة لعام 1439هـ
- Publication date (as shown by the site): 11 أغسطس 2018
- Date line signed in the statement: السبت التاسع والعشرين من شهر ذي القعدة 1439هـ
- Method: adopted the Saudi Supreme Court decision (no own sighting result stated)
- Quote: "وعلى ذلك تُعلن دار الإفتاء المصرية أن يومَ الأحد الثاني عشر من شهر أغسطس لعام ألفينِ وثمانية عشر ميلاديًّا هو أول أيام شهر ذي الحجة لعام ألفٍ وأَربَعِمِائةٍ وتسعةٍ وثلاثين هجريًّا، وأن يومَ الإثنين الموافق العشرين من شهر أغسطس لعام ألفينِ وثمانية عشر ميلاديًّا هو التاسعُ من ذي الحجة وهو يومُ عرفة، وأن يومَ الثلاثاء الموافقَ الحادي والعشرين من شهر أغسطس لعام ألفينِ وثمانية عشر ميلاديًّا هو العاشرُ من ذي الحجة وهو أول أيام عيد الأضحى المبارك"


### 1440 H

#### Muharram 1440 H: day 1 = 2018-09-11 (Tuesday)

- Source: https://www.dar-alifta.org/ar/articles/details/6003
- Source type: dar-alifta.org article 6003
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر المحرم لعام 1440هـ
- Publication date (as shown by the site): 09 سبتمبر 2018
- Date line signed in the statement: الأحد التاسع والعشرين من شهر ذي الحجة 1439هـ التاسع من شهر سبتمبر 2018م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الإثنين الموافق العاشر من شهر سبتمبر لعام ألفين وثمانية عشر ميلاديًّا هو المتمم لشهر ذي الحجة لعام ألفٍ وأربعمائةٍ وتسعةٍ وثلاثين هجريًّا، وأن يوم الثلاثاء الموافق الحادي عشر من شهر سبتمبر هو أول أيام شهر المحرم لعام ألفٍ وأربعمائةٍ وأربعين هجريًّا"

#### Rabi al-Awwal 1440 H: day 1 = 2018-11-09 (Friday)

- Source: https://www.dar-alifta.org/ar/articles/details/6134
- Source type: dar-alifta.org article 6134
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر ربيع الأول لعام 1440هـ
- Publication date (as shown by the site): 07 نوفمبر 2018
- Date line signed in the statement: الأربعاء التاسع والعشرين من شهر صفر 1440هـ السابع من شهر نوفمبر 2018م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الخميس الموافق الثامن من شهر نوفمبر لعام ألفين وثمانية عشر ميلاديًّا هو المتمم لشهر صفر لعام ألفٍ وأربعمائةٍ وأربعين هجريًّا، وأن يوم الجمعة الموافق التاسع من شهر نوفمبر هو أول أيام شهر ربيع الأول لعام ألفٍ وأربعمائةٍ وأربعين هجريًّا"

#### Rajab 1440 H: day 1 = 2019-03-08 (Friday)

- Source: https://www.dar-alifta.org/ar/articles/details/6393
- Source type: dar-alifta.org article 6393
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر رجب لعام 1440هـ
- Publication date (as shown by the site): 06 مارس 2019
- Date line signed in the statement: الأربعاء التاسع والعشرين من شهر جمادى الآخرة 1440هـ السادس من شهر مارس 2019م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الخميس الموافق السابع من شهر مارس لعام ألفين وتسعة عشر ميلاديًّا هو المتمم لشهر جمادى الآخرة لعام ألفٍ وأربعمائةٍ وأربعين هجريًّا، وأن يوم الجمعة الموافق الثامن من شهر مارس هو أول أيام شهر رجب لعام ألفٍ وأربعمائةٍ وأربعين هجريًّا"

#### Sha'ban 1440 H: day 1 = 2019-04-07 (Sunday)

- Source: https://www.dar-alifta.org/ar/articles/details/6462
- Source type: dar-alifta.org article 6462
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر شعبان لعام 1440هـ
- Publication date (as shown by the site): 05 أبريل 2019
- Date line signed in the statement: الجمعة التاسع والعشرين من شهر رجب 1440هـ الخامس من شهر إبريل 2019م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ السبت الموافق السادس من شهر إبريل لعام ألفين وتسعة عشر ميلاديًّا هو المتمم لشهر رجب لعام ألفٍ وأربعمائةٍ وأربعين هجريًّا، وأن يوم الأحد الموافق السابع من شهر إبريل هو أول أيام شهر شعبان لعام ألفٍ وأربعمائةٍ وأربعين هجريًّا"

#### Ramadan 1440 H: day 1 = 2019-05-06 (Monday)

- Source: https://www.dar-alifta.org/ar/articles/details/6523
- Source type: dar-alifta.org article 6523
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر رمضان المبارك لعام 1440هـ
- Publication date (as shown by the site): 05 مايو 2019
- Date line signed in the statement: الأحد التاسع والعشرين من شهر شعبان 1440هـ الخامس من شهر مايو 2019م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الاثنين الموافق السادس من شهر مايو لعام ألفين وتسعة عشر ميلاديًّا هو أول أيام شهر رمضان لعام ألفٍ وأربعمائةٍ وأربعين هجريًّا"

#### Shawwal 1440 H: day 1 = 2019-06-05 (Wednesday)

- Source: https://www.dar-alifta.org/ar/articles/details/6557
- Source type: dar-alifta.org article 6557
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر شوال لعام 1440هـ
- Publication date (as shown by the site): 03 يونيو 2019
- Date line signed in the statement: الإثنين التاسع والعشرين من شهر رمضان 1440هـ الثالث من شهر يونيو 2019م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الثلاثاء الموافق الرابع من شهر يونيو لعام ألفين وتسعة عشر ميلاديًّا هو المتمم لشهر رمضان لعام ألفٍ وأربعمائةٍ وأربعين هجريًّا، وأن يوم الأربعاء الموافق الخامس من شهر يونيو هو أول أيام عيد الفطر المبارك لعام ألفٍ وأربعمائةٍ وأربعين هجريًّا"

#### Dhu al-Qa'dah 1440 H: day 1 = 2019-07-04 (Thursday)

- Source: https://www.dar-alifta.org/ar/articles/details/6589
- Source type: dar-alifta.org article 6589
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر ذي القعدة لعام 1440هـ
- Publication date (as shown by the site): 03 يوليو 2019
- Date line signed in the statement: الأربعاء التاسع والعشرين من شهر شوال1440هـ الثالث من شهر يوليو 2019م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الخميس الموافق الرابع من شهر يوليو لعام ألفين وتسعة عشر ميلاديًّا هو أول أيام شهر ذي القعدة لعام ألفٍ وأربعمائةٍ وأربعين هجريًّا"

#### Dhu al-Hijjah 1440 H: day 1 = 2019-08-02 (Friday)

- Source: https://www.dar-alifta.org/ar/articles/details/6628
- Source type: dar-alifta.org article 6628
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر ذي الحجة لعام 1440هـ
- Publication date (as shown by the site): 01 أغسطس 2019
- Date line signed in the statement: الخميس التاسع والعشرين من شهر ذي القعدة 1440هـ الأول من شهر أغسطس 2019م
- Method: adopted the Saudi Supreme Court decision (no own sighting result stated)
- Quote: "وعلى ذلك تُعلن دار الإفتاء المصرية أن يومَ الجمعة الثاني من شهر أغسطس لعام ألفينِ وتِسعة عشر ميلاديًّا هو أول أيام شهر ذي الحجة لعام ألفٍ وأَربَعِمِائةٍ وأربعين من الهجرة، وأن يومَ السبت الموافق العاشر من شهر أغسطس لعام ألفينِ وتِسعة عشر ميلاديًّا هو التاسعُ من ذي الحجة وهو يومُ عرفة، وأن يومَ الأحد الموافقَ الحادي عشر من شهر أغسطس لعام ألفينِ وتِسعة عشر ميلاديًّا هو العاشرُ من ذي الحجة وهو أول أيام عيد الأضحى المبارك"

#### Dhu al-Hijjah 1440 H: day 1 = 2019-08-02 (Friday)

- Source: https://www.youm7.com/story/2019/8/1/%D8%AF%D8%A7%D8%B1-%D8%A7%D9%84%D8%A5%D9%81%D8%AA%D8%A7%D8%A1-%D8%BA%D8%AF%D9%8B%D8%A7-%D8%A3%D9%88%D9%84-%D8%A3%D9%8A%D8%A7%D9%85-%D8%B4%D9%87%D8%B1-%D8%B0%D9%89-%D8%A7%D9%84%D8%AD%D8%AC%D8%A9-%D9%88%D8%A7%D9%84%D8%A3%D8%AD%D8%AF-11/4358927
- Source type: Youm7 (youm7.com)
- Title: دار الإفتاء: غدًا أول أيام شهر ذى الحجة والأحد 11 أغسطس عيد الأضحى
- Publication date (as shown by the site): الخميس، 01 أغسطس 2019 06:58 م
- Method: not stated in the article (Youm7 places it next to the Saudi announcement of the same evening)
- Note: 'غدًا الجمعة' in an article dated Thursday 1 Aug 2019 resolves to 2019-08-02. Cross-check: Eid 11 Aug 2019 minus 9 days = 2 Aug.
- Quote: "أعلنت دار الإفتاء المصرية، أن غدًا الجمعة أول أيام شهر ذي الحجة، وأن يوم السبت الموافق 10 هو يوم 9 ذي الحجة "وقفة عرفات" وأكدت دار الإفتاء المضرية أن عيد الأضحى المبارك يوم الأحد الموافق 11-8-2019."


### 1441 H

#### Safar 1441 H: day 1 = 2019-09-30 (Monday)

- Source: https://www.dar-alifta.org/ar/articles/details/6710
- Source type: dar-alifta.org article 6710
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر صَفَرَ لعام 1441هـ
- Publication date (as shown by the site): 28 سبتمبر 2019
- Date line signed in the statement: السبت التاسع والعشرين من شهر المحرم 1441هـ الثامن والعشرين من شهر سبتمبر 2019م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الأحد الموافق التاسع والعشرين من شهر سبتمبر لعام ألفين وتسعة عشر ميلاديًّا هو المتمم لشهر المحرم لعام ألفٍ وأربعمائةٍ وواحد وأربعين هجريًّا، وأن يوم الإثنين الموافق الثلاثين من شهر سبتمبر هو أول أيام شهر صَفَرَ لعام ألفٍ وأربعمائةٍ وواحد وأربعين هجريًّا"

#### Jumada al-Ula 1441 H: day 1 = 2019-12-27 (Friday)

- Source: https://www.dar-alifta.org/ar/articles/details/6920
- Source type: dar-alifta.org article 6920
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر جمادى الأولى لعام 1441هـ
- Publication date (as shown by the site): 26 ديسمبر 2019
- Date line signed in the statement: الخميس التاسع والعشرين من شهر ربيع الآخر1441هـ السادس والعشرين من شهر ديسمبر2019م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الجمعة الموافق السابع والعشرين من شهر ديسمبر لعام ألفين وتسعة عشر ميلاديًّا هو أول أيام شهر جمادى الأولى لعام ألفٍ وأربعمائةٍ وواحد وأربعين هجريًّا"

#### Rajab 1441 H: day 1 = 2020-02-25 (Tuesday)

- Source: https://www.dar-alifta.org/ar/articles/details/7019
- Source type: dar-alifta.org article 7019
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر رجب لعام 1441هـ
- Publication date (as shown by the site): 23 فبراير 2020
- Date line signed in the statement: الأحد التاسع والعشرين من شهر جمادى الآخرة 1441 هـ الثالث والعشرين من شهر فبراير 2020م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الإثنين الموافق الرابع والعشرين من شهر فبراير لعام ألفين وعشرين ميلاديًّا هو المتمم لشهر جمادى الآخرة لعام ألفٍ وأربعمائةٍ وواحد وأربعين هجريًّا، وأن يوم الثلاثاء الموافق الخامس والعشرين من شهر فبراير هو أول أيام شهر رجب لعام ألفٍ وأربعمائةٍ وواحد وأربعين هجريًّا"

#### Ramadan 1441 H: day 1 = 2020-04-24 (Friday)

- Source: https://www.dar-alifta.org/ar/articles/details/7098
- Source type: dar-alifta.org article 7098
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر رمضان لعام 1441هـ
- Publication date (as shown by the site): 22 أبريل 2020
- Date line signed in the statement: الأربعاء التاسع والعشرين من شهر شعبان1441 هـ الثاني والعشرين من شهر إبريل 2020م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الخميس الموافق الثالث والعشرين من شهر إبريل لعام ألفين وعشرين ميلاديًّا هو المتمم لشهر شعبان لعام ألفٍ وأربعمائةٍ وواحد وأربعين هجريًّا، وأن يوم الجمعة الموافق الرابع والعشرين من شهر إبريل هو أول أيام شهر رمضان لعام ألفٍ وأربعمائةٍ وواحد وأربعين هجريًّا"

#### Shawwal 1441 H: day 1 = 2020-05-24 (Sunday)

- Source: https://www.dar-alifta.org/ar/articles/details/7129
- Source type: dar-alifta.org article 7129
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر شوال لعام 1441هـ
- Publication date (as shown by the site): 22 مايو 2020
- Date line signed in the statement: الجمعة التاسع والعشرين من شهر رمضان 1441 هـ الثاني والعشرين من شهر مايو 2020م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ السبت الموافق الثالث والعشرين من شهر مايو لعام ألفين وعشرين ميلاديًّا هو المتمم لشهر رمضان لعام ألفٍ وأربعمائةٍ وواحد وأربعين هجريًّا، وأن يوم الأحد الموافق الرابع والعشرين من شهر مايو هو أول أيام شهر شوال لعام ألفٍ وأربعمائةٍ وواحد وأربعين هجريًّا"

#### Dhu al-Hijjah 1441 H: day 1 = 2020-07-22 (Wednesday)

- Source: https://www.dar-alifta.org/ar/articles/details/7176
- Source type: dar-alifta.org article 7176
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر ذي الحجة لعام 1441هـ
- Publication date (as shown by the site): 20 يوليو 2020
- Date line signed in the statement: الإثنين التاسع والعشرين من شهر ذي القعدة 1441هـ العشرين من شهر يوليه 2020م
- Method: adopted the Saudi Supreme Court decision (no own sighting result stated)
- Quote: "وعلى ذلك تُعلن دار الإفتاء المصرية أن يومَ الأربعاء الموافق الثاني والعشرين من شهر يوليه لعام ألفينِ وعشرين ميلاديًّا هو أول أيام شهر ذي الحجة لعام ألفٍ وأَربَعِمِائةٍ وواحد وأربعين من الهجرة، وأن يومَ الخميس الموافق الثلاثين من شهر يوليه لعام ألفينِ وعشرين ميلاديًّا هو التاسعُ من ذي الحجة وهو يومُ عرفة، وأن يومَ الجمعة الموافقَ الواحد والثلاثين من شهر يوليه لعام ألفينِ وعشرين ميلاديًّا هو العاشرُ من ذي الحجة وهو أول أيام عيد الأضحى المبارك"


### 1442 H

#### Safar 1442 H: day 1 = 2020-09-18 (Friday)

- Source: https://www.dar-alifta.org/ar/articles/details/7234
- Source type: dar-alifta.org article 7234
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر صَفر لعام 1442هـ
- Publication date (as shown by the site): 17 سبتمبر 2020
- Date line signed in the statement: الخميس التاسع والعشرين من شهر المحرم 1442هـ السابع عشر من شهر سبتمبر 2020م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الجمعة الموافق الثامن عشر من شهر سبتمبر لعام ألفين وعشرين ميلاديًّا هو أول أيام شهر صَفر لعام ألفٍ وأربعمائةٍ واثنين وأربعين هجريًّا"

#### Rajab 1442 H: day 1 = 2021-02-13 (Saturday)

- Source: https://www.dar-alifta.org/ar/articles/details/7508
- Source type: dar-alifta.org article 7508
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر رجب لعام 1442هـ
- Publication date (as shown by the site): 11 فبراير 2021
- Date line signed in the statement: الخميس التاسع والعشرين من شهر جمادى الآخرة 1442 هـ الحادي عشر من شهر فبراير 2021م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الجمعة الموافق الثاني عشر من شهر فبراير لعام ألفين وواحد وعشرين ميلاديًّا هو المتمم لشهر جمادى الآخرة لعام ألفٍ وأربعمائةٍ واثنين وأربعين هجريًّا، وأن يوم السبت الموافق الثالث عشر من شهر فبراير هو أول أيام شهر رجب لعام ألفٍ وأربعمائةٍ واثنين وأربعين هجريًّا"

#### Sha'ban 1442 H: day 1 = 2021-03-14 (Sunday)

- Source: https://www.dar-alifta.org/ar/articles/details/7564
- Source type: dar-alifta.org article 7564
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر شعبان لعام 1442هـ
- Publication date (as shown by the site): 13 مارس 2021
- Date line signed in the statement: السبت التاسع والعشرين من شهر رجب 1442 هـ الثالث عشر من شهر مارس 2021م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الأحد الموافق الرابع عشر من شهر مارس لعام ألفين وواحد وعشرين ميلاديًّا هو أول أيام شهر شعبان لعام ألفٍ وأربعمائةٍ واثنين وأربعين هجريًّا"

#### Ramadan 1442 H: day 1 = 2021-04-13 (Tuesday)

- Source: https://www.dar-alifta.org/ar/articles/details/7614
- Source type: dar-alifta.org article 7614
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر رمضان لعام 1442هـ
- Publication date (as shown by the site): 11 أبريل 2021
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الاثنينِ الموافقَ الثانيَ عشرَ مِن شهرِ إبريلَ، لعام ألفينِ وواحدٍ وعشرين مِن الميلاد: هو المتمِّمُ لشهرِ شعبانَ لعامِ ألفٍ وأربعمائةٍ واثنين وأربعينَ من الهجرة، وأن يومَ الثلاثاءِ الموافقَ الثالثَ عشرَ مِن شهرِ إبريلَ: هو أولُ أيامِ شهرِ رمضانَ لعامِ ألفٍ وأربعمائةٍ واثنين وأربعين من الهجرة"

#### Shawwal 1442 H: day 1 = 2021-05-13 (Thursday)

- Source: https://www.dar-alifta.org/ar/articles/details/7681
- Source type: dar-alifta.org article 7681
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر شوال لعام 1442هـ
- Publication date (as shown by the site): 11 مايو 2021
- Date line signed in the statement: الثلاثاء التاسع والعشرين من شهر رمضان 1442 هـ الحادي عَشر من شهر مايو 2021م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الأربعاء الموافق الثاني عَشر من شهر مايو لعام ألفين وواحد وعشرين ميلاديًّا هو المتمم لشهر رمضان لعام ألفٍ وأربعمائةٍ واثنين وأربعين هجريًّا، وأن يوم الخميس الموافق الثالث عَشر من شهر مايو هو أول أيام شهر شوال لعام ألفٍ وأربعمائةٍ واثنين وأربعين هجريًّا"

#### Dhu al-Qa'dah 1442 H: day 1 = 2021-06-12 (Saturday)

- Source: https://www.dar-alifta.org/ar/articles/details/7720
- Source type: dar-alifta.org article 7720
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر ذي القعدة لعام 1442هـ
- Publication date (as shown by the site): 10 يونيو 2021
- Date line signed in the statement: الخميس التاسع والعشرين من شهر شوال1442هـ العاشر من شهر يونيه2021م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الجمعة الموافق الحادي عشر من شهر يونيه لعام ألفين وواحد وعشرين ميلاديًّا هو المتمّمُ لشهِر شوالٍ لعام ألفٍ وأربعمائةٍ واثنين وأربعين هجريًّا، وأن يوم السبت الموافق الثاني عشر من شهر يونيه هو أولُ أيامِ شهرِ ذي القعدةِ لعام ألفٍ وأربعمائةٍ واثنين وأربعين هجريًّا"

#### Dhu al-Hijjah 1442 H: day 1 = 2021-07-11 (Sunday)

- Source: https://www.dar-alifta.org/ar/articles/details/7773
- Source type: dar-alifta.org article 7773
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر ذي الحجة لعام 1442هـ
- Publication date (as shown by the site): 10 يوليو 2021
- Method: adopted the Saudi Supreme Court decision no. 146, and states the crescent was also seen by the Egyptian sharia committees (وقد تمت رؤية الهلال بواسطة اللجان الشرعية)
- Quote: "وعلى ذلك كله تُعلن دار الإفتاء المصرية أن يومَ الأحد الموافق 11 من شهر يوليه 2021م هو أول أيام ذي الحجة 1442هـ، وأن يومَ الإثنين الموافق 19 من شهر يوليه 2021م هو التاسعُ من ذي الحجة وهو يومُ عرفة، وأن يومَ الثلاثاء الموافقَ 20 من شهر يوليه 2021م هو العاشرُ من ذي الحجة وهو أول أيام عيد الأضحى المبارك"

#### Dhu al-Hijjah 1442 H: day 1 = 2021-07-11 (Sunday)

- Source: https://www.youm7.com/story/2021/7/10/%D8%AF%D8%A7%D8%B1-%D8%A7%D9%84%D8%A5%D9%81%D8%AA%D8%A7%D8%A1-%D8%A7%D9%84%D8%A7%D8%AB%D9%86%D9%8A%D9%86-19-%D9%8A%D9%88%D9%84%D9%8A%D9%88-%D9%88%D9%82%D9%81%D8%A9-%D8%B9%D8%B1%D9%81%D8%A7%D8%AA-%D9%88%D8%A7%D9%84%D8%AB%D9%84%D8%A7%D8%AB%D8%A7%D8%A1-20-%D8%A3%D9%88%D9%84/5385681
- Source type: Youm7 (youm7.com)
- Title: دار الإفتاء: الاثنين 19 يوليو وقفة عرفات والثلاثاء 20 أول أيام عيد الأضحى
- Publication date (as shown by the site): السبت، 10 يوليو 2021 07:16 م
- Method: follows the Saudi Supreme Court statement (Dhu al-Qa'dah ended on Saturday 10 Jul 2021)
- Quote: "أعلنت دار الإفتاء المصرية أن غدا الأحد الحادى عشر من شهر يوليو لعام 2021 هو أول أيام شهر ذي الحجة لعام 1442 من الهجرة، وأن يوم الاثنين الموافق 19 يوليو هو التاسعُ من ذي الحجة وهو يومُ عرفة، وأن يومَ الثلاثاء 20 يوليو هو العاشرُ من ذي الحجة وهو أول أيام عيد الأضحى المبارك."


### 1443 H

#### Safar 1443 H: day 1 = 2021-09-08 (Wednesday)

- Source: https://www.dar-alifta.org/ar/articles/details/7937
- Source type: dar-alifta.org article 7937
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر صَفر لعام 1443هـ
- Publication date (as shown by the site): 06 سبتمبر 2021
- Date line signed in the statement: الاثنين التاسع والعشرين من شهر المحرم 1443هـ السادس من شهر سبتمبر 2021م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الثلاثاء الموافق السابع من شهر سبتمبر لعام ألفين وواحد وعشرين ميلاديًّا هو المتمم لشهر المحرم لعام ألفٍ وأربعمائةٍ وثلاثة وأربعين هجريًّا، وأن يوم الأربعاء الموافق الثامن من شهر سبتمبر هو أول أيام شهر صَفر لعام ألفٍ وأربعمائةٍ وثلاثة وأربعين هجريًّا"

#### Rajab 1443 H: day 1 = 2022-02-02 (Wednesday)

- Source: https://www.dar-alifta.org/ar/articles/details/8160
- Source type: dar-alifta.org article 8160
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر رجب لعام 1443هـ
- Publication date (as shown by the site): 01 فبراير 2022
- Date line signed in the statement: الثلاثاء التاسع والعشرين من شهر جمادى الآخرة 1443هـ الأول من شهر فبراير 2022م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الأربعاء الموافق الثاني من شهر فبراير لعام ألفين واثنين وعشرين ميلاديًّا هو أول أيام شهر رجب لعام ألفٍ وأربعمائةٍ وثلاثة وأربعين هجريًّا"

#### Sha'ban 1443 H: day 1 = 2022-03-04 (Friday)

- Source: https://www.dar-alifta.org/ar/articles/details/8191
- Source type: dar-alifta.org article 8191
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر شعبان لعام 1443هـ
- Publication date (as shown by the site): 02 مارس 2022
- Date line signed in the statement: الأربعاء التاسع والعشرين من رجب 1443هـ الثاني من شهر مارس 2022 م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الخميس الموافق الثالث من شهر مارس لعام ألفين واثنين وعشرين ميلاديًّا هو المتمم لشهر رجب لعام ألفٍ وأربعمائةٍ وثلاثة وأربعين هجريًّا، وأن يوم الجمعة الموافق الرابع من شهر مارس هو أول أيام شهر شعبان لعام ألفٍ وأربعمائةٍ وثلاثة وأربعين هجريًّا"

#### Ramadan 1443 H: day 1 = 2022-04-02 (Saturday)

- Source: https://www.dar-alifta.org/ar/articles/details/8218
- Source type: dar-alifta.org article 8218
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر رمضان لعام 1443هـ
- Publication date (as shown by the site): 01 أبريل 2022
- Date line signed in the statement: الجمعة التاسع والعشرين من شهر شعبان1443هـ الأول من شهر أبريل2022م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ السبت الموافق الثاني من شهر أبريل لعام ألفين واثنين وعشرين ميلاديًّا هو أول أيام شهر رمضان لعام ألفٍ وأربعمائةٍ وثلاثة وأربعين هجريًّا"

#### Shawwal 1443 H: day 1 = 2022-05-02 (Monday)

- Source: https://www.dar-alifta.org/ar/articles/details/8262
- Source type: dar-alifta.org article 8262
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر شوال لعام 1443هـ
- Publication date (as shown by the site): 30 أبريل 2022
- Date line signed in the statement: السبت التاسع والعشرين من رمضان 1443هـ الثلاثين من شهر أبريل2022 م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الأحد الموافق الأول من شهر مايو لعام ألفين واثنين وعشرين ميلاديًّا هو المتمم لشهر رمضان لعام ألفٍ وأربعمائةٍ وثلاثة وأربعين هجريًّا، وأن يوم الإثنين الموافق الثاني من شهر مايو هو أول أيام شهر شوال لعام ألفٍ وأربعمائةٍ وثلاثة وأربعين هجريًّا"

#### Dhu al-Qa'dah 1443 H: day 1 = 2022-06-01 (Wednesday)

- Source: https://www.dar-alifta.org/ar/articles/details/8341
- Source type: dar-alifta.org article 8341
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر ذي القعدة لعام 1443هـ
- Publication date (as shown by the site): 30 مايو 2022
- Date line signed in the statement: الإثنين التاسع والعشرين من شوال 1443هـ الثلاثين من شهر مايو2022 م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الثلاثاء الموافق الواحد والثلاثين من شهر مايو لعام ألفين واثنين وعشرين ميلاديًّا هو المتمم لشهر شوال لعام ألفٍ وأربعمائةٍ وثلاثة وأربعين هجريًّا، وأن يوم الأربعاء الموافق الأول من شهر يونيو هو أول أيام شهر ذي القعدة لعام ألفٍ وأربعمائةٍ وثلاثة وأربعين هجريًّا"

#### Dhu al-Hijjah 1443 H: day 1 = 2022-06-30 (Thursday)

- Source: https://www.dar-alifta.org/ar/articles/details/8418
- Source type: dar-alifta.org article 8418
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر ذي الحجة لعام 1443هـ
- Publication date (as shown by the site): 29 يونيو 2022
- Date line signed in the statement: الأربعاء التاسع والعشرين من ذي القعدة 1443هـ التاسع والعشرين من شهر يونية 2022م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الخميس الموافق الثلاثين من شهر يونية لعام ألفين واثنين وعشرين ميلاديًّا هو أول أيام شهر ذي الحجة لعام ألفٍ وأربعمائةٍ وثلاثة وأربعين هجريًّا"


### 1444 H

#### Safar 1444 H: day 1 = 2022-08-28 (Sunday)

- Source: https://www.dar-alifta.org/ar/articles/details/8459
- Source type: dar-alifta.org article 8459
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر صفر لعام 1444هـ
- Publication date (as shown by the site): 27 أغسطس 2022
- Date line signed in the statement: السبت التاسع والعشرين من المحرم 1444هـ السابع والعشرين من شهر أغسطس 2022م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الأحد الموافق الثامن والعشرين من شهر أغسطس لعام ألفين واثنين وعشرين ميلاديًّا هو أول أيام شهر صفر لعام ألفٍ وأربعمائةٍ وأربعة وأربعين هجريًّا"

#### Rabi al-Awwal 1444 H: day 1 = 2022-09-27 (Tuesday)

- Source: https://www.dar-alifta.org/ar/articles/details/8480
- Source type: dar-alifta.org article 8480
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر ربيع الأول لعام 1444هـ
- Publication date (as shown by the site): 25 سبتمبر 2022
- Date line signed in the statement: الأحد التاسع والعشرين من صفر 1444هـ الخامس والعشرين من شهر سبتمبر 2022م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الإثنين الموافق السادس والعشرين من شهر سبتمبر لعام ألفين واثنين وعشرين ميلاديًّا هو المتمم لشهر صفر لعام ألف وأربعمائةٍ وأربعة وأربعين هجريًّا، وأن يوم الثلاثاء الموافق السابع والعشرين من شهر سبتمبر لعام ألفين واثنين وعشرين ميلاديًّا هو أول أيام شهر ربيع الأول لعام ألف وأربعمائة وأربعة وأربعين هجريًّا"

#### Jumada al-Akhira 1444 H: day 1 = 2022-12-25 (Sunday)

- Source: https://www.dar-alifta.org/ar/articles/details/8605
- Source type: dar-alifta.org article 8605
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر جمادى الآخرة لعام 1444هـ
- Publication date (as shown by the site): 23 ديسمبر 2022
- Date line signed in the statement: الجمعة التاسع والعشرين من جمادى الأولى 1444هـ الثالث والعشرين من شهر ديسمبر 2022م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ السبت الموافق الرابع والعشرين من شهر ديسمبر لعام ألفين واثنين وعشرين ميلاديًّا هو المتمم لشهر جمادى الأولى لعام ألف وأربعمائة وأربعة وأربعين هجريًّا، وأن يوم الأحد الموافق الخامس والعشرين من شهر ديسمبر لعام ألفين واثنين وعشرين ميلاديًّا هو أول أيام شهر جمادى الآخرة لعام ألف وأربعمائة وأربعة وأربعين هجريًّا"

#### Rajab 1444 H: day 1 = 2023-01-23 (Monday)

- Source: https://www.dar-alifta.org/ar/articles/details/8631
- Source type: dar-alifta.org article 8631
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر رجب لعام 1444هـ
- Publication date (as shown by the site): 22 يناير 2023
- Date line signed in the statement: الأحد التاسع والعشرين من جمادى الآخرة 1444هـ الثاني والعشرين من شهر يناير 2023م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الإثنين الموافق الثالث والعشرين من شهر يناير لعام ألفين وثلاثة وعشرين ميلاديًّا هو أول أيام شهر رجب لعام ألفٍ وأربعمائةٍ وأربعة وأربعين هجريًّا"

#### Sha'ban 1444 H: day 1 = 2023-02-21 (Tuesday)

- Source: https://www.dar-alifta.org/ar/articles/details/8656
- Source type: dar-alifta.org article 8656
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر شعبان لعام 1444هـ
- Publication date (as shown by the site): 20 فبراير 2023
- Date line signed in the statement: الإثنين التاسع والعشرين من شهر رجب 1444هـ العشرين من شهر فبراير 2023م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الثلاثاء الموافق الواحد والعشرين من شهر فبراير لعام ألفين وثلاثة وعشرين ميلاديًّا هو أول أيام شهر شعبان لعام ألفٍ وأربعمائةٍ وأربعة وأربعين هجريًّا"

#### Ramadan 1444 H: day 1 = 2023-03-23 (Thursday)

- Source: https://www.dar-alifta.org/ar/articles/details/8690
- Source type: dar-alifta.org article 8690
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر رمضان لعام 1444هـ
- Publication date (as shown by the site): 21 مارس 2023
- Date line signed in the statement: الثلاثاء التاسع والعشرين من شهر شعبان 1444هـ الواحد والعشرين من شهر مارس 2023م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الأربعاء الموافق الثاني والعشرين من شهر مارس لعام ألفين وثلاثة وعشرين ميلاديًّا هو المتمم لشهر شعبان لعام ألف وأربعمائة وأربعة وأربعين هجريًّا، وأن يوم الخميس الموافق الثالث والعشرين من شهر مارس لعام ألفين وثلاثة وعشرين ميلاديًّا هو أول أيام شهر رمضان لعام ألف وأربعمائة وأربعة وأربعين هجريًّا"

#### Shawwal 1444 H: day 1 = 2023-04-21 (Friday)

- Source: https://www.dar-alifta.org/ar/articles/details/8764
- Source type: dar-alifta.org article 8764
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر شوال لعام 1444هـ
- Publication date (as shown by the site): 20 أبريل 2023
- Date line signed in the statement: الخميس التاسع والعشرين من شهر رمضان 1444هـ العشرين من شهر إبريل 2023م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الجمعة الموافق الواحد والعشرين من شهر إبريل لعام ألفين وثلاثة وعشرين ميلاديًّا هو أول أيام شهر شوال لعام ألفٍ وأربعمائةٍ وأربعة وأربعين هجريًّا"

#### Dhu al-Hijjah 1444 H: day 1 = 2023-06-19 (Monday)

- Source: https://www.dar-alifta.org/ar/articles/details/8844
- Source type: dar-alifta.org article 8844
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر ذي الحجة لعام 1444هـ
- Publication date (as shown by the site): 18 يونيو 2023
- Date line signed in the statement: الأحد التاسع والعشرين من شهر ذي القعدة 1444هـ الثامن عشر من شهر يونيو 2023م
- Method: sighted (statement cites own sighting committees plus the Saudi Supreme Court decision)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الإثنين الموافق التاسع عشر من شهر يونيو لعام ألفين وثلاثة وعشرين ميلاديًّا هو أول أيام شهر ذي الحجة لعام ألفٍ وأربعمائةٍ وأربعة وأربعين هجريًّا"


### 1445 H

#### Muharram 1445 H: day 1 = 2023-07-19 (Wednesday)

- Source: https://www.dar-alifta.org/ar/articles/details/8865
- Source type: dar-alifta.org article 8865
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر المحرم لعام 1445هـ
- Publication date (as shown by the site): 17 يوليو 2023
- Date line signed in the statement: الإثنين التاسع والعشرين من شهر ذي الحجة 1444هـ السابع عشر من شهر يوليو 2023م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الثلاثاء الموافق الثامن عشر من شهر يوليو لعام ألفين وثلاثة وعشرين ميلاديًّا هو المتمم لشهر ذي الحجة لعام ألف وأربعمائة وأربعة وأربعين هجريًّا، وأن يوم الأربعاء الموافق التاسع عشر من شهر يوليو لعام ألفين وثلاثة وعشرين ميلاديًّا هو أول أيام شهر المحرم لعام ألف وأربعمائة وخمسة وأربعين هجريًّا"

#### Safar 1445 H: day 1 = 2023-08-17 (Thursday)

- Source: https://www.dar-alifta.org/ar/articles/details/8876
- Source type: dar-alifta.org article 8876
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر صفر لعام 1445هـ
- Publication date (as shown by the site): 16 أغسطس 2023
- Date line signed in the statement: الأربعاء التاسع والعشرين من شهر المحرم 1445هـ السادس عشر من شهر أغسطس 2023م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الخميس الموافق السابع عشر من شهر أغسطس لعام ألفين وثلاثة وعشرين ميلاديًّا هو أول أيام شهر صفر لعام ألفٍ وأربعمائةٍ وخمسة وأربعين هجريًّا"

#### Rabi al-Akhir 1445 H: day 1 = 2023-10-16 (Monday)

- Source: https://www.dar-alifta.org/ar/articles/details/8939
- Source type: dar-alifta.org article 8939
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر ربيع الآخر لعام 1445هـ
- Publication date (as shown by the site): 14 أكتوبر 2023
- Date line signed in the statement: السبت التاسع والعشرين من شهر ربيع الأول 1445هـ الرابع عشر من شهر أكتوبر 2023م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الأحد الموافق الخامس عشر من شهر أكتوبر لعام ألفين وثلاثة وعشرين ميلاديًّا هو المتمم لشهر ربيع الأول لعام ألف وأربعمائة وخمسة وأربعين هجريًّا، وأن يوم الإثنين الموافق السادس عشر من شهر أكتوبر لعام ألفين وثلاثة وعشرين ميلاديًّا هو أول أيام شهر ربيع الآخر لعام ألف وأربعمائة وخمسة وأربعين هجريًّا"

#### Jumada al-Ula 1445 H: day 1 = 2023-11-15 (Wednesday)

- Source: https://www.dar-alifta.org/ar/articles/details/9053
- Source type: dar-alifta.org article 9053
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر جمادى الأولى لعام 1445هـ
- Publication date (as shown by the site): 13 نوفمبر 2023
- Date line signed in the statement: الإثنين التاسع والعشرين من شهر ربيع الآخر 1445هـ الثالث عشر من شهر نوفمبر 2023م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الثلاثاء الموافق الرابع عشر من شهر نوفمبر لعام ألفين وثلاثة وعشرين ميلاديًّا هو المتمم لشهر جمادى الأولى لعام ألف وأربعمائة وخمسة وأربعين هجريًّا، وأن يوم الأربعاء الموافق الخامس عشر من شهر نوفمبر لعام ألفين وثلاثة وعشرين ميلاديًّا هو أول أيام شهر جمادى الأولى لعام ألف وأربعمائة وخمسة وأربعين هجريًّا"

#### Rajab 1445 H: day 1 = 2024-01-13 (Saturday)

- Source: https://www.dar-alifta.org/ar/articles/details/9173
- Source type: dar-alifta.org article 9173
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر رجب لعام 1445هـ
- Publication date (as shown by the site): 11 يناير 2024
- Date line signed in the statement: الخميس التاسع والعشرين من شهر جمادى الآخرة 1445هـ الحادي عشر من شهر يناير 2024م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الجمعة الموافق الثاني عشر من شهر يناير لعام ألفين وأربعة وعشرين ميلاديًّا هو المتمم لشهر جمادى الآخرة لعام ألف وأربعمائة وخمسة وأربعين هجريًّا، وأن يوم السبت الموافق الثالث عشر من شهر يناير لعام ألفين وأربعة وعشرين ميلاديًّا هو أول أيام شهر رجب لعام ألف وأربعمائة وخمسة وأربعين هجريًّا"

#### Sha'ban 1445 H: day 1 = 2024-02-11 (Sunday)

- Source: https://www.dar-alifta.org/ar/articles/details/9244
- Source type: dar-alifta.org article 9244
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر شعبان لعام 1445هـ
- Publication date (as shown by the site): 10 فبراير 2024
- Date line signed in the statement: السبت التاسع والعشرين من شهر رجب 1445هـ العاشر من شهر فبراير 2024م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الأحد الموافق الحادي عشر من شهر فبراير لعام ألفين وأربعة وعشرين ميلاديًّا هو أول أيام شهر شعبان لعام ألفٍ وأربعمائةٍ وخمسة وأربعين هجريًّا"

#### Ramadan 1445 H: day 1 = 2024-03-11 (Monday)

- Source: https://www.dar-alifta.org/ar/articles/details/9285
- Source type: dar-alifta.org article 9285
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر رمضان لعام 1445هـ
- Publication date (as shown by the site): 10 مارس 2024
- Date line signed in the statement: الأحد التاسع والعشرين من شهر شعبان 1445هـ العاشر من شهر مارس 2024م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الإثنين الموافق الحادي عشر من شهر مارس لعام ألفين وأربعة وعشرين ميلاديًّا هو أول أيام شهر رمضان لعام ألفٍ وأربعمائةٍ وخمسة وأربعين هجريًّا"

#### Shawwal 1445 H: day 1 = 2024-04-10 (Wednesday)

- Source: https://www.dar-alifta.org/ar/articles/details/9352
- Source type: dar-alifta.org article 9352
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر شوال لعام 1445هـ
- Publication date (as shown by the site): 08 أبريل 2024
- Date line signed in the statement: الإثنين التاسع والعشرين من شهر رمضان 1445هـ الثامن من شهر إبريل 2024م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الثلاثاء الموافق التاسع من شهر إبريل لعام ألفين وأربعة وعشرين ميلاديًّا هو المتمم لشهر رمضان لعام ألف وأربعمائة وخمسة وأربعين هجريًّا، وأن يوم الأربعاء الموافق العاشر من شهر إبريل لعام ألفين وأربعة وعشرين ميلاديًّا هو أول أيام شهر شوال لعام ألف وأربعمائة وخمسة وأربعين هجريًّا"

#### Dhu al-Hijjah 1445 H: day 1 = 2024-06-07 (Friday)

- Source: https://www.dar-alifta.org/ar/articles/details/9403
- Source type: dar-alifta.org article 9403
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر ذي الحجة لعام 1445هـ
- Publication date (as shown by the site): 06 يونيو 2024
- Date line signed in the statement: الخميس التاسع والعشرين من شهر ذي القعدة 1445هـ السادس من شهر يونيه 2024م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الجمعة الموافق السابع من شهر يونيه لعام ألفين وأربعة وعشرين ميلاديًّا هو أول أيام شهر ذي الحجة لعام ألفٍ وأربعمائةٍ وخمسة وأربعين هجريًّا"


### 1446 H

#### Muharram 1446 H: day 1 = 2024-07-07 (Sunday)

- Source: https://www.dar-alifta.org/ar/articles/details/9427
- Source type: dar-alifta.org article 9427
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر المحرم لعام 1446هـ
- Publication date (as shown by the site): 05 يوليو 2024
- Date line signed in the statement: الجمعة التاسع والعشرين من شهر ذي الحجة 1445هـ الخامس من شهر يوليو 2024م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ السبت الموافق السادس من شهر يوليو لعام ألفين وأربعة وعشرين ميلاديًّا هو المتمم لشهر ذي الحجة لعام ألف وأربعمائة وخمسة وأربعين هجريًّا، وأن يوم الأحد الموافق السابع من شهر يوليو لعام ألفين وأربعة وعشرين ميلاديًّا هو أول أيام شهر المحرم لعام ألف وأربعمائة وستة وأربعين هجريًّا"

#### Safar 1446 H: day 1 = 2024-08-06 (Tuesday)

- Source: https://www.dar-alifta.org/ar/articles/details/9552
- Source type: dar-alifta.org article 9552
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر صفر لعام 1446هـ
- Publication date (as shown by the site): 04 أغسطس 2024
- Date line signed in the statement: الأحد التاسع والعشرين من شهر المحرم 1446هـ الرابع من شهر أغسطس2024م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الإثنين الموافق الخامس من شهر أغسطس لعام ألفين وأربعة وعشرين ميلاديًّا هو المتمم لشهر المحرم لعام ألف وأربعمائة وستة وأربعين هجريًّا، وأن يوم الثلاثاء الموافق السادس من شهر أغسطس لعام ألفين وأربعة وعشرين ميلاديًّا هو أول أيام شهر صفر لعام ألف وأربعمائة وستة وأربعين هجريًّا"

#### Rabi al-Awwal 1446 H: day 1 = 2024-09-04 (Wednesday)

- Source: https://www.dar-alifta.org/ar/articles/details/9616
- Source type: dar-alifta.org article 9616
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر ربيع الأول لعام 1446هـ
- Publication date (as shown by the site): 03 سبتمبر 2024
- Date line signed in the statement: الثلاثاء التاسع والعشرين من شهر صفر1446هـ الثالث من شهر سبتمبر 2024م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الأربعاء الموافق الرابع من شهر سبتمبر لعام ألفين وأربعة وعشرين ميلاديًّا هو أول أيام شهر ربيع الأول لعام ألفٍ وأربعمائةٍ وستة وأربعين هجريًّا"

#### Rajab 1446 H: day 1 = 2025-01-01 (Wednesday)

- Source: https://www.dar-alifta.org/ar/articles/details/9859
- Source type: dar-alifta.org article 9859
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر رجب لعام 1446هـ
- Publication date (as shown by the site): 31 ديسمبر 2024
- Date line signed in the statement: الثلاثاء التاسع والعشرين من شهر جمادى الآخرة 1446هـ الحادي والثلاثين من شهر ديسمبر 2024 م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الأربعاء الموافق الأول من شهر يناير لعام ألفين وخمسة وعشرين ميلاديًّا هو أول أيام شهر رجب لعام ألفٍ وأربعمائةٍ وستة وأربعين هجريًّا"

#### Sha'ban 1446 H: day 1 = 2025-01-31 (Friday)

- Source: https://www.dar-alifta.org/ar/articles/details/9914
- Source type: dar-alifta.org article 9914
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر شعبان لعام 1446هـ
- Publication date (as shown by the site): 29 يناير 2025
- Date line signed in the statement: الأربعاء التاسع والعشرين من شهر رجب 1446هـ الموافق التاسع والعشرين من شهر يناير 2025م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الخميس الموافق الثلاثين من شهر يناير لعام ألفين وخمسة وعشرين ميلاديًّا هو المتمم لشهر رجب لعام ألف وأربعمائة وستة وأربعين هجريًّا، وأن يوم الجمعة الموافق الواحد والثلاثين من شهر يناير لعام ألفين وخمسة وعشرين ميلاديًّا هو أول أيام شهر شعبان لعام ألف وأربعمائة وستة وأربعين هجريًّا"

#### Ramadan 1446 H: day 1 = 2025-03-01 (Saturday)

- Source: https://www.dar-alifta.org/ar/articles/details/10004
- Source type: dar-alifta.org article 10004
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر رمضان لعام 1446هـ
- Publication date (as shown by the site): 28 فبراير 2025
- Date line signed in the statement: الجمعة التاسع والعشرين من شهر شعبان 1446هـ الموافق الثامن والعشرين من شهر فبراير 2025م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ السبت الموافق الأول من شهر مارس لعام ألفين وخمسة وعشرين ميلاديًّا هو أول أيام شهر رمضان لعام ألفٍ وأربعمائةٍ وستة وأربعين هجريًّا"

#### Shawwal 1446 H: day 1 = 2025-03-31 (Monday)

- Source: https://www.dar-alifta.org/ar/articles/details/10089
- Source type: dar-alifta.org article 10089
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر شوال لعام 1446هـ
- Publication date (as shown by the site): 29 مارس 2025
- Date line signed in the statement: السبت التاسع والعشرين من شهر رمضان 1446هـ الموافق التاسع والعشرين من شهر مارس 2025م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الأحد الموافق الثلاثين من شهر مارس لعام ألفين وخمسة وعشرين ميلاديًّا هو المتمم لشهر رمضان لعام ألف وأربعمائة وستة وأربعين هجريًّا، وأن يوم الإثنين الموافق الواحد والثلاثين من شهر مارس لعام ألفين وخمسة وعشرين ميلاديًّا هو أول أيام شهر شوال لعام ألف وأربعمائة وستة وأربعين هجريًّا"

#### Dhu al-Qa'dah 1446 H: day 1 = 2025-04-29 (Tuesday)

- Source: https://www.dar-alifta.org/ar/articles/details/10152
- Source type: dar-alifta.org article 10152
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر ذي القعدة لعام 1446هـ
- Publication date (as shown by the site): 28 أبريل 2025
- Date line signed in the statement: الإثنين التاسع والعشرين من شهر شوال 1446هـ الموافق الثامن والعشرين من شهر إبريل 2025م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الثلاثاء الموافق التاسع والعشرين من شهر إبريل لعام ألفين وخمسة وعشرين ميلاديًّا هو أول أيام شهر ذي القعدة لعام ألفٍ وأربعمائةٍ وستة وأربعين هجريًّا"

#### Dhu al-Hijjah 1446 H: day 1 = 2025-05-28 (Wednesday)

- Source: https://www.dar-alifta.org/ar/articles/details/10212
- Source type: dar-alifta.org article 10212
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر ذي الحجة لعام 1446هـ
- Publication date (as shown by the site): 27 مايو 2025
- Date line signed in the statement: الثلاثاء التاسع والعشرين من شهر ذي القعدة 1446هـ الموافق السابع والعشرين من شهر مايو 2025م
- Method: sighted (statement cites own sighting committees plus the Saudi Supreme Court decision)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الأربعاء الموافق الثامن والعشرين من شهر مايو لعام ألفين وخمسة وعشرين ميلاديًّا هو أول أيام شهر ذي الحجة لعام ألفٍ وأربعمائةٍ وستة وأربعين هجريًّا"


### 1447 H

#### Muharram 1447 H: day 1 = 2025-06-26 (Thursday)

- Source: https://www.dar-alifta.org/ar/articles/details/10277
- Source type: dar-alifta.org article 10277
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر المحرم لعام 1447هـ
- Publication date (as shown by the site): 25 يونيو 2025
- Date line signed in the statement: الأربعاء التاسع والعشرين من شهر ذي الحجة 1446هـ الموافق الخامس والعشرين من شهر يونيو 2025م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الخميس الموافق السادس والعشرين من شهر يونيو لعام ألفين وخمسة وعشرين ميلاديًّا هو أول أيام شهر المحرم لعام ألفٍ وأربعمائةٍ وسبعة وأربعين هجريًّا"

#### Safar 1447 H: day 1 = 2025-07-26 (Saturday)

- Source: https://www.dar-alifta.org/ar/articles/details/10353
- Source type: dar-alifta.org article 10353
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر صفر لعام 1447هـ
- Publication date (as shown by the site): 24 يوليو 2025
- Date line signed in the statement: الخميس التاسع والعشرين من شهر المحرم 1447هـ الموافق الرابع والعشرين من شهر يوليو 2025م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الجمعة الموافق الخامس والعشرين من شهر يوليو لعام ألفين وخمسة وعشرين ميلاديًّا هو المتمم لشهر المحرم لعام ألف وأربعمائة وسبعة وأربعين هجريًّا، وأن يوم السبت الموافق السادس والعشرين من شهر يوليو لعام ألفين وخمسة وعشرين ميلاديًّا هو أول أيام شهر صفر لعام ألف وأربعمائة وسبعة وأربعين هجريًّا"

#### Rabi al-Awwal 1447 H: day 1 = 2025-08-24 (Sunday)

- Source: https://www.dar-alifta.org/ar/articles/details/10466
- Source type: dar-alifta.org article 10466
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر ربيع الأول لعام 1447هـ
- Publication date (as shown by the site): 23 أغسطس 2025
- Date line signed in the statement: السبت التاسع والعشرين من شهر صفر 1447هـ الموافق الثالث والعشرين من شهر أغسطس2025م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الأحد الموافق الرابع والعشرين من شهر أغسطس لعام ألفين وخمسة وعشرين ميلاديًّا هو أول أيام شهر ربيع الأول لعام ألفٍ وأربعمائةٍ وسبعة وأربعين هجريًّا"

#### Rabi al-Akhir 1447 H: day 1 = 2025-09-23 (Tuesday)

- Source: https://www.dar-alifta.org/ar/articles/details/10558
- Source type: dar-alifta.org article 10558
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر ربيع الآخر لعام 1447هـ
- Publication date (as shown by the site): 21 سبتمبر 2025
- Date line signed in the statement: الأحد التاسع والعشرين من شهر ربيع الأول 1447هـ الموافق الواحد والعشرين من شهر سبتمبر2025م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الإثنين الموافق الثاني والعشرين من شهر سبتمبر لعام ألفين وخمسة وعشرين ميلاديًّا هو المتمم لشهر ربيع الأول لعام ألف وأربعمائة وسبعة وأربعين هجريًّا، وأن يوم الثلاثاء الموافق الثالث والعشرين من شهر سبتمبر لعام ألفين وخمسة وعشرين ميلاديًّا هو أول أيام شهر ربيع الآخر لعام ألف وأربعمائة وسبعة وأربعين هجريًّا"

#### Jumada al-Ula 1447 H: day 1 = 2025-10-23 (Thursday)

- Source: https://www.dar-alifta.org/ar/articles/details/10643
- Source type: dar-alifta.org article 10643
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر جمادى الأولى لعام 1447هـ
- Publication date (as shown by the site): 21 أكتوبر 2025
- Date line signed in the statement: الثلاثاء التاسع والعشرين من شهر ربيع الآخر 1447هـ الموافق الواحد والعشرين من شهر أكتوبر 2025م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الأربعاء الموافق الثاني والعشرين من شهر أكتوبر لعام ألفين وخمسة وعشرين ميلاديًّا هو المتمم لشهر ربيع الآخر لعام ألف وأربعمائة وسبعة وأربعين هجريًّا، وأن يوم الخميس الموافق الثالث والعشرين من شهر أكتوبر لعام ألفين وخمسة وعشرين ميلاديًّا هو أول أيام شهر جمادى الأولى لعام ألف وأربعمائة وسبعة وأربعين هجريًّا"

#### Jumada al-Akhira 1447 H: day 1 = 2025-11-22 (Saturday)

- Source: https://www.dar-alifta.org/ar/articles/details/10727
- Source type: dar-alifta.org article 10727
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر جمادى الآخرة لعام 1447هـ
- Publication date (as shown by the site): 20 نوفمبر 2025
- Date line signed in the statement: الخميس التاسع والعشرين من شهر جمادى الأولى 1447هـ الموافق العشرين من شهر نوفمبر 2025م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الجمعة الموافق الواحد والعشرين من شهر نوفمبر لعام ألفين وخمسة وعشرين ميلاديًّا هو المتمم لشهر جمادى الأولى لعام ألف وأربعمائة وسبعة وأربعين هجريًّا، وأن يوم السبت الموافق الثاني والعشرين من شهر نوفمبر لعام ألفين وخمسة وعشرين ميلاديًّا هو أول أيام شهر جمادى الآخرة لعام ألف وأربعمائة وسبعة وأربعين هجريًّا"

#### Rajab 1447 H: day 1 = 2025-12-21 (Sunday)

- Source: https://www.dar-alifta.org/ar/articles/details/10846
- Source type: dar-alifta.org article 10846
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر رجب لعام 1447هـ
- Publication date (as shown by the site): 20 ديسمبر 2025
- Date line signed in the statement: السبت التاسع والعشرين من شهر جمادى الآخرة 1447هـ الموافق العشرين من شهر ديسمبر 2025م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الأحد الموافق الواحد والعشرين من شهر ديسمبر لعام ألفين وخمسة وعشرين ميلاديًّا هو أول أيام شهر رجب لعام ألفٍ وأربعمائةٍ وسبعة وأربعين هجريًّا"

#### Sha'ban 1447 H: day 1 = 2026-01-20 (Tuesday)

- Source: https://www.dar-alifta.org/ar/articles/details/10915
- Source type: dar-alifta.org article 10915
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر شعبان لعام 1447هـ
- Publication date (as shown by the site): 18 يناير 2026
- Date line signed in the statement: الأحد التاسع والعشرين من شهر رجب 1447هـ الموافق الثامن عشر من شهر يناير2026م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الإثنين الموافق التاسع عشر من شهر يناير لعام ألفين وستة وعشرين ميلاديًّا هو المتمم لشهر رجب لعام ألف وأربعمائة وسبعة وأربعين هجريًّا، وأن يوم الثلاثاء الموافق العشرين من شهر يناير لعام ألفين وستة وعشرين ميلاديًّا هو أول أيام شهر شعبان لعام ألف وأربعمائة وسبعة وأربعين هجريًّا"

#### Ramadan 1447 H: day 1 = 2026-02-19 (Thursday)

- Source: https://www.dar-alifta.org/ar/articles/details/11039
- Source type: dar-alifta.org article 11039
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر رمضان لعام 1447هـ
- Publication date (as shown by the site): 17 فبراير 2026
- Date line signed in the statement: الثلاثاء التاسع والعشرين من شهر شعبان 1447هـ الموافق السابع عشر من شهر فبراير2026م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الأربعاء الموافق الثامن عشر من شهر فبراير لعام ألفين وستة وعشرين ميلاديًّا هو المتمم لشهر شعبان لعام ألف وأربعمائة وسبعة وأربعين هجريًّا، وأن يوم الخميس الموافق التاسع عشر من شهر فبراير لعام ألفين وستة وعشرين ميلاديًّا هو أول أيام شهر رمضان لعام ألف وأربعمائة وسبعة وأربعين هجريًّا"

#### Shawwal 1447 H: day 1 = 2026-03-20 (Friday)

- Source: https://www.dar-alifta.org/ar/articles/details/11079
- Source type: dar-alifta.org article 11079
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر شوال لعام 1447هـ
- Publication date (as shown by the site): 19 مارس 2026
- Date line signed in the statement: الخميس التاسع والعشرين من شهر رمضان 1447هـ الموافق التاسع عشر من شهر مارس 2026م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الجمعة الموافق العشرين من شهر مارس لعام ألفين وستة وعشرين ميلاديًّا هو أول أيام شهر شوال لعام ألفٍ وأربعمائةٍ وسبعة وأربعين هجريًّا"

#### Dhu al-Qa'dah 1447 H: day 1 = 2026-04-19 (Sunday)

- Source: https://www.dar-alifta.org/ar/articles/details/11112
- Source type: dar-alifta.org article 11112
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر ذي القعدة لعام 1447هـ
- Publication date (as shown by the site): 17 أبريل 2026
- Date line signed in the statement: الجمعة التاسع والعشرين من شهر شوال 1447هـ الموافق السابع عشر من شهر إبريل 2026م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ السبت الموافق الثامن عشر من شهر إبريل لعام ألفين وستة وعشرين ميلاديًّا هو المتمم لشهر شوال لعام ألف وأربعمائة وسبعة وأربعين هجريًّا، وأن يوم الأحد الموافق التاسع عشر من شهر إبريل لعام ألفين وستة وعشرين ميلاديًّا هو أول أيام شهر ذي القعدة لعام ألف وأربعمائة وسبعة وأربعين هجريًّا"

#### Dhu al-Hijjah 1447 H: day 1 = 2026-05-18 (Monday)

- Source: https://akhbarelyom.com/news/newdetails/4822524/1/
- Source type: Akhbar el-Yom (akhbarelyom.com, state-owned daily, not on the preferred list), second source agreeing with Youm7
- Title: دار الإفتاء: الأربعاء 27 مايو أول أيام عيد الأضحى المبارك
- Publication date (as shown by the site): الأحد، 17 مايو 2026 - 06:11 م
- Method: sighted, in step with the Saudi Supreme Court
- Note: 'غدًا الاثنين' in an article dated Sunday 17 May 2026 resolves to 2026-05-18.
- Quote: "أعلنت دار الإفتاء المصرية أن غدًا الاثنين هو أول أيام شهر ذي الحجة لعام 1447 هجريًا، على أن توافق وقفة عرفات يوم الثلاثاء 26 مايو 2026، ويحل أول أيام عيد الأضحى المبارك يوم الأربعاء 27 مايو 2026. وجاء إعلان دار الإفتاء بعد ثبوت رؤية هلال شهر ذي الحجة"

#### Dhu al-Hijjah 1447 H: day 1 = 2026-05-18 (Monday)

- Source: https://www.dar-alifta.org/ar/articles/details/11149
- Source type: dar-alifta.org article 11149
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر ذي الحجة لعام 1447هـ
- Publication date (as shown by the site): 17 مايو 2026
- Date line signed in the statement: الأحد التاسع والعشرين من شهر ذي القعدة 1447هـ الموافق السابع عشر من شهر مايو 2026م
- Method: sighted (statement cites own sighting committees plus the Saudi Supreme Court decision)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الإثنين الموافق الثامن عشر من شهر مايو لعام ألفين وستة وعشرين ميلاديًّا هو أول أيام شهر ذي الحجة لعام ألفٍ وأربعمائةٍ وسبعة وأربعين هجريًّا"

#### Dhu al-Hijjah 1447 H: day 1 = 2026-05-18 (Monday)

- Source: https://www.youm7.com/story/2026/5/17/x/7419252
- Source type: Youm7 (youm7.com), reproduces the full Dar al-Ifta statement
- Title: الإفتاء المصرية: غدا غرة ذى الحجة ووقفة عرفات الثلاثاء 26 مايو وعيد الأضحى الأربعاء
- Publication date (as shown by the site): الأحد، 17 مايو 2026 08:11 م
- Method: sighted (statement cites own sighting committees plus the Saudi Supreme Court decision)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الإثنين الموافق الثامن عشر من شهر مايو لعام ألفين وستة وعشرين ميلاديًّا هو أول أيام شهر ذي الحجة لعام ألفٍ وأربعمائةٍ وسبعة وأربعين هجريًّا، وعلي ذلك يكون يوم الثلاثاء الموافق 26 مايو هو يوم عرفة، والاربعاء 27 مايو هو أول أيام عيد الاضحى المبارك."


### 1448 H

#### Muharram 1448 H: day 1 = 2026-06-16 (Tuesday)

- Source: https://www.dar-alifta.org/ar/articles/details/11188
- Source type: dar-alifta.org article 11188
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر المحرم لعام 1448هـ
- Publication date (as shown by the site): 15 يونيو 2026
- Date line signed in the statement: الإثنين التاسع والعشرين من شهر ذي الحجة 1447هـ الموافق الخامس عشر من شهر يونيو2026م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الثلاثاء الموافق السادس عشر من شهر يونيو لعام ألفين وستة وعشرين ميلاديًّا هو أول أيام شهر المحرم لعام ألفٍ وأربعمائةٍ وثمانية وأربعين هجريًّا"

#### Safar 1448 H: day 1 = 2026-07-15 (Wednesday)

- Source: https://www.dar-alifta.org/ar/articles/details/12259
- Source type: dar-alifta.org article 12259
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر صفر لعام 1448هـ
- Publication date (as shown by the site): 14 يوليو 2026
- Date line signed in the statement: الثلاثاء التاسع والعشرين من شهر المحرم 1448هـ الموافق الرابع عشر من شهر يوليو 2026م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الأربعاء الموافق الخامس عشر من شهر يوليو لعام ألفين وستة وعشرين ميلاديًّا هو أول أيام شهر صفر لعام ألفٍ وأربعمائةٍ وثمانية وأربعين هجريًّا"

#### Rabi al-Awwal 1448 H: day 1 = 2026-08-14 (Friday)

- Source: https://www.dar-alifta.org/ar/articles/details/12309
- Source type: dar-alifta.org article 12309
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر ربيع الأول لعام 1448هـ
- Publication date (as shown by the site): 12 أغسطس 2026
- Date line signed in the statement: الأربعاء التاسع والعشرين من شهر صفر 1448هـ الموافق الثاني عشر من شهر أغسطس2026 م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الخميس الموافق الثالث عشر من شهر أغسطس لعام ألفين وستة وعشرين ميلاديًّا هو المتمم لشهر صفر لعام ألف وأربعمائة وثمانية وأربعين هجريًّا، وأن يوم الجمعة الموافق الرابع عشر من شهر أغسطس لعام ألفين وستة وعشرين ميلاديًّا هو أول أيام شهر ربيع الأول لعام ألف وأربعمائة وثمانية وأربعين هجريًّا"

#### Rabi al-Akhir 1448 H: day 1 = 2026-09-13 (Sunday)

- Source: https://www.dar-alifta.org/ar/articles/details/12415
- Source type: dar-alifta.org article 12415
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر ربيع الآخر لعام 1448هـ
- Publication date (as shown by the site): 11 سبتمبر 2026
- Date line signed in the statement: الجمعة التاسع والعشرين من شهر ربيع الأول 1448هـ الموافق الحادي عشر من شهر سبتمبر 2026م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ السبت الموافق الثاني عشر من شهر سبتمبر لعام ألفين وستة وعشرين ميلاديًّا هو المتمم لشهر ربيع الأول لعام ألف وأربعمائة وثمانية وأربعين هجريًّا، وأن يوم الأحد الموافق الثالث عشر من شهر سبتمبر لعام ألفين وستة وعشرين ميلاديًّا هو أول أيام شهر ربيع الآخر لعام ألف وأربعمائة وثمانية وأربعين هجريًّا"

#### Jumada al-Ula 1448 H: day 1 = 2026-10-12 (Monday)

- Source: https://www.dar-alifta.org/ar/articles/details/12468
- Source type: dar-alifta.org article 12468
- Title: بيان دار الإفتاء المصرية حول رؤية هلال شهر جمادى الأولى لعام 1448هـ
- Publication date (as shown by the site): 11 أكتوبر 2026
- Date line signed in the statement: الأحد التاسع والعشرين من شهر ربيع الآخر 1448هـ الموافق الحادي عشر من شهر أكتوبر 2026م
- Method: sighted (ثبوت رؤية)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الإثنين الموافق الثاني عشر من شهر أكتوبر لعام ألفين وستة وعشرين ميلاديًّا هو أول أيام شهر جمادى الأولى لعام ألفٍ وأربعمائةٍ وثمانية وأربعين هجريًّا"

#### Jumada al-Akhira 1448 H: day 1 = 2026-11-11 (Wednesday)

- Source: https://www.dar-alifta.org/ar/articles/details/12529
- Source type: dar-alifta.org article 12529
- Title: بيان دار الإفتاء المصرية  حول رؤية هلال شهر جمادى الآخرة لعام 1448هـ
- Publication date (as shown by the site): 09 نوفمبر 2026
- Date line signed in the statement: الإثنين التاسع والعشرين من شهر جمادى الأولى 1448هـ الموافق التاسع من شهر نوفمبر 2026م
- Method: not sighted (عدم ثبوت رؤية), previous month completed to 30 days (المتمم)
- Quote: "وعلى ذلك تُعلن دارُ الإفتاءِ المصريةُ أن يومَ الثلاثاء الموافق العاشر من شهر نوفمبر لعام ألفين وستة وعشرين ميلاديًّا هو المتمم لشهر جمادى الأولى لعام ألف وأربعمائة وثمانية وأربعين هجريًّا، وأن يوم الأربعاء الموافق الحادي عشر من شهر نوفمبر لعام ألفين وستة وعشرين ميلاديًّا هو أول أيام شهر جمادى الآخرة لعام ألف وأربعمائة وثمانية وأربعين هجريًّا"


### Fixture table

| hijri_year | hijri_month | gregorian | derived | source |
|---|---|---|---|---|
| 1436 | 9 | 2015-06-18 | no | https://english.ahram.org.eg/News/132951.aspx |
| 1436 | 10 | 2015-07-17 | no | https://www.masrawy.com/News/News_PublicAffairs/details/2015/7/16/620802/ |
| 1436 | 12 | 2015-09-15 | no | https://www.almasdar.com/26408 |
| 1437 | 7 | 2016-04-08 | no | https://www.dar-alifta.org/ar/articles/details/3940 |
| 1437 | 8 | 2016-05-08 | no | https://www.dar-alifta.org/ar/articles/details/3954 |
| 1437 | 9 | 2016-06-06 | no | https://www.dar-alifta.org/ar/articles/details/4010 |
| 1437 | 10 | 2016-07-06 | no | https://www.dar-alifta.org/ar/articles/details/4041 |
| 1437 | 12 | 2016-09-03 | no | https://www.dar-alifta.org/ar/articles/details/4248 |
| 1438 | 2 | 2016-10-31 | no | https://www.dar-alifta.org/ar/articles/details/4349 |
| 1438 | 3 | 2016-11-30 | no | https://www.dar-alifta.org/ar/articles/details/4460 |
| 1438 | 7 | 2017-03-29 | no | https://www.dar-alifta.org/ar/articles/details/4888 |
| 1438 | 8 | 2017-04-27 | no | https://www.dar-alifta.org/ar/articles/details/4985 |
| 1438 | 9 | 2017-05-27 | no | https://www.dar-alifta.org/ar/articles/details/5065 |
| 1438 | 10 | 2017-06-25 | no | https://www.dar-alifta.org/ar/articles/details/5160 |
| 1438 | 11 | 2017-07-24 | no | https://www.dar-alifta.org/ar/articles/details/5219 |
| 1438 | 12 | 2017-08-23 | no | https://www.dar-alifta.org/ar/articles/details/5339 |
| 1439 | 2 | 2017-10-21 | no | https://www.dar-alifta.org/ar/articles/details/5460 |
| 1439 | 3 | 2017-11-20 | no | https://www.dar-alifta.org/ar/articles/details/5540 |
| 1439 | 8 | 2018-04-17 | no | https://www.dar-alifta.org/ar/articles/details/5766 |
| 1439 | 9 | 2018-05-17 | no | https://www.dar-alifta.org/ar/articles/details/5802 |
| 1439 | 10 | 2018-06-15 | no | https://www.dar-alifta.org/ar/articles/details/5842 |
| 1439 | 12 | 2018-08-12 | no | https://www.dar-alifta.org/ar/articles/details/5963 |
| 1440 | 1 | 2018-09-11 | no | https://www.dar-alifta.org/ar/articles/details/6003 |
| 1440 | 3 | 2018-11-09 | no | https://www.dar-alifta.org/ar/articles/details/6134 |
| 1440 | 7 | 2019-03-08 | no | https://www.dar-alifta.org/ar/articles/details/6393 |
| 1440 | 8 | 2019-04-07 | no | https://www.dar-alifta.org/ar/articles/details/6462 |
| 1440 | 9 | 2019-05-06 | no | https://www.dar-alifta.org/ar/articles/details/6523 |
| 1440 | 10 | 2019-06-05 | no | https://www.dar-alifta.org/ar/articles/details/6557 |
| 1440 | 11 | 2019-07-04 | no | https://www.dar-alifta.org/ar/articles/details/6589 |
| 1440 | 12 | 2019-08-02 | no | https://www.dar-alifta.org/ar/articles/details/6628 |
| 1441 | 2 | 2019-09-30 | no | https://www.dar-alifta.org/ar/articles/details/6710 |
| 1441 | 5 | 2019-12-27 | no | https://www.dar-alifta.org/ar/articles/details/6920 |
| 1441 | 7 | 2020-02-25 | no | https://www.dar-alifta.org/ar/articles/details/7019 |
| 1441 | 9 | 2020-04-24 | no | https://www.dar-alifta.org/ar/articles/details/7098 |
| 1441 | 10 | 2020-05-24 | no | https://www.dar-alifta.org/ar/articles/details/7129 |
| 1441 | 12 | 2020-07-22 | no | https://www.dar-alifta.org/ar/articles/details/7176 |
| 1442 | 2 | 2020-09-18 | no | https://www.dar-alifta.org/ar/articles/details/7234 |
| 1442 | 7 | 2021-02-13 | no | https://www.dar-alifta.org/ar/articles/details/7508 |
| 1442 | 8 | 2021-03-14 | no | https://www.dar-alifta.org/ar/articles/details/7564 |
| 1442 | 9 | 2021-04-13 | no | https://www.dar-alifta.org/ar/articles/details/7614 |
| 1442 | 10 | 2021-05-13 | no | https://www.dar-alifta.org/ar/articles/details/7681 |
| 1442 | 11 | 2021-06-12 | no | https://www.dar-alifta.org/ar/articles/details/7720 |
| 1442 | 12 | 2021-07-11 | no | https://www.dar-alifta.org/ar/articles/details/7773 |
| 1443 | 2 | 2021-09-08 | no | https://www.dar-alifta.org/ar/articles/details/7937 |
| 1443 | 7 | 2022-02-02 | no | https://www.dar-alifta.org/ar/articles/details/8160 |
| 1443 | 8 | 2022-03-04 | no | https://www.dar-alifta.org/ar/articles/details/8191 |
| 1443 | 9 | 2022-04-02 | no | https://www.dar-alifta.org/ar/articles/details/8218 |
| 1443 | 10 | 2022-05-02 | no | https://www.dar-alifta.org/ar/articles/details/8262 |
| 1443 | 11 | 2022-06-01 | no | https://www.dar-alifta.org/ar/articles/details/8341 |
| 1443 | 12 | 2022-06-30 | no | https://www.dar-alifta.org/ar/articles/details/8418 |
| 1444 | 2 | 2022-08-28 | no | https://www.dar-alifta.org/ar/articles/details/8459 |
| 1444 | 3 | 2022-09-27 | no | https://www.dar-alifta.org/ar/articles/details/8480 |
| 1444 | 6 | 2022-12-25 | no | https://www.dar-alifta.org/ar/articles/details/8605 |
| 1444 | 7 | 2023-01-23 | no | https://www.dar-alifta.org/ar/articles/details/8631 |
| 1444 | 8 | 2023-02-21 | no | https://www.dar-alifta.org/ar/articles/details/8656 |
| 1444 | 9 | 2023-03-23 | no | https://www.dar-alifta.org/ar/articles/details/8690 |
| 1444 | 10 | 2023-04-21 | no | https://www.dar-alifta.org/ar/articles/details/8764 |
| 1444 | 12 | 2023-06-19 | no | https://www.dar-alifta.org/ar/articles/details/8844 |
| 1445 | 1 | 2023-07-19 | no | https://www.dar-alifta.org/ar/articles/details/8865 |
| 1445 | 2 | 2023-08-17 | no | https://www.dar-alifta.org/ar/articles/details/8876 |
| 1445 | 4 | 2023-10-16 | no | https://www.dar-alifta.org/ar/articles/details/8939 |
| 1445 | 5 | 2023-11-15 | no | https://www.dar-alifta.org/ar/articles/details/9053 |
| 1445 | 7 | 2024-01-13 | no | https://www.dar-alifta.org/ar/articles/details/9173 |
| 1445 | 8 | 2024-02-11 | no | https://www.dar-alifta.org/ar/articles/details/9244 |
| 1445 | 9 | 2024-03-11 | no | https://www.dar-alifta.org/ar/articles/details/9285 |
| 1445 | 10 | 2024-04-10 | no | https://www.dar-alifta.org/ar/articles/details/9352 |
| 1445 | 12 | 2024-06-07 | no | https://www.dar-alifta.org/ar/articles/details/9403 |
| 1446 | 1 | 2024-07-07 | no | https://www.dar-alifta.org/ar/articles/details/9427 |
| 1446 | 2 | 2024-08-06 | no | https://www.dar-alifta.org/ar/articles/details/9552 |
| 1446 | 3 | 2024-09-04 | no | https://www.dar-alifta.org/ar/articles/details/9616 |
| 1446 | 7 | 2025-01-01 | no | https://www.dar-alifta.org/ar/articles/details/9859 |
| 1446 | 8 | 2025-01-31 | no | https://www.dar-alifta.org/ar/articles/details/9914 |
| 1446 | 9 | 2025-03-01 | no | https://www.dar-alifta.org/ar/articles/details/10004 |
| 1446 | 10 | 2025-03-31 | no | https://www.dar-alifta.org/ar/articles/details/10089 |
| 1446 | 11 | 2025-04-29 | no | https://www.dar-alifta.org/ar/articles/details/10152 |
| 1446 | 12 | 2025-05-28 | no | https://www.dar-alifta.org/ar/articles/details/10212 |
| 1447 | 1 | 2025-06-26 | no | https://www.dar-alifta.org/ar/articles/details/10277 |
| 1447 | 2 | 2025-07-26 | no | https://www.dar-alifta.org/ar/articles/details/10353 |
| 1447 | 3 | 2025-08-24 | no | https://www.dar-alifta.org/ar/articles/details/10466 |
| 1447 | 4 | 2025-09-23 | no | https://www.dar-alifta.org/ar/articles/details/10558 |
| 1447 | 5 | 2025-10-23 | no | https://www.dar-alifta.org/ar/articles/details/10643 |
| 1447 | 6 | 2025-11-22 | no | https://www.dar-alifta.org/ar/articles/details/10727 |
| 1447 | 7 | 2025-12-21 | no | https://www.dar-alifta.org/ar/articles/details/10846 |
| 1447 | 8 | 2026-01-20 | no | https://www.dar-alifta.org/ar/articles/details/10915 |
| 1447 | 9 | 2026-02-19 | no | https://www.dar-alifta.org/ar/articles/details/11039 |
| 1447 | 10 | 2026-03-20 | no | https://www.dar-alifta.org/ar/articles/details/11079 |
| 1447 | 11 | 2026-04-19 | no | https://www.dar-alifta.org/ar/articles/details/11112 |
| 1447 | 12 | 2026-05-18 | no | https://www.dar-alifta.org/ar/articles/details/11149 |
| 1448 | 1 | 2026-06-16 | no | https://www.dar-alifta.org/ar/articles/details/11188 |
| 1448 | 2 | 2026-07-15 | no | https://www.dar-alifta.org/ar/articles/details/12259 |
| 1448 | 3 | 2026-08-14 | no | https://www.dar-alifta.org/ar/articles/details/12309 |
| 1448 | 4 | 2026-09-13 | no | https://www.dar-alifta.org/ar/articles/details/12415 |
| 1448 | 5 | 2026-10-12 | no | https://www.dar-alifta.org/ar/articles/details/12468 |
| 1448 | 6 | 2026-11-11 | no | https://www.dar-alifta.org/ar/articles/details/12529 |

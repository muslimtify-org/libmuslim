# Diyanet calendar

**Date:** 2026-10-17

This note records where `HIJRI_AUTHORITY_DIYANET`'s table comes from, how it was read and checked, and what was measured about the rule behind it. Every web source below was retrieved on 2026-10-17.

## The rule

Diyanet İşleri Başkanlığı states its rule in a bulletin published through Diyanet America, "Beginning of the month of Ramadan 1447 AH" (https://diyanetamerica.org/wp-content/uploads/2026/02/BEGINNING-OF-THE-MONTH-OF-RAMADAN-1447-AH-1.pdf, English, Turkish and Arabic). It determines month starts "in accordance with the resolutions adopted at the International Hijrî Calendar Union Congress held in Istanbul in 2016", ignores differences of horizon, and requires four conditions together:

- After conjunction, the angular separation between the Moon and the Sun is at least 8 degrees.
- At sunset, the Moon's altitude above the horizon is no less than 5 degrees.
- "In the case of the occurrence of sighting (ru'yah), the conjunction must take place before imsak at the world's easternmost point (New Zealand/Wellington)."
- "For the new lunar month to begin, crescent sighting must occur on the main landmasses of North and South America."

The bulletin's worked case: conjunction 2026-02-17 12:01 GMT, first visibility 2026-02-18 03:42 GMT "beginning from the Pacific region", and 1 Ramazan 1447 on 2026-02-19.

The bulletin does not define the frame of either angle, the search window, or how the third and fourth conditions combine with visibility before 00:00 UT. `docs/research/2026-09-07-khgt-sources.md` found the same gap in the congress proceedings. Diyanet's page "Hicri Takvim Farkı" (https://vakithesaplama.diyanet.gov.tr/hicri_farki.php) adds only that its calculations take the crescent being visible anywhere in the world into account. Its page of meetings and congresses (https://vakithesaplama.diyanet.gov.tr/icerik.php?icerik=101) lists decisions up to 2012 and not the 2016 declaration.

## The published calendar

Diyanet publishes its month starts in advance on two of its own sites.

**mobil.diyanet.gov.tr**, https://mobil.diyanet.gov.tr/mobile/dinigunler/dinigunler.html, static HTML, one table for 2021 to 2028. Each row is a day cell (`<strong>14</strong><br>Perşembe`), a name and Hijri date (`<strong>C.Ahir Başlangıcı</strong><br><span>1 C.Ahir</span>`) and a Gregorian month and year (`<span>Ocak<br>2021</span>`). A month start is any row whose Hijri date is day 1: the "X Başlangıcı" rows, "Üç Aylar" (1 Receb), "Ramazan" (1 Ramazan), "Hicri Yılbaşı" (1 Muharrem), and "Ramazan Bayramı" for 1 Şevval, whose Hijri cell reads "1-2-3 Şevval". In that row the day cell is a range, written "13..15" in some years and "20-22" in others, and the month cell can span two months, "Mart-Nisan", in which case the first day is in the first month. No Hijri years are printed. Read this way the page gives 99 month starts, 2021-01-14 (1 Cemaziyelahir) to 2028-12-17 (1 Şaban), each 29 or 30 days after the last and in Hijri month order. The Hijri years were counted forward from 1442 for the first.

**vakithesaplama.diyanet.gov.tr**, Diyanet's astronomy site. Its yearly lists for 2027 to 2035 are https://vakithesaplama.diyanet.gov.tr/icerik.php?icerik=N for N = 154 (2027) and 185 to 192 (2028 to 2035). The server answers 403 to a request without a browser user agent. Each list is a table whose text reads "01 ZİLKADE 1456 11 OCAK-2035 PERŞEMBE": Hijri day, month and year, then Gregorian day and month-year. The 2027 list abbreviates four month names as R.EVVEL, R.AHİR, C.EVVEL and C.AHİR. Read as every row with Hijri day 01, the nine lists give 112 month starts, 2027-01-09 (1 Şaban 1448) to 2035-12-31 (1 Zilkade 1457), each 29 or 30 days after the last and in order. The site's year pages, https://vakithesaplama.diyanet.gov.tr/dinigunler.php?yil=N, carry a list for N = 2023 to 2026 in the same row format. Read the same way they give 49 month starts, every one also on the mobile page and every one agreeing with it. They are a third copy and not a fixture. The page for 2022 is laid out differently and was not parsed, and the pages for 2016, 2017, 2030, 2035 and 2036 return a template with no list.

**Cross-check.** The two sources share 25 starts, 2027-01-09 to 2028-12-17. They agree on all 25, including the Hijri year counted for the mobile page against the year the astronomy site prints. Their union is 186 consecutive starts from 2021-01-14 to 2035-12-31, every gap 29 or 30 days. That union is `hijri__diyanet_starts` in `hijri.h`, and the two lists are the fixtures in `tests/test_hijri.c`.

The length of the last month, Zilkade 1457, needs the first start of Diyanet's 2036 list, which is not published. The table therefore answers 2035-12-31 as day 1 and returns nothing after it.

## Measurements

These are research measurements made with `hijri.h` v0.3.1 against the 99 starts of the mobile page. None of them ships.

**Single points.** `HIJRI_PREDICATE_ALTITUDE_5_ELONGATION_8` evaluated at one point, start S counted supported when the evening before S passes and early when the evening two days before passes:

| Point | Supported of 99 | Early |
|---|---|---|
| Istanbul | 29 | 0 |
| Mecca | 38 | 0 |
| Dakar | 63 | 0 |
| Rio de Janeiro | 65 | 0 |
| Santiago | 66 | 0 |
| Los Angeles | 73 | 0 |
| Lima | 80 | 0 |
| Mexico City | 82 | 0 |

No point is ever early and no point is complete, which is what a rule satisfied somewhere on Earth produces. That predicate's elongation is geocentric.

**Global search.** A prototype evaluated `hijri_compute_evening_parameters` with `HIJRI_SUNSET_CONVENTION_ASTRONOMICAL` at every 2 degrees of latitude from 60 S to 60 N and every 2 degrees of longitude, on the local evening before a candidate day D. A point passes when its topocentric unrefracted centre altitude is at least 5 degrees, its elongation is at least 8 degrees and the conjunction precedes its sunset. Two readings of the rule were scored. A: D is the 1st when a passing point's sunset is before 00:00 UT of D. B: as A, or when a passing point lies on the Americas mainland, approximated by a longitude and latitude box with the Pacific and Atlantic cut out, and the conjunction is before 17:00 UT of the day before D, an approximation of dawn in Wellington. A start is reproduced when the reading holds for S and not for S minus one day.

| Elongation | Reading A | Reading B |
|---|---|---|
| geocentric | 89 of 99 | 95 of 99 |
| topocentric | 81 of 99 | 99 of 99 |

Reading B with topocentric elongation reproduces every start. The Americas condition decides 18 of the 99. The four geocentric misses, 2022-10-27, 2023-05-21, 2025-01-31 and 2026-02-19, each pass on the Americas mainland a day early, the last being the bulletin's own example. With topocentric elongation, a run that printed every row showed that all 99 start evenings pass on the Americas mainland with the conjunction before the cutoff, and that none of the 99 evenings before a start passes on the Americas mainland. Two things follow. The Wellington condition never decided a start, so these 99 months do not test it. And a third reading, C, where D is the 1st only when a passing point lies on the Americas mainland and the conjunction precedes the cutoff, also reproduces all 99. These months cannot tell B from C.

Each evening evaluation costs about 1 ms, so one evening on the 2 degree grid costs about 11 s, and the geocentric run over 99 starts took 4 min 20 s on 16 cores. That cost, and a search window and exception known only from a bulletin, are why `hijri.h` ships the table and not the rule.

## Not admitted

Egypt and Pakistan were researched on the same day and neither has a record that can be fixture-tested. Dar al-Ifta al-Misriyya decides by sighting on the 29th, uses calculation only to rule a claimed sighting out (https://www.dar-alifta.org/en/fatwa/details/24176/manner-of-determining-the-beginning-of-lunar-month-in-the-hijri-calendar), and publishes no archive of past month starts. Pakistan's Central Ruet-e-Hilal Committee decides from sighting reports. The Ministry of Religious Affairs (https://mora.gov.pk) posts each month's sighting arrangements and result as news items, the front page on 2026-10-17 showing them for Jumada al-Awwal 1448, and no archive of past month starts was found.

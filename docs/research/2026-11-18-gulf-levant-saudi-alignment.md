# Gulf, Levant and Egypt against Saudi Arabia's two calendars

Research date: 2026-11-18. This note measures whether the UAE, Qatar, Kuwait, Bahrain, Oman, Jordan and Egypt start Ramadan, Shawwal and Dhu al-Hijjah on the days of the Umm al-Qura table, which `HIJRI_AUTHORITY_UMM_AL_QURA` reads, or on the days of the Saudi Supreme Court's sighting rulings, which can differ from the table by a day.

## What was measured

Day 1 of Ramadan, Shawwal and Dhu al-Hijjah from 1436 to 1447 H, as each country announced it for itself. Every date was read on a fetched page, and months no page gave are left out. Dhu al-Hijjah is stated or derived as Eid al-Adha minus 9 days, flagged derived. The fixtures and their sources are under "Fixtures" below. Egypt's rows are the Ramadan, Shawwal and Dhu al-Hijjah rows of `docs/research/2026-11-18-egypt-dar-al-ifta.md`.

Most sources are press. The official sites and agencies of Saudi Arabia, Kuwait, Bahrain and Oman refused automated fetches or could not be found, so their rows rest on newspapers and broadcasters reporting the announcement. Jordan's Ramadan rows from 1438 and most Shawwal rows come from the General Iftaa' Department's own resolutions. Each fixture section names its source type row by row.

Scoring, by the program under "Program" below:

- Against the table: `hijri_umm_al_qura_from_gregorian` on the country's start gives day 1 of the stated month (exact), day 2 (the country is a day late), day 29 or 30 of the month before (the country is a day early), or anything else (other).
- Against the court: the country's start equals, is one day after (late), or one day before (early) Saudi Arabia's start for the same month. Months with no Saudi row are left out.

## Scores

| Country | Starts | Table exact | Table, country a day late | Table, country a day early | Table other | Court months | Court exact | Court, country a day late | Court, country a day early | Court other |
|---|---|---|---|---|---|---|---|---|---|---|
| Saudi | 36 | 33 | 3 | 0 | 0 | n/a | n/a | n/a | n/a | n/a |
| UAE | 27 | 26 | 1 | 0 | 0 | 27 | 27 | 0 | 0 | 0 |
| Qatar | 25 | 24 | 1 | 0 | 0 | 25 | 25 | 0 | 0 | 0 |
| Kuwait | 19 | 18 | 1 | 0 | 0 | 19 | 19 | 0 | 0 | 0 |
| Bahrain | 13 | 12 | 1 | 0 | 0 | 13 | 13 | 0 | 0 | 0 |
| Oman | 32 | 16 | 16 | 0 | 0 | 32 | 18 | 14 | 0 | 0 |
| Jordan | 26 | 19 | 6 | 1 | 0 | 26 | 20 | 5 | 1 | 0 |
| Egypt | 36 | 30 | 6 | 0 | 0 | 36 | 33 | 3 | 0 | 0 |

## The map

The rule was fixed before scoring. A country uses `HIJRI_AUTHORITY_UMM_AL_QURA` when it is exact against the table on at least 90% of its sourced starts. Otherwise it gets no entry, and the better of the table and the Saudi court is named with its score. A country with fewer than 12 sourced starts gets "insufficient record". Egypt has its own entry and is listed for comparison only. "Follows the Saudi court" is noted when a country's court exact count is above its table exact count.

| Country | Table exact | Court exact | Use |
|---|---|---|---|
| UAE | 26 of 27 | 27 of 27 | `HIJRI_AUTHORITY_UMM_AL_QURA`, follows the Saudi court |
| Qatar | 24 of 25 | 25 of 25 | `HIJRI_AUTHORITY_UMM_AL_QURA`, follows the Saudi court |
| Kuwait | 18 of 19 | 19 of 19 | `HIJRI_AUTHORITY_UMM_AL_QURA`, follows the Saudi court |
| Bahrain | 12 of 13 | 13 of 13 | `HIJRI_AUTHORITY_UMM_AL_QURA`, follows the Saudi court |
| Oman | 16 of 32 | 18 of 32 | no entry, the Saudi court is the better match at 18 of 32 |
| Jordan | 19 of 26 | 20 of 26 | no entry, the Saudi court is the better match at 20 of 26 |
| Egypt | 30 of 36 | 33 of 36 | has its own entry, `HIJRI_AUTHORITY_EGYPT_DAR_AL_IFTA`, listed for comparison. Of the two Saudi calendars the court is the better match |

## What the scores say

- The UAE, Qatar, Kuwait and Bahrain started every sourced month on the Saudi court's day. The Umm al-Qura table matches them except where the court itself departed from the table, which it did on 3 of its 36 starts here, each time a day after the table.
- Oman sights for itself and started 14 of its 32 sourced months a day after the Saudi court and 16 a day after the table. Neither Saudi calendar predicts it well.
- Jordan follows the court on 20 of 26 and is a day later on 5 and a day earlier on 1.
- Egypt follows the court on 33 of 36 of these months, and the table on 30. For these three months the table is a better predictor of Egypt than `HIJRI_AUTHORITY_EGYPT_DAR_AL_IFTA`, which gives day 1 on 22 of these 36 starts and on 60 of all 94 Egyptian starts.

Whatever the map says, the dates a country announces are the authority. Pass them to `hijri_authority_from_gregorian_announced` for the months they cover.

## Decisions on the data

- Jordan, Ramadan 1445: the Iftaa' Board's own resolution 324 (https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=1734) says Monday 11 March 2024, and same-night press (Roya, Jordan News, Ammon) says Tuesday 12 March. The fixture table scores the official 2024-03-11. The Jordan section below records both.
- Jordan, several resolutions print the meeting date next to the weekday of day 1. The Jordan section below records each case and the day used.

## Program

Build from the repository root with `cc -O2 -std=c11 -I. -o score score.c -lm && ./score`. Its output is the table under "Scores".

```c
/* Scores announced Ramadan, Shawwal and Dhu al-Hijjah starts against the
 * Umm al-Qura table and against the Saudi Supreme Court's rulings.
 * Build from the repository root:
 *   cc -O2 -std=c11 -I. -o score score.c -lm && ./score */
#define HIJRI_IMPLEMENTATION
#include "hijri.h"
#include <stdio.h>
#include <string.h>

typedef struct {
  const char *country;
  int hy, hm, y, m, d;
} Start;

static const Start STARTS[] = {
    {"Saudi", 1436, 9, 2015, 6, 18},
    {"Saudi", 1436, 10, 2015, 7, 17},
    {"Saudi", 1436, 12, 2015, 9, 15},
    {"Saudi", 1437, 9, 2016, 6, 6},
    {"Saudi", 1437, 10, 2016, 7, 6},
    {"Saudi", 1437, 12, 2016, 9, 3},
    {"Saudi", 1438, 9, 2017, 5, 27},
    {"Saudi", 1438, 10, 2017, 6, 25},
    {"Saudi", 1438, 12, 2017, 8, 23},
    {"Saudi", 1439, 9, 2018, 5, 17},
    {"Saudi", 1439, 10, 2018, 6, 15},
    {"Saudi", 1439, 12, 2018, 8, 12},
    {"Saudi", 1440, 9, 2019, 5, 6},
    {"Saudi", 1440, 10, 2019, 6, 4},
    {"Saudi", 1440, 12, 2019, 8, 2},
    {"Saudi", 1441, 9, 2020, 4, 24},
    {"Saudi", 1441, 10, 2020, 5, 24},
    {"Saudi", 1441, 12, 2020, 7, 22},
    {"Saudi", 1442, 9, 2021, 4, 13},
    {"Saudi", 1442, 10, 2021, 5, 13},
    {"Saudi", 1442, 12, 2021, 7, 11},
    {"Saudi", 1443, 9, 2022, 4, 2},
    {"Saudi", 1443, 10, 2022, 5, 2},
    {"Saudi", 1443, 12, 2022, 6, 30},
    {"Saudi", 1444, 9, 2023, 3, 23},
    {"Saudi", 1444, 10, 2023, 4, 21},
    {"Saudi", 1444, 12, 2023, 6, 19},
    {"Saudi", 1445, 9, 2024, 3, 11},
    {"Saudi", 1445, 10, 2024, 4, 10},
    {"Saudi", 1445, 12, 2024, 6, 7},
    {"Saudi", 1446, 9, 2025, 3, 1},
    {"Saudi", 1446, 10, 2025, 3, 30},
    {"Saudi", 1446, 12, 2025, 5, 28},
    {"Saudi", 1447, 9, 2026, 2, 18},
    {"Saudi", 1447, 10, 2026, 3, 20},
    {"Saudi", 1447, 12, 2026, 5, 18},
    {"UAE", 1436, 9, 2015, 6, 18},
    {"UAE", 1436, 10, 2015, 7, 17},
    {"UAE", 1437, 9, 2016, 6, 6},
    {"UAE", 1437, 10, 2016, 7, 6},
    {"UAE", 1438, 9, 2017, 5, 27},
    {"UAE", 1438, 10, 2017, 6, 25},
    {"UAE", 1439, 9, 2018, 5, 17},
    {"UAE", 1439, 10, 2018, 6, 15},
    {"UAE", 1440, 9, 2019, 5, 6},
    {"UAE", 1440, 10, 2019, 6, 4},
    {"UAE", 1440, 12, 2019, 8, 2},
    {"UAE", 1441, 9, 2020, 4, 24},
    {"UAE", 1441, 10, 2020, 5, 24},
    {"UAE", 1442, 9, 2021, 4, 13},
    {"UAE", 1442, 10, 2021, 5, 13},
    {"UAE", 1443, 9, 2022, 4, 2},
    {"UAE", 1443, 10, 2022, 5, 2},
    {"UAE", 1444, 9, 2023, 3, 23},
    {"UAE", 1444, 10, 2023, 4, 21},
    {"UAE", 1445, 9, 2024, 3, 11},
    {"UAE", 1445, 10, 2024, 4, 10},
    {"UAE", 1446, 9, 2025, 3, 1},
    {"UAE", 1446, 10, 2025, 3, 30},
    {"UAE", 1446, 12, 2025, 5, 28},
    {"UAE", 1447, 9, 2026, 2, 18},
    {"UAE", 1447, 10, 2026, 3, 20},
    {"UAE", 1447, 12, 2026, 5, 18},
    {"Qatar", 1436, 9, 2015, 6, 18},
    {"Qatar", 1436, 10, 2015, 7, 17},
    {"Qatar", 1437, 9, 2016, 6, 6},
    {"Qatar", 1437, 10, 2016, 7, 6},
    {"Qatar", 1438, 9, 2017, 5, 27},
    {"Qatar", 1438, 10, 2017, 6, 25},
    {"Qatar", 1439, 9, 2018, 5, 17},
    {"Qatar", 1439, 10, 2018, 6, 15},
    {"Qatar", 1440, 9, 2019, 5, 6},
    {"Qatar", 1440, 10, 2019, 6, 4},
    {"Qatar", 1441, 9, 2020, 4, 24},
    {"Qatar", 1441, 10, 2020, 5, 24},
    {"Qatar", 1442, 9, 2021, 4, 13},
    {"Qatar", 1442, 10, 2021, 5, 13},
    {"Qatar", 1443, 9, 2022, 4, 2},
    {"Qatar", 1443, 10, 2022, 5, 2},
    {"Qatar", 1444, 9, 2023, 3, 23},
    {"Qatar", 1444, 10, 2023, 4, 21},
    {"Qatar", 1445, 9, 2024, 3, 11},
    {"Qatar", 1445, 10, 2024, 4, 10},
    {"Qatar", 1446, 9, 2025, 3, 1},
    {"Qatar", 1446, 10, 2025, 3, 30},
    {"Qatar", 1446, 12, 2025, 5, 28},
    {"Qatar", 1447, 9, 2026, 2, 18},
    {"Qatar", 1447, 10, 2026, 3, 20},
    {"Kuwait", 1437, 10, 2016, 7, 6},
    {"Kuwait", 1438, 9, 2017, 5, 27},
    {"Kuwait", 1439, 9, 2018, 5, 17},
    {"Kuwait", 1440, 9, 2019, 5, 6},
    {"Kuwait", 1440, 10, 2019, 6, 4},
    {"Kuwait", 1441, 9, 2020, 4, 24},
    {"Kuwait", 1441, 10, 2020, 5, 24},
    {"Kuwait", 1442, 9, 2021, 4, 13},
    {"Kuwait", 1442, 10, 2021, 5, 13},
    {"Kuwait", 1443, 9, 2022, 4, 2},
    {"Kuwait", 1443, 10, 2022, 5, 2},
    {"Kuwait", 1444, 9, 2023, 3, 23},
    {"Kuwait", 1444, 10, 2023, 4, 21},
    {"Kuwait", 1445, 9, 2024, 3, 11},
    {"Kuwait", 1445, 10, 2024, 4, 10},
    {"Kuwait", 1446, 9, 2025, 3, 1},
    {"Kuwait", 1446, 10, 2025, 3, 30},
    {"Kuwait", 1447, 9, 2026, 2, 18},
    {"Kuwait", 1447, 10, 2026, 3, 20},
    {"Bahrain", 1437, 10, 2016, 7, 6},
    {"Bahrain", 1438, 9, 2017, 5, 27},
    {"Bahrain", 1439, 9, 2018, 5, 17},
    {"Bahrain", 1440, 9, 2019, 5, 6},
    {"Bahrain", 1443, 9, 2022, 4, 2},
    {"Bahrain", 1443, 10, 2022, 5, 2},
    {"Bahrain", 1444, 9, 2023, 3, 23},
    {"Bahrain", 1445, 9, 2024, 3, 11},
    {"Bahrain", 1445, 10, 2024, 4, 10},
    {"Bahrain", 1446, 9, 2025, 3, 1},
    {"Bahrain", 1446, 10, 2025, 3, 30},
    {"Bahrain", 1447, 9, 2026, 2, 18},
    {"Bahrain", 1447, 10, 2026, 3, 20},
    {"Oman", 1436, 10, 2015, 7, 18},
    {"Oman", 1437, 9, 2016, 6, 7},
    {"Oman", 1437, 12, 2016, 9, 3},
    {"Oman", 1438, 9, 2017, 5, 27},
    {"Oman", 1438, 10, 2017, 6, 26},
    {"Oman", 1439, 9, 2018, 5, 17},
    {"Oman", 1439, 10, 2018, 6, 15},
    {"Oman", 1439, 12, 2018, 8, 12},
    {"Oman", 1440, 9, 2019, 5, 7},
    {"Oman", 1440, 10, 2019, 6, 5},
    {"Oman", 1440, 12, 2019, 8, 3},
    {"Oman", 1441, 9, 2020, 4, 25},
    {"Oman", 1441, 10, 2020, 5, 24},
    {"Oman", 1441, 12, 2020, 7, 22},
    {"Oman", 1442, 9, 2021, 4, 14},
    {"Oman", 1442, 10, 2021, 5, 13},
    {"Oman", 1442, 12, 2021, 7, 11},
    {"Oman", 1443, 9, 2022, 4, 3},
    {"Oman", 1443, 10, 2022, 5, 2},
    {"Oman", 1443, 12, 2022, 6, 30},
    {"Oman", 1444, 9, 2023, 3, 23},
    {"Oman", 1444, 10, 2023, 4, 22},
    {"Oman", 1444, 12, 2023, 6, 19},
    {"Oman", 1445, 9, 2024, 3, 12},
    {"Oman", 1445, 10, 2024, 4, 10},
    {"Oman", 1445, 12, 2024, 6, 8},
    {"Oman", 1446, 9, 2025, 3, 1},
    {"Oman", 1446, 10, 2025, 3, 31},
    {"Oman", 1446, 12, 2025, 5, 28},
    {"Oman", 1447, 9, 2026, 2, 19},
    {"Oman", 1447, 10, 2026, 3, 20},
    {"Oman", 1447, 12, 2026, 5, 18},
    {"Jordan", 1436, 9, 2015, 6, 18},
    {"Jordan", 1436, 10, 2015, 7, 17},
    {"Jordan", 1438, 9, 2017, 5, 27},
    {"Jordan", 1439, 9, 2018, 5, 17},
    {"Jordan", 1439, 10, 2018, 6, 15},
    {"Jordan", 1440, 9, 2019, 5, 7},
    {"Jordan", 1440, 10, 2019, 6, 5},
    {"Jordan", 1441, 9, 2020, 4, 24},
    {"Jordan", 1441, 10, 2020, 5, 23},
    {"Jordan", 1442, 9, 2021, 4, 13},
    {"Jordan", 1442, 10, 2021, 5, 13},
    {"Jordan", 1442, 12, 2021, 7, 11},
    {"Jordan", 1443, 9, 2022, 4, 3},
    {"Jordan", 1443, 10, 2022, 5, 2},
    {"Jordan", 1443, 12, 2022, 6, 30},
    {"Jordan", 1444, 9, 2023, 3, 23},
    {"Jordan", 1444, 10, 2023, 4, 21},
    {"Jordan", 1445, 9, 2024, 3, 11},
    {"Jordan", 1445, 10, 2024, 4, 10},
    {"Jordan", 1445, 12, 2024, 6, 7},
    {"Jordan", 1446, 9, 2025, 3, 1},
    {"Jordan", 1446, 10, 2025, 3, 31},
    {"Jordan", 1446, 12, 2025, 5, 28},
    {"Jordan", 1447, 9, 2026, 2, 19},
    {"Jordan", 1447, 10, 2026, 3, 20},
    {"Jordan", 1447, 12, 2026, 5, 18},
    {"Egypt", 1436, 9, 2015, 6, 18},
    {"Egypt", 1436, 10, 2015, 7, 17},
    {"Egypt", 1436, 12, 2015, 9, 15},
    {"Egypt", 1437, 9, 2016, 6, 6},
    {"Egypt", 1437, 10, 2016, 7, 6},
    {"Egypt", 1437, 12, 2016, 9, 3},
    {"Egypt", 1438, 9, 2017, 5, 27},
    {"Egypt", 1438, 10, 2017, 6, 25},
    {"Egypt", 1438, 12, 2017, 8, 23},
    {"Egypt", 1439, 9, 2018, 5, 17},
    {"Egypt", 1439, 10, 2018, 6, 15},
    {"Egypt", 1439, 12, 2018, 8, 12},
    {"Egypt", 1440, 9, 2019, 5, 6},
    {"Egypt", 1440, 10, 2019, 6, 5},
    {"Egypt", 1440, 12, 2019, 8, 2},
    {"Egypt", 1441, 9, 2020, 4, 24},
    {"Egypt", 1441, 10, 2020, 5, 24},
    {"Egypt", 1441, 12, 2020, 7, 22},
    {"Egypt", 1442, 9, 2021, 4, 13},
    {"Egypt", 1442, 10, 2021, 5, 13},
    {"Egypt", 1442, 12, 2021, 7, 11},
    {"Egypt", 1443, 9, 2022, 4, 2},
    {"Egypt", 1443, 10, 2022, 5, 2},
    {"Egypt", 1443, 12, 2022, 6, 30},
    {"Egypt", 1444, 9, 2023, 3, 23},
    {"Egypt", 1444, 10, 2023, 4, 21},
    {"Egypt", 1444, 12, 2023, 6, 19},
    {"Egypt", 1445, 9, 2024, 3, 11},
    {"Egypt", 1445, 10, 2024, 4, 10},
    {"Egypt", 1445, 12, 2024, 6, 7},
    {"Egypt", 1446, 9, 2025, 3, 1},
    {"Egypt", 1446, 10, 2025, 3, 31},
    {"Egypt", 1446, 12, 2025, 5, 28},
    {"Egypt", 1447, 9, 2026, 2, 19},
    {"Egypt", 1447, 10, 2026, 3, 20},
    {"Egypt", 1447, 12, 2026, 5, 18}
};
#define N_STARTS (int)(sizeof STARTS / sizeof STARTS[0])

static const char *COUNTRIES[] = {"UAE", "Qatar", "Kuwait", "Bahrain",
                                  "Oman", "Jordan", "Egypt"};

static double jd_of(const Start *s) {
  return floor(hijri_jd_from_gregorian(s->y, s->m, (double)s->d));
}

static const Start *saudi_for(int hy, int hm) {
  int i;
  for (i = 0; i < N_STARTS; i++)
    if (!strcmp(STARTS[i].country, "Saudi") && STARTS[i].hy == hy &&
        STARTS[i].hm == hm)
      return &STARTS[i];
  return 0;
}

int main(void) {
  size_t c;
  printf("| Country | Starts | Table exact | Table, country a day late | "
         "Table, country a day early | Table other | Court months | "
         "Court exact | Court, country a day late | Court, country a day "
         "early | Court other |\n");
  printf("|---|---|---|---|---|---|---|---|---|---|---|\n");
  for (c = 0; c < sizeof COUNTRIES / sizeof COUNTRIES[0] + 1; c++) {
    const char *name = c == 0 ? "Saudi" : COUNTRIES[c - 1];
    int n = 0, te = 0, tl = 0, ty = 0, to = 0;
    int cn = 0, ce = 0, cl = 0, cy = 0, co = 0, i;
    for (i = 0; i < N_STARTS; i++) {
      const Start *s = &STARTS[i];
      HijriDate t;
      const Start *k;
      if (strcmp(s->country, name))
        continue;
      n++;
      hijri_umm_al_qura_from_gregorian(s->y, s->m, s->d, &t);
      if (t.year == s->hy && t.month == s->hm && t.day == 1)
        te++;
      else if (t.year == s->hy && t.month == s->hm && t.day == 2)
        tl++;
      else if (t.day >= 29 && ((t.month == s->hm - 1 && t.year == s->hy) ||
                               (s->hm == 1 && t.month == 12)))
        ty++;
      else
        to++;
      if (c == 0)
        continue;
      k = saudi_for(s->hy, s->hm);
      if (!k)
        continue;
      cn++;
      {
        double diff = jd_of(s) - jd_of(k);
        if (diff == 0.0)
          ce++;
        else if (diff == 1.0)
          cl++;
        else if (diff == -1.0)
          cy++;
        else
          co++;
      }
    }
    if (c == 0)
      printf("| %s | %d | %d | %d | %d | %d | n/a | n/a | n/a | n/a | n/a |\n",
             name, n, te, tl, ty, to);
    else
      printf("| %s | %d | %d | %d | %d | %d | %d | %d | %d | %d | %d |\n",
             name, n, te, tl, ty, to, cn, ce, cl, cy, co);
  }
  return 0;
}
```

## Fixtures

### Saudi Arabia: Supreme Court announced month starts, 1436-1447

Announcer for every row is the Saudi Supreme Court (sometimes relayed by the Royal Court via SPA). Source type is press (Arab News, Asharq Al-Awsat, Morocco World News) for every row, because spa.gov.sa, saudigazette.com.sa, okaz.com.sa and alarabiya.net refused or failed to load (HTTP 403/402 or empty), and web.archive.org is blocked for the fetch tool. Quotes below are what the fetch tool returned from each page. Saudi is the reference country, so no "follows Saudi" sentences apply.

Notes: pages show the current date (18 Nov 2026) in their banner, which was ignored. All weekdays match the dates unless flagged.

#### Ramadan 1436 (9)
- Gregorian: 2015-06-18 (Thursday). Source: https://www.arabnews.com/saudi-arabia/ramadan-to-start-thursday-762886 (press). Published 2015-06-17 23:46.
- Quote: "Tomorrow, June 18, is the start of holy month of Ramadan, the Supreme Court said on Tuesday." Derived: no.

#### Shawwal 1436 (10)
- Gregorian: 2015-07-17 (Friday). Source: https://arabnews.com/node/777491 (press). Published 2015-07-17 23:18 as shown.
- Quote: "The Supreme Court declared Thursday as the last day of Ramadan 2015 and Friday as the start of the Eid al-Fitr holidays." The page gives only weekdays, the date follows from the page date and Friday. Derived: no.

#### Dhu al-Hijjah 1436 (12)
- Gregorian: 2015-09-15 (Tuesday). Source: https://www.moroccoworldnews.com/2015/09/115597/saudi-arabia-says-eid-al-adha-to-be-celebrated-on-sep-24/ (press quoting the court). Published 2015-09-13.
- Quote: "Tuesday, September 15, 2015, will mark the beginning of the lunar month of Dhul Hijjah," per the Saudi Supreme Court. The same page: Eid al-Adha Thursday, September 24 (consistent, 15+9). Derived: no. Weaker source than the others (non-Saudi outlet).

#### Ramadan 1437 (9)
- Gregorian: 2016-06-06 (Monday). Source: https://www.arabnews.com/node/935046/saudi-arabia (press). Published 2016-06-04, updated 2016-06-06.
- Quote: "JEDDAH: Monday, June 6, 2016, is the first day of Ramadan, the Royal Court announced." Derived: no.

#### Shawwal 1437 (10)
- Gregorian: 2016-07-06 (Wednesday). Source: http://www.arabnews.com/node/949136/saudi-arabia (press). Published 2016-07-04 23:31.
- Quote: "Eid will be on Wednesday, the Saudi Supreme Court ruled." and "the first day of the month of Shawwal in the Hijri calendar falls on Wednesday." Page gives no calendar date, 2016-07-06 follows from the page date plus Wednesday and "Tuesday is the final day of Ramadan". Derived: no (weekday conversion).

#### Dhu al-Hijjah 1437 (12)
- Gregorian: 2016-09-03 (Saturday). Source: https://www.arabnews.com/node/978741/saudi-arabia (press). Published 2016-09-01, updated 2016-09-02.
- Quote: "Saturday, Sept. 3 will be the first day of Dul Hijjah". Attributed on the page to the semi-official Sabq.org citing sources, with the Supreme Court having separately announced that the current month would have 30 days. Supporting page https://www.arabnews.com/node/978011/saudi-arabia (published 2016-08-31): "If there is no validated sighting Thursday night, the first day of Dul Hijjah will be on Sept. 3." Not a direct court quote on the final date. Derived: no.

#### Ramadan 1438 (9)
- Gregorian: 2017-05-27 (Saturday). Source: https://www.arabnews.com/saudi-arabia/saturday-is-the-first-day-of-ramadan-in-saudi-arabia-1104721 (press). Published 2017-05-25.
- Quote: "JEDDAH: Saturday, May 27, is the start of holy month of Ramadan, the Supreme Court announced on Thursday." Derived: no.

#### Shawwal 1438 (10)
- Gregorian: 2017-06-25 (Sunday). Source: https://arabnews.com/node/1119816/saudi-arabia (press). Published 2017-06-24.
- Quote: "Sunday, June 25, is the first day of Eid Al-Fitr in Saudi Arabia after reported sightings of the new moon." Derived: no.

#### Dhu al-Hijjah 1438 (12)
- Gregorian: 2017-08-23 (Wednesday). Source: https://www.arabnews.com/node/1149021 (press). Published 2017-08-22 23:47.
- Quote: "The first day of the Islamic month of Dul-Hijjah this year is August 23, Saudi Arabia's Supreme Court declared on Tuesday." Derived: no.

#### Ramadan 1439 (9)
- Gregorian: 2018-05-17 (Thursday). Source: https://www.arabnews.com/node/1302826/saudi-arabia (press). Published 2018-05-15.
- Quote: "The Supreme Court has announced that due to the inability to witness the crescent moon on May 15, the month of Shaban will continue for a 30th day, and the first day of Ramadan will therefore be on Thursday." The date May 17 appears on the page only in an astronomer's quote, so the date is the weekday converted. Derived: no (weekday conversion, Shaban 30 completed). Conflicting earlier item: https://www.arabnews.com/node/1301711 (published 2018-05-13) said Ramadan "expected to begin on Tuesday", which was superseded by the court's 15 May decision. Official body's later statement used.

#### Shawwal 1439 (10)
- Gregorian: 2018-06-15 (Friday). Source: https://www.arabnews.com/saudi-arabia/saudi-arabia-celebrates-eid-al-fitr-on-friday-1320956 (press). Published 2018-06-13.
- Quote: "Eid Al-Fitr celebrations will begin in Saudi Arabia on Friday, June 15, the Saudi Supreme Court has announced." Derived: no.

#### Dhu al-Hijjah 1439 (12)
- Gregorian: 2018-08-12 (Sunday). Source: https://www.arabnews.com/node/1354501/saudi-arabia (press). Published 2018-08-11 19:40.
- Quote: "Dhu Al-Hijjah, the 12th month in the Islamic calendar, begins on Sunday (August 12), it was confirmed on Saturday night." Page does not name the body. Derived: no.

#### Ramadan 1440 (9)
- Gregorian: 2019-05-06 (Monday). Source: https://www.arabnews.com/node/1492921/saudi-arabia (press). Published 2019-05-05.
- Quote: "Saudi Arabian moon observers said they sighted the crescent of a new moon Sunday, confirming that Monday will be the start of the holy month of Ramadan." Supporting https://www.arabnews.com/node/1491456/amp (published 2019-05-02): "The Holy Month has been expected tom start on Monday, May 6" (that article is internally inconsistent, also says Sunday). Derived: no.

#### Shawwal 1440 (10)
- Gregorian: 2019-06-04 (Tuesday). Source: https://www.arabnews.com/node/1505831/saudi-arabia (press). Published 2019-06-03.
- Quote: "Eid Al-Fitr celebrations will begin in Saudi Arabia on Tuesday, June 4, the Saudi Supreme Court has announced." Derived: no.

#### Dhu al-Hijjah 1440 (12)
- Gregorian: 2019-08-02 (Friday). Sources: https://www.arabnews.com/saudi-arabia/first-day-of-hajj-confirmed-as-aug-9-1534151 (press, published 2019-08-01) and https://english.aawsat.com//home/article/1839651/saudi-arabia-says-eid-al-adha-sunday-aug-11 (press, published 2019-08-01).
- Quotes: "The Saudi Supreme Court confirmed Thursday that the crescent moon of Dhu Al-Hijjah had been sighted." and "Hajj will therefore begin on Friday August 9" and "Saudi Arabia announced Sunday, August 11 as the first day of Eid al-Adha." No page states 1 Dhu al-Hijjah. Derived: yes. Eid 2019-08-11 minus 9 days = 2019-08-02, matches Hajj start (8 Dhu al-Hijjah) on 08-09.

#### Ramadan 1441 (9)
- Gregorian: 2020-04-24 (Friday). Source: https://www.arabnews.com/saudi-arabia/ramadan-2020-will-start-on-friday-saudi-arabia-1660636 (press). Published 2020-04-23.
- Quote: "Ramadan 2020 will start on Friday April 24, Saudi Arabia's Supreme Court announced on Thursday." Derived: no.

#### Shawwal 1441 (10)
- Gregorian: 2020-05-24 (Sunday). Source: https://www.arabnews.com/saudi-arabia/saudi-arabia-announces-eid-al-fitr-will-start-on-sunday-1678461 (press). Published 2020-05-22.
- Quote: "Eid Al-Fitr will begin on Sunday May 24, authorities in Saudi Arabia have announced." Page names no specific body. Derived: no.

#### Dhu al-Hijjah 1441 (12)
- Gregorian: 2020-07-22 (Wednesday). Source: https://www.arabnews.com/saudi-arabia/dhul-al-hijjah-moon-not-sighted-eid-al-adha-begins-on-july-31-1707491 (press). Published 2020-07-20.
- Quote: "Dul Al-Hijjah will begin on Wednesday, July 22" and "The Saudi Supreme Court on Monday said that". Derived: no.

#### Ramadan 1442 (9)
- Gregorian: 2021-04-13 (Tuesday). Sources: https://www.arabnews.com/node/1841176/saudi-arabia (press, published 2021-04-12) and https://english.aawsat.com/home/article/2914961/saudi-arabia-declares-tuesday-first-day-ramadan (press, 2021-04-12 19:06, page header "01 Ramadan 1442").
- Quote: "Nearly 2 billion Muslims worldwide will mark the first day of Ramadan on Tuesday" and "Saudi Arabia's Supreme Court said on Monday evening". Date from weekday plus page date. Derived: no.

#### Shawwal 1442 (10)
- Gregorian: 2021-05-13 (Thursday). Source: https://www.arabnews.com/node/1857296/saudi-arabia (press). Published 2021-05-11.
- Quote: "Thursday will be the first day of Eid Al-Fitr, the Saudi Royal Court said on Tuesday night." Derived: no.

#### Dhu al-Hijjah 1442 (12)
- Gregorian: 2021-07-11 (Sunday). Source: https://www.arabnews.com/node/1891456/saudi-arabia (press). Published 2021-07-09.
- Quote: "Dul Hijjah, which this year begins on July 11," and "Eid Al-Adha will be celebrated on Tuesday, July 20". Page does not name the announcer. Derived: no.

#### Ramadan 1443 (9)
- Gregorian: 2022-04-02 (Saturday). Source: https://www.arabnews.com/saudi-arabia/ramadan-2022-to-start-on-saturday-saudi-supreme-court-2055161 (press). Published 2022-04-01.
- Quote: "Most of the world's 2 billion Muslims will mark the first day of Ramadan on Saturday". announcement by the Saudi Supreme Court on Friday. Derived: no.

#### Shawwal 1443 (10)
- Gregorian: 2022-05-02 (Monday). Source: https://www.arabnews.com/node/2073716/saudi-arabia (press). Published 2022-04-30.
- Quote: "meaning Eid Al-Fitr would begin on Monday May 2." Announced by the Supreme Court on Saturday. Derived: no.

#### Dhu al-Hijjah 1443 (12)
- Gregorian: 2022-06-30 (Thursday). Source: https://www.arabnews.com/node/2113431/saudi-arabia (press). Published 2022-06-29.
- Quote: "Dul Hijjah, which begins this year on June 30, is the month during which the Hajj pilgrimage takes place." and "the five-day Eid Al-Adha celebration will begin on July 9." Announcer not named on the page. Derived: no.

#### Ramadan 1444 (9)
- Gregorian: 2023-03-23 (Thursday). Source: https://www.arabnews.com/node/2272956/saudi-arabia (press). Published 2023-03-21. Also https://english.aawsat.com/home/article/4225486/saudi-arabia-declares-thursday-first-day-ramadan (2023-03-21 16:48, "29 Sha'ban 1444").
- Quote: "Thursday, March 23, will be the start of the holy month of Ramadan." Derived: no.

#### Shawwal 1444 (10)
- Gregorian: 2023-04-21 (Friday). Source: https://www.arabnews.com/node/2290266/saudi-arabia (press). Published 2023-04-20.
- Quote: "Eid Al-Fitr celebrations to begin on Friday, April 21, 2023, the Crescent Sighting Committee announced." Derived: no.

#### Dhu al-Hijjah 1444 (12)
- Gregorian: 2023-06-19 (Monday). Source: https://www.arabnews.com/node/2323821/saudi-arabia (press). Published 2023-06-18.
- Quotes: "has been sighted, Saudi Arabia's Supreme Court announced on Sunday." and "Those Muslims who are not performing the pilgrimage this year will celebrate Eid Al-Adha on Wednesday June 28." and Hajj "will start on June 26". Page does not state 1 Dhu al-Hijjah. Derived: yes. Eid 2023-06-28 minus 9 = 2023-06-19, matches Hajj start 8 Dhu al-Hijjah on 06-26.

#### Ramadan 1445 (9)
- Gregorian: 2024-03-11 (Monday). Source: https://english.aawsat.com/gulf/4903476-saudi-arabia-says-ramadan-starts-monday (press). Published 2024-03-10 17:42.
- Quote: "Saudi state television reported authorities saw the crescent moon, saying Ramadan starts Monday." The page also says the Supreme Court announced the sighting. Date from weekday plus page date. Derived: no.

#### Shawwal 1445 (10)
- Gregorian: 2024-04-10 (Wednesday). Source: https://english.aawsat.com/gulf/4955741-saudi-arabia-declares-wednesday-first-day-eid-al-fitr (press). Published 2024-04-08 17:45 (header "30 Ramadan 1445").
- Quote: "making Wednesday the first day of Eid al-Fitr, announced the Kingdom's Supreme Court." Date from weekday plus page date. Derived: no.

#### Dhu al-Hijjah 1445 (12)
- Gregorian: 2024-06-07 (Friday). Source: https://www.arabnews.com/node/2525256/saudi-arabia (press). Published 2024-06-06.
- Quote: "The month of Dhul Hijjah will start on Friday June 7, Saudi Arabia's Supreme Court annonced on Thursday." Derived: no.

#### Ramadan 1446 (9)
- Gregorian: 2025-03-01 (Saturday). Source: https://arabnews.com/node/2595035/saudi-arabia (press). Published 2025-03-27.
- Quote: "The sighting of the Shawwal crescent moon marks the end of the fasting month of Ramadan that this year began on March 1." Weak: retrospective, no weekday, announcer is the Supreme Court only for the Shawwal call. The direct 2025-02-28 announcement pages (arabnews.pk 2591957, aawsat) failed to fetch (empty result). Derived: no.

#### Shawwal 1446 (10)
- Gregorian: 2025-03-30 (Sunday). Source: https://www.arabnews.com/saudi-arabia/crescent-moon-sighted-eid-al-fitr-on-sunday-2595268 (press). Published 2025-03-29.
- Quote: "begins on Sunday, March 30." Supporting https://english.aawsat.com/gulf/5127058-saudi-arabia-gulf-countries-declare-sunday-first-day-eid-al-fitr (2025-03-29): "declared on Saturday that Sunday will be the first day of Eid al-Fitr." Derived: no.

#### Dhu al-Hijjah 1446 (12)
- Gregorian: 2025-05-28 (Wednesday). Source: https://www.arabnews.com/node/2602375/saudi-arabia (press). Published 2025-05-27.
- Quotes: "Muslims who are not performing the pilgrimage this year will celebrate Eid Al-Adha on June 6." and "the Supreme Court announced on Tuesday." Page does not state 1 Dhu al-Hijjah. Derived: yes. Eid 2025-06-06 minus 9 = 2025-05-28 (sighted Tuesday 27 May evening).

#### Ramadan 1447 (9)
- Gregorian: 2026-02-18 (Wednesday). Source: https://www.arabnews.com/node/2633313/amp (press). Published 2026-02-17.
- Quote: "The first day of fasting will begin on Wednesday, February 18" and Saudi Supreme Court "announced on Tuesday". Also https://english.aawsat.com/gulf/5241844-saudi-arabia-declares-wednesday-first-day-ramadan (2026-02-17): "Saudi Arabia declared on Tuesday that the holy fasting month of Ramadan will begin on Wednesday." Derived: no.

#### Shawwal 1447 (10)
- Gregorian: 2026-03-20 (Friday). Source: https://english.aawsat.com/gulf/5252760-saudi-arabia-gulf-countries-declare-friday-first-day-eid-al-fitr (press). Published 2026-03-18 16:59 ("30 Ramadan 1447").
- Quote: "Saudi Arabia announced on Wednesday that Friday was the first day of Eid al-Fitr." and "The Saudi Supreme Court made the decision after observers failed to sight the Shawwal crescent." No calendar date on page, 2026-03-20 is the Friday after 2026-03-18. Derived: no (weekday conversion).

#### Dhu al-Hijjah 1447 (12)
- Gregorian: 2026-05-18 (Monday). Source: https://www.arabnews.com/saudi-arabia/eid-al-adha-on-may-27-after-crescent-sighted-in-saudi-arabia-2643929 (press). Published 2026-05-17.
- Quote: "Eid Al-Adha will be celebrated on Wednesday May 27 after the crescent moon of Dhul Hijjah was sighted on Sunday evening." Announcer not named, no explicit 1 Dhu al-Hijjah. Derived: yes. Eid 2026-05-27 minus 9 = 2026-05-18, consistent with sighting on Sunday 17 May evening.

#### Missing
None. Disagreements: Ramadan 1439 early Arab News item said Tuesday (superseded by court). Ramadan 1440 Arab News 2019-05-02 article is internally inconsistent (Monday vs Sunday), the 2019-05-05 article settles Monday. 1437 Dhu al-Hijjah rests on Sabq via Arab News plus the court's 30-day statement. 1436 Dhu al-Hijjah rests on Morocco World News.

| country | hijri_year | hijri_month | gregorian | derived | source |
|---|---|---|---|---|---|
| Saudi | 1436 | 9 | 2015-06-18 | no | https://www.arabnews.com/saudi-arabia/ramadan-to-start-thursday-762886 |
| Saudi | 1436 | 10 | 2015-07-17 | no | https://arabnews.com/node/777491 |
| Saudi | 1436 | 12 | 2015-09-15 | no | https://www.moroccoworldnews.com/2015/09/115597/saudi-arabia-says-eid-al-adha-to-be-celebrated-on-sep-24/ |
| Saudi | 1437 | 9 | 2016-06-06 | no | https://www.arabnews.com/node/935046/saudi-arabia |
| Saudi | 1437 | 10 | 2016-07-06 | no | http://www.arabnews.com/node/949136/saudi-arabia |
| Saudi | 1437 | 12 | 2016-09-03 | no | https://www.arabnews.com/node/978741/saudi-arabia |
| Saudi | 1438 | 9 | 2017-05-27 | no | https://www.arabnews.com/saudi-arabia/saturday-is-the-first-day-of-ramadan-in-saudi-arabia-1104721 |
| Saudi | 1438 | 10 | 2017-06-25 | no | https://arabnews.com/node/1119816/saudi-arabia |
| Saudi | 1438 | 12 | 2017-08-23 | no | https://www.arabnews.com/node/1149021 |
| Saudi | 1439 | 9 | 2018-05-17 | no | https://www.arabnews.com/node/1302826/saudi-arabia |
| Saudi | 1439 | 10 | 2018-06-15 | no | https://www.arabnews.com/saudi-arabia/saudi-arabia-celebrates-eid-al-fitr-on-friday-1320956 |
| Saudi | 1439 | 12 | 2018-08-12 | no | https://www.arabnews.com/node/1354501/saudi-arabia |
| Saudi | 1440 | 9 | 2019-05-06 | no | https://www.arabnews.com/node/1492921/saudi-arabia |
| Saudi | 1440 | 10 | 2019-06-04 | no | https://www.arabnews.com/node/1505831/saudi-arabia |
| Saudi | 1440 | 12 | 2019-08-02 | yes | https://english.aawsat.com//home/article/1839651/saudi-arabia-says-eid-al-adha-sunday-aug-11 |
| Saudi | 1441 | 9 | 2020-04-24 | no | https://www.arabnews.com/saudi-arabia/ramadan-2020-will-start-on-friday-saudi-arabia-1660636 |
| Saudi | 1441 | 10 | 2020-05-24 | no | https://www.arabnews.com/saudi-arabia/saudi-arabia-announces-eid-al-fitr-will-start-on-sunday-1678461 |
| Saudi | 1441 | 12 | 2020-07-22 | no | https://www.arabnews.com/saudi-arabia/dhul-al-hijjah-moon-not-sighted-eid-al-adha-begins-on-july-31-1707491 |
| Saudi | 1442 | 9 | 2021-04-13 | no | https://www.arabnews.com/node/1841176/saudi-arabia |
| Saudi | 1442 | 10 | 2021-05-13 | no | https://www.arabnews.com/node/1857296/saudi-arabia |
| Saudi | 1442 | 12 | 2021-07-11 | no | https://www.arabnews.com/node/1891456/saudi-arabia |
| Saudi | 1443 | 9 | 2022-04-02 | no | https://www.arabnews.com/saudi-arabia/ramadan-2022-to-start-on-saturday-saudi-supreme-court-2055161 |
| Saudi | 1443 | 10 | 2022-05-02 | no | https://www.arabnews.com/node/2073716/saudi-arabia |
| Saudi | 1443 | 12 | 2022-06-30 | no | https://www.arabnews.com/node/2113431/saudi-arabia |
| Saudi | 1444 | 9 | 2023-03-23 | no | https://www.arabnews.com/node/2272956/saudi-arabia |
| Saudi | 1444 | 10 | 2023-04-21 | no | https://www.arabnews.com/node/2290266/saudi-arabia |
| Saudi | 1444 | 12 | 2023-06-19 | yes | https://www.arabnews.com/node/2323821/saudi-arabia |
| Saudi | 1445 | 9 | 2024-03-11 | no | https://english.aawsat.com/gulf/4903476-saudi-arabia-says-ramadan-starts-monday |
| Saudi | 1445 | 10 | 2024-04-10 | no | https://english.aawsat.com/gulf/4955741-saudi-arabia-declares-wednesday-first-day-eid-al-fitr |
| Saudi | 1445 | 12 | 2024-06-07 | no | https://www.arabnews.com/node/2525256/saudi-arabia |
| Saudi | 1446 | 9 | 2025-03-01 | no | https://arabnews.com/node/2595035/saudi-arabia |
| Saudi | 1446 | 10 | 2025-03-30 | no | https://www.arabnews.com/saudi-arabia/crescent-moon-sighted-eid-al-fitr-on-sunday-2595268 |
| Saudi | 1446 | 12 | 2025-05-28 | yes | https://www.arabnews.com/node/2602375/saudi-arabia |
| Saudi | 1447 | 9 | 2026-02-18 | no | https://www.arabnews.com/node/2633313/amp |
| Saudi | 1447 | 10 | 2026-03-20 | no | https://english.aawsat.com/gulf/5252760-saudi-arabia-gulf-countries-declare-friday-first-day-eid-al-fitr |
| Saudi | 1447 | 12 | 2026-05-18 | yes | https://www.arabnews.com/saudi-arabia/eid-al-adha-on-may-27-after-crescent-sighted-in-saudi-arabia-2643929 |

### UAE announced Hijri month starts, 1436 to 1447

Official body: UAE Moon Sighting Committee (chaired by the Justice Minister to 2024, then the UAE Council for Fatwa committees, announced via the Presidential Court / WAM). wam.ae pages returned empty text or HTTP 403 to the fetcher, so press reports of the official announcement are used. All sources below are press unless stated. Gulf News pages show only a "Last updated" stamp (often a 2018 CMS migration date), so that is not the real publication date.

Missing: Dhu al-Hijjah 1436, 1437, 1438, 1439, 1441, 1442, 1443, 1444, 1445 (no UAE-specific announcement found. the pages found only report the Saudi announcement or astronomy-centre sightings and a UAE Eid date inferred from holidays). See the notes at the end.

#### Ramadan 1436
- Gregorian: 2015-06-18 (Thursday). Source: https://www.thenationalnews.com/uae/ramadan-2015-fasting-to-begin-on-thursday-1.95466 (press, published 2015-06-15). Quote: "Fasting for Ramadan will begin on Thursday, the official moon-sighting committee has announced." The calendar date June 18 comes from search excerpts of Khaleej Times/Coastal Digest (Sha'ban ended Wednesday June 17), not from this fetched page. Derived: no. Saudi sentence: none.

#### Shawwal 1436
- 2015-07-17 (Friday). https://gulfnews.com/amp/story/uae%2Fgovernment%2Feid-al-fitr-in-the-uae-on-friday-july-17-1.1551657 (press, last updated 2018-11-08). Quote: "Abu Dhabi: Eid Al Fitr in the UAE is on Friday, July 17, announced the UAE Moon Sighting Committee." Derived: no. Saudi: "Other Gulf Cooperation Council countries, Saudi Arabia, Kuwait, Qatar, and Bahrain, will also celebrate Eid on Friday."

#### Ramadan 1437
- 2016-06-06 (Monday). https://www.thenationalnews.com/uae/ramadan-2016-fasting-to-begin-on-monday-1.199937/ (press, 2016-06-05). Quote: "The holy month of Ramadan in the UAE for the Hijri year 1437 will begin tomorrow, Monday, June 6, 2016," said a statement by the committee. Derived: no.

#### Shawwal 1437
- 2016-07-06 (Wednesday). Only seen in a search excerpt (gulfbusiness / Dubai Media Office tweet "#UAE declares Wednesday first day of Eid Al-Fitr"). the Gulf Business page returned HTTP 403. Candidate page: https://gulfbusiness.com/?p=85883 . Same excerpt said the committee set Tuesday July 5 as the last day of Ramadan. Derived: no. Weaker evidence: not read on a fetched page.

#### Ramadan 1438
- 2017-05-27 (Saturday). https://thenationalnews.com/uae/ramadan-2017-fasting-set-to-begin-on-saturday-in-uae-1.643416 (press, 2017-05-25). Quote: the first day of Ramadan is "set to be on Saturday" (no calendar date in the page). The date 2017-05-27 comes from search excerpts of Dubai Gazette/Gulf Business ("Friday was the last day of Shaban Hijri 1438, and Saturday, May 27, is the first day of Ramadan", committee announcement Friday May 26). Derived: no. Note the page is dated May 25, before the final decision, so it is a weaker source than the excerpts.

#### Shawwal 1438
- 2017-06-25 (Sunday). https://gulfnews.com/uae/sunday-is-first-day-of-eid-al-fitr-in-uae-1.2047314 (press, last updated 2018-11-08). Quote: "Sunday, June 25 will be the first day of the month of Shawwal and the first day of Eid Al Fitr." Saudi: reports the moon was seen in Saudi Arabia that Saturday evening. Derived: no.

#### Ramadan 1439
- 2018-05-17 (Thursday). https://gulfnews.com/uae/ramadan-2018-starts-on-may-17-in-the-uae-1.2221540 (press, last updated 2018-11-07). Quote: "Abu Dhabi: The UAE Moon Sighting Committee has announced the first day of Ramadan this year will be May 17." Also: "Thursday is the first day of the holy month." Saudi: "Saudi Arabia, Kuwait, Egypt, Jordan and Lebanon have also all officially declared" May 17. Derived: no.

#### Shawwal 1439
- 2018-06-15 (Friday). Search excerpt of Gulf Business/The National: Justice Minister Al Badi said "Friday, June 15, 2018 will be the first day of Shawwal 1439 Hijri." Candidate page https://www.thenationalnews.com/uae/uae-moon-sighting-committee-announces-eid-al-fitr-1.740274 (not fetched). Derived: no. Weaker evidence.

#### Ramadan 1440
- 2019-05-06 (Monday). https://khaleejtimes.com/uae/uae-announces-first-day-of-ramadan-after-moon-sighting (press, 2019-05-05). Quote: "UAE has officially confirmed that Ramadan 1440 will begin on Monday (May 6)". Saudi: subheading "Saudi Arabia also spots Ramadan 1440 moon." Derived: no.

#### Shawwal 1440
- 2019-06-04 (Tuesday). https://www.thenationalnews.com/uae/eid-al-fitr-2019-in-uae-to-begin-on-tuesday-says-moon-sighting-committee-1.869973 (press, 2019-06-04). Quote: "Tuesday will be the first day of the month of Shawwal, and therefore the first day of Eid Al Fitr." The date June 4 comes from search excerpts (statement: "Tomorrow, Tuesday, the 4th of June, will be the first day of Shawwal 1440 H"). Derived: no.

#### Dhu al-Hijjah 1440
- 2019-08-02 (derived). https://gulfnews.com/amp/story/uae%2Feid-al-adha-2019-dates-confirmed-5-day-weekend-for-uae-residents-1.1564671132828 (press, last updated 2019-08-01). Quote: "The moon-sighting committee of the UAE confirmed late on Thursday that Eid Al Adha would be celebrated on August 11." Eid Aug 11 minus 9 days = Aug 2. Page also says Arafat Day is Saturday Aug 10, consistent. Derived: yes. Saudi: page says Saudi authorities urged sighting on Thursday.

#### Ramadan 1441
- 2020-04-24 (Friday). https://gulfnews.com/uae/uae-announced-ramadan-2020-starts-friday-april-24-1.1587640361659 (press, last updated 2020-04-24). Quote: "Abu Dhabi: Ramadan will start on Friday, April 24, the UAE's moon-sighting committee announced late on Thursday." Derived: no.

#### Shawwal 1441
- 2020-05-24 (Sunday). https://gulfnews.com/uae/eid-al-fitr-2020-uae-announces-sunday-as-the-first-day-of-eid-al-fitr-1.1590155834006 (press, last updated 2020-05-22). Quote: "Sunday, the 24th of May will be the first day of Shawwal 1441 H, marking the beginning of Eid Al Fitr". Derived: no.

#### Ramadan 1442
- 2021-04-13 (Tuesday). https://gulfnews.com/uae/ramadan/ramadan-2021-uae-confirms-first-day-of-ramadan-as-tuesday-april-13-1.1618139774704 (press, last updated 2021-04-12). Quote: "Dubai: The first day of Ramadan will be observed on April 13, Tuesday, UAE's moon-sighting committee announced on Monday." Saudi: "Saudi Arabia has also announced Tuesday marks the first day of Ramadan." Derived: no.

#### Shawwal 1442
- 2021-05-13 (Thursday). Search excerpt of WAM ("Eid al-Fitr Thursday in UAE", https://www.wam.ae/en/article/hszrcaje-eid-al-fitr-thursday-uae , agency, page text did not load) and Gulf News https://gulfnews.com/uae/eid-al-fitr-2021-uae-confirms-thursday-as-the-first-day-of-eid-al-fitr-1.1620734557603 (not fetched): committee announced Tuesday May 11 that May 12 was the last day of Ramadan, Eid Thursday May 13. Derived: no. Weaker evidence.

#### Ramadan 1443
- 2022-04-02 (Saturday). https://gulfnews.com/uae/ramadan/ramadan-2022-uae-confirms-first-day-of-ramadan-1.1648821472533 (press, last updated 2022-04-02). Quote: "The first day of Ramadan 2022 will be observed on Saturday, April 2, the UAE's moon-sighting committee announced on Friday." Derived: no.

#### Shawwal 1443
- 2022-05-02 (Monday). https://khaleejtimes.com/ramadan/eid-al-fitr-2022-shawwal-moon-not-sighted-in-uae (press, 2022-04-30). Quote: "The UAE has officially declared Monday, May 2, will be the first day of Eid-Al-Fitr." Page says this was the first day of Shawwal 1443 AH, announced by WAM. Derived: no.

#### Ramadan 1444
- 2023-03-23 (Thursday). Search excerpts only (Gulf Business "UAE confirms first day of Ramadan 2023", WAM via Esquire): "The first day of Ramadan will be observed on Thursday, March 23, UAE's moon-sighting committee announced on Tuesday." Sha'ban completed 30 days (March 22 = 30th). Gulf Today page returned HTTP 403. Candidate: https://gulfnews.com/uae/ramadan/uae-confirms-first-day-of-ramadan-2023-1.1679399521774 . Derived: no. Weaker evidence.

#### Shawwal 1444
- 2023-04-21 (Friday). Search excerpts only (WAM via ProPakistani/Lokmat, Gulf Today): the UAE Moon-sighting Committee said Friday April 21, 2023 corresponding to 1 Shawwal is the first day of Eid. Candidate: https://www.arabianbusiness.com/gcc/uae/uae-announces-eid-al-fitr-2023-will-start-on-friday . Derived: no. Weaker evidence.

#### Ramadan 1445
- 2024-03-11 (Monday). https://gulfnews.com/uae/ramadan/uae-announces-first-day-of-ramadan-2024-1.1710071272335 (press, last updated 2024-03-11). Page text: UAE Moon-sighting Committee set Monday, 11 March 2024 as the start of Ramadan. verbatim fragment "will be the first day of Ramadan in the UAE." Search excerpt: Sunday March 10 was the last day of Sha'ban. Saudi sentence in excerpt: "It follows a decision from Saudi Arabia that Ramadan would commence on Monday" (not on the fetched page). Derived: no.

#### Shawwal 1445
- 2024-04-10 (Wednesday). https://gulfnews.com/uae/eid-al-fitr-2024-uae-confirms-first-day-of-eid-1.1712511876281 (press, last updated 2024-04-08). Quote: "The UAE has announced that Wednesday, April 10, 2024, corresponding to 1st Shawwal, will be the first day of Eid Al Fitr." Derived: no.

#### Ramadan 1446
- 2025-03-01 (Saturday). https://gulfnews.com/uae/ramadan/ramadan-2025-uae-saudi-arabia-gear-up-for-crescent-moon-sighting-1.500049341 (press, last updated 2025-02-28). Quote: "tomorrow, Saturday, March 1, 2025, will mark the first day of the holy month of Ramadan." Announced by the Presidential Court after the Ramadan Crescent Sighting Committee (UAE Council for Fatwa). Derived: no.

#### Shawwal 1446
- 2025-03-30 (Sunday). https://www.khaleejtimes.com/uae/eid-al-fitr-uae-start-date-moon-sighted (press, 2025-03-29). Quote: "The UAE will celebrate Eid Al Fitr on Sunday, March 30, authorities confirmed after the Shawwal crescent was spotted today." Derived: no.

#### Dhu al-Hijjah 1446
- 2025-05-28 (Wednesday). https://www.dubaieye1038.com/news/local/uae-announces-first-day-of-eid-al-adha/ (press, 2025-05-27). Quote: "The UAE Presidential Court has confirmed the sighting of the crescent moon, marking the start of the month of Dhu al-Hijjah on Wednesday, May 28." Also "Eid Al Adha will be observed on Friday, June 6". Derived: no.

#### Ramadan 1447
- 2026-02-18 (Wednesday). https://gulfnews.com/uae/ramadan/ramadan-2026-uae-confirms-first-day-after-crescent-moon-sighting-1.500446056 (press, last updated 2026-02-17). Quote: "Accordingly, Wednesday, February 18, will be the first day of the holy month in the country." Derived: no. Saudi: also Feb 18 per search excerpts (not on fetched page).

#### Shawwal 1447
- 2026-03-20 (Friday). Search excerpt of WAM headline "BREAKING: Thursday completes Ramadan. Friday marks first day of Eid Al-Fitr in UAE: Presidential Court" (https://www.wam.ae/en/article/bz9l2ze-breaking-thursday-completes-ramadan-friday-marks , agency. the page text did not load, headline and snippet seen only in search results): Thursday 19 March 2026 last day of Ramadan, Friday 20 March 2026 first day of Eid. Derived: no.

#### Dhu al-Hijjah 1447
- 2026-05-18 (Monday). https://en.aletihad.ae/amp/news/uae/4666426/uae-council-for-fatwa-confirms-sighting-of-dhul-hijjah-cresc (WAM text republished by Aletihad, official body, 2026-05-17). Quote: "Monday, May 18, 2026, will mark the first day of Dhul Hijjah 1447 AH" and "Wednesday, May 27, 2026, will be the first day of Eid Al Adha in the United Arab Emirates." Derived: no.

#### Notes on the missing Dhu al-Hijjah starts
- 1436 (Sept 2015): pages found give the Saudi announcement (Dhu al-Hijjah Sept 15, Eid Sept 24) and UAE private-sector holiday dates only. Khaleej Times headline "Eid Al Adha to be celebrated on September 24 in Saudi Arabia. UAE announcement soon" never found its UAE follow-up.
- 1437, 1438, 1439, 1441, 1442: results only reported Saudi announcements or pre-sighting UAE forecasts. A Gulf Business piece of 2020-07-21 said "no official announcement ... by UAE authorities" yet.
- 1443 (2022): holiday dates and Saudi sighting only. 1444 (2023): Khaleej Times (https://khaleejtimes.com/uae/eid-al-adha-in-uae-crescent-moon-of-dhul-hijjah-spotted-in-abu-dhabi?amp=1, 2023-06-19) shows the Saudi Supreme Court announced Monday June 19 and the Astronomy Centre photographed the crescent. no UAE committee statement. 1445 (2024): same pattern (Astronomy Centre photo June 7, Saudi announcement, Eid June 16 by holiday). These would be Umm al-Qura or Saudi dates, so they are excluded under the brief.
- Pre-2019 the UAE did not appear to convene a Dhu al-Hijjah sighting. the 2019 Gulf News article is the first UAE committee statement found.

| country | hijri_year | hijri_month | gregorian | derived | source |
|---|---|---|---|---|---|
| UAE | 1436 | 9 | 2015-06-18 | no | https://www.thenationalnews.com/uae/ramadan-2015-fasting-to-begin-on-thursday-1.95466 |
| UAE | 1436 | 10 | 2015-07-17 | no | https://gulfnews.com/amp/story/uae%2Fgovernment%2Feid-al-fitr-in-the-uae-on-friday-july-17-1.1551657 |
| UAE | 1437 | 9 | 2016-06-06 | no | https://www.thenationalnews.com/uae/ramadan-2016-fasting-to-begin-on-monday-1.199937/ |
| UAE | 1437 | 10 | 2016-07-06 | no | https://gulfbusiness.com/?p=85883 |
| UAE | 1438 | 9 | 2017-05-27 | no | https://thenationalnews.com/uae/ramadan-2017-fasting-set-to-begin-on-saturday-in-uae-1.643416 |
| UAE | 1438 | 10 | 2017-06-25 | no | https://gulfnews.com/uae/sunday-is-first-day-of-eid-al-fitr-in-uae-1.2047314 |
| UAE | 1439 | 9 | 2018-05-17 | no | https://gulfnews.com/uae/ramadan-2018-starts-on-may-17-in-the-uae-1.2221540 |
| UAE | 1439 | 10 | 2018-06-15 | no | https://www.thenationalnews.com/uae/uae-moon-sighting-committee-announces-eid-al-fitr-1.740274 |
| UAE | 1440 | 9 | 2019-05-06 | no | https://khaleejtimes.com/uae/uae-announces-first-day-of-ramadan-after-moon-sighting |
| UAE | 1440 | 10 | 2019-06-04 | no | https://www.thenationalnews.com/uae/eid-al-fitr-2019-in-uae-to-begin-on-tuesday-says-moon-sighting-committee-1.869973 |
| UAE | 1440 | 12 | 2019-08-02 | yes | https://gulfnews.com/amp/story/uae%2Feid-al-adha-2019-dates-confirmed-5-day-weekend-for-uae-residents-1.1564671132828 |
| UAE | 1441 | 9 | 2020-04-24 | no | https://gulfnews.com/uae/uae-announced-ramadan-2020-starts-friday-april-24-1.1587640361659 |
| UAE | 1441 | 10 | 2020-05-24 | no | https://gulfnews.com/uae/eid-al-fitr-2020-uae-announces-sunday-as-the-first-day-of-eid-al-fitr-1.1590155834006 |
| UAE | 1442 | 9 | 2021-04-13 | no | https://gulfnews.com/uae/ramadan/ramadan-2021-uae-confirms-first-day-of-ramadan-as-tuesday-april-13-1.1618139774704 |
| UAE | 1442 | 10 | 2021-05-13 | no | https://gulfnews.com/uae/eid-al-fitr-2021-uae-confirms-thursday-as-the-first-day-of-eid-al-fitr-1.1620734557603 |
| UAE | 1443 | 9 | 2022-04-02 | no | https://gulfnews.com/uae/ramadan/ramadan-2022-uae-confirms-first-day-of-ramadan-1.1648821472533 |
| UAE | 1443 | 10 | 2022-05-02 | no | https://khaleejtimes.com/ramadan/eid-al-fitr-2022-shawwal-moon-not-sighted-in-uae |
| UAE | 1444 | 9 | 2023-03-23 | no | https://gulfnews.com/uae/ramadan/uae-confirms-first-day-of-ramadan-2023-1.1679399521774 |
| UAE | 1444 | 10 | 2023-04-21 | no | https://www.arabianbusiness.com/gcc/uae/uae-announces-eid-al-fitr-2023-will-start-on-friday |
| UAE | 1445 | 9 | 2024-03-11 | no | https://gulfnews.com/uae/ramadan/uae-announces-first-day-of-ramadan-2024-1.1710071272335 |
| UAE | 1445 | 10 | 2024-04-10 | no | https://gulfnews.com/uae/eid-al-fitr-2024-uae-confirms-first-day-of-eid-1.1712511876281 |
| UAE | 1446 | 9 | 2025-03-01 | no | https://gulfnews.com/uae/ramadan/ramadan-2025-uae-saudi-arabia-gear-up-for-crescent-moon-sighting-1.500049341 |
| UAE | 1446 | 10 | 2025-03-30 | no | https://www.khaleejtimes.com/uae/eid-al-fitr-uae-start-date-moon-sighted |
| UAE | 1446 | 12 | 2025-05-28 | no | https://www.dubaieye1038.com/news/local/uae-announces-first-day-of-eid-al-adha/ |
| UAE | 1447 | 9 | 2026-02-18 | no | https://gulfnews.com/uae/ramadan/ramadan-2026-uae-confirms-first-day-after-crescent-moon-sighting-1.500446056 |
| UAE | 1447 | 10 | 2026-03-20 | no | https://www.wam.ae/en/article/bz9l2ze-breaking-thursday-completes-ramadan-friday-marks |
| UAE | 1447 | 12 | 2026-05-18 | no | https://en.aletihad.ae/amp/news/uae/4666426/uae-council-for-fatwa-confirms-sighting-of-dhul-hijjah-cresc |

### Qatar announced Hijri month starts, 1436 to 1447

Official body: Crescent Sighting Committee (Moon Sighting Committee), Ministry of Endowments and Islamic Affairs (Awqaf), chaired by Sheikh Dr Thaqeel Al Shammari. Announced on Qatar TV, X and QNA. Sources are The Peninsula Qatar, Gulf Times, Qatar Living (press, repeating the committee statement or QNA wire text) and QNA (agency) where noted. Peninsula pages show "last updated" stamps from a 2021 CMS migration, so the publication date quoted is the original "Published" time.

Missing: Dhu al-Hijjah 1436, 1437, 1438, 1439, 1440, 1441, 1442, 1443, 1444, 1445, 1447. Qatar's Eid Al Adha / Dhu al-Hijjah dates in these years appear in the press only as Saudi announcements or as Qatar Calendar House astronomical calculations (which do not count), plus Amiri Diwan holiday notices and Awqaf prayer arrangements that name no moon-sighting decision. Dhu al-Hijjah 1446 is included with a caveat (below). No Qatar committee statement on Dhu al-Hijjah was found for any year.

#### Ramadan 1436
- 2015-06-18 (Thursday). https://www.qatarliving.com/en/article/ramadan-qatar-starts-thursday (press, 2015-06-16). Quote: "Fasting for the month of Ramadan this year will begin on Thursday, June 18th" announced by the Ministry of Awqaf and Islamic Affairs. Derived: no. Saudi sentence: "Qatar's neighbouring countries, Saudi Arabia, UAE and other GCC nations will also observe the holy month."

#### Shawwal 1436
- 2015-07-17 (Friday). https://thepeninsulaqatar.com/article/16/07/2015/eid-al-fitr-begins-friday-in-qatar (press via QNA, 2015-07-16). Quote: "Friday, July 17,2015 will be the first day of the blessed Eid Al-Fitr in Qatar." Announced by the Crescent Moon Sighting Committee of the Ministry of Awqaf. Derived: no.

#### Ramadan 1437
- 2016-06-06 (Monday). https://www.gulf-times.com/story/496956/monday-is-first-day-of-ramadan-in-qatar/amp (press, 2016-06-05). Quote: "Monday, June 6, will be the first day of Ramadan" announced by Qatar's moon-sighting panel of the endowments and Islamic affairs ministry. Saudi: an AFP addition says Ramadan was also set to start Monday in Saudi Arabia, the UAE and Yemen. Derived: no.

#### Shawwal 1437
- 2016-07-06 (Wednesday). https://thepeninsulaqatar.com/article/05/07/2016/Eid-Al-Fitr-in-Qatar-on-Wednesday (press, 2016-07-05). Quote: "Wednesday July 6 will be the first day of Eid Al-Fitr" (Crescent Sighting Committee, posted by Awqaf on Twitter). Derived: no.

#### Ramadan 1438
- 2017-05-27 (Saturday). https://thepeninsulaqatar.com/article/27/05/2017/Ramadan-begins-today-in-Qatar (press, 2017-05-27 00:34). Quote: "announced that today, May 27, will be the first day of Ramadan." Announced by the Ramadan Crescent Sighting Committee of the Ministry of Awqaf. Derived: no. A May 26 Qatar Tribune item (search excerpt) said the committee first postponed its decision, then confirmed the sighting that night.

#### Shawwal 1438
- 2017-06-25 (Sunday). https://thepeninsulaqatar.com/article/24/06/2017/Eid-Al-Fitr-to-be-celebrated-tomorrow-(Sunday) (press, 2017-06-24). Quote: "tomorrow, Sunday, June 25, 2017 is the first day of the blessed Eid al-Fitr in Qatar." Moon Sighting Committee. Derived: no.

#### Ramadan 1439
- 2018-05-17 (Thursday). https://thepeninsulaqatar.com/article/15/05/2018/Thursday-will-be-first-day-of-Ramadan-in-Qatar (press, 2018-05-15). Quote: "the day after tomorrow,Thursday, May 17, will be the first day of the holy month of Ramadan." Moon Sighting Committee. Derived: no.

#### Shawwal 1439
- 2018-06-15 (Friday). https://www.gulf-times.com/story/596282/friday-is-first-day-of-eid-al-fitr-in-qatar (press, 2018-06-14). Quote: "Friday is the first day of the blessed Eid al-Fitr." The page shows weekday only. the date June 15 comes from the search excerpt of the same story (committee met Thursday June 14 and confirmed the sighting at Al Ghazlania). Derived: no.

#### Ramadan 1440
- 2019-05-06 (Monday). https://thepeninsulaqatar.com/article/04/05/2019/Monday-will-be-first-day-of-Ramadan-in-Qatar (press, 2019-05-04). Quote: Monday, May 6, 2019 "will be the first day of the holy month of Ramadan." Moon Sighting Committee. Derived: no.

#### Shawwal 1440
- 2019-06-04 (Tuesday). https://thepeninsulaqatar.com/article/03/06/2019/Eid-Al-Fitr-to-be-celebrated-tomorrow (press, 2019-06-03). Quote: "tomorrow, Tuesday, June 4, 2019, is the first day of the blessed Eid Al Fitr in Qatar." Moon Sighting Committee. Derived: no. Note: the Qatar Astronomical Society had expected June 5. the committee decided June 4.

#### Ramadan 1441
- 2020-04-24 (Friday). https://s.thepeninsula.qa/article/23/04/2020/Friday-will-be-first-day-of-Ramadan-in-Qatar (press, 2020-04-23). Quote: "tomorrow, Friday, April 24, 2020 will be the first day of the holy month of Ramadan." Moon Sighting Committee. Derived: no.

#### Shawwal 1441
- 2020-05-24 (Sunday). https://thepeninsulaqatar.com/article/22/05/2020/First-day-of-Eid-Al-Fitr-on-Sunday-in-Qatar (press, 2020-05-22). Quote: "Sunday, May 24, 2020, is the first day of the blessed Eid Al Fitr in Qatar." Derived: no.

#### Ramadan 1442
- 2021-04-13 (Tuesday). https://thepeninsulaqatar.com/article/12/04/2021/Crescent-sighted,-Ramadan-2021-starts-on-Tuesday-in-Qatar-Awqaf (press, 2021-04-12). Quote: "tomorrow, Tuesday, April 13, 2021 will be the first day of the holy month of Ramadan." Derived: no. Note: on April 11 Awqaf posted then withdrew a tweet naming Tuesday (search excerpts of Peninsula and Qatar Tribune). the official announcement came April 12.

#### Shawwal 1442
- 2021-05-13 (Thursday). https://thepeninsulaqatar.com/article/11/05/2021/First-day-of-Eid-Al-Fitr-on-Thursday-in-Qatar (press, 2021-05-11). Quote: "Thursday, May 13, 2021, is the first day of the blessed Eid Al Fitr in Qatar." Derived: no.

#### Ramadan 1443
- 2022-04-02 (Saturday). https://thepeninsulaqatar.com/article/01/04/2022/ramadan-2022-starts-on-saturday-in-qatar-awqaf (press, 2022-04-01). Quote: "tomorrow, Saturday, April 2, 2022 will be the first day of the holy month of Ramadan". Derived: no.

#### Shawwal 1443
- 2022-05-02 (Monday). https://thepeninsulaqatar.com/article/30/04/2022/first-day-of-eid-al-fitr-2022-on-monday-in-qatar (press, 2022-04-30). Quote: "Monday, May 2, 2022, is the first day of the blessed Eid Al Fitr in Qatar." Derived: no.

#### Ramadan 1444
- 2023-03-23 (Thursday). https://www.qna.org.qa/en/News-Area/News/2023-03/21/0066-crescent-sighting-committee-announces-thursday-as-first-day-of-ramadan (agency, QNA dateline Doha, March 21 2023). Quote: "after tomorrow Thursday, March 23, will be the first day of the holy month of Ramadan." Crescent Sighting Committee chaired by Sheikh Dr. Thuqail Al Shammari. Derived: no.

#### Shawwal 1444
- 2023-04-21 (Friday). https://thepeninsulaqatar.com/article/20/04/2023/first-day-of-eid-al-fitr-2023-in-qatar-on-friday (press, 2023-04-20). Quote: "Friday, April 21, 2023, is the first day of the blessed Eid Al Fitr in Qatar." Derived: no.

#### Ramadan 1445
- 2024-03-11 (Monday). https://s.thepeninsula.qa/article/10/03/2024/ramadan-in-qatar-begins-tomorrow-march-11-2024 (press, 2024-03-10). Quote: "Tomorrow, March 11, 2024 will be the first day of the holy month of Ramadan in Qatar." Derived: no.

#### Shawwal 1445
- 2024-04-10 (Wednesday). https://thepeninsulaqatar.com/article/08/04/2024/eid-al-fitr-2024-qatar-officially-confirms-first-day-of-eid (press, 2024-04-08). Quote: "It added that Wednesday, April 10, 2024, is the first day of the blessed Eid Al Fitr in Qatar." A QNA copy exists (https://www.qna.org.qa/en/News%20Area/News/2024-04/08/0028-crescent-sighting-committee-announces-wednesday-first-day-of-eid-al-fitr) but returned HTTP 404 to the fetcher. Derived: no.

#### Ramadan 1446
- 2025-03-01 (Saturday). https://s.thepeninsula.qa/article/28/02/2025/ramadan-in-qatar-begins-tomorrow-march-1-2025 (press, 2025-02-28). Quote: "Tomorrow, March 1, 2025 will be the first day of the holy month of Ramadan in Qatar." Derived: no.

#### Shawwal 1446
- 2025-03-30 (Sunday). https://thepeninsulaqatar.com/article/29/03/2025/eid-al-fitr-2025-qatar-confirms-march-30-as-first-day-of-eid (press, 2025-03-29). Quote: "Sunday, March 30, 2025, is the first day of the blessed Eid Al Fitr in Qatar." Announced on Qatar Television by the Crescent Sighting Committee. Derived: no.

#### Dhu al-Hijjah 1446
- 2025-05-28 (Wednesday). https://gulfnews.com/uae/eid-al-adha-2025-dhu-al-hijjah-crescent-moon-sighting-across-the-world-1.500141613 (press, 2025). Quote: "Qatar has declared that Wednesday, May 28, marks the first day of Dhul Hijjah 1446 AH. Consequently, Eid Al Adha will be observed on Friday, June 6." The page attributes it to "the State of Qatar" and names no body. It could not be tied to the Awqaf committee or QNA, and Qatar Calendar House had forecast May 28 beforehand, so treat as weak. Derived: no.

#### Ramadan 1447
- 2026-02-18 (Wednesday). https://qna.org.qa/en/News-Area/News/2026-2/17/qatar-announces-start-of-ramadan-on-wednesday (agency, QNA dateline Doha, February 17 2026). Quote: "Wednesday (February 18) will mark the first day of the holy month of Ramadan". Ministry of Endowments and Islamic Affairs via its Moon Sighting Committee. Derived: no.

#### Shawwal 1447
- 2026-03-20 (Friday). https://qna.org.qa/en/News-Area/News/2026-3/18/crescent-moon-sighting-committee-eid-al-fitr-starts-on-friday (agency, 2026-03-18). Quote: "the first day of the blessed Eid Al Fitr in the State of Qatar will be Friday, March 20, 2026." Derived: no.

#### Notes on missing Dhu al-Hijjah starts
- 1436 (2015): The Peninsula (https://thepeninsulaqatar.com/article/14/09/2015/Eid-Al-Adha-to-Start-on-September-24, 2015-09-14) says "The Kingdom of Saudi Arabia announced that Eid Al-Adha will start on Thursday, September 24". This is a Saudi announcement, not Qatar's, so excluded.
- 1437 (2016): Qatar Living (https://www.qatarliving.com/en/article/eid-al-adha-starts-september-12-after-moon-not-sighted) attributes the Sept 12 date to Saudi Arabia and says only "Qatar usually has Eid on the same day."
- 1438, 1439, 1443, 1444, 1445, 1447: results show Qatar Calendar House astronomical forecasts (for example https://www.gulf-times.com/article/663213/qatar/first-day-of-eid-al-adha-on-june-28-qch , https://www.khaleejtimes.com/world/gulf/qatar-eid-al-adha-2026-dates which says "as per astronomical calculations"), Saudi announcements, Amiri Diwan holidays and Awqaf prayer lists. None quotes a Qatar committee decision on the Dhu al-Hijjah crescent. 1440, 1441, 1442: nothing Qatar-specific found.

| country | hijri_year | hijri_month | gregorian | derived | source |
|---|---|---|---|---|---|
| Qatar | 1436 | 9 | 2015-06-18 | no | https://www.qatarliving.com/en/article/ramadan-qatar-starts-thursday |
| Qatar | 1436 | 10 | 2015-07-17 | no | https://thepeninsulaqatar.com/article/16/07/2015/eid-al-fitr-begins-friday-in-qatar |
| Qatar | 1437 | 9 | 2016-06-06 | no | https://www.gulf-times.com/story/496956/monday-is-first-day-of-ramadan-in-qatar/amp |
| Qatar | 1437 | 10 | 2016-07-06 | no | https://thepeninsulaqatar.com/article/05/07/2016/Eid-Al-Fitr-in-Qatar-on-Wednesday |
| Qatar | 1438 | 9 | 2017-05-27 | no | https://thepeninsulaqatar.com/article/27/05/2017/Ramadan-begins-today-in-Qatar |
| Qatar | 1438 | 10 | 2017-06-25 | no | https://thepeninsulaqatar.com/article/24/06/2017/Eid-Al-Fitr-to-be-celebrated-tomorrow-(Sunday) |
| Qatar | 1439 | 9 | 2018-05-17 | no | https://thepeninsulaqatar.com/article/15/05/2018/Thursday-will-be-first-day-of-Ramadan-in-Qatar |
| Qatar | 1439 | 10 | 2018-06-15 | no | https://www.gulf-times.com/story/596282/friday-is-first-day-of-eid-al-fitr-in-qatar |
| Qatar | 1440 | 9 | 2019-05-06 | no | https://thepeninsulaqatar.com/article/04/05/2019/Monday-will-be-first-day-of-Ramadan-in-Qatar |
| Qatar | 1440 | 10 | 2019-06-04 | no | https://thepeninsulaqatar.com/article/03/06/2019/Eid-Al-Fitr-to-be-celebrated-tomorrow |
| Qatar | 1441 | 9 | 2020-04-24 | no | https://s.thepeninsula.qa/article/23/04/2020/Friday-will-be-first-day-of-Ramadan-in-Qatar |
| Qatar | 1441 | 10 | 2020-05-24 | no | https://thepeninsulaqatar.com/article/22/05/2020/First-day-of-Eid-Al-Fitr-on-Sunday-in-Qatar |
| Qatar | 1442 | 9 | 2021-04-13 | no | https://thepeninsulaqatar.com/article/12/04/2021/Crescent-sighted,-Ramadan-2021-starts-on-Tuesday-in-Qatar-Awqaf |
| Qatar | 1442 | 10 | 2021-05-13 | no | https://thepeninsulaqatar.com/article/11/05/2021/First-day-of-Eid-Al-Fitr-on-Thursday-in-Qatar |
| Qatar | 1443 | 9 | 2022-04-02 | no | https://thepeninsulaqatar.com/article/01/04/2022/ramadan-2022-starts-on-saturday-in-qatar-awqaf |
| Qatar | 1443 | 10 | 2022-05-02 | no | https://thepeninsulaqatar.com/article/30/04/2022/first-day-of-eid-al-fitr-2022-on-monday-in-qatar |
| Qatar | 1444 | 9 | 2023-03-23 | no | https://www.qna.org.qa/en/News-Area/News/2023-03/21/0066-crescent-sighting-committee-announces-thursday-as-first-day-of-ramadan |
| Qatar | 1444 | 10 | 2023-04-21 | no | https://thepeninsulaqatar.com/article/20/04/2023/first-day-of-eid-al-fitr-2023-in-qatar-on-friday |
| Qatar | 1445 | 9 | 2024-03-11 | no | https://s.thepeninsula.qa/article/10/03/2024/ramadan-in-qatar-begins-tomorrow-march-11-2024 |
| Qatar | 1445 | 10 | 2024-04-10 | no | https://thepeninsulaqatar.com/article/08/04/2024/eid-al-fitr-2024-qatar-officially-confirms-first-day-of-eid |
| Qatar | 1446 | 9 | 2025-03-01 | no | https://s.thepeninsula.qa/article/28/02/2025/ramadan-in-qatar-begins-tomorrow-march-1-2025 |
| Qatar | 1446 | 10 | 2025-03-30 | no | https://thepeninsulaqatar.com/article/29/03/2025/eid-al-fitr-2025-qatar-confirms-march-30-as-first-day-of-eid |
| Qatar | 1446 | 12 | 2025-05-28 | no | https://gulfnews.com/uae/eid-al-adha-2025-dhu-al-hijjah-crescent-moon-sighting-across-the-world-1.500141613 |
| Qatar | 1447 | 9 | 2026-02-18 | no | https://qna.org.qa/en/News-Area/News/2026-2/17/qatar-announces-start-of-ramadan-on-wednesday |
| Qatar | 1447 | 10 | 2026-03-20 | no | https://qna.org.qa/en/News-Area/News/2026-3/18/crescent-moon-sighting-committee-eid-al-fitr-starts-on-friday |

### Kuwait announced Hijri month starts, 1436-1447

Official body: Shariah Moon Sighting Committee (هيئة الرؤية الشرعية), Ministry of Justice / Supreme Judicial Council. Agency: KUNA.

Access notes. kuna.net.kw does not resolve from this environment (DNS ENOTFOUND) and web.archive.org is blocked, so no KUNA page was fetched. Where a row cites KUNA it is the KUNA headline and dateline as returned in a search result, and that is stated per row ("KUNA headline via search"). alanba.com.kw, gulfbusiness.com and several others returned 403. No official-body site (justice.gov.kw) was found with these announcements. Every Gregorian date below was checked against the weekday named in the source, and no mismatch was found.

Dhu al-Hijjah (month 12): no Kuwaiti sighting-committee announcement was found for any year 1436-1447. Kuwaiti press only carries astronomer forecasts (Kuwait Science Club, Al-Ujairi Center) and the Saudi decision, which do not count. All 12 are missing.

#### Ramadan 1438 (month 9): 2017-05-27

- Source: https://www.ajnet.me/news/2017/5/26/دول-عربية-وإسلامية-تعلن-السبت-أول-أيام (Al Jazeera Arabic, press), 26 May 2017
- Quote: "وفي الكويت أكدت هيئة الرؤية الشرعية ثبوث رؤية هلال رمضان" and, for the group of countries, "أن يوم السبت هو أول أيام شهر رمضان المبارك"
- Derived: no. Weak: the Saturday sentence is general, not Kuwait-specific. Al Jazeera English 27 May 2017 lists Kuwait under "Confirmed May 27". Kuwait's own announcement not read.

#### Ramadan 1439: 2018-05-17

- Source: https://www.albayan.ae/one-world/arabs/2018-05-15-1.3265808 (Al Bayan, press), image path suggests 15 May 2018
- Quote: "وأعلنت هيئة الرؤية الشرعية الكويتية ان بعد غد الخميس أول أيام شهر رمضان المبارك."
- Derived: no. Also Gulf News (last updated 7 Nov 2018): "Kuwait, Egypt, Jordan and Lebanon have also all officially declared that May 17 will be the first day of Ramadan".

#### Ramadan 1440: 2019-05-06

- Source: https://timeskuwait.com/news/moonsighting-committee-ramadan-to-start-on-monday-in-kuwait (press), 5 May 2019
- Quote: "the holy fasting month of Ramadan will start on Monday"
- Derived: no. Also https://ajel.sa/international/zlqskv (5 May 2019) lists Kuwait among countries announcing Monday.

#### Shawwal 1440: 2019-06-04

- Source: KUNA headline via search, 03/06/2019, https://www.kuna.net.kw/ArticleDetails.aspx?id=2799533 (agency, page not fetched)
- Quote: "(الرؤية الشرعية): ثبوت رؤية هلال شهر شوال وغدا الثلاثاء عيد الفطر"
- Derived: no. Corroboration read: Arab News 3 Jun 2019, "The UAE, Kuwait, and other Gulf countries will all celebrate Eid on Tuesday".

#### Ramadan 1441: 2020-04-24

- Source: https://www.arabtimesonline.com/news/kuwait-announces-tomorrow-friday-the-blessed-month-of-ramadan-for-this-year/ (press), 23/04/2020
- Quote: "Kuwait's Moonsighting Committee has announced that tomorrow (Friday) will be the start of the holy fasting month of Ramadan."
- Derived: no.

#### Shawwal 1441: 2020-05-24

- Source: KUNA headline via search, 22/05/2020, https://www.kuna.net.kw/ArticleDetails.aspx?id=2896099&Language=ar (agency, page not fetched)
- Quote: "(الرؤية الشرعية) الكويتية: غدا السبت المتمم لشهر رمضان المبارك والأحد أول أيام عيد الفطر"
- Derived: no.

#### Ramadan 1442: 2021-04-13

- Source: https://gulfnews.com/world/gulf/tuesday-is-first-day-of-ramadan-in-egypt-kuwait-lebanon-1.1618164034508 (press), last updated 11 Apr 2021
- Quote: "Egypt, Lebanon and Kuwait have announced that Tuesday, April 13, will be the first day of Ramadan"
- Derived: no.

#### Shawwal 1442: 2021-05-13

- Source: KUNA headline via search, 11/05/2021, https://www.kuna.net.kw/ArticleDetails.aspx?id=2977107 (agency, page not fetched)
- Quote: "(الرؤية الشرعية) الكويتية: غدا الأربعاء المتمم لشهر رمضان المبارك.. والخميس أول أيام عيد الفطر"
- Derived: no.

#### Ramadan 1443: 2022-04-02

- Source: https://www.arabtimesonline.com/news/tomorrow-saturday-is-the-first-day-of-ramadan-in-kuwait/ (press), 01/04/2022
- Quote: "After seeing the crescent moon this night, Saturday will be the first day of Ramadan of 1443, he said."
- Derived: no. KUNA headline via search, 01/04/2022 (id=3033058): "هيئة الرؤية الشرعية الكويتية: غدا السبت أول أيام شهر رمضان المبارك".

#### Shawwal 1443: 2022-05-02

- Source: https://www.arabtimesonline.com/news/eid-on-monday-amir-extends-receives-greetings (press), 30 April 2022
- Quote: "The Kuwaiti Crescent Sighting Authority declared on Saturday that tomorrow (Sunday) would be the last day..." and Eid Al-Fitr falls on Monday.
- Derived: no. Gulf News (30 Apr 2022): "Monday May 2 will be the first day of Shawwal and Eid Al Fitr in Kuwai."

#### Ramadan 1444: 2023-03-23

- Source: https://gulfnews.com/amp/gulfnews/uae/ramadan/saudi-arabia-kuwait-qatar-and-bahrain-announce-first-day-of-ramadan-1.1679399189255 (press), updated 22 Mar 2023
- Quote: "Kuwait also announced that Thursday, March 23, will be the first day of Ramadan."
- Derived: no. KUNA headline via search 21/03/2023 (id=3085430): "(الرؤية الشرعية): تعذر رؤية الهلال في الكويت.. والخميس أول أيام شهر رمضان المبارك". Not seen means Sha'ban completed 30 days.

#### Shawwal 1444: 2023-04-21

- Source: https://arabtimesonline.com/news/eid-al-fitr-to-be-celebrated-tomorrow-in-kuwait-saudi-arabia-qatar-and-the-uae (press), 20/04/2023
- Quote: "The Sharia Vision Authority of Kuwait announced that Eid Al-Fitr for the year 1444 AH will be tomorrow, Friday."
- Derived: no.

#### Ramadan 1445: 2024-03-11

- Source: https://www.arabtimesonline.com/news/monday-first-day-of-the-holy-month-of-ramadan-in-kuwait/ (press), 10/03/2024
- Quote: "Kuwait Sharia Vision Board declared tomorrow, Monday, is the first day of the holy month of Ramadan for the year 1445 AH."
- Derived: no. KUNA headline via search 10/03/2024 (id=3141503).

#### Shawwal 1445: 2024-04-10

- Source: KUNA headline via search, 08/04/2024, https://www.kuna.net.kw/ArticleDetails.aspx?id=3144662&Language=ar (agency, page not fetched)
- Quote: "(الرؤية الشرعية) الكويتية: غدا الثلاثاء المتمم لشهر رمضان المبارك.. والأربعاء أول أيام عيد الفطر"
- Derived: no. Headline dated 8 April, Tuesday 9 April is the completion day, so Wednesday is 10 April. The dateline of the headline is the only date read, the "8 April" appears to be the evening of 29 Ramadan.

#### Ramadan 1446: 2025-03-01

- Source: https://www.alraimedia.com/article/1719388/ (Al-Rai, press), 28 فبراير 2025
- Quote: "وبذلك يكون يوم غد السبت أول أيام الرمضان الفضيل."
- Derived: no. KUNA headline via search 28/02/2025 (id=3221121): "هيئة الرؤية الشرعية الكويتية.. ثبوت رؤية الهلال وغدا السبت أول أيام شهر رمضان المبارك".

#### Shawwal 1446: 2025-03-30

- Source: https://www.alraimedia.com/article/1722295/ (Al-Rai, press), 29 مارس 2025
- Quote: "وأن غدا الأحد هو أول أيام عيد الفطر السعيد"
- Derived: no. KUNA headline via search 29/03/2025 (id=3224285): "ثبوت رؤية هلال شوال وغدا الأحد أول أيام عيد الفطر السعيد".

#### Ramadan 1447: 2026-02-18

- Source: https://www.arabtimesonline.com/news/ramadan-kareem-kuwait-announces-start-of-holy-month/ (press), 17/02/2026
- Quote: "Accordingly, Ramadan 1447 AH will commence tonight, with Muslims across the country set to begin fasting at dawn tomorrow."
- Derived: no. The date 18 Feb is inferred from the 17 Feb publication date. Corroboration by search title: Al-Rai 1757010 "«الرؤية الشرعية»: الأربعاء.. غرة رمضان" (18 Feb 2026 is a Wednesday).

#### Shawwal 1447: 2026-03-20

- Source: https://www.aljarida.com/article/126411 (Al-Jarida, press), 18-03-2026 18:48
- Quote: "أعلنت هيئة الرؤية الشرعية في دولة الكويت غداً المتمم وعيد الفطر الجمعة."
- Derived: no. Al-Rai (18 Mar 2026): "وبعد غد الجمعة أول أيام عيد الفطر المبارك".

#### Candidates not in the table (only a search-result snippet, no fetched page)

- Ramadan 1436: 2015-06-18. Search snippet quoting KUNA via pilipinosakuwait.com (HTTP 403 on fetch): "Kuwait Tuesday announced that Thursday would be the first day of Muslims' Holy Month of Ramadan."
- Ramadan 1437: 2016-06-06. Only an astronomer forecast was read (Arab Times, 23/05/2016). Official announcement not read.
- Shawwal 1436: 2015-07-17. Gulf News snippet: Kuwait, Saudi, Qatar, Bahrain celebrate Eid Friday. Not fetched.
- Shawwal 1438: 2017-06-25. Search snippet of an Arabic report that Kuwait's sighting committee confirmed the Shawwal crescent. Arab Times 24 June 2017 snippet also says Sunday. Not fetched (eremnews 403).
- Shawwal 1439: 2018-06-15. Search snippet citing KUNA 14 June 2018. The fetched Arab Times page of 07/06/2018 is only a Scientific Club forecast, which does not count.

#### Missing

Dhu al-Hijjah 1436-1447 (all 12), Ramadan 1436, Ramadan 1437, Shawwal 1436, Shawwal 1438, Shawwal 1439 (candidates above).

Shawwal 1437 note: Kuwait Justice Ministry committee announcement was read at https://www.arabtimesonline.com/news/eid-wednesday-diwan-conveys-amiri-greeting/ (05/07/2016, quote "Wednesday will be the first day of Eid Al-Fitr."), included in the table below.

| country | hijri_year | hijri_month | gregorian | derived | source |
|---|---|---|---|---|---|
| Kuwait | 1437 | 10 | 2016-07-06 | no | https://www.arabtimesonline.com/news/eid-wednesday-diwan-conveys-amiri-greeting/ |
| Kuwait | 1438 | 9 | 2017-05-27 | no | https://www.ajnet.me/news/2017/5/26/دول-عربية-وإسلامية-تعلن-السبت-أول-أيام |
| Kuwait | 1439 | 9 | 2018-05-17 | no | https://www.albayan.ae/one-world/arabs/2018-05-15-1.3265808 |
| Kuwait | 1440 | 9 | 2019-05-06 | no | https://timeskuwait.com/news/moonsighting-committee-ramadan-to-start-on-monday-in-kuwait |
| Kuwait | 1440 | 10 | 2019-06-04 | no | https://www.kuna.net.kw/ArticleDetails.aspx?id=2799533 |
| Kuwait | 1441 | 9 | 2020-04-24 | no | https://www.arabtimesonline.com/news/kuwait-announces-tomorrow-friday-the-blessed-month-of-ramadan-for-this-year/ |
| Kuwait | 1441 | 10 | 2020-05-24 | no | https://www.kuna.net.kw/ArticleDetails.aspx?id=2896099&Language=ar |
| Kuwait | 1442 | 9 | 2021-04-13 | no | https://gulfnews.com/world/gulf/tuesday-is-first-day-of-ramadan-in-egypt-kuwait-lebanon-1.1618164034508 |
| Kuwait | 1442 | 10 | 2021-05-13 | no | https://www.kuna.net.kw/ArticleDetails.aspx?id=2977107 |
| Kuwait | 1443 | 9 | 2022-04-02 | no | https://www.arabtimesonline.com/news/tomorrow-saturday-is-the-first-day-of-ramadan-in-kuwait/ |
| Kuwait | 1443 | 10 | 2022-05-02 | no | https://www.arabtimesonline.com/news/eid-on-monday-amir-extends-receives-greetings |
| Kuwait | 1444 | 9 | 2023-03-23 | no | https://gulfnews.com/amp/gulfnews/uae/ramadan/saudi-arabia-kuwait-qatar-and-bahrain-announce-first-day-of-ramadan-1.1679399189255 |
| Kuwait | 1444 | 10 | 2023-04-21 | no | https://arabtimesonline.com/news/eid-al-fitr-to-be-celebrated-tomorrow-in-kuwait-saudi-arabia-qatar-and-the-uae |
| Kuwait | 1445 | 9 | 2024-03-11 | no | https://www.arabtimesonline.com/news/monday-first-day-of-the-holy-month-of-ramadan-in-kuwait/ |
| Kuwait | 1445 | 10 | 2024-04-10 | no | https://www.kuna.net.kw/ArticleDetails.aspx?id=3144662&Language=ar |
| Kuwait | 1446 | 9 | 2025-03-01 | no | https://www.alraimedia.com/article/1719388/ |
| Kuwait | 1446 | 10 | 2025-03-30 | no | https://www.alraimedia.com/article/1722295/ |
| Kuwait | 1447 | 9 | 2026-02-18 | no | https://www.arabtimesonline.com/news/ramadan-kareem-kuwait-announces-start-of-holy-month/ |
| Kuwait | 1447 | 10 | 2026-03-20 | no | https://www.aljarida.com/article/126411 |

### Bahrain announced Hijri month starts, 1436-1447

Official body: Supreme Council for Islamic Affairs (SCIA) moon-sighting committee, Ministry of Justice, Islamic Affairs and Waqf. Agency: BNA.

Access notes. bna.bh was not reached by any search or fetch. Bahraini Arabic papers (Al-Bilad, Al-Ayam, Al-Wasat) returned HTTP 403 on every fetch, so Al-Ayam headlines appear only as search-result titles. Gulf Daily News and tradearabia old pages mostly returned "Article Not Available". Only Daily Tribune (newsofbahrain.com) and tradearabia 2026 pages loaded. Where a row rests on a search-result headline or on a regional roundup, it says so. Weekdays named in sources were checked against the dates, no mismatch found.

Dhu al-Hijjah (month 12): no Bahraini announcement was read for any year. Only found: a Daily Tribune item of 21 July 2020 that relays the Saudi Supreme Court ("tomorrow will be the first day of Dhul Hijja", Eid Friday 31 July, so 1441 Dhu al-Hijjah would be 2020-07-22), which is Saudi's announcement, not Bahrain's, so excluded. A Lovin Bahrain page (unreachable) and a local astronomer give 2026-05-18 for 1447, not an official announcement. All 12 are missing.

Behaviour note. Bahrain's committee has often adopted Saudi's sighting when no local testimony arrives. Sunni Endowments and Shia scholars have sometimes diverged historically (older Al-Wasat results from 2009 and 2010), not examined for 2015-2026.

#### Ramadan 1438: 2017-05-27

- Source: https://www.ajnet.me/news/2017/5/26/دول-عربية-وإسلامية-تعلن-السبت-أول-أيام (Al Jazeera Arabic, press), 26 May 2017
- Quote: "وهيئة الرؤية الشرعية في البحرين" (listed with Kuwait's committee as confirming the Ramadan crescent) and "أن يوم السبت هو أول أيام شهر رمضان المبارك"
- Derived: no. Weak: the Saturday sentence is general, not Bahrain-specific.

#### Ramadan 1439: 2018-05-17

- Source: https://www.albayan.ae/one-world/arabs/2018-05-15-1.3265808 (Al Bayan, press), approx. 15 May 2018
- Quote: "والأربعاء متمم لشهر شعبان، وبعد غد الخميس أول أيام الشهر الكريم."
- Derived: no. Wednesday completes Sha'ban, so Thursday 17 May.

#### Ramadan 1440: 2019-05-06

- Source: https://ajel.sa/international/zlqskv (press), 5 May 2019 23:12
- Quote: "أعلنت كلٌ من مصر والإمارات والبحرين، والأردن والكويت والعراق" ... "يوم غدٍ الإثنين- هو غُرة شهر رمضان المبارك"
- Derived: no.

#### Ramadan 1443: 2022-04-02

- Source: https://www.gdnonline.com/Details/1038345/First-day-of-Ramadan-on-Saturday (Gulf Daily News, press), 1 Apr 2022
- Quote: "Ramadan will start on Saturday in Bahrain, the moonsighting committee has officially announced."
- Derived: no.

#### Ramadan 1444: 2023-03-23

- Source: https://gulfnews.com/amp/gulfnews/uae/ramadan/saudi-arabia-kuwait-qatar-and-bahrain-announce-first-day-of-ramadan-1.1679399189255 (press), updated 22 Mar 2023
- Quote: "The moonsighting committee in Bahrain has announced that Thursday, March 23, will be the first day of the month of Ramadan" (the page names the Bahrain News Agency as source)
- Derived: no.

#### Ramadan 1445: 2024-03-11

- Source: https://khaleejtimes.com/world/gulf/ramadan-2024-bahrain-announces-start-of-holy-month-on-march-11 (press), 10 Mar 2024 21:31
- Quote: "Bahrain observe the first day of Ramadan on March 11, 2024." (committee met at SCIA headquarters)
- Derived: no.

#### Ramadan 1446: 2025-03-01

- Source: https://www.newsofbahrain.com/bahrain/109320.html (Daily Tribune, press), 28 Feb 2025
- Quote: "In accordance with this, Bahrain has officially declared the following day, March 1, as the first day of fasting." Tradearabia 109351-era page: "the committee has decided that March 1, will be the first day of Ramadhan for 2025."
- Derived: no. Announced by SCIA. Follows Saudi: the search summary reports the committee relied on the Saudi sighting, the exact sentence was not read on a fetched page.

#### Ramadan 1447: 2026-02-18

- Source: https://tradearabia.com/News/389399/Bahrain,-Saudi-Arabia,-UAE-declare-Wednesday-as-first-day-of-Ramadan- (press), Tue 17 Feb 2026
- Quote: "Accordingly, it declared Wednesday, February 18, as the first day of Ramadhan 1447 AH."
- Derived: no. "Accordingly" refers to a sighting confirmed in Saudi Arabia (search summary), so this follows Saudi's sighting.

#### Shawwal 1437: 2016-07-06

- Source: https://www.aljazeera.com/news/2016/7/4/eid-al-fitr-saudi-arabia-declares-wednesday-first-day (press), 4 Jul 2016, and Peninsula 5 Jul 2016
- Quote: "The official news agencies of Saudi Arabia, Qatar, the United Arab Emirates, Bahrain and Kuwait quoted religious authorities as saying on Monday that the annual feast will begin on Wednesday..." Peninsula: "The Muslim feast of Eid al-Fitr marking the end of the holy month of Ramadan will start on Wednesday"
- Derived: no. Press roundup of the Bahrain agency, not the body's own text.

#### Shawwal 1443: 2022-05-02

- Source: https://gulfnews.com/uae/ramadan/eid-al-fitr-2022-first-day-of-shawwal-moon-sighting-across-the-world-1.1651325049438 (press), 30 Apr 2022
- Quote: "For Bahrain, Lebanon and Palestine, Sunday, will be the last day of Ramadan", with Monday the first day of Shawwal and Eid Al Fitr.
- Derived: no.

#### Shawwal 1445: 2024-04-10

- Source: https://www.alayam.com/online/local/1051373/News.html (Al-Ayam, press), search-result headline only, page 403
- Quote: "البحرين تعلن الثلاثاء المتمم لشهر رمضان والأربعاء أول أيام عيد الفطر المبارك"
- Derived: no. Headline date not shown, year set by the 10 April 2024 weekday (Wednesday). The search summary says the committee received no sighting and completed 30 days.

#### Shawwal 1446: 2025-03-30

- Source: https://www.newsofbahrain.com/en/article/bahrainaTMs-moon-sighting-committee-confirms-eid-al-fitr-on-sunday-march-wk_post-110718 (Daily Tribune, press), 29 Mar 2025
- Quote: "As a result, Sunday, March 30, 2025, will be the first day of Eid Al-Fitr."
- Derived: no. Follows Saudi: Al-Ayam search summary says no local witness came and the committee adopted the Saudi sighting. Exact sentence not read.

#### Shawwal 1447: 2026-03-20

- Source: https://tradearabia.com/News/390427/Saudi-GCC-announce-Friday-as-first-day-of-Eid/MEDIA (press), Wed 18 Mar 2026
- Quote: "that Friday (March 20) will be the first day of Eid Al Fitr."
- Derived: no. Announced by Bahrain's Moon Sighting Committee.

#### Candidates not in the table (snippet only, no fetched page naming Bahrain)

- Shawwal 1438: 2017-06-25. Arabic search snippet: Bahrain and Kuwait sighting bodies confirmed the Shawwal crescent, Sunday first day of Eid. Source eremnews 887446 returned 403.
- Shawwal 1439: 2018-06-15. Search snippets only (newsofbahrain, tradearabia).
- Shawwal 1440: 2019-06-04. Arab News 3 Jun 2019 fetched but names only "the UAE, Kuwait, and other Gulf countries".
- Dhu al-Hijjah 1441: 2020-07-22, Saudi relay only (see above). Dhu al-Hijjah 1447: 2026-05-18, secondary only.

#### Missing

Ramadan 1436, 1437, 1441, 1442. Shawwal 1436, 1438, 1439, 1440, 1441, 1442, 1444. Dhu al-Hijjah 1436-1447 (all 12).

| country | hijri_year | hijri_month | gregorian | derived | source |
|---|---|---|---|---|---|
| Bahrain | 1437 | 10 | 2016-07-06 | no | https://www.aljazeera.com/news/2016/7/4/eid-al-fitr-saudi-arabia-declares-wednesday-first-day |
| Bahrain | 1438 | 9 | 2017-05-27 | no | https://www.ajnet.me/news/2017/5/26/دول-عربية-وإسلامية-تعلن-السبت-أول-أيام |
| Bahrain | 1439 | 9 | 2018-05-17 | no | https://www.albayan.ae/one-world/arabs/2018-05-15-1.3265808 |
| Bahrain | 1440 | 9 | 2019-05-06 | no | https://ajel.sa/international/zlqskv |
| Bahrain | 1443 | 9 | 2022-04-02 | no | https://www.gdnonline.com/Details/1038345/First-day-of-Ramadan-on-Saturday |
| Bahrain | 1443 | 10 | 2022-05-02 | no | https://gulfnews.com/uae/ramadan/eid-al-fitr-2022-first-day-of-shawwal-moon-sighting-across-the-world-1.1651325049438 |
| Bahrain | 1444 | 9 | 2023-03-23 | no | https://gulfnews.com/amp/gulfnews/uae/ramadan/saudi-arabia-kuwait-qatar-and-bahrain-announce-first-day-of-ramadan-1.1679399189255 |
| Bahrain | 1445 | 9 | 2024-03-11 | no | https://khaleejtimes.com/world/gulf/ramadan-2024-bahrain-announces-start-of-holy-month-on-march-11 |
| Bahrain | 1445 | 10 | 2024-04-10 | no | https://www.alayam.com/online/local/1051373/News.html |
| Bahrain | 1446 | 9 | 2025-03-01 | no | https://www.newsofbahrain.com/bahrain/109320.html |
| Bahrain | 1446 | 10 | 2025-03-30 | no | https://www.newsofbahrain.com/en/article/bahrainaTMs-moon-sighting-committee-confirms-eid-al-fitr-on-sunday-march-wk_post-110718 |
| Bahrain | 1447 | 9 | 2026-02-18 | no | https://tradearabia.com/News/389399/Bahrain,-Saudi-Arabia,-UAE-declare-Wednesday-as-first-day-of-Ramadan- |
| Bahrain | 1447 | 10 | 2026-03-20 | no | https://tradearabia.com/News/390427/Saudi-GCC-announce-Friday-as-first-day-of-Eid/MEDIA |

### Oman: announced Hijri month starts, 1436 to 1447

Official body: Main Committee for Sighting the Crescent, Ministry of Endowments and Religious Affairs (MERA, formerly MARA). Agency ONA. No page on the ministry's own site (maraoman.gov.om / mara.gov.om) or omannews.gov.om was reachable by search, so every row is press (Oman Observer, which republishes the ministry/ONA statements, plus Times of Oman, Gulf News and others). Muscat Daily is behind Cloudflare and could not be fetched. Page texts were fetched directly (no Wayback copies used).

Oman sights independently of Saudi Arabia, and several pages note it ran a day behind the Gulf states (see per-row notes. Saudi dates in notes come from the cited articles or search snippets, not from a Saudi source). No page says Oman "follows Saudi" for any row.

Missing: Ramadan 1436 (2015), Shawwal 1437 (2016), Dhu al-Hijjah 1436 (2015), Dhu al-Hijjah 1438 (2017). No fetchable page gave Oman's own date for these.

#### Shawwal 1436 AH

- Gregorian date: 2015-07-18
- Source URL: https://www.emirates247.com/news/emirates/eid-al-fitr-in-oman-today-2015-07-16-1.596859
- Source type: press (Emirates 24|7)
- Publication date: 2015-07-16
- Quote: "While the Sultanate of Oman announced that Saturday would be the first day of Eid Al Fitr. It cited that Friday is the complementary to the month of Ramadan, and therefore, Saturday is the first day of Eid Al Fitr in Oman."
- Derived: no
- Follows Saudi sentence: none
- Notes: Saturday 18 July 2015 matches weekday. Oman one day after Saudi/UAE (17 July). No Omani primary page found.

#### Ramadan 1437 AH

- Gregorian date: 2016-06-07
- Source URL: https://www.deccanchronicle.com/world/middle-east/060616/muslim-holy-month-of-ramadan-starts-today-in-gulf.html
- Source type: press (Deccan Chronicle, PTI/Gulf report)
- Publication date: 2016-06-06
- Quote: "The Moon Sighting Committee in Oman said that the moon was not sighted and therefore the fasting month will begin on Tuesday. "The committee did not receive any notifications about Ramadan moon sighting, therefore Monday will be the 30th of Sha'aban and Tuesday 7th of June will be the first day of Ramadan," it said."
- Derived: no
- Follows Saudi sentence: none
- Notes: Tuesday 7 June 2016 matches. Same text at business-standard.com (PTI, 2016-06-06). Oman differed from Saudi (6 June).

#### Dhu al-Hijjah 1437 AH

- Gregorian date: 2016-09-03
- Source URL: https://suroorasia.blogspot.com/2016/09/eid-al-adha-holiday-announced-in-oman.html
- Source type: press (blog repost of ONA)
- Publication date: 2016-09-05
- Quote: "The Eid al Adha holiday for employees at the ministries, public authorities and related agencies will be from 9 Dhulhijjah 1437, or 11 September 2016, till 13 Dhulhijjah, corresponding to 15 September 2016. ... The news was announced by the Oman News Agency (ONA)"
- Derived: yes
- Follows Saudi sentence: none
- Notes: WEAK. Derived from a government holiday decree (9 Dhu al-Hijjah = 11 Sept 2016, so 1 Dhu al-Hijjah = 3 Sept 2016, Saturday), not a moon-sighting statement. Eid = 12 Sept would match Saudi. Oman's own sighting statement was not found. Treat as low confidence.

#### Ramadan 1438 AH

- Gregorian date: 2017-05-27
- Source URL: https://www.omanobserver.om/article/81252
- Source type: press (Oman Observer)
- Publication date: 2017-05-26
- Quote: "The Main Committee for Moon Sighting, which met on Friday, has announced that the first day of Ramadhan in Oman will be on Saturday, May 27."
- Derived: no
- Follows Saudi sentence: none
- Notes: Saturday 27 May 2017 matches.

#### Shawwal 1438 AH

- Gregorian date: 2017-06-26
- Source URL: https://www.omanobserver.om/article/78856
- Source type: press (Oman Observer)
- Publication date: 2017-06-24
- Quote: "The committee concluded that today (Sunday) will be the 30th day of Ramadhan, 1438 AH and tomorrow (Monday) will be the first day of Shawwal."
- Derived: no
- Follows Saudi sentence: none
- Notes: Published Saturday night/Sunday. Monday 26 June 2017 matches. Committee chaired by the Minister of Endowments and Religious Affairs. Oman one day after Saudi (25 June).

#### Ramadan 1439 AH

- Gregorian date: 2018-05-17
- Source URL: https://www.omanobserver.om/article/56902
- Source type: press (Oman Observer)
- Publication date: 2018-05-06
- Quote: "The Moon Sighting Committee of the Ministry of Endowments and Religious Affairs announced that the holy month of Ramadhan moon was sighted via astronomical calculations, therefore (Thursday), 17 May 2018 will be the first day of the holy month of Ramadhan."
- Derived: no
- Follows Saudi sentence: none
- Notes: Announced 11 days ahead using astronomical calculation. The National (https://thenationalnews.com/uae/heritage/oman-announces-first-day-of-ramadan-1.727777, 2018-05-06) says: "The first day of Ramadan will be Thursday May 17 in Oman, the country's Ministry of Endowments and Religious Affairs has announced." Thursday matches.

#### Shawwal 1439 AH

- Gregorian date: 2018-06-15
- Source URL: https://www.omanobserver.om/article/54540
- Source type: press (Oman Observer)
- Publication date: 2018-06-14
- Quote: "The Ministry of Endowments and Religious Affairs committee has confirmed sighting of the moon. Friday will be first day of Shawwal."
- Derived: no
- Follows Saudi sentence: none
- Notes: Friday 15 June 2018 matches.

#### Dhu al-Hijjah 1439 AH

- Gregorian date: 2018-08-12
- Source URL: https://www.omanobserver.om/article/50532
- Source type: press (Oman Observer)
- Publication date: 2018-08-12
- Quote: "The Ministry of Endowments and Religious Affairs (MARA) has said that August 21, 2018 will be the first day of Eid al-Adha. A statement issued by the ministry said, "The day of Arafah, the ninth of Dhu al-Hijjah 1439 AH will be on August 20, according to the calendar of Mecca.""
- Derived: yes
- Follows Saudi sentence: none
- Notes: Derived: Eid 21 Aug minus 9 days = 12 Aug 2018 (Sunday). Weak: the ministry gave Eid and Arafah, published 12 Aug (the day 1 Dhu al-Hijjah began), so it is a calendar-based announcement, not a sighting report. Tuesday 21 Aug matches.

#### Ramadan 1440 AH

- Gregorian date: 2019-05-07
- Source URL: https://www.omanobserver.om/tuesday-first-day-of-ramadhan-in-oman/
- Source type: press (Oman Observer)
- Publication date: 2019-05-05
- Quote: "The Moon Sighting Committee of the Ministry of Endowments and Religious Affairs announced that the first day of the holy month of Ramadhan in the Sultanate will start on Tuesday, May 7, 2019."
- Derived: no
- Follows Saudi sentence: none
- Notes: Tuesday 7 May 2019 matches. Second Observer page (first-day-of-ramadhan-in-sultanate-on-tuesday/, 2019-05-05) confirms: "Monday will be the complementing day of the month of Shaaban, and Tuesday will be the first day of the blessed month of Ramadhan." An earlier Observer item (2019-04-30, first-day-of-ramadhan-on-monday/) gave a 6 May prediction from ministry astronomical calculations, superseded by the committee decision.

#### Shawwal 1440 AH

- Gregorian date: 2019-06-05
- Source URL: https://www.omanobserver.om/article/30845
- Source type: press (Oman Observer)
- Publication date: 2019-06-04
- Quote: "Muscat: The Ministry of Endowments and Religious Affairs committee confirmed the sighting of the moon of Shawaal. Wednesday will be the first day of Shawwal."
- Derived: no
- Follows Saudi sentence: none
- Notes: Wednesday 5 June 2019 matches.

#### Dhu al-Hijjah 1440 AH

- Gregorian date: 2019-08-03
- Source URL: https://www.omanobserver.om/article/27374
- Source type: press (Oman Observer)
- Publication date: 2019-08-01
- Quote: "The main moon-sighting committee didn't report the sighting of the moon today. Accordingly, the first day of Dhu al Hijjah month will be on Saturday."
- Derived: no
- Follows Saudi sentence: none
- Notes: Saturday 3 August 2019 matches. Another Observer page (ampArticle/27342) gives Eid likely 12 August. Oman one day after Saudi (Eid 11 Aug).

#### Ramadan 1441 AH

- Gregorian date: 2020-04-25
- Source URL: https://www.omanobserver.om/article/13744
- Source type: press (Oman Observer)
- Publication date: 2020-04-23
- Quote: "The main committee for moon sighting of Ramadhan 1441 AH, which met on Thursday at the Ministry of Endowment and Religious Affairs, has announced that the first day of the holy month of Ramadhan will be on Saturday, April 25, in the Sultanate."
- Derived: no
- Follows Saudi sentence: none
- Notes: Saturday 25 April 2020 matches. Oman one day after Saudi (24 April).

#### Shawwal 1441 AH

- Gregorian date: 2020-05-24
- Source URL: https://gulfnews.com/world/gulf/oman/oman-begins-eid-on-sunday-1.71661242
- Source type: press (Gulf News, citing ONA)
- Publication date: 2020-05-24
- Quote: "Oman has announced that Sunday marks the first day of Eid Al Fitr, joining the other Gulf countries in determining Sunday as the start of the holy occasion. ... A committee in charge of sighting the new moon of the month of Shawwal announced Saturday night that the moon was spotted, meaning that the first day of the new month and the Eid falls on Sunday, the Omani news agency ONA reported."
- Derived: no
- Follows Saudi sentence: none
- Notes: Sunday 24 May 2020 matches. Only a regional-press source fetched for this month.

#### Dhu al-Hijjah 1441 AH

- Gregorian date: 2020-07-22
- Source URL: https://www.omanobserver.om/article/11774
- Source type: press (Oman Observer)
- Publication date: 2020-07-22
- Quote: "The Main Committee for Moon Sighting of Dhul Hijjah 1441 AH issued a statement on Tuesday affirming that Dhul Hijja moon has been sighted. Therefore, tomorrow (Wednesday) will be the first day of Dhul Hijja 1441 AH, and Eid Al Adha will be on Friday, 31 July 2020."
- Derived: no
- Follows Saudi sentence: none
- Notes: Wednesday 22 July 2020 matches. Times of Oman (https://timesofoman.com/article/92466-first-day-of-eid-al-adha-announced-in-oman-2, 2020-07-21): "Hence, Wednesday, shall be 1 Dhul Hijja 1441H and Friday, July 31, shall be the first day of Eid Al Adha."

#### Ramadan 1442 AH

- Gregorian date: 2021-04-14
- Source URL: https://www.omanobserver.om/article/773
- Source type: press (Oman Observer)
- Publication date: 2021-04-12
- Quote: "The Main Committee for Moon Sighting of Ramadhan 1442 AH issued a statement on Monday declaring that Ramadhan moon could not be sighted today. Therefore, the first day of Ramadhan 1442 AH will be on Wednesday, April 14, 2021 in the Sultanate."
- Derived: no
- Follows Saudi sentence: none
- Notes: Wednesday 14 April 2021 matches. Times of Oman (ONA) page https://timesofoman.com/article/100198-moon-not-sighted-in-oman-ramadan-from-wednesday agrees. Oman one day after Saudi (13 April).

#### Shawwal 1442 AH

- Gregorian date: 2021-05-13
- Source URL: https://www.omanobserver.om/ampArticle/1100748
- Source type: press (Oman Observer)
- Publication date: 2021-05-12
- Quote: "The Ministry of Endowments and Religious Affairs said that tomorrow, Thursday, May 13, 2021, will be the first day of the blessed month of Shawwal and the first day of Eid al-Fitr."
- Derived: no
- Follows Saudi sentence: none
- Notes: Thursday 13 May 2021 matches.

#### Dhu al-Hijjah 1442 AH

- Gregorian date: 2021-07-11
- Source URL: https://www.omanobserver.om/ampArticle/1103676
- Source type: press (Oman Observer)
- Publication date: 2021-07-10
- Quote: "The committee said it received evidence of sighting the crescent, therefore it decided after reviewing the evidence to approve the sighting of the crescent of month Dhul Hijjah 1442 AH. 'Hence, Sunday (July 11,) will mark ..."
- Derived: no
- Follows Saudi sentence: none
- Notes: Sunday 11 July 2021 matches. Headline: "First day of Dhul Hijjah tomorrow, Eid on July 20". The quote is cut at the point my extraction stopped. headline and body state 11 July.

#### Ramadan 1443 AH

- Gregorian date: 2022-04-03
- Source URL: https://www.omanobserver.om/article/1117058
- Source type: press (Oman Observer)
- Publication date: 2022-04-01
- Quote: "The first day of Ramadhan has been announced in the Sultanate of Oman by the moon sighting committee on Friday. The first day of Ramadhan 1443AH will be Sunday, April 3, 2022."
- Derived: no
- Follows Saudi sentence: none
- Notes: Sunday 3 April 2022 matches. Times of Oman (https://timesofoman.com/article/115094-first-day-of-ramadan-announced-in-oman-4, 2022-04-01): "Sunday, April 3, will be the first day of Ramadan, Oman's Ministry of Endowment and Religious Affairs announced." Oman one day after Saudi (2 April).

#### Shawwal 1443 AH

- Gregorian date: 2022-05-02
- Source URL: https://www.omanobserver.om/ampArticle/1118547
- Source type: press (Oman Observer)
- Publication date: 2022-05-01
- Quote: "The Ministry of Endowments and Religious Affairs on Sunday said that tomorrow, Monday, May 02, 2022, will be the first day of the blessed month of Shawwal and the first day of Eid al-Fitr."
- Derived: no
- Follows Saudi sentence: none
- Notes: Monday 2 May 2022 matches.

#### Dhu al-Hijjah 1443 AH

- Gregorian date: 2022-06-30
- Source URL: https://www.omanobserver.om/article/1121538
- Source type: press (Oman Observer)
- Publication date: 2022-06-29
- Quote: "The Ministry of Awqaf and Religious Affairs on Wednesday announced that the first day of Dhul Hijjah 1443 AH will start from Thursday (June 30). Hence, the first day of Eid al Adha will be on Saturday, Dhul Hijjah 10, 1443 AH corresponding to July 9."
- Derived: no
- Follows Saudi sentence: none
- Notes: Thursday 30 June 2022 matches.

#### Ramadan 1444 AH

- Gregorian date: 2023-03-23
- Source URL: https://www.omanobserver.om/ampArticle/1134527
- Source type: press (Oman Observer)
- Publication date: 2023-03-22
- Quote: "The main committee for the moon sighting of Ramadhan 1444 AH, which met on Wednesday at the Ministry of Endowment and Religious Affairs, has announced that the first day of the holy month of Ramadhan will be on Thursday, March 23, in the Sultanate."
- Derived: no
- Follows Saudi sentence: none
- Notes: Thursday 23 March 2023 matches. Oman same day as Saudi.

#### Shawwal 1444 AH

- Gregorian date: 2023-04-22
- Source URL: https://www.omanobserver.om/ampArticle/1135965
- Source type: press (Oman Observer)
- Publication date: 2023-04-20
- Quote: "The first day of Eid al Fitr 2023 in the Sultanate will be on Saturday, April 22, according to the moon sighting main committee that met on Thursday"
- Derived: no
- Follows Saudi sentence: none
- Notes: Saturday 22 April 2023 matches. Eid day equals 1 Shawwal.

#### Dhu al-Hijjah 1444 AH

- Gregorian date: 2023-06-19
- Source URL: https://www.omanobserver.om/article/1138909
- Source type: press (Oman Observer)
- Publication date: 2023-06-18
- Quote: "The Main Committee on sighting the crescent moon of Dhul Hijjah 1444 AH has announced that the moonsighting was confirmed on Sunday. Thus, the first day of Eid Al Adha (10 Dhul Hijjah 1444 AH) will fall on Wednesday, June 28, 2023."
- Derived: yes
- Follows Saudi sentence: none
- Notes: Derived: 28 June minus 9 days = 19 June 2023 (Monday). Sighting confirmed Sunday 18 June evening, consistent.

#### Ramadan 1445 AH

- Gregorian date: 2024-03-12
- Source URL: https://www.omanobserver.om/ampArticle/1150795
- Source type: press (Oman Observer)
- Publication date: 2024-03-10
- Quote: "The Ministry of Endowments and Religious Affairs announced on Sunday that Tuesday, March 12, 2024 will be the first day of the holy month of Ramadhan 1445 AH, due to the lack of confirmation this evening of sighting the crescent of the holy month. Therefore, Monday March 11 will be the completing day of the month of Shaaban."
- Derived: no
- Follows Saudi sentence: none
- Notes: Tuesday 12 March 2024 matches. Times of Oman (ONA) https://timesofoman.com/article/143081-ramadan-to-start-from-march-12-in-oman agrees. Oman one day after Saudi (11 March).

#### Shawwal 1445 AH

- Gregorian date: 2024-04-10
- Source URL: https://www.omanobserver.om/ampArticle/1152139
- Source type: press (Oman Observer)
- Publication date: 2024-04-09
- Quote: "The first day of Eid al Fitr in Oman will be on Wednesday, the Main Moon Sighting Committee announced on Tu[esday]"
- Derived: no
- Follows Saudi sentence: none
- Notes: Wednesday 10 April 2024 matches (quote truncated by my extraction).

#### Dhu al-Hijjah 1445 AH

- Gregorian date: 2024-06-08
- Source URL: https://www.omanobserver.om/ampArticle/1154789
- Source type: press (Oman Observer)
- Publication date: 2024-06-06
- Quote: "The moonsighting committee of the Ministry of Awqaf and Religious Affairs announced that the crescent moon of Dhul Hijjah 1445 AH was not sighted on Thursday. Hence, Friday, June 7, will be Dhul Qa'ada 30, and Monday, June 17, will be the first day of Eid al Adha."
- Derived: yes
- Follows Saudi sentence: none
- Notes: Derived: Dhul Qa'ada 30 = Friday 7 June so 1 Dhu al-Hijjah = Saturday 8 June 2024. Eid 17 June minus 9 = 8 June, consistent. Oman one day after Saudi (Eid 16 June).

#### Ramadan 1446 AH

- Gregorian date: 2025-03-01
- Source URL: https://www.omanobserver.om/article/1166986
- Source type: press (Oman Observer)
- Publication date: 2025-02-28
- Quote: "Muscat: The first day of Ramadhan will be on Saturday, March 1, 2025. The Main Committee for Moon Sighting met on Friday evening at the Ministry of Endowments and Religious Affairs (MERA) to observe the crescent of the holy month of Ramadhan for the year 1446 AH."
- Derived: no
- Follows Saudi sentence: none
- Notes: Saturday 1 March 2025 matches.

#### Shawwal 1446 AH

- Gregorian date: 2025-03-31
- Source URL: https://www.omanobserver.om/article/1168335
- Source type: press (Oman Observer)
- Publication date: 2025-03-29
- Quote: "The first day of Eid al Fitr 2025 in the Sultanate of Oman (1 of Shawwal 1446 AH) will be Monday, March 31, according to the moon sighting main committee that met on Saturday."
- Derived: no
- Follows Saudi sentence: none
- Notes: Monday 31 March 2025 matches. Oman one day after Saudi (30 March).

#### Dhu al-Hijjah 1446 AH

- Gregorian date: 2025-05-28
- Source URL: https://www.omanobserver.om/ampArticle/1171178
- Source type: press (Oman Observer)
- Publication date: 2025-05-27
- Quote: "Accordingly, tomorrow, Wednesday, will be the first day of Dhul Hijjah and Friday, June 6, will be the first day of Eid Al Adha."
- Derived: no
- Follows Saudi sentence: none
- Notes: Wednesday 28 May 2025 matches (article of Tuesday 27 May). Gulf News (ONA via X) https://gulfnews.com/world/gulf/oman/first-day-of-eid-al-adha-announced-in-oman-1.500142258 has same text.

#### Ramadan 1447 AH

- Gregorian date: 2026-02-19
- Source URL: https://www.omanobserver.om/article/1184239
- Source type: press (Oman Observer)
- Publication date: 2026-02-11
- Quote: "The Main Committee for Moon Sighting has announced that Wednesday, February 18, 2026, will mark the completion of the month of Shaaban 1447 AH. Accordingly, Thursday, February 19, 2026, will be the first day of the Holy Month of Ramadhan 1447 AH."
- Derived: no
- Follows Saudi sentence: none
- Notes: Thursday 19 February 2026 matches. Decision based on astronomical impossibility of sighting. Times of Oman (ONA) https://timesofoman.com/article/168180-thursday-19-february-2026-is-first-day-of-ramadan-1447-ah agrees.

#### Shawwal 1447 AH

- Gregorian date: 2026-03-20
- Source URL: https://www.omanobserver.om/article/1186409
- Source type: press (Oman Observer)
- Publication date: 2026-03-19
- Quote: "The sighting of the crescent moon for the month of Shawwal has been confirmed. Accordingly, tomorrow (Friday), March 20 will be the first day of Eid al-Fitr."
- Derived: no
- Follows Saudi sentence: none
- Notes: Friday 20 March 2026 matches.

#### Dhu al-Hijjah 1447 AH

- Gregorian date: 2026-05-18
- Source URL: https://www.omanobserver.om/ampArticle/1189777
- Source type: press (Oman Observer)
- Publication date: 2026-05-17
- Quote: "The crescent moon of Dhul Hijjah, 1447 AH has been sighted making Monday May 18 the first day of Dhul Hijjah while the first day of Eid Al Adha will fall on Wednesday, May 27, the Main Committee for Determining the Crescent Moons of the Islamic Months, announced on Sunday."
- Derived: no
- Follows Saudi sentence: none
- Notes: Monday 18 May 2026 matches.

| country | hijri_year | hijri_month | gregorian | derived | source |
|---|---|---|---|---|---|
| Oman | 1436 | 10 | 2015-07-18 | no | https://www.emirates247.com/news/emirates/eid-al-fitr-in-oman-today-2015-07-16-1.596859 |
| Oman | 1437 | 9 | 2016-06-07 | no | https://www.deccanchronicle.com/world/middle-east/060616/muslim-holy-month-of-ramadan-starts-today-in-gulf.html |
| Oman | 1437 | 12 | 2016-09-03 | yes | https://suroorasia.blogspot.com/2016/09/eid-al-adha-holiday-announced-in-oman.html |
| Oman | 1438 | 9 | 2017-05-27 | no | https://www.omanobserver.om/article/81252 |
| Oman | 1438 | 10 | 2017-06-26 | no | https://www.omanobserver.om/article/78856 |
| Oman | 1439 | 9 | 2018-05-17 | no | https://www.omanobserver.om/article/56902 |
| Oman | 1439 | 10 | 2018-06-15 | no | https://www.omanobserver.om/article/54540 |
| Oman | 1439 | 12 | 2018-08-12 | yes | https://www.omanobserver.om/article/50532 |
| Oman | 1440 | 9 | 2019-05-07 | no | https://www.omanobserver.om/tuesday-first-day-of-ramadhan-in-oman/ |
| Oman | 1440 | 10 | 2019-06-05 | no | https://www.omanobserver.om/article/30845 |
| Oman | 1440 | 12 | 2019-08-03 | no | https://www.omanobserver.om/article/27374 |
| Oman | 1441 | 9 | 2020-04-25 | no | https://www.omanobserver.om/article/13744 |
| Oman | 1441 | 10 | 2020-05-24 | no | https://gulfnews.com/world/gulf/oman/oman-begins-eid-on-sunday-1.71661242 |
| Oman | 1441 | 12 | 2020-07-22 | no | https://www.omanobserver.om/article/11774 |
| Oman | 1442 | 9 | 2021-04-14 | no | https://www.omanobserver.om/article/773 |
| Oman | 1442 | 10 | 2021-05-13 | no | https://www.omanobserver.om/ampArticle/1100748 |
| Oman | 1442 | 12 | 2021-07-11 | no | https://www.omanobserver.om/ampArticle/1103676 |
| Oman | 1443 | 9 | 2022-04-03 | no | https://www.omanobserver.om/article/1117058 |
| Oman | 1443 | 10 | 2022-05-02 | no | https://www.omanobserver.om/ampArticle/1118547 |
| Oman | 1443 | 12 | 2022-06-30 | no | https://www.omanobserver.om/article/1121538 |
| Oman | 1444 | 9 | 2023-03-23 | no | https://www.omanobserver.om/ampArticle/1134527 |
| Oman | 1444 | 10 | 2023-04-22 | no | https://www.omanobserver.om/ampArticle/1135965 |
| Oman | 1444 | 12 | 2023-06-19 | yes | https://www.omanobserver.om/article/1138909 |
| Oman | 1445 | 9 | 2024-03-12 | no | https://www.omanobserver.om/ampArticle/1150795 |
| Oman | 1445 | 10 | 2024-04-10 | no | https://www.omanobserver.om/ampArticle/1152139 |
| Oman | 1445 | 12 | 2024-06-08 | yes | https://www.omanobserver.om/ampArticle/1154789 |
| Oman | 1446 | 9 | 2025-03-01 | no | https://www.omanobserver.om/article/1166986 |
| Oman | 1446 | 10 | 2025-03-31 | no | https://www.omanobserver.om/article/1168335 |
| Oman | 1446 | 12 | 2025-05-28 | no | https://www.omanobserver.om/ampArticle/1171178 |
| Oman | 1447 | 9 | 2026-02-19 | no | https://www.omanobserver.om/article/1184239 |
| Oman | 1447 | 10 | 2026-03-20 | no | https://www.omanobserver.om/article/1186409 |
| Oman | 1447 | 12 | 2026-05-18 | no | https://www.omanobserver.om/ampArticle/1189777 |

### Jordan: announced Hijri month starts, 1436 to 1447

Official body: General Iftaa' Department (Board of Iftaa', Research and Islamic Studies), which publishes numbered resolutions at aliftaa.jo for Ramadan and Shawwal sightings (fetched directly, English pages). Before the 2019 change in post the announcement came from the Chief Islamic Justice (Ahmad Hilayel, per Ammon 2015). Dhu al-Hijjah has no Board resolution on the site, so those rows are press or agency. Jordan Times is behind Cloudflare and could not be fetched. The Wayback Machine could not be used (access was denied by the tool environment). Petra's old-format pages were mostly not fetchable. No Wayback copies were used.

Jordan says it announces after Saudi or other Arab states where a sighting is plausible, but no fetched page contains a 'follows Saudi' sentence for these rows. Jordan began Ramadan on a different day from Oman in 1442 (Jordan 13 April, Oman 14 April), among others.

Missing (no fetchable Jordanian page): Ramadan 1437, Shawwal 1437, Shawwal 1438, Dhu al-Hijjah 1436, 1437, 1438, 1439, 1440, 1441 and 1444.

Pattern in the official pages: several print the meeting date or a wrong Gregorian date next to the weekday (rows for Ramadan 1438, 1439, 1440, Shawwal 1440, Shawwal 1442). In each case the notes say which was used.

#### Ramadan 1436 AH

- Gregorian date: 2015-06-18
- Source URL: https://en.ammonnews.net/article/29191
- Source type: press (Ammon News)
- Publication date: 2015-06-17
- Quote: "Ramadan, the Muslim holy month of fasting, will begin on Thursday in Jordan, Chief Islamic Justice Ahmad Hilayel announced on Tuesday. Hilayel said the crescent moon (hilal) was not spotted in Jordan on Tuesday, which makes Thursday the first day of the holy month."
- Derived: no
- Follows Saudi sentence: none
- Notes: The page gives no Gregorian date. 18 June 2015 comes from the page date (17-06-2015) plus 'Thursday'. Announced by the Chief Islamic Justice Department.

#### Shawwal 1436 AH

- Gregorian date: 2015-07-17
- Source URL: https://www.emirates247.com/news/emirates/eid-al-fitr-in-oman-today-2015-07-16-1.596859
- Source type: press (Emirates 24|7, regional)
- Publication date: 2015-07-16
- Quote: "Egypt, Jordan, Kuwait, Qatar, Yemen, Iraq, Lebanon and Palestine announced today that tomorrow, Friday, will be the first day of Eid Al Fitr, after the sighting of Shawwal Crescent for year 1436 H."
- Derived: no
- Follows Saudi sentence: none
- Notes: WEAK: a regional list, not a Jordanian page. Friday 17 July 2015 matches. No Jordanian page for this month could be fetched (Jordan Times is blocked).

#### Ramadan 1438 AH

- Gregorian date: 2017-05-27
- Source URL: https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=527
- Source type: official (Iftaa' Board Resolution 238)
- Publication date: 2017-05-26 (meeting date as printed. added to site 29-05-2017)
- Quote: "the Iftaa` Board convened on Friday evening (29th of Sha`baan, 1438 AH, corresponding to 26,5,2017) ... tomorrow, Saturday, is the first day of the holy month of Ramadan for the year 1438 AH, corresponding to 26,5,2017AD."
- Derived: no
- Follows Saudi sentence: none
- Notes: MISMATCH: the page prints 26,5,2017 as the Gregorian date of the first day, but 26 May 2017 was a Friday. The page says Saturday, and the 29th of Sha'ban meeting was Friday 26 May, so day 1 is Saturday 27 May 2017. Used 27 May.

#### Ramadan 1439 AH

- Gregorian date: 2018-05-17
- Source URL: https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=583
- Source type: official (Iftaa' Board Resolution 258)
- Publication date: 2018-05-15 (meeting date as printed. added to site 24-05-2018)
- Quote: "convened on Tuesday evening (29th of Sha`baan, 1439 AH, corresponding to 15/5/2018) ... tomorrow, Wednesday, will complement the month of Sha`baan, and so Thursday will be the first day of the holy month of Ramadan for the year 1439 AH, corresponding to 15/5/2018 AD."
- Derived: no
- Follows Saudi sentence: none
- Notes: MISMATCH: the page prints 15/5/2018 as the Gregorian date of day 1, which is the meeting day (a Tuesday). Wednesday completes Sha'ban, so Thursday 17 May 2018 is day 1. Used 17 May.

#### Shawwal 1439 AH

- Gregorian date: 2018-06-15
- Source URL: https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=591
- Source type: official (Iftaa' Board Resolution 261)
- Publication date: 2018-06-14 (meeting date as printed. added to site 24-07-2018)
- Quote: "it was ascertained to the Board that tomorrow, Friday, will be the first of Shawwal for this year: (1439 AH), corresponding to (15/6/2018 AD), and so it is the first day of the Eid of Fitr."
- Derived: no
- Follows Saudi sentence: none
- Notes: Friday 15 June 2018 matches.

#### Ramadan 1440 AH

- Gregorian date: 2019-05-07
- Source URL: https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=618
- Source type: official (Iftaa' Board Resolution 275)
- Publication date: 2019-05-05 (meeting date as printed)
- Quote: "convened on Sunday evening (29th of Sha`baan, 1440 AH, corresponding to 5/5/2019) ... tomorrow, Monday, will complement the month of Sha`baan. Consequently, Tuseday will be the first day of the holy month of Ramadan for the year 1440 AH, 5/5/2019 AD."
- Derived: no
- Follows Saudi sentence: none
- Notes: MISMATCH: the page prints 5/5/2019 (a Sunday, the meeting day) next to 'Tuesday'. Monday completes Sha'ban, so Tuesday 7 May 2019 is day 1. Used 7 May.

#### Shawwal 1440 AH

- Gregorian date: 2019-06-05
- Source URL: https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=650
- Source type: official (Iftaa' Board Resolution 280)
- Publication date: 2019-06-04 (inferred. page added 08-06-2020)
- Quote: "it was ascertained to the Board that tomorrow, Friday, will be the first of Shawwal for this year: (1440 AH), corresponding to (5/6/2019 AD), and so it is the first day of the Eid of Fitr."
- Derived: no
- Follows Saudi sentence: none
- Notes: MISMATCH: the page says Friday but 5 June 2019 was a Wednesday. Used the printed date 5 June 2019 (the Board met on 29 Ramadan, Tuesday 4 June, if Ramadan began Tuesday 7 May). An unfetched Petra snippet also gave Wednesday 5 June, not used as a source.

#### Ramadan 1441 AH

- Gregorian date: 2020-04-24
- Source URL: https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=645
- Source type: official (Iftaa' Board Resolution 285)
- Publication date: 2020-04-23 (meeting date as printed)
- Quote: "convened on Thursday evening (29th of Sha`baan, 1441 AH, corresponding to 23/4/2020AD) ... tomorrow, Friday (24/ 4/2020), will be the first day of the holy month of Ramadan for the Hijri year 1441."
- Derived: no
- Follows Saudi sentence: none
- Notes: Friday 24 April 2020 matches. Roya News (https://en.royanews.tv/news/20817, 2020-04-23) agrees: "the holy month of Ramadan will begin tomorrow, April 24, 2020".

#### Shawwal 1441 AH

- Gregorian date: 2020-05-23
- Source URL: https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=646
- Source type: official (Iftaa' Board Resolution 289)
- Publication date: 2020-05-22 (meeting date as printed. added to site 28-05-2020)
- Quote: "Date:(29/Ramadan/1441 H), corresponding to (22/5/2020) ... it was ascertained to the Board that tomorrow, Saturday will be the first of Shawwal for this year: (1441 H), and so it is the first day of the Eid of Fitr."
- Derived: no
- Follows Saudi sentence: none
- Notes: The page prints no Gregorian date for day 1. 22/5/2020 is the meeting date and 'tomorrow, Saturday' is 23 May 2020. Derived by the day after the printed meeting date, so derived is marked no but the date is inferred from the meeting date.

#### Ramadan 1442 AH

- Gregorian date: 2021-04-13
- Source URL: https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=1682
- Source type: official (Iftaa' Board Resolution 301)
- Publication date: 2021-04-12 (meeting date as printed. added to site 14-04-2021)
- Quote: "it was ascertained to the Board that tomorrow, Tuesday, (13/4/2021) is the first day of the holy month of Ramadan for this blessed year, 1442 AH."
- Derived: no
- Follows Saudi sentence: none
- Notes: Tuesday 13 April 2021 matches. Roya News https://en.royanews.tv/news/27004 (2021-04-12) agrees. Jordan one day before Oman (14 April).

#### Shawwal 1442 AH

- Gregorian date: 2021-05-13
- Source URL: https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=1713
- Source type: official (Iftaa' Board Resolution 303)
- Publication date: 2021-05-11 (meeting date as printed. added to site 24-10-2022)
- Quote: "Date :(29/Ramadan/1442H), corresponding to (11/5/2021) ... tomorrow, Thursday, 12/5/2021, will be the first of Shawwal for this year: (1442 H)"
- Derived: no
- Follows Saudi sentence: none
- Notes: MISMATCH: the page says Thursday but prints 12/5/2021, a Wednesday. Jordan News (https://www.jordannews.jo/Section-109/News/Jordan-to-observe-Eid-Al-Fitr-Thursday-2801, 2021-05-11) says: "Grand Mufti Abdul Karim Al-Khasawneh announced Tuesday that Wednesday will be the 30th of Ramadan , followed Thursday by Eid Al-Fitr". Both say Thursday, so used Thursday 13 May 2021.

#### Dhu al-Hijjah 1442 AH

- Gregorian date: 2021-07-11
- Source URL: https://en.royanews.tv/news/29594
- Source type: press (Roya News)
- Publication date: 2021-07-10
- Quote: "Grand Mufti of the Hashemite Kingdom of Jordan, Sheikh Abdul Karim Al-Khasawneh, announced that Sunday is the first day of Dhul-Hijjah, and accordingly, Monday, July 19, will be the day of standing in Arafat, and on Tuesday, July 20, the blessed Eid al-Adha."
- Derived: no
- Follows Saudi sentence: none
- Notes: Sunday 11 July 2021 matches. Jordan News https://www.jordannews.jo/Section-109/News/Jordan-marks-Eid-Al-Adha-on-July-20-5018 and ArabiaWeather agree.

#### Ramadan 1443 AH

- Gregorian date: 2022-04-03
- Source URL: https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=1701
- Source type: official (Iftaa' Board Resolution 310)
- Publication date: 2022-04-01 (meeting date as printed. added to site 03-04-2022)
- Quote: "convened on Friday evening (29th of Sha`ban, 1442 AH, corresponding to 1/4/2022) ... tomorrow, Sunday, (3/4/2022) is the first day of the holy month of Ramadan for this blessed year, 1443AH."
- Derived: no
- Follows Saudi sentence: none
- Notes: Sunday 3 April 2022 matches (the page misprints 1442 for 1443 in one place). Roya News https://en.royanews.tv/news/35034 agrees.

#### Shawwal 1443 AH

- Gregorian date: 2022-05-02
- Source URL: https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=1705
- Source type: official (Iftaa' Board Resolution 312)
- Publication date: 2022-05-01
- Quote: "it was ascertained to the Board that tomorrow, Monday, 2/5/2022, will be the first of Shawwal for this year: (1443 H), and so it is the first day of the Eid of Fitr."
- Derived: no
- Follows Saudi sentence: none
- Notes: Monday 2 May 2022 matches. Jordan News https://www.jordannews.jo/Section-109/News/Tomorrow-marks-first-day-of-Eid-Al-Fitr-Grand-Mufti-16236 (2022-05-01): "announced, May 2 as the first day of Eid Al-Fitr".

#### Dhu al-Hijjah 1443 AH

- Gregorian date: 2022-06-30
- Source URL: https://www.arabiaweather.com/en/content/reply-now-the-mufti-of-jordan-announces-the-start-of-the-month-of-dhu-alhijjah-and-eid-aladha-after-the-sighting-of-the-crescent-has-been-confirmed
- Source type: press (ArabiaWeather, machine-translated from Arabic)
- Publication date: 2022-06-29
- Quote: "the Grand Mufti of the Kingdom, Sheikh Abdul Karim Al-Khasawneh, announced that after the sighting of the crescent of the month of Dhul-Hijjah was proven, that tomorrow, Thursday, June 30, 2022, is the first of the month of Dhul-Hijjah, and accordingly, Friday, July 8, will be the day of standing on Arafat, and Saturday 9 July is the first day of Eid al-Adha."
- Derived: no
- Follows Saudi sentence: none
- Notes: Thursday 30 June 2022 matches.

#### Ramadan 1444 AH

- Gregorian date: 2023-03-23
- Source URL: https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=1726
- Source type: official (Iftaa' Board Resolution 321)
- Publication date: 2023-03-22 (meeting date as printed. added to site 23-03-2023)
- Quote: "convened on Wednesday evening (29th of Sha`ban, 1444 AH, corresponding to 22/3/2023) ... tomorrow, Thursday, (23/3/2023) is the first day of the holy month of Ramadan for this blessed year, 1444AH."
- Derived: no
- Follows Saudi sentence: none
- Notes: Thursday 23 March 2023 matches.

#### Shawwal 1444 AH

- Gregorian date: 2023-04-21
- Source URL: https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=1730
- Source type: official (Iftaa' Board Resolution 323)
- Publication date: 2023-04-20 (added to site 27-04-2023)
- Quote: "it was ascertained to the Board that tomorrow, Friday, 21/4/2023, will be the first of Shawwal for this year: (1444 H), and so it is the first day of the Eid of Fitr."
- Derived: no
- Follows Saudi sentence: none
- Notes: Friday 21 April 2023 matches. Jordan News https://www.jordannews.jo/Section-109/News/Grand-Mufti-announces-Friday-first-day-to-Eid-Al-Fitr-28229 (2023-04-20) agrees.

#### Ramadan 1445 AH

- Gregorian date: 2024-03-12
- Source URL: https://en.royanews.tv/news/49980
- Source type: press (Roya News. also Jordan News and Ammon News)
- Publication date: 2024-03-10
- Quote: "Jordan has officially announced that Tuesday, March 12, is the first day of the holy month of Ramadan. This came after the General Ifta' Department in Jordan made the announcement at around 7:10 pm as the department investigated the crescent of the blessed month of Ramadan."
- Derived: no
- Follows Saudi sentence: none
- Notes: SOURCES DISAGREE. The official Board page (https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=1734, Resolution 324, added 21-03-2024) says: "convened on Sunday evening (29th of Sha`ban, 1445 AH, corresponding to 10/3/2024) ... tomorrow, Monday, (11/3/2024) is the first day of the holy month of Ramadan". Press reports of the same evening (Roya 2024-03-10, Jordan News https://www.jordannews.jo/Section-109/News/Ramadan-to-start-on-Tuesday-March-12-in-Jordan-34746 "On Sunday, Jordan announced that Tuesday, March 12 will be the first day", Ammon https://en.ammonnews.net/article/71233) say Tuesday 12 March. The Board page is the official body's text, but it contradicts the same department's announcement as reported by three outlets and looks like a copy-forward error. The table uses 12 March. Caller should decide.

#### Shawwal 1445 AH

- Gregorian date: 2024-04-10
- Source URL: https://en.royanews.tv/news/50581
- Source type: press (Roya News)
- Publication date: 2024-04-09
- Quote: "Jordan announced that Eid al-Fitr will fall on Wednesday, as Tuesday marked the final day of Ramadan. The Grand Mufti of the Kingdom, Dr. Ahmed Al-Hasanat, had called for sighting the Shawwal crescent on Tuesday."
- Derived: no
- Follows Saudi sentence: none
- Notes: Gregorian date is from the Roya article's date (9 Apr 2024, Tuesday) plus Wednesday, so 10 April 2024. Jordan News (https://www.jordannews.jo/Section-109/News/Jordan-Announces-Eid-al-Fitr-Celebration-Start-Date-Wednesday-Confirmed-35206) says: "Jordan is set to celebrate Eid al-Fitr 2024 on Wednesday ... declared Wednesday, April 10, 2024, as the beginning of Eid al-Fitr".

#### Dhu al-Hijjah 1445 AH

- Gregorian date: 2024-06-07
- Source URL: https://en.royanews.tv/news/51891
- Source type: press (Roya News)
- Publication date: 2024-06-07
- Quote: "The Grand Mufti of the Kingdom, Ahmad Al-Hasanat, announced that Friday will mark the first day of Dhu Al-Hijjah for the Hijri year 1445. Consequently, the Day of Arafah is set for Saturday, June 15, and the first day of Eid Al-Adha will be celebrated on Sunday, June 16."
- Derived: no
- Follows Saudi sentence: none
- Notes: Friday 7 June 2024 matches (Roya dated 7 June 2024). ArabiaWeather (2024-06-06) https://arabiaweather.com/en/amp/content/الأردن-غدًا-الجُمعة-أول-أيام-شهر-ذي-الحجة-ويوم-الأحد-16-حزيران-أول-أيام-عيد-الأضحى-المُبارك says the same.

#### Ramadan 1446 AH

- Gregorian date: 2025-03-01
- Source URL: https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=1741
- Source type: official (Iftaa' Board Resolution 326)
- Publication date: 2025-02-28 (meeting date as printed. added to site 03-03-2025)
- Quote: "tomorrow, Saturday, (1/3/2025) is the first day of the holy month of Ramadan for this blessed year, 1446 AH."
- Derived: no
- Follows Saudi sentence: none
- Notes: Saturday 1 March 2025 matches. Roya News https://en.royanews.tv/news/57845 agrees.

#### Shawwal 1446 AH

- Gregorian date: 2025-03-31
- Source URL: https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=1743
- Source type: official (Iftaa' Board Resolution 328)
- Publication date: 2025-03-29 (added to site 13-05-2025)
- Quote: "tomorrow, Sunday, 30/3/2025, is the last day of Ramadan while the day after tomorrow, Monday, will be the first of Shawwal for this year: (1446 AH), i.e. the first day of the Eid Al-Fitr."
- Derived: no
- Follows Saudi sentence: none
- Notes: Monday 31 March 2025 matches.

#### Dhu al-Hijjah 1446 AH

- Gregorian date: 2025-05-28
- Source URL: https://en.royanews.tv/news/59973
- Source type: press (Roya News)
- Publication date: 2025-05-27
- Quote: "Jordan's Iftaa' Department announced that Wednesday, May 28, will be the first day of Dhu al-Hijjah 1446. Accordingly, the first day of Eid al-Adha will fall on Friday, June 6."
- Derived: no
- Follows Saudi sentence: none
- Notes: Wednesday 28 May 2025 matches.

#### Ramadan 1447 AH

- Gregorian date: 2026-02-19
- Source URL: https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=1749
- Source type: official (Iftaa' Board Resolution 330)
- Publication date: 2026-02-17 (meeting date as printed. added to site 23-02-2026)
- Quote: "tomorrow, Wednesday, (18/2/2026) is the last day of the month of Sha`ban, and that after tomorrow, Thursday, (19/2/2026) is the first day of the blessed month of Ramadan for this year, 1447 AH."
- Derived: no
- Follows Saudi sentence: none
- Notes: Thursday 19 February 2026 matches. Petra https://petra.gov.jo/en/news/thursday-first-day-of-ramadan-in-jordan-after-crescent-not-sighted (2026-02-17): "Thursday will mark the first day of Ramadan for the year 1447 AH".

#### Shawwal 1447 AH

- Gregorian date: 2026-03-20
- Source URL: https://www.almamlakatv.com/news/194158-الأردن-يعلن-الجمعة-أول-أيام-عيد-الفطر
- Source type: press (Al Mamlaka TV, Arabic)
- Publication date: 2026-03-19
- Quote: "أعلن مفتي عام المملكة، أحمد الحسنات، أن الجمعة هو غرة شهر شوال للعام الهجري 1447، وأول أيام عيد الفطر المبارك."
- Derived: no
- Follows Saudi sentence: none
- Notes: The page gives no Gregorian date. 20 March 2026 is 'Friday' after the page date 19-03-2026 (Thursday evening event). Roya News https://en.royanews.tv/news/68410 (2026-03-18) confirms the sighting call for Thursday 19 March.

#### Dhu al-Hijjah 1447 AH

- Gregorian date: 2026-05-18
- Source URL: https://petra.gov.jo/en/news/jordan-declares-start-of-dhu-al-hijjah-setting-date-for-eid-al-adha
- Source type: agency (Petra)
- Publication date: 2026-05-17
- Quote: "The Grand Mufti of Jordan Dr. Ahmad Al-Hasanat, announced on Sunday that Monday will mark the first day of the Islamic holy month of Dhu al-Hijjah for the Hijri year 1447."
- Derived: no
- Follows Saudi sentence: none
- Notes: Monday 18 May 2026 matches (Sunday was 17 May). Roya News https://en.royanews.tv/news/69923 and Jordan News agree.

| country | hijri_year | hijri_month | gregorian | derived | source |
|---|---|---|---|---|---|
| Jordan | 1436 | 9 | 2015-06-18 | no | https://en.ammonnews.net/article/29191 |
| Jordan | 1436 | 10 | 2015-07-17 | no | https://www.emirates247.com/news/emirates/eid-al-fitr-in-oman-today-2015-07-16-1.596859 |
| Jordan | 1438 | 9 | 2017-05-27 | no | https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=527 |
| Jordan | 1439 | 9 | 2018-05-17 | no | https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=583 |
| Jordan | 1439 | 10 | 2018-06-15 | no | https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=591 |
| Jordan | 1440 | 9 | 2019-05-07 | no | https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=618 |
| Jordan | 1440 | 10 | 2019-06-05 | no | https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=650 |
| Jordan | 1441 | 9 | 2020-04-24 | no | https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=645 |
| Jordan | 1441 | 10 | 2020-05-23 | no | https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=646 |
| Jordan | 1442 | 9 | 2021-04-13 | no | https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=1682 |
| Jordan | 1442 | 10 | 2021-05-13 | no | https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=1713 |
| Jordan | 1442 | 12 | 2021-07-11 | no | https://en.royanews.tv/news/29594 |
| Jordan | 1443 | 9 | 2022-04-03 | no | https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=1701 |
| Jordan | 1443 | 10 | 2022-05-02 | no | https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=1705 |
| Jordan | 1443 | 12 | 2022-06-30 | no | https://www.arabiaweather.com/en/content/reply-now-the-mufti-of-jordan-announces-the-start-of-the-month-of-dhu-alhijjah-and-eid-aladha-after-the-sighting-of-the-crescent-has-been-confirmed |
| Jordan | 1444 | 9 | 2023-03-23 | no | https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=1726 |
| Jordan | 1444 | 10 | 2023-04-21 | no | https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=1730 |
| Jordan | 1445 | 9 | 2024-03-11 | no | https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=1734 |
| Jordan | 1445 | 10 | 2024-04-10 | no | https://en.royanews.tv/news/50581 |
| Jordan | 1445 | 12 | 2024-06-07 | no | https://en.royanews.tv/news/51891 |
| Jordan | 1446 | 9 | 2025-03-01 | no | https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=1741 |
| Jordan | 1446 | 10 | 2025-03-31 | no | https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=1743 |
| Jordan | 1446 | 12 | 2025-05-28 | no | https://en.royanews.tv/news/59973 |
| Jordan | 1447 | 9 | 2026-02-19 | no | https://www.aliftaa.jo/DecisionEn.aspx?DecisionId=1749 |
| Jordan | 1447 | 10 | 2026-03-20 | no | https://www.almamlakatv.com/news/194158-الأردن-يعلن-الجمعة-أول-أيام-عيد-الفطر |
| Jordan | 1447 | 12 | 2026-05-18 | no | https://petra.gov.jo/en/news/jordan-declares-start-of-dhu-al-hijjah-setting-date-for-eid-al-adha |

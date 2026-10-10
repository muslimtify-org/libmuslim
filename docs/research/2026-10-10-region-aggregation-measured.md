# Region aggregation, measured and deferred

**Date:** 2026-10-10

Kemenag evaluates MABIMS 2021 "di seluruh wilayah Indonesia", across the whole territory, and Malaysia observes at 29 sites. `hijri.h` evaluates a predicate at one point. The obvious next feature was a region rule: a month starts if the predicate passes anywhere in a territory. This note records the measurement taken before building it, and why it was not built.

## What was measured

`HIJRI_PREDICATE_MABIMS_2021` was evaluated through `hijri_evaluate_evening` at every whole degree of a box covering Indonesia, latitude 11 south to 6 north and longitude 95 to 141 east, 846 points. For each of the 37 Kemenag month starts in `tests/test_hijri.c` this was done on the decision evening, S minus 1, and on the evening before it, S minus 2. A start counts as supported if any point passes on S minus 1, and as early if any point passes on S minus 2. The box includes sea, and it excludes nothing inside Indonesia, so it is a superset of the territory.

| Rule | Supported | One day early |
|---|---|---|
| Sabang alone, 5.8926 north, 95.3238 east | 33 of 37 | 0 of 37 |
| Any point in the box | 33 of 37 | 0 of 37 |

The two rules gave the same verdict on all 74 evenings.

## The four unsupported starts are not a geography problem

For each start that Sabang does not support, the point in the box with the largest margin, where margin is the smaller of altitude minus 3 and topocentric elongation minus 6.4:

| Start | Best point | Altitude | Topocentric elongation | Margin |
|---|---|---|---|---|
| 2025-03-01 | 11 south, 95 east | 4.056 | 5.599 | -0.801 |
| 2025-12-21 | 2 north, 95 east | 2.606 | 5.947 | -0.453 |
| 2026-06-16 | 3 south, 95 east | 2.911 | 6.208 | -0.192 |
| 2026-12-10 | 1 north, 95 east | 2.622 | 5.990 | -0.410 |

No point in the box passes. The smallest miss, 0.192 degrees, is 27 times the 0.0070 degree error bar `hijri.h` documents. These four months are in the official calendar for a reason other than where the criterion is evaluated, which `2026-08-01-kemenag-reference.md` already left open.

The best point is on the western edge of the box every time, at longitude 95. Its latitude moves between 11 south and 2 north. Sabang's latitude was not the limiting factor in any of these months.

## Cost

The run took 57 seconds for 74 evenings at 846 points, 62604 evaluations, a little under one millisecond each. A region rule at this resolution costs about 0.8 seconds per evening against about one millisecond for one point.

## Malaysia

On less data the result is the same. `2026-10-04-mabims-states-fixtures.md` records that Langkawi, in the west of the country, and Kuala Lumpur both support all seven announced starts with none early.

## Decision

Region aggregation is deferred. In every month measured, a territory-wide rule is reproduced by one reference point on the territory's western side, and a general region API would add public surface and a large runtime cost for no change in any answer. The authority registry in `hijri.h` therefore carries one reference point per entry.

## What this does not show

The evidence is 37 months for Indonesia and 7 for Malaysia. A month where a point south-west of Sabang passes and Sabang fails is geometrically possible and was not seen. The grid step is one degree, so a pass confined to an area smaller than that between grid points would be missed, though the margins above are far larger than anything a one degree step could hide. If a longer official record ever shows such a month, this note is the baseline to rerun.

## Reproducing it

Save the program below as `region_probe.c` outside the repository and build it against the header, with `REPO` as the repository root:

```sh
gcc -std=c11 -O2 -IREPO region_probe.c -lm -o region_probe && ./region_probe
```

```c
#define HIJRI_IMPLEMENTATION
#include "hijri.h"
#include <stdio.h>
static const int K[][3]={{2024,1,13},{2024,2,11},{2024,3,12},{2024,4,10},{2024,5,10},{2024,6,8},{2024,7,7},{2024,8,6},{2024,9,5},{2024,10,4},{2024,11,3},{2024,12,3},{2025,1,1},{2025,1,31},{2025,3,1},{2025,3,31},{2025,4,29},{2025,5,28},{2025,6,27},{2025,7,26},{2025,8,25},{2025,9,23},{2025,10,23},{2025,11,22},{2025,12,21},{2026,1,20},{2026,2,19},{2026,3,21},{2026,4,19},{2026,5,18},{2026,6,16},{2026,7,16},{2026,8,14},{2026,9,13},{2026,10,12},{2026,11,11},{2026,12,10}};
int main(void){
  int sup=0,early=0,sab_sup=0; HijriLocation sab={5.8926,95.3238,10,"Sabang"};
  for(unsigned i=0;i<37;i++){double j=floor(hijri_jd_from_gregorian(K[i][0],K[i][1],K[i][2]));
    for(int o=1;o<=2;o++){int y,m;double d;hijri_gregorian_from_jd(j-o+0.5,&y,&m,&d);int dd=(int)floor(d);
      int any=0,npass=0;double best=-99,blat=0,blon=0,balt=0,bel=0;
      for(int la=-11;la<=6;la++)for(int lo=95;lo<=141;lo++){
        HijriLocation L={la,lo,0,"g"};HijriMonthDecision r=hijri_evaluate_evening(y,m,dd,&L,HIJRI_PREDICATE_MABIMS_2021);
        if(r.parameters.sunset_status!=HIJRI_EVENT_OK)continue;
        double ma=r.parameters.moon_center_geometric_altitude_deg-3.0,me=r.parameters.topocentric_elongation_deg-6.4;
        double mn=ma<me?ma:me; if(mn>best){best=mn;blat=la;blon=lo;balt=ma+3;bel=me+6.4;}
        if(r.month_starts_next_day){any=1;npass++;}}
      HijriMonthDecision rs=hijri_evaluate_evening(y,m,dd,&sab,HIJRI_PREDICATE_MABIMS_2021);
      if(o==1){sup+=any;sab_sup+=rs.month_starts_next_day;
        if(!rs.month_starts_next_day||!any)printf("start %04d-%02d-%02d S-1: sabang=%d grid_any=%d npass=%d best(lat %.0f lon %.0f alt %.3f elong %.3f margin %+.3f)\n",K[i][0],K[i][1],K[i][2],rs.month_starts_next_day,any,npass,blat,blon,balt,bel,best);}
      else{early+=any; if(any)printf("start %04d-%02d-%02d S-2 EARLY: npass=%d best(lat %.0f lon %.0f alt %.3f elong %.3f)\n",K[i][0],K[i][1],K[i][2],npass,blat,blon,balt,bel);}
    }}
  printf("sabang support %d/37, grid-any support %d/37, grid-any early %d/37\n",sab_sup,sup,early);return 0;}
```

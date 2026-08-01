# Wujudul Hilal: the official Muhammadiyah computation, from the primary source

**Date:** 2026-08-01

## Question

`HIJRI_PREDICATE_WUJUDUL_HILAL` implements Muhammadiyah's criterion as "the
Moon's upper limb apparent altitude is above 0° at sunset". The prose
criterion ("at sunset the Moon is above the horizon") admits at least four
mathematical readings spanning almost a full degree:

```
reading             Moon-center true altitude at sunset must exceed
center, true          0.00°
center, apparent     -0.57°   (refraction credit)
limb, true           -0.25°   (semidiameter credit)
limb, apparent       -0.83°   (both credits)
```

Which reading does Muhammadiyah's official computation use? This mattered
because the library's moonset solver and its upper-limb quantity used
*different* rows of that table (the internal-inconsistency defect logged
2026-08-01), and fixing the inconsistency requires choosing a convention.

## Answer: fully specified in the primary source

**Pedoman Hisab Muhammadiyah**, Majelis Tarjih dan Tajdid Pimpinan Pusat
Muhammadiyah, Yogyakarta, 2009, ISBN 979-97993-2-5 — the organisation's
official computation manual. PDF mirrored at
`https://luk.staff.ugm.ac.id/kmi/iptek/gapai/Pedoman_Hisab_Muhammadiyah.pdf`
(retrieved 2026-08-01; a copy is kept in the session scratchpad, and the
procedure below is quoted from pp. 88–95).

Step 5 defines the starting quantity:

> "Hitunglah tinggi bulan hakiki (hb) (tinggi titik pusat Bulan dilihat dari
> titik pusat bumi)"

— the **true geocentric altitude of the Moon's centre**. Step 9 then gives
the operative formula:

> "Hitunglah tinggi Bulan mar'i (h′b) dengan rumus:
> **(h′b) = (hb – Pb) + R′b + s.db + Dip**"

with step 10 stating what the result means:

> "Ini menunjukkan tinggi piringan atas Bulan menurut pengamat."
> (*This gives the altitude of the Moon's upper limb as seen by the
> observer.*)

The month begins when the three cumulative conditions hold at sunset:
ijtimak occurred, ijtimak before sunset, and h′b > 0.

## Every term, with the worked example's values

The book's own example (29 Ramadan 1429 H = 2008-09-29, Yogyakarta,
φ = −7°48′, λ = 110°21′, elevation 90 m):

| Term | Meaning | Example value |
|---|---|---|
| `hb` | true geocentric altitude, Moon's centre | −1°02′03.87″ |
| `−Pb` | parallax, `Pb = cos(hb) × HP` → topocentric | −0°56′19.28″ |
| `+R′b` | atmospheric refraction at the horizon, **added** | +0°34′30″ |
| `+s.db` | Moon's semidiameter → upper limb | +0°15′21.04″ |
| `+Dip` | horizon dip from observer elevation | +0°16′41.81″ |
| `h′b` | apparent upper-limb altitude, observer's horizon | **−0°52′03.82″** |

h′b < 0, so 1 Syawal 1429 was NOT declared for the next day; Ramadan
completed 30 days. The arithmetic reproduces: −62.06′ − 56.32′ + 34.5′ +
15.35′ + 16.70′ = −51.83′ ≈ the printed −52′03.82″.

The Dip value confirms the standard formula `Dip ≈ 1.76′ × √(elevation m)`:
1.76 × √90 = 16.7′, matching the printed 16′41.81″.

So the official convention is: **upper limb, apparent (refraction added),
topocentric (parallax applied), with an elevation dip credit** — the most
permissive row of the table, plus dip.

Corroborating secondary sources, all agreeing on "piringan atas" (upper
limb): muhammadiyah.or.id "Hisab Hakiki Wujudul Hilal, Apa dan Bagaimana?"
(2022); Jurnal Tarjih vol. 13(2) 1438H/2016, "Hisab Hakiki Wujūd al-Hilāl
sebagai Penentuan Awal Bulan Kamariah" (quotes the Pedoman's definition);
suaramuhammadiyah.id and muhammadiyahponorogo.or.id explainers.

## Comparison with the library (as of merge of PR #9)

`hijri.h` computes `moon_upper_limb_apparent_altitude_deg` as
topocentric-centre geometric altitude + semidiameter + 34′ refraction:

| Term | Pedoman | library | delta |
|---|---|---|---|
| parallax/topocentric | yes | yes (`hijri_moon_topocentric`) | equivalent to first order |
| semidiameter | yes | yes | none |
| refraction | 34′30″ | 34′00″ | 0.5′, negligible |
| **dip** | **yes, 1.76′√elev** | **omitted** | **16.7′ at 90 m, 0 at sea level** |

Two consequences:

1. The library's Wujudul Hilal is systematically ~17′ *stricter* than the
   official computation whenever the location has elevation — enough to flip
   borderline months. (Muhammadiyah's reference site, Yogyakarta, is at
   ~90 m.)
2. The library's moonset solver crosses at centre = −34′ (no semidiameter,
   no dip), a *different* convention again — the already-logged
   inconsistency, now with an authoritative resolution: under the Pedoman,
   "the Moon has set" means h′b < 0.

## Status

Research only — no code changed by this note. The Phase 1 design (moonset /
upper-limb consistency fix) will implement `HIJRI_PREDICATE_WUJUDUL_HILAL`
per the Pedoman formula, cited here, and align the moonset threshold used by
the equivalence so the two formulations of the criterion ("moon above
horizon at sunset" and "moonset after sunset") provably agree.

Not yet done, deliberately: no baseline measurement of how many historical
months flip under the corrected formula, and no comparison against
Muhammadiyah's announced calendar dates — those belong to the Phase 1 plan
and its fixtures, with the oracle-validation rule from
[2026-08-01-umm-al-qura-oracle.md](2026-08-01-umm-al-qura-oracle.md)
applied before any reference is trusted.

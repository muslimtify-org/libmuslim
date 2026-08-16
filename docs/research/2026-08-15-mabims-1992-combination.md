# MABIMS 1992 (2-3-8) combination rule

## Question

`HIJRI_PREDICATE_MABIMS_1992` evaluates three parameters, altitude at least
2 deg, elongation at least 3 deg, and moon age at least 8 hours. Before this
change the code combined them as `(altitude AND elongation) OR age`. No
source located for this project states that form. This note records what
was found, what it settles, and what it leaves open.

## Sources

1. Kemenag, Direktorat Jenderal Bimas Islam.
   https://bimasislam.kemenag.go.id/post/berita/-sejarah-dan-perkembangan-kriteria-hilal-mabims-dalam-penentuan-awal-bulan-hijriah
   > parameter 2-3-8 mencakup tinggi hilal minimal 2 derajat, elongasi 3
   > derajat, serta umur bulan minimal 8 jam setelah ijtimak
   and
   > Sejak 1992, negara-negara anggota MABIMS menggunakan kriteria imkanur
   > rukyat dengan parameter 2-3-8
   The connector is `serta`, conjunctive.

2. T. Djamaluddin, 2010.
   https://tdjamaluddin.wordpress.com/2010/06/22/kriteria-imkanur-rukyat-khas-indonesia-titik-temu-penyatuan-hari-raya-dan-awal-ramadhan/
   > tinggi hilal minimum 2 derajat, jarak bulan dari matahari minimum 3
   > derajat, dan umur bulan (dihitung sejak saat ijtima') pada saat
   > matahari terbenam minimum 8 jam
   The connector is `dan`, conjunctive.

3. T. Djamaluddin, 2016. Same author as source 2, six years later.
   https://tdjamaluddin.wordpress.com/2016/10/05/menuju-kriteria-baru-mabims-berbasis-astronomi/
   > Kriteria lama MABIMS yang dikenal sebagai kriteria (2,3,8) adalah
   > tinggi minimal 2 derajat, jarak sudut bulan-matahari (elongasi)
   > minimal 3 derajat atau umur bulan minimal 8 jam
   The connector before the last term is `atau`, disjunctive, and it binds
   the last two items (elongation and age).

4. Fitriyani, Isfihani and Octasari, Jurnal Mediasas 7(2), 2024, citing
   Sado (2014), p. 25.
   https://www.journal.staisar.ac.id/index.php/mediasas/article/download/197/191
   The criteria are listed as items a), b) and c), a three-item list with no
   disjunctive connector between any of them. This is also the source used
   here for the supersession instrument, below.

5. T. Djamaluddin, 2023. This source settles the NEW MABIMS criteria only,
   and says nothing about the 1992 criterion.
   https://tdjamaluddin.wordpress.com/2023/01/24/elongasi-kriteria-baru-mabims-toposentrik-atau-geosentrik/
   > Jadi, secara lengkap Kriteria Baru MABIMS adalah tinggi bulan
   > toposentrik 3 derajat dan elongasi toposentrik 6,4 derajat
   and
   > Elongasi geosentrik tidak bisa disandingkan bersama dengan parameter
   > tinggi bulan secara toposentrik

## Tally

Of the four sources describing the 1992 criterion, three word it
conjunctively (source 1 with `serta`, source 2 with `dan`, source 4 as an
a/b/c list), and one words the last two terms disjunctively with `atau`
(source 3, the same author as source 2). The form the code shipped before
this change, `(altitude AND elongation) OR age`, matched none of the four.
It was not consistent with the conjunctive majority, and it was not
consistent with source 3's disjunction either, since source 3's `atau`
binds elongation and age, not altitude-and-elongation against age.

The code was changed to the fully conjunctive form, `altitude AND
elongation AND age`, on the strength of the 3-to-1 majority among these
four secondary sources.

## What this is, and is not

Three convergent secondary sources are not a technical standard. No primary
MABIMS document has been located for this project. This resolution rests on
press-register and personal-blog prose from an involved astronomer and a
government directorate's own summary, not on a specification text. Anyone
who later locates the actual MABIMS instrument should treat this as
provisional and verify against it rather than assume the question was
closed authoritatively by this note.

## What stays open

The elongation frame, geocentric or topocentric, is not stated by any
source located for the 1992 criterion. Source 5 settles the frame for the
NEW MABIMS criteria (altitude 3 deg, elongation 6.4 deg, both topocentric)
only, and is silent on the 1992 criterion. Its geometric argument, that a
geocentric elongation cannot be sensibly paired with a topocentric
altitude, would carry over to the 1992 criterion only if the 1992 altitude
is itself topocentric, and that premise is itself unspecified by any source
found. `hijri.h` keeps `HIJRI_PREDICATE_MABIMS_1992` evaluating a geocentric
elongation, and the code comment keeps this flagged as unresolved rather
than inferring an answer.

## Supersession

Kemenag's Dirjen Bimas Islam circular B-79/DJ.III/HM.00/02/2022, issued 25
February 2022, replaced the 2-3-8 criterion with altitude 3 deg and
elongation 6.4 deg for the four MABIMS states (Brunei, Indonesia, Malaysia,
Singapore). It was first applied for Ramadan 1443 H. NU's Lembaga Falakiyah
also adopted the 3 and 6.4 deg parameters. Muhammadiyah retained wujudul
hilal, which this library ships separately as
`HIJRI_PREDICATE_WUJUDUL_HILAL`. `HIJRI_PREDICATE_MABIMS_1992` is retained
in this library for historical evaluation, not as the currently governing
rule.

## Measured impact

Changing the combination rule from `(altitude AND elongation) OR age` to
`altitude AND elongation AND age` was implemented and measured in a prior
task of this same change:

- The anti-drift invariant in `tests/test_hijri.c` flagged 4 of its 96 grid
  rows as changing verdict under the new combination rule, one at Jakarta
  and three at latitude 60.
- Exactly 1 of the 132 baseline CSV rows in
  `docs/research/hijri-2020-2025-baseline.csv` changed: Bandar Seri Begawan,
  2021-04-12. Its altitude was 2.788482 deg, geocentric elongation 5.154217
  deg, and signed moon age 7.956010 hours, so it passed altitude and
  elongation and missed the 8 hour age threshold by 0.044 hours. Under the
  old combination rule this row passed on the altitude-and-elongation
  branch alone. Under the new rule it fails, since age no longer stands
  alone. The decision went from 1 to 0.
- That row was already flagged in `docs/research/hijri-2020-2025-sources.md`,
  at the row for `M92-BSB-B`, as "near age boundary", years before this
  change was made. The one row that moved under the new combination rule is
  the one this project's own earlier research had already, independently,
  identified as sitting on the boundary.
- Check counts in `test_hijri` went from 2403 to 2499.

# Rule periods for the authority registry

**Date:** 2026-10-10

Retrieval date for every web source below: **2026-10-10**.

Each entry in the `hijri.h` authority registry carries `rule_from` and `rule_until`, the first and last Gregorian dates the library has a source for the entry's rule governing. They err narrow. A date outside them may still have been governed by the rule, and a bound is never guessed. This note records where each bound comes from and what is still missing.

## Summary

| Entry | `rule_from` | `rule_until` | Basis for the bound |
|---|---|---|---|
| Kemenag | 2022-04-03 | open | 1 Ramadan 1443 H, the first month the 3 and 6.4 criterion was applied |
| JAKIM | 2021-11-25 | open | Conference of Rulers agreement, second day |
| MUIS | 2022-05-02 | open | date of the Mufti's Syawal 1443 clarification |
| Brunei ministry | 2024-03-12 | open | first validated month start, no adoption source found |
| Umm al-Qura | 1882-11-12 | 2174-11-25 | coverage of the published table |
| Muhammadiyah, wujudul hilal | open | 2025-06-25 | day before 1 Muharram 1447 H |

## Kemenag

`hijri.h` already cites the Dirjen Bimas Islam circular B-79/DJ.III/HM.00/02/2022 of 25 February 2022, which replaced the 2, 3, 8 criterion with altitude 3 degrees and elongation 6.4 degrees, first applied for Ramadan 1443 H. The Gregorian date of that month start was not recorded in the repository.

Press reports of the itsbat session give it. Tirto, [Keputusan Hasil Sidang Isbat 1 Ramadhan 2022](https://tirto.id/keputusan-hasil-sidang-isbat-1-ramadhan-2022-puasa-minggu-3-april-gqwB), reports that the session on Friday 1 April 2022 decided 1 Ramadan 1443 H falls on Sunday 3 April 2022, and quotes the Minister saying the hilal across Indonesia did not meet "kriteria MABIMS baru, yaitu tinggi hilal 3 derajat dan sudut elongasi 6,4 derajat". Kompas TV, [Kemenag: Posisi Hilal Ramadan 1443 H Masih di Bawah Kriteria MABIMS](https://www.kompas.tv/article/276010/kemenag-posisi-hilal-ramadan-1443-h-masih-di-bawah-kriteria-mabims), gives the same date.

These two were read through search result summaries and not fetched in full, and both are press, not Kemenag's own page. The date is corroborated inside the repository: the Muhammadiyah fixture holds 2022-04-02 for 1 Ramadan 1443 H, and both reports state Muhammadiyah began one day before the government.

Bound: 2022-04-03.

## JAKIM, Malaysia

Two sources, and they frame the start differently.

Utusan Malaysia, [Jakim anjur program Bulan Falak Malaysia Tahun 2021](https://www.utusan.com.my/terkini/2021/08/jakim-anjur-program-bulan-falak-malaysia-tahun-2021/), published 9 August 2021, fetched and read. It quotes JAKIM's Director-General, Datuk Abdul Aziz Jusoh, on the "Pelancaran Penggunaan Kriteria Imkanur Rukyah (KIR) Baharu MABIMS (dalam penyediaan Takwim Hijrah bermula Muharam 1443H)", the launch of the new criterion for preparing the Takwim Hijrah from Muharam 1443 H. The article gives no altitude or elongation figure.

RTM, [Penentuan tarikh Aidiladha dilaksana petang ini](https://berita.rtm.gov.my/nasional/senarai-berita-nasional/senarai-artikel/penentuan-tarikh-aidiladha-dilaksana-petang-ini/), published 27 May 2025, fetched and read. It describes the "kriteria Imkanur Rukyah yang dipersetujui Mesyuarat Majlis Raja-Raja Kali ke-257 pada 24 dan 25 November 2021", gives altitude not less than 3 degrees and elongation not less than 6.4 degrees, and says "Kaedah ini berkuat kuasa sejak 1443H (2022)."

So JAKIM prepared its Takwim under the new criterion from Muharam 1443 H, around August 2021, and the Conference of Rulers, the body whose consent the declaration needs, agreed it on 24 and 25 November 2021. The bound takes the later, narrower event, which is also the one with a stated Gregorian date.

Bound: 2021-11-25.

Gap: the Gregorian date of 1 Muharam 1443 H in Malaysia was not sourced. If it is, and JAKIM's launch is judged the governing event, the bound moves about three and a half months earlier.

## MUIS, Singapore

MUIS, [Syawal clarification by the Office of the Mufti](https://www.muis.gov.sg/resources/media-releases/2-may-22-syawal-clarification-by-the-office-of-the-mufti_mly/), dated 2 May 2022, fetched and read. Paragraph 4 says "Singapura telah berbincang dengan negara-negara MABIMS sejak tahun 2017 dan pada tahun lepas mengemaskini kriteria imkan ar-rukyah kepada kriteria yang lebih baik", Singapore discussed with the MABIMS states since 2017 and last year updated the criterion. "Last year" from a 2022 document is 2021, with no month given.

The document exists to explain the Syawal 1443 H determination under the updated criterion, so the criterion demonstrably governed by its date.

Bound: 2022-05-02.

Gap: the update happened in 2021 and the month is not stated, so the true start is earlier by somewhere between five and sixteen months.

## Brunei

No source from Brunei's own authorities was found for when the criterion began to govern there. A document of the Perak state mufti's office, seen only as a search result summary, says the criterion applied from Muharram 1443 H to Malaysia and the regional states including Brunei. That is a Malaysian state office describing Brunei, and it is not admitted as a source for Brunei's date. `2026-09-25-mabims-takwim-sources.md` records that Brunei's Taqwim for 2024 to 2026 states its MABIMS basis, without saying since when.

Bound: 2024-03-12, the first month start in the Brunei fixture. The entry's comment in `hijri.h` says the true start is earlier and unsourced.

Gap: the whole adoption date. The Taqwim PDFs for 2022 and 2023 at `mora.gov.bn` are the place to look next.

## Umm al-Qura

The bounds are the coverage of the published table as `hijri.h` documents it, 1 Muharram 1300 H to the last day of 1600 H. Outside them the existing function answers from the astronomical reconstruction, and the registry reports that as outside the rule period.

## Muhammadiyah, wujudul hilal

`2026-09-07-khgt-sources.md` records that Muhammadiyah's Tanfidz put KHGT into effect from 1 Muharram 1447 H, 26 June 2025. The last day under wujudul hilal is the day before.

Bound: until 2025-06-25. The start is open, because no source in the repository states when wujudul hilal began to govern. The fixture behind this entry covers 1443 to 1446 H only.

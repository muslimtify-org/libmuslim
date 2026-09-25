# Takwim sources for Singapore, Malaysia and Brunei

**Date:** 2026-09-25

This note asks whether a Singapore, Malaysia or Brunei national Hijri calendar
(Takwim) can be admitted as a validated oracle, the same way
[2026-08-01-kemenag-reference.md](2026-08-01-kemenag-reference.md) admitted
Indonesia's Kemenag calendar. It records what each country's religious
authority actually publishes, what month starts were obtained, and how each
figure was independently cross-checked, per the oracle rule in
[2026-08-01-umm-al-qura-oracle.md](2026-08-01-umm-al-qura-oracle.md): no
oracle is admitted without a cross-check against values known from outside
the reference itself. It deliberately does not measure anything against this
library. No predicate was run and no date was scored against any calendar
here. `docs/research/hijri-2020-2025-sources.md` records three earlier
rejected candidates for these same three countries (`M21-SG-F`, `M21-KUL-B`,
`M21-BSB-B`), rejected for source precision, provenance and (for Brunei)
naming the wrong national authority. This note pursues a different kind of
source, dated official month-start figures rather than single numerical hilal
readings, and is not bound by those rejections, though it was read first so
the same dead ends are not repeated.

The window aimed for was 2024 to 2026, to match the Kemenag note. All three
countries yielded a shorter run, seven month starts each, 1445H through
1447H (2024 to 2026), rather than the 37 Kemenag obtained.

## Singapore

MUIS (Majlis Ugama Islam Singapura) publishes two things. An annual printed
Hijri calendar, distributed as a PDF from
`muis.gov.sg/resources/islamic-calendar/` for 2024, 2025 and 2026. And,
separately, individually dated "Announcement by Mufti" media releases, issued
close to the event, in both English and Malay as separate pages, stating the
Gregorian date, the Hijri date, and the astronomical values behind the
decision. For example, the Ramadan 1446H announcement states the moon at
sunset on 28 February 2025 at 5.1 degrees elongation and 4.3 degrees
altitude, explicitly measured against "the criteria of imkanur rukyah as
agreed upon by the member countries of MABIMS" (MABIMS 2021: altitude at
least 3 degrees, elongation at least 6.4 degrees). Altitude passed, elongation
did not, so Syaaban was completed to 30 days.

The annual calendar PDFs are image-laid-out documents. Neither this
session's WebFetch tool nor a local `pdftotext` pass could extract readable
text from them, and one direct download was blocked by the hosting CDN, so
the printed calendar's own dates were not independently re-extracted in this
note. The figures below come entirely from the separate announcement pages.

### The seven month starts obtained

| Hijri event | Gregorian date | Source |
|---|---|---|
| 1 Ramadan 1445H | Tue 12 Mar 2024 | announcement dated 10 Mar 2024 |
| 1 Syawal 1445H | Wed 10 Apr 2024 | announcement, English and Malay pages |
| 1 Ramadan 1446H | Sun 2 Mar 2025 | announcement dated 28 Feb 2025 |
| 1 Syawal 1446H | Mon 31 Mar 2025 | announcement, English and Malay pages |
| 1 Zulhijjah 1446H | Thu 29 May 2025 | announcement, also gives Hari Raya Aidiladha Sat 7 Jun 2025 |
| 1 Ramadan 1447H | Thu 19 Feb 2026 | announcement, moonset 4 minutes before sunset |
| 1 Syawal 1447H | Sat 21 Mar 2026 | announcement, English and Malay pages |

All at `muis.gov.sg/resources/media-releases/`, one page per event.

### Cross-check

Each announcement exists as two separately published pages, English and
Malay, issued by the same office but as distinct documents, agreeing on
every date checked. Independent, non-MUIS press reporting, published on a
separate date from the MUIS release, corroborates two of the dates directly.
Mothership.sg's 2 March 2025 article cites the 28 February MUIS release by
name. Malay Mail's 9 April 2024 article, reporting that Singapore, Indonesia
and Thailand would mark Aidilfitri "tomorrow", matches the 10 April 2024
Syawal figure.

What this note did not obtain is the kind of cross-check Kemenag's note used,
an annual printed calendar cross-checked against the year's separate
announcements. The printed MUIS calendar could not be read, so the
cross-check here is between the announcement documents themselves and
outside press reporting, not between two differently produced official
artifacts.

### Encodes national decision

The month starts are a Mufti and Fatwa Committee decision applying the
MABIMS criterion, stated with the same authority Indonesia's itsbat carries,
not a raw, untouched predicate output. Any fixture claim must be worded as
reproducing MUIS's announced month starts, not the MABIMS predicate's raw
output.

### Verdict

Admitted. Seven separately dated official announcement documents across
2024 to 2026, in English and Malay twin pairs, with two of the seven
independently corroborated by non-government press reporting published on a
different date, is a genuine cross-check against values from outside a
single reference document. The gap is that the more calendar-shaped artifact
analogous to Kemenag's, the annual printed PDF, could not be read, so a
future attempt should retry extracting it, ideally with OCR rather than text
extraction.

## Malaysia

Malaysia's month starts are not decided by JAKIM (Jabatan Kemajuan Islam
Malaysia) alone. JAKIM's Expert Panel on Falak (Panel Pakar Falak) computes
the position of the moon against Malaysia's own KIR2021 criterion (Kaedah
Imkanur Rukyah 2021, adopted by the Conference of Rulers on 24 to 25
November 2021, the same 3 degrees altitude and 6.4 degrees elongation
threshold as MABIMS 2021), and organises a physical rukyah program at 29
observation sites nationwide. The actual declaration, combining both, is made
by the Penyimpan Mohor Besar Raja-Raja (Keeper of the Great Seal of the
Rulers), with the consent of the Yang di-Pertuan Agong and the Conference of
Rulers, broadcast live on RTM (Radio Televisyen Malaysia) television and
radio. `islam.gov.my` hosts JAKIM's procedural media statements, the
observation schedule and the criteria, but this note found no persistent
JAKIM or Istana Negara page stating the final decided date. The declaration
itself is a one-time broadcast.

That broadcast is reported, the same evening, as a news article on
`berita.rtm.gov.my`, the official state broadcaster's own news portal, a
`.gov.my` domain distinct from `islam.gov.my`. That is the primary source
used below. A first pass of secondary Malaysian aggregator blogs produced one
outright contradiction, a site claiming Ramadan 1446H began 1 March 2025
(Saudi Arabia's date) rather than 2 March, which the RTM article and every
other source checked, including the criteria numbers JAKIM published (moon
at 4 degrees 15 minutes altitude, 5 degrees 17 minutes elongation, both short
of KIR2021), rule out. This is recorded as a caution about secondary
Malaysian sites generally, and as the reason the RTM portal was preferred
over blog aggregation for every figure below.

### The seven month starts obtained

| Hijri event | Gregorian date | Source |
|---|---|---|
| 1 Ramadan 1445H | Tue 12 Mar 2024 | RTM, Sinar Harian |
| 1 Syawal 1445H | Wed 10 Apr 2024 | Facebook/TikTok record of the Penyimpan Mohor Besar Raja-Raja broadcast, Malay Mail |
| 1 Ramadan 1446H | Sun 2 Mar 2025 | berita.rtm.gov.my |
| 1 Syawal 1446H | Mon 31 Mar 2025 | berita.rtm.gov.my, Malay Mail |
| 1 Zulhijjah 1446H | Thu 29 May 2025 | berita.rtm.gov.my (states the date directly), Hari Raya Aidiladha Sat 7 Jun 2025 |
| 1 Ramadan 1447H | Thu 19 Feb 2026 | berita.rtm.gov.my, article dated 17 Feb 2026 |
| 1 Syawal 1447H | Sat 21 Mar 2026 | berita.rtm.gov.my |

Every one of these seven dates is identical to the Singapore figure for the
same Hijri event.

### Cross-check

The primary source is `berita.rtm.gov.my`, the state broadcaster's own
archived report of its own live declaration. Independent corroboration comes
from Malaysia's other national press, not government owned, reporting the
same dates on the same or adjacent days: Sinar Harian and Utusan Malaysia for
the Aidiladha 2025 date, Malay Mail for the Syawal 1445H and 1446H dates.
Cross-country agreement with Singapore's separately announced figures, and
with Brunei's below, for all seven events, is additional corroboration, since
the three authorities decide independently even when using a shared regional
criterion. (Indonesia's Kemenag note recorded 1 Ramadan 1446H as 1 March
2025, one day earlier than Singapore, Malaysia and Brunei's 2 March. This is
a real, documented divergence between MABIMS countries in the same year, not
an error here. Each nation's rukyah acceptance is its own decision even under
a nominally shared criterion.)

### Encodes national decision

Explicitly both hisab and physical rukyah, decided by royal proclamation.
This is the strongest national-decision layer of the three countries in this
note, since Malaysia's own description of its method names actual sighting
observation as a co-equal input, not merely a criterion check.

### Verdict

Admitted. The RTM state broadcaster's own archived news portal, corroborated
by two other national newspapers and by the cross-country agreement with
Singapore and Brunei, is a genuine cross-check against values from outside
a single reference document. The one caution is that no stable JAKIM or
Istana Negara page states these dates directly, so this fixture's provenance
rests on the broadcaster's report of its own broadcast, not on a
government-authored document with the date printed on it.

## Brunei

The national authority is Pusat Da'wah Islamiah (PDI), a department of
Kementerian Hal Ehwal Ugama (KHEU, the Ministry of Religious Affairs),
hosted at `mora.gov.bn`. This matters because the previous rejected Brunei
candidate, `M21-BSB-B`, was rejected partly for naming the wrong national
authority, having used an Indonesian rule (`MABIMS-RI-RULE`) as if it applied
to Brunei. `mora.gov.bn` also hosts a separate department, Jabatan Hal Ehwal
Syar'iah, whose own page states its function is halal food certification,
family advisory services and religious law enforcement, not calendar
publication, so that department was checked and ruled out before settling on
PDI.

PDI publishes an annual "Taqwim" PDF, one per year, at
`mora.gov.bn/Muat Turun Library/PDI/`. Unlike the Singapore and Malaysia
calendar-shaped documents, these are genuine text-layer PDFs, and
`pdftotext -layout` extracted their content directly for 2024, 2025 and 2026.
The Taqwim states its own basis in its introduction: "ru'yah dan taqwim Islam
serantau iaitu Negara Brunei Darussalam, Indonesia, Malaysia dan Singapura
(MABIMS)" (regional rukyah and Islamic calendar coordination among Brunei,
Indonesia, Malaysia and Singapore, MABIMS). Every predicted month start in
the Taqwim carries the note "(Tertakluk kepada pengumuman penglihatan anak
bulan ...)" (subject to the announcement of the moon sighting), meaning the
printed calendar is a prediction, not the final word. The final word is a
royal proclamation, made with the consent of the Sultan and broadcast on RTB
(Radio Television Brunei).

### The seven month starts obtained

| Hijri event | Gregorian date | Source |
|---|---|---|
| 1 Ramadan 1445H | Tue 12 Mar 2024 | Taqwim 2024, Borneo Bulletin |
| 1 Syawal 1445H | Wed 10 Apr 2024 | Taqwim 2024 (extracted directly, line 56/409) |
| 1 Ramadan 1446H | Sun 2 Mar 2025 | Taqwim 2025 (line 28/280), Borneo Bulletin |
| 1 Syawal 1446H | Mon 31 Mar 2025 | Taqwim 2025 (line 26/280/338), Borneo Bulletin |
| 1 Zulhijjah 1446H | Thu 29 May 2025 | Taqwim 2025, states 10 Zulhijah as Hari Raya Idul Adha, Sat 7 Jun 2025 |
| 1 Ramadan 1447H | Thu 19 Feb 2026 | Taqwim 2026 (line 58 to 59) |
| 1 Syawal 1447H | Sat 21 Mar 2026 | Taqwim 2026 (line 82 to 84) |

### Cross-check

Two independently produced artifacts agree for every one of the six 2024 and
2025 dates that has already passed: PDI's Taqwim, published in advance of the
year with a predicted date, and Borneo Bulletin, a private national
newspaper, reporting the actual RTB broadcast after the fact. This is
structurally the closest of the three countries in this note to the Kemenag
cross-check, a computed calendar checked against a separately timed,
separately produced announcement. For the two 2026 dates, which had also
already passed by this note's retrieval date, the Taqwim's predicted dates
match Singapore's and Malaysia's independently announced dates for the same
events.

### Encodes national decision

Explicitly flagged in the Taqwim itself, every predicted month start carries
the "subject to the announcement" qualifier, and the actual date is a royal
proclamation with the Sultan's consent, broadcast on RTB. The Taqwim's
printed date and the announced date agreed in every case checked here, but
the document does not claim they must.

### Verdict

Admitted. This is the best-sourced of the three countries in this note: a
genuine text-layer government PDF read directly, rather than summarized
secondhand, cross-checked against a separately produced national newspaper
report of the actual broadcast, for six of seven dates, plus agreement with
two other countries' independently announced dates for the remaining two.

## Consequences

Singapore: a fixture cycle is now possible. Seven admitted month starts,
2024 to 2026, cross-checked as described above.

Malaysia: a fixture cycle is now possible. Seven admitted month starts,
2024 to 2026, identical in every case to Singapore's figures for the same
Hijri event, cross-checked as described above.

Brunei: a fixture cycle is now possible. Seven admitted month starts, 2024
to 2026, identical in every case to Singapore's and Malaysia's figures for
the same Hijri event, cross-checked as described above, with the most
directly read primary source of the three.

None of this decides whether a fixture cycle for any of the three countries
is worth building. No comparison against this library's predicate was made
in this note, and none should be inferred from it.

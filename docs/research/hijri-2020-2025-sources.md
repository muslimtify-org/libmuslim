# Hijri 2020–2025 Reference Research

Retrieval date for every web source below: **2026-07-29**.

## Rules

Permanent fixtures must be offline and reproducible. Each admitted fixture
records the authority or original paper, retrieval date, coordinates,
elevation, civil date, evaluation instant, time scale, topocentric/geocentric
convention, expected value, source precision, tolerance rationale, and policy
scope. A secondary calculator can corroborate a value but cannot be its only
provenance.

Disposition is `admit` only when the expected value and its convention are
unambiguous. It is `reject` when observation, geographic aggregation, policy,
or an unavailable convention prevents deterministic reproduction. No
numerical value in the generated baseline is treated as reference truth.

## Confirmed sources

| ID | Subject | Primary or authority source | What it establishes |
|---|---|---|---|
| MABIMS-RI-RULE | MABIMS history | Kementerian Agama RI, [Sejarah dan Perkembangan Kriteria Hilal MABIMS](https://kemenag.go.id/en/nasional/sejarah-dan-perkembangan-kriteria-hilal-mabims-dalam-penentuan-awal-bulan-hijriah-8kdmn) | Old 2°–3°–8 h rule; new 3°/6.4° rule; Indonesian use of the new rule from 2022; final Indonesian decisions also use observation and an official meeting. |
| MABIMS-MY-2019 | New MABIMS rule | JAKIM, [Pertemuan Pakar Falak MABIMS 2019](https://www.islam.gov.my/en/berita/1084-pertemuan-pakar-falak-mabims-2019) | 3° altitude and 6.4° elongation agreement. |
| MABIMS-SG-2022 | Singapore implementation | MUIS, [Syawal 2022 clarification](https://www.muis.gov.sg/resources/media-releases/2-may-22-syawal-clarification-by-the-office-of-the-mufti_mly/) | Singapore adopted the revised method; wording is “exceed” 3° and 6.4°, not `>=`. |
| MABIMS-SG-2023 | National-policy distinction | MUIS, [Statement on Syawal 2023](https://www.muis.gov.sg/resources/media-releases/9-apr-23-statement-in-response-to-queries-on-determining-of-syawal-2023-1444h/) | MABIMS members can produce different dates because their national decision processes differ. |
| WUJUD-RULE | Wujudul Hilal | Muhammadiyah, [Hisab Hakiki Wujudul Hilal](https://muhammadiyah.or.id/2022/02/hisab-hakiki-wujudul-hilal-apa-dan-bagaimana/) | Conjunction, conjunction before sunset, and the Moon's upper limb above the horizon at sunset are cumulative requirements. |
| WUJUD-2022-ZH | Published numerical example | Muhammadiyah, [Awal Zulhijah 1443](https://muhammadiyah.or.id/2022/06/awal-zulhijah-berdasarkan-kriteria-wujudul-hilal-dan-kalender-islam-global/) | On 2022-06-29: conjunction 09:55:07 WIB; Yogyakarta lunar altitude +01°58′28″; Moon above horizon throughout Indonesia. The page does not state coordinates, elevation, refraction, limb correction, ephemeris, or rounding. |
| WUJUD-2025-R | Published decision | Muhammadiyah, [Ramadan, Syawal, and Zulhijah 1446](https://muhammadiyah.or.id/2025/02/maklumat-pimpinan-pusat-muhammadiyah-tentang-penetapan-hasil-hisab-ramadan-syawal-dan-zulhijah-1446-h/) | 2025-02-28 conjunction 07:46:49 WIB and Moon above horizon throughout Indonesia at sunset. |
| UMM-OFFICIAL | Umm al-Qura authority | KACST-maintained [Umm al-Qura Calendar](https://www.ummulqura.org.sa/en) | Official Saudi civil calendar; Holy Mosque coordinates are the calculation reference. The public page does not specify the event algorithms or expose historical numerical event data. |
| EGYPT-OFFICIAL | Egyptian month decision | Dar al-Ifta al-Misriyyah, [Relying on physical sightings](https://www.dar-alifta.org/en/article/details/516/relying-on-physical-sightings-of-the-new-moon-is-the-principle-in-determining-the) | Official religious month determination is physical sighting at sunset on Sha'ban 29. It does not document a deterministic `moon lag >= 5 min` calendar rule. |
| TURKEY-2016 | Turkey/2016 global criterion | Diyanet, [International Hijri Calendar Union Congress declaration](https://istanbul.diyanet.gov.tr/sayfalar/contentdetail.aspx?contentid=233&menucategory=kurumsal) | Congress/policy provenance; the decision is a unified/global calendar rather than independent tests at Turkish cities. |
| TURKEY-DETAIL | 5°/8° global policy details | Muhammadiyah, [Awal Zulhijah and Istanbul criterion](https://muhammadiyah.or.id/2022/06/awal-zulhijah-berdasarkan-kriteria-wujudul-hilal-dan-kalender-islam-global/) | Visibility anywhere before 00:00 UT, 5° altitude and 8° elongation, plus a post-00:00 UT exception. This is an authority explanation, but not Diyanet's technical specification. |
| ODEH-2004 | Odeh classifier | M. S. Odeh, “New Criterion for Lunar Crescent Visibility,” *Experimental Astronomy* 18, 39–64, [DOI 10.1007/s10686-005-9002-5](https://doi.org/10.1007/s10686-005-9002-5) | 737 observations; airless topocentric ARCV and topocentric crescent width at best time `sunset + 4/9 lag`; four zones and polynomial. |
| YALLOP-1998 | Yallop classifier | B. D. Yallop, “A Method for Predicting the First Sighting of the New Crescent Moon,” NAO Technical Note 69, updated April 1998 | Best-time method, q polynomial, and six original zones. **Obtained 2026-07-30** from two independent mirrors (Utrecht/van Gent and IAC/ICOP); still not the original publisher, whose site fails certificate validation. Table 4 carries 295 observations 1859-1996 with Yallop's own ARCL/ARCV/DAZ/age/lag/parallax/W'/q and the observed outcome. HMNAO reserves copyright and requires a licence for commercial use, so his computed columns cannot be reproduced here; the observations themselves are credited to Schaefer. See [2026-07-30-findings.md](2026-07-30-findings.md). |

No official ECFR or FCNA/ISNA primary technical document supporting the
header's shared **local** `altitude >= 5° && elongation >= 8°` rule was found.
Absence of a source is recorded as a gap, not evidence that the rule is false.

## Interpretation audit of `hijri.h`

| Model or predicate | Corrected API interpretation | Correction disposition | Remaining fixture-admission gap |
|---|---|---|---|
| MABIMS 1992 | Local predicate using Moon-centre geometric altitude, geocentric elongation, and signed age relative to the relevant conjunction | Corrected: pre-conjunction evenings now have negative age and cannot pass through a previous-lunation age. | Official decisions can additionally depend on observation; verify inclusive boundary semantics from a technical standard. |
| MABIMS 2021 | Local predicate using Moon-centre topocentric altitude and **topocentric** elongation, both inclusive | **Superseded 2026-07-30.** The predicate consumed the geocentric field; that was wrong and is fixed. T. Djamaluddin (formerly LAPAN) states both parameters are topocentric, and pairing a geocentric elongation with a topocentric altitude measures them from different origins. Measured impact: 0.7271 deg mean difference, 6 of 132 baseline rows flip. See [2026-07-30-findings.md](2026-07-30-findings.md). | MUIS wording says “exceed” while `hijri.h` uses `>=`; centre-vs-limb and refraction still unspecified; primary technical standard still not located. |
| Wujudul Hilal | Local predicate using conjunction-before-sunset and apparent upper-limb altitude above the horizon | Corrected: the centre and upper-limb fields are separate and the local predicate consumes the upper limb. | A caller must still implement any geographic or national aggregation policy. |
| Umm al-Qura | Dedicated `hijri_umm_al_qura_from_gregorian()` policy evaluated only at the fixed Mecca reference | Corrected: the public function accepts no caller location; the neutral conjunction/moonset predicate remains separate. | KACST event definitions and historical numerical event data remain unavailable. |
| Five-minute lag | Neutral local `HIJRI_PREDICATE_LAG_AT_LEAST_5_MINUTES` | Corrected: the unsupported Egypt authority name and claim were removed. | No claim about the official Egyptian calendar is made. |
| Local 5°/8° | Neutral local `HIJRI_PREDICATE_ALTITUDE_5_ELONGATION_8` | Corrected: Turkey/ICOP, ECFR, and ISNA authority names and mappings were removed. | No global or authority policy is inferred from this local calculation. |
| Yallop | Dedicated best-time result with q and six zones A–F | Corrected: the probe calls the model-specific evaluator rather than classifying generic sunset fields. | A stable original-publisher source and matched modern observations are still needed for real-case fixtures. |
| Odeh | Dedicated best-time result with airless topocentric ARCV, topocentric width, V, and four zones | Corrected: the probe calls the model-specific evaluator at `sunset + 4/9 lag`. | Matched published observation/classification cases are still needed. |

## Baseline observations

| ID | Case | Convention | Corrected observation | Category | Disposition |
|---|---|---|---|---|---|
| OBS-CONJUNCTION-AGE | Evenings immediately before the relevant astronomical conjunction | Age must be relative to the conjunction whose crescent is being assessed and is negative before it. | Corrected. The 2020-05-22 and 2023-03-21 rows now report small negative signed ages and MABIMS 1992 no longer passes through the previous-lunation age branch. | formula/orchestration | resolved in implementation; rows remain diagnostics, not reference fixtures |
| OBS-MOONSET-NAN | Several pre-conjunction baseline evenings | A failed post-sunset moonset search is not itself proof that no relevant moonset exists; event-search interval/status must be specified. | Clarified. `sunset_status` and `moonset_status` are emitted explicitly; `moonset_jd`, lag, q, and V remain `nan` when the event/model result is unavailable. | event-solver/convention | explicit diagnostic status added; independent event-solver validation is still required |
| OBS-YALLOP-TIME | Every real Yallop classification in the baseline | Yallop quantities are evaluated at prescribed best time. | Corrected. `yallop_model_q` and `yallop_model_zone` come from `hijri_yallop_evaluate_evening()`, including all six zones and `UNAVAILABLE` when inputs are unavailable. | evaluation-time/convention | resolved in implementation; real-case fixture still rejected for missing matched evidence |
| OBS-ODEH-TIME | Every real Odeh classification in the baseline | Odeh uses airless topocentric quantities at best time. | Corrected. `odeh_model_v` and `odeh_model_zone` come from `hijri_odeh_evaluate_evening()` at model best time and report `UNAVAILABLE` when inputs are unavailable. | evaluation-time/convention | resolved in implementation; real-case fixture still rejected for missing matched evidence |
| OBS-UMM-LOCATION | Umm al-Qura evaluation outside Mecca | Calendar decision site is the Holy Mosque in Mecca. | Corrected. The public Umm al-Qura conversion accepts no observer location; the Mecca probe row is explicitly the neutral conjunction-and-moonset predicate. | geographic policy | resolved API scope; historical numerical identity remains unverified |

## Regenerated baseline change audit

All 132 data rows changed structurally because the ambiguous `criterion`
column was replaced by an explicit local `predicate` plus dedicated Yallop and
Odeh model columns. The locations and six civil dates are unchanged.

| Changed rows | Explanation | Disposition |
|---|---|---|
| All 132 | Geographic `group` labels are neutral; Egypt, Turkey/ICOP, ECFR, and ISNA are not emitted as local authority mappings. | Intended breaking-name correction. |
| All 132 | Sunset/moonset statuses, Moon-centre and upper-limb altitudes, both elongation frames, and signed age are explicit. | Intended diagnostic-schema correction. |
| Pre-conjunction rows, notably 2020-05-22 and 2023-03-21 | Relevant-conjunction selection changes age from roughly one lunation old to a small negative value and changes dependent local decisions. | Previous-lunation defect resolved. |
| All rows with available sunset and moonset | Yallop q/zone and Odeh V/zone are recomputed by their dedicated best-time evaluators. | Previous generic-sunset classification defect resolved. |
| Rows without an available moonset result | q, V, and model zones are reported as unavailable rather than presenting a default zone as a computed classification. | Availability is now explicit; solver accuracy remains a research question. |

## Candidate fixture matrix

“Corrected diagnostic” values are outputs from
`hijri-2020-2025-baseline.csv`; they are deliberately rounded here and never
serve as expected values. “Near” means near a threshold according to the
corrected implementation, not according to an independent reference. The
authority-oriented candidate IDs below are retained only to preserve the
research history; they are not names emitted by the probe.

| ID | Criterion/model | Location | Civil evening | Primary source | Evaluation instant and convention | Reference value | Corrected diagnostic | Difference | Category | Disposition |
|---|---|---|---|---|---|---|---|---|---|---|
| M92-JKT-P | MABIMS 1992 | Jakarta | 2021-04-12 | MABIMS-RI-RULE | Sunset; source does not give site/conventions | Rule only; clear pass candidate | Local predicate uses centre altitude 3.197°, geocentric elongation 5.134°, and signed age 8.039 h; pass | Numerical reference unavailable | provenance | reject |
| M92-KUL-N | MABIMS 1992 | Kuala Lumpur | 2022-04-01 | MABIMS-RI-RULE | Sunset; source does not give site/conventions | Rule only; clear fail candidate | Corrected local predicate result retained in CSV | Numerical reference unavailable | provenance | reject |
| M92-BSB-B | MABIMS 1992 | Bandar Seri Begawan | 2021-04-12 | MABIMS-RI-RULE | Sunset; source does not give site/conventions | Rule only; near age boundary | Corrected local predicate uses explicit centre altitude, geocentric elongation, and signed age | Numerical reference and official boundary convention unavailable | convention | reject |
| M21-SG-F | MABIMS 2021 | Singapore | 2022-04-01 | MABIMS-SG-2022 | Sunset on day 29; national calculation | Official outcome: crescent not visible; numerical threshold status not explicit | Corrected local predicate result retained in CSV | Source does not explicitly confirm the calculated threshold result | source precision | reject |
| M21-JKT-C | MABIMS 2021 | Jakarta | 2021-04-12 | MABIMS-RI-RULE | New rule not yet used by Indonesia | Rule only; counterfactual near-altitude case | Centre altitude 3.197° and geocentric elongation 5.134°; fail | No applicable official decision | historical policy | reject |
| M21-KUL-B | MABIMS 2021 | Kuala Lumpur | 2024-04-08 | MABIMS-MY-2019 | Sunset; source has no numerical case | Rule only; candidate pass/fail to be determined | Corrected local predicate result retained in CSV | Numerical reference unavailable | provenance | reject |
| M21-BSB-B | MABIMS 2021 | Bandar Seri Begawan | 2025-02-28 | MABIMS-RI-RULE | Sunset; Brunei observation policy not specified by this source | Rule only | Corrected local predicate result retained in CSV | Wrong national authority and no numerical reference | geographic policy | reject |
| WUJ-YOG-P | Wujudul Hilal | Yogyakarta | 2022-06-29 | WUJUD-2022-ZH | Sunset; upper limb; unspecified coordinates/refraction | conjunction 09:55:07 WIB; altitude +01°58′28″ | Date absent from baseline | Cannot compare without regenerating matched case; source convention incomplete | convention | reject |
| WUJ-YOG-F | Wujudul Hilal | Yogyakarta | 2023-03-21 | WUJUD-RULE | Sunset; upper limb | Rule only | Upper-limb altitude −2.451° with small negative signed age; local fail | Numerical reference unavailable | provenance | reject |
| WUJ-MKS-P | Wujudul Hilal | Makassar | 2025-02-28 | WUJUD-2025-R | Sunset; upper limb; national claim | Moon above horizon throughout Indonesia | Corrected upper-limb local predicate result retained in CSV | No site-specific numerical value | source precision | reject |
| WUJ-JYP-B | Wujudul Hilal | Jayapura | 2022-04-01 | WUJUD-RULE | Sunset; upper limb | Rule only; near-horizon candidate | Corrected upper-limb local predicate result retained in CSV | Independent upper-limb reference unavailable | convention | reject |
| UMM-MEC-P | Umm al-Qura | Mecca | 2022-04-01 | UMM-OFFICIAL | Holy Mosque reference; official algorithms unspecified | Official calendar date can be retrieved, event values cannot | Probe emits the neutral Mecca conjunction/moonset predicate; dedicated policy is location-free | End-date alone cannot validate both predicates | provenance | reject |
| UMM-MEC-F | Umm al-Qura | Mecca | 2020-05-22 | UMM-OFFICIAL | Holy Mosque reference | No historical event values exposed | Relevant conjunction now selected; explicit moonset status is unavailable and local predicate fails | Historical event reference unavailable | provenance | reject |
| UMM-MEC-B | Umm al-Qura | Mecca | 2024-04-08 | UMM-OFFICIAL | Holy Mosque reference | Calendar result only | Probe emits the neutral Mecca conjunction/moonset predicate | Event definitions/precision unavailable | convention | reject |
| EGY-CAI-P | Egypt | Cairo | 2022-04-01 | EGYPT-OFFICIAL | Physical sighting at sunset on Sha'ban 29 | No deterministic five-minute value | Cairo row is explicitly the neutral five-minute lag predicate; pass | Different policy | policy | reject |
| EGY-ALX-F | Egypt | Alexandria | 2020-05-22 | EGYPT-OFFICIAL | Physical sighting | No deterministic five-minute value | Alexandria row is explicitly neutral and includes unavailable event status | Different policy and unavailable event result | policy | reject |
| EGY-ASW-B | Egypt | Aswan | 2025-02-28 | EGYPT-OFFICIAL | Physical sighting | No deterministic five-minute value | Aswan row is explicitly the neutral five-minute lag predicate | Different policy | policy | reject |
| TUR-IST-F | Turkey global | Istanbul | 2021-04-12 | TURKEY-2016, TURKEY-DETAIL | Must search whole Earth before 00:00 UT; local row is insufficient | Global decision, not Istanbul predicate | Istanbul row is explicitly the neutral local 5°/8° predicate; fail | Scope mismatch | geographic policy | reject |
| TUR-ANK-F | Turkey global | Ankara | 2022-04-01 | TURKEY-2016, TURKEY-DETAIL | Global/time-bounded | Global decision, not Ankara predicate | Ankara row is explicitly the neutral local 5°/8° predicate; fail | Scope mismatch | geographic policy | reject |
| TUR-ERZ-B | Turkey global | Erzurum | 2025-02-28 | TURKEY-2016, TURKEY-DETAIL | Global/time-bounded with land exception | Global decision, not Erzurum predicate | Erzurum row is explicitly the neutral local 5°/8° predicate | Scope and primary technical conventions unavailable | geographic policy | reject |
| ECFR-LON-C | ECFR | London | 2021-04-12 | No primary technical source found | Unknown | Unknown | London row is explicitly the neutral local 5°/8° predicate; fail | No ECFR authority claim remains | provenance | reject |
| ECFR-NYC-C | ECFR | New York | 2022-04-01 | No primary technical source found | Unknown | Unknown | New York row is explicitly the neutral local 5°/8° predicate; pass | No ECFR authority claim remains | provenance | reject |
| ISNA-TOR-C | FCNA/ISNA | Toronto | 2024-04-08 | No primary technical source found | Unknown | Unknown | Toronto row is explicitly the neutral local 5°/8° predicate | No FCNA/ISNA authority claim remains | provenance | reject |
| YAL-JKT-F | Yallop | Jakarta | 2022-04-01 | YALLOP-1998 | Best time; original airless/topocentric conventions | Published formula only; no 2022 observation/classification at site | Dedicated model q/zone at best time retained in CSV | Evaluation-time defect resolved; no modern primary observed class | provenance | reject |
| YAL-LON-C | Yallop | London | 2021-04-12 | YALLOP-1998 | Best time | Published formula only | Dedicated model q/zone at best time retained in CSV | No modern primary observed class | provenance | reject |
| YAL-NYC-P | Yallop | New York | 2022-04-01 | YALLOP-1998 | Best time | Published formula only | Dedicated model q/zone at best time retained in CSV | Evaluation-time defect resolved; no published zone for matched case | provenance | reject |
| YAL-SYD-B | Yallop | Sydney | 2024-04-08 | YALLOP-1998 | Best time | Published formula only | Model result explicitly unavailable because required event input is unavailable | No modern primary observed class | provenance | reject |
| ODE-JKT-F | Odeh | Jakarta | 2022-04-01 | ODEH-2004 | `sunset + 4/9 lag`; airless topocentric ARCV and topocentric width | Published formula only | Dedicated model V/zone at best time retained in CSV | Evaluation-time defect resolved; no matched modern observation | provenance | reject |
| ODE-MEC-C | Odeh | Mecca | 2021-04-12 | ODEH-2004 | Best time; airless topocentric | Published formula only | Dedicated model V/zone at best time retained in CSV | No matched modern observation | provenance | reject |
| ODE-NYC-P | Odeh | New York | 2022-04-01 | ODEH-2004 | Best time; airless topocentric | Published formula only | Dedicated model V/zone at best time retained in CSV | Evaluation-time defect resolved; no matched published zone | provenance | reject |
| ODE-SYD-B | Odeh | Sydney | 2024-04-08 | ODEH-2004 | Best time; airless topocentric | Published formula only | Model result explicitly unavailable because required event input is unavailable | No matched modern observation | provenance | reject |

Every candidate row ends in `reject`. Consequently there are no admitted
numerical values to cross-check and no tolerance is proposed. This is the
intended gate outcome: the source inventory supports permanent synthetic
formula/threshold tests, but does **not** yet support authoritative real-evening
numerical fixtures for the current API.

## Independent cross-check status

No candidate was admitted, so a numerical cross-check against JPL Horizons,
JPL DE440, or another high-precision ephemeris would not cure the missing
authority conventions. A later cross-check must name the ephemeris and version,
time scale, Earth orientation data, atmospheric model, limb/refraction choice,
and site coordinates. Agreement between two calculators is insufficient when
the authority's definition is unknown.

## Required evidence still missing

- Official 2020–2025 numerical hisab tables with coordinates and conventions
  for MABIMS member states, including clear pass, fail, and boundary evenings.
- The technical MABIMS standard defining topocentric altitude, geocentric
  elongation, equality/rounding, refraction, limb, and geographic aggregation.
- Muhammadiyah numerical tables that specify the Yogyakarta markaz,
  upper-limb/refraction model, ephemeris, and precision.
- KACST's Umm al-Qura event algorithms and historical event values for Mecca.
- A primary Egyptian source for a five-minute computational rule, if one
  exists; otherwise the enum/documentation must stop calling it the official
  Egyptian criterion.
- Diyanet's primary technical proceedings for the 2016 global criterion,
  including coordinate conventions and the after-00:00 UT exception.
- Separate primary policy specifications for ECFR and FCNA/ISNA.
- Modern 2020–2025 observation records or official visibility maps giving
  location, time, instrument, outcome, and Yallop/Odeh zone. Original formula
  papers alone cannot supply modern golden classifications.

## Required output for the fixture implementation plan

A follow-up real-evening fixture plan may be written only after:

- every supported criterion/model has three or more relevant locations, except
  Umm al-Qura's single Mecca decision site;
- each group contains independently confirmed clear-pass, clear-fail, and
  near-boundary cases;
- Yallop and Odeh cases identify a published classification zone;
- all admitted cases have numerical reference values and justified tolerances;
- every rejected case states why it cannot be a deterministic assertion;
- formula or orchestration defects discovered here have a minimal proposed
  correction and a regression case.

The current evidence satisfies the rejection/documentation gate but not the
admission gate. The permanent suite should retain exact arithmetic and
synthetic criterion tests while research continues.

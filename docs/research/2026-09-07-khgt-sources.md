# KHGT sources

Retrieval date for every web source below: **2026-09-07**.

This note asks a narrow question.
KHGT (Kalender Hijriah Global Tunggal), the criterion Muhammadiyah adopted from 1447 H and traces to the Istanbul 2016 congress, is a global predicate.
It asks whether the crescent is visible anywhere on Earth within a bounded window, not at one observer point.
Before any implementation work starts, this note asks whether a primary technical specification exists that states the visibility thresholds, the search window in UT, and the treatment of the case where visibility occurs after 00:00 UT, precisely enough to implement without inference.

## What was searched

Indonesian: Majelis Tarjih dan Tajdid Pimpinan Pusat Muhammadiyah (MTT PPM) material published around the Munas XXXII Tarjih decision (13-15 Syakban 1445 H, 23-25 February 2024) and the Tanfidz decision that put KHGT into effect from 1 Muharram 1447 H (26 June 2025).
This included the official KHGT site (khgt.muhammadiyah.or.id) and its methodology section, and the official Pedoman Hisab book published by Pimpinan Pusat Muhammadiyah, titled *Kalender Hijriah Global Tunggal (KHGT)*, hosted at tarjih.or.id.

Turkish: Diyanet (Turkish Presidency of Religious Affairs) publications on the congress it hosted in Istanbul, 28-30 May 2016 (21-23 Sha'ban 1437 H).
This included Diyanet's own announcement pages and its digital library (dijital.diyanet.gov.tr), which hosts the full congress proceedings as a downloadable book.

The congress final declaration: the proceedings book's own closing "Decisions and Recommendations" (al-Qararat wa at-Tawsiyat) section was read directly, not summarized secondhand.

Academic papers on the unified calendar were located (for example a paper in *Al-Jami'ah: Journal of Islamic Studies* on the unified Islamic calendar) and treated only as corroboration, per this project's convention that a secondary summary quoting thresholds is never a primary source.

## What was found

**Muhammadiyah's official Pedoman Hisab book.**
*Kalender Hijriah Global Tunggal (KHGT)*, published by Pimpinan Pusat Muhammadiyah (tarjih.or.id/wp-content/uploads/2025/06/KHGT_Indonesia.pdf), states the KHGT parameter as elongation of 8 degrees or more and lunar altitude above the horizon at sunset of at least 5 degrees, both explicitly marked geocentric in a footnote.
It states the global window as: the parameter must be met somewhere on Earth before 24:00 GMT.
It then states a post-00:00 GMT case: if the parameter is met after midnight GMT, the new month still starts under two listed conditions, the parameter being met somewhere in the world with conjunction in New Zealand occurring before dawn, and that same fulfillment reaching the mainland of the American continent.
The book's own footnote for this exception cites a different, underlying document: a working paper titled (in Arabic) "al-Milaff al-Muhtawi Ma'ayir Mashru'ay at-Taqwim al-Uhadi wa ath-Thuna'i al-Manwi Taqdimuhu ila al-Mu'tamar Ma'a an-Namadhij at-Tatbiqiyyah", prepared by the congress's Scientific (Steering) Committee and presented at the Istanbul 2016 congress, page 9.
That working paper was not independently located or read for this note.
The book itself is an official publication by the body that adopted KHGT for Muhammadiyah, but on this specific exception it is explicitly quoting a separate committee document, and it does not state whether its two listed conditions combine with AND or OR, nor does it define the geographic boundary of "the mainland of the American continent."

**Diyanet's own congress proceedings.**
The full proceedings book of the 2016 Istanbul congress was obtained directly from Diyanet's digital library (dijital.diyanet.gov.tr) and read in full.
It is a transcript-style record of presentations and floor discussion, not a single technical standard.
Inside it, a comparison section (pages 76-78 of the book) describes what it calls the "current Turkish (Diyanet) calendar" and a "modified Turkish (Diyanet) calendar" as two separate candidate proposals, both discussed among several others considered at the congress.
Both use 8 degrees elongation and 5 degrees altitude at sunset, but they differ in the search window: the "current" calendar's window ends strictly at 24:00 GMT worldwide, with no exception, so the text gives a worked example where a crescent that becomes visible at 24:01 GMT anywhere pushes the whole month start back a full day.
The "modified" calendar instead ends its window at dawn in Mecca rather than at GMT midnight.
Neither of these two Diyanet-described variants states the New Zealand-conjunction-plus-Americas exception that Muhammadiyah's book attributes to "the Istanbul 2016 criterion."
Later in the same proceedings (around page 351), a Malaysian astronomer's floor presentation proposes yet another distinct numeric pair, 5 degrees of elongation with a required 3-degree lag, based on Malaysian sighting data, showing the numeric threshold itself was still being contested during the congress.
The book's own closing "Decisions and Recommendations" section (pages 383-391), which is the congress's actual final declaration, adopts "the single calendar" as the accepted general approach over "the dual calendar" alternative, states general principles (reliance on possibility of sighting, no recognition of differing local horizons), and instructs Diyanet's Scientific Committee to prepare and print a ten-year single calendar going forward.
It does not itself state a specific altitude, elongation, UT window boundary, or post-00:00 UT exception.
The concrete numeric specification that ultimately shipped is therefore not present in the congress's own closing declaration text.

No ECFR, FCNA/ISNA, or other named-authority technical document was searched for in this task, since it is out of scope for KHGT specifically.

## Verdict

No primary document was located that specifies the KHGT criterion (thresholds, UT search window, and post-00:00 UT treatment) precisely enough to implement without inference.

Muhammadiyah's official Pedoman Hisab book is an official publication by the body that adopted the criterion, and it does state the 5-degree altitude and 8-degree elongation thresholds unambiguously as geocentric, along with a window described as "before 24:00 GMT."
But its post-00:00 UT exception is sourced from a separate Istanbul 2016 Scientific Committee working paper that this note did not obtain, and even as quoted, the exception's own internal logic (whether its two listed conditions are cumulative or alternative) and its geographic scope ("mainland of the Americas") are not defined precisely enough to implement without guessing.

Diyanet's own 400-page congress proceedings were read directly, not through a secondary summary, and they show that at least two differently-windowed 8-degree/5-degree proposals were on the table at the congress (a strict 24:00 GMT cutoff with no exception, and a Mecca-dawn cutoff), plus at least one entirely different threshold pair proposed from the floor.
The congress's own closing declaration commits to adopting a single global calendar in principle but defers the concrete numeric specification to further committee work, and does not itself publish that specification.

This is recorded as a gap, not as evidence that the 5-degree/8-degree KHGT criterion is false or arbitrary.
It plainly is the criterion Muhammadiyah uses, and there is no dispute about the two threshold numbers themselves, since every source found (Muhammadiyah's book, Diyanet's proceedings, secondary academic papers) agrees on 5 degrees altitude and 8 degrees elongation.
What is missing is a document from the body that defined the criterion (Diyanet's Scientific Committee) or an unambiguous adoption document from the body that adopted it (Muhammadiyah) that pins down the search window's exact boundary and the post-00:00 UT exception's exact logic well enough to code without inference.

## Consequences

The global criterion work referred to under "General policy boundary" and the KHGT entry in `ROADMAP.md` stays blocked.
KHGT remains a research candidate, not a planned API, consistent with the admission criteria already stated in `ROADMAP.md`: primary documentation must define the actual decision rule, and geographic aggregation and time limits must be known.
Neither condition is met yet.

No implementation work is started here.
A future attempt should try to obtain the Scientific Committee's own working paper, "al-Milaff al-Muhtawi Ma'ayir Mashru'ay at-Taqwim al-Uhadi wa ath-Thuna'i al-Manwi Taqdimuhu ila al-Mu'tamar Ma'a an-Namadhij at-Tatbiqiyyah", cited at page 9 by Muhammadiyah's own Pedoman Hisab book, since that is the most specific named document this search turned up that has not yet been read.

## A numeric coincidence in the existing code

`hijri.h` already ships `HIJRI_PREDICATE_ALTITUDE_5_ELONGATION_8`, defined by `HIJRI_RESEARCH_ALTITUDE_DEG 5.0` and `HIJRI_RESEARCH_ELONGATION_DEG 8.0`, with its elongation stated explicitly as geocentric.
That is the same threshold pair, 5 degrees altitude and 8 degrees elongation, in the same geocentric frame, that this note found stated in Muhammadiyah's Pedoman Hisab book and in Diyanet's congress proceedings for KHGT.
`docs/research/hijri-2020-2025-sources.md` records that no primary source was ever located for that predicate either, and that it infers no global or authority policy.

This is recorded here as an observation, not as a claim of provenance.
There is no evidence that `HIJRI_PREDICATE_ALTITUDE_5_ELONGATION_8` was derived from KHGT, and the two are not being asserted to be the same rule.
They are not interchangeable in any case: KHGT is a global criterion asking whether the crescent is visible anywhere on Earth within a bounded window, while `HIJRI_PREDICATE_ALTITUDE_5_ELONGATION_8` evaluates at one observer point.
`ROADMAP.md`'s own admission criteria name exactly this hazard, in the bullet reading "The implementation represents the complete documented policy, not a similarly shaped local threshold."

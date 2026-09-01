# What the ulama say when a calculation authority publishes no polar rule

## Question

Issue #51 asks what `prayertimes.h` should return inside the polar circle for a method whose own authority published nothing. `docs/research/2026-08-18-high-latitude-conventions.md` established that 20 of the 22 methods are in that position and that the praytimes.org family cannot answer, because all three of its rules measure in units of a night that does not exist. It left the fiqh side thin: one decree, one unverified ECFR position, and two named conventions with no sources behind them.

This note fills that gap. The question here is narrow and it is not "what does method X say". It is: when the method is silent, is the library choosing from a void, or is there a body of ulama opinion it can name?

The answer is that there is a body of opinion, it is old, it is explicit that this is ijtihad rather than revelation, and it has settled into exactly two rules. That matters for #51 because it changes the shape of the decision. The library is not inventing a convention. It is picking one of two that scholars have already argued to a draw and declared both valid.

## The textual basis both opinions share

Every source below reasons from the same hadith, the one about the Dajjal. The Companions ask how long he will stay on earth, and are told forty days, one like a year, one like a month, one like a week, and the rest like ordinary days. They ask whether one day's prayers suffice for the day that lasts a year. The reply is no, `اقدروا له قدره`, estimate its measure.

This is the whole warrant for computing a prayer time when its sign never occurs. Ibn Taymiyya draws the consequence directly, in `Mukhtasar al-Fatawa al-Misriyyah` 1/38, as quoted in the Egyptian fatwa below:

> The [prayer] times which Jibril (peace be upon him) taught the Prophet [pbuh] and which the Prophet [pbuh] taught his community, are those which the scholars mentioned in their books and which refer to normal days. There is a different ruling for the day of which the Prophet said will extend to the length of one year. Concerning this day, he said, "Estimate [the timings of prayer]."

So the scriptural instruction is to estimate and nothing more. It names no reference. Which reference to estimate against is the entire disagreement, and it is a disagreement among jurists rather than between texts.

## Opinion one: the nearest land where the signs occur

Called ittiba' aqrab al-bilad. This is the majority position and the one with the most institutional weight behind it.

**The Permanent Committee for Scholarly Research and Ifta, Saudi Arabia.** Fatwa No. 2769, `Fatawa al-Lajnah al-Da'imah` volume 6, pages 130 to 136, issued on the basis of a decision of the Council of Senior Scholars. Retrieved in English at <https://islamqa.info/en/answers/5842>. The operative rule: those in a land where the sun does not set in summer or does not rise in winter must still pray five prayers in each twenty four hours, and must estimate their times by the nearest land in which the times of the five prayers are distinguishable from one another. The fatwa separates the two cases explicitly. Where night is still distinguishable from day, however short, the local signs govern and no substitution applies. Only where the signs vanish does the nearest land come in. That is the same boundary `prayertimes.h` already draws between the night-exists case and the no-night case, reached independently from the code.

The same rule in Arabic, attributed to the Council of Senior Scholars, at <https://www.islamweb.net/ar/fatwa/165595/>: those in lands of continuous day or night must pray five times in twenty four hours `وأن يقدروا لها أوقاتها معتمدين على أقرب بلاد إليهم تتمايز فيها أوقات الصلوات`, estimating their times in reliance on the nearest land to them in which the prayer times are distinguishable.

**The Islamic Fiqh Council of the Muslim World League**, ninth session, 12 to 19 Rajab 1406, held in Makkah. This is the decree already recorded in the 2026-08-18 note, and it is a form of aqrab al-bilad with the reference pinned rather than left to the reader. Its third zone, beyond 66 degrees to the poles, sets all the times by proportional measurement in analogy to latitude 45, dividing the 24 hours the way they are divided at 45. Confirmed again this pass at <https://fiqh.islamonline.net/en/praying-and-fasting-at-high-latitudes/>, which also records that the Council of the Islamic Fiqh Academy affirmed the same resolution.

**The European Council for Fatwa and Research**, twenty second ordinary session. The session page at <https://www.e-cfr.org/en/2020/06/23/the-22nd-ordinary-session-of-the-european-council-for-fatwa-and-research/> returned HTTP 403 to direct retrieval again this pass, so this is reported from search summaries rather than verified, the same status the previous note gave it. The reported substance is that the ECFR affirms the MWL resolution's zones, and separately sees no harm in relying on other bodies' estimations such as a 12 degree depression for fajr and isha, or a fixed hour and a half interval. Treat as unverified.

**The Hanafi school as applied today.** SeekersGuidance, answering in the Hanafi school and citing `Fath al-Mulhim`, gives aqrab al-bilad and makes the reference concrete: use the sunset, isha and dawn of a city on latitude 48. <https://seekersguidance.org/answers/hanafi-fiqh/pray-country-sun-doesnt-set/>. Note that this is a third reference latitude, after the MWL's 45 and the Iceland paper's London at 51.5. The choice of reference is not settled by anyone.

## Opinion two: the timings of Makkah or Madinah

The minority position, and the one behind the Tromso practice that prompted the second ruling on #51.

**Dar al-Ifta al-Misriyyah, Fatwa No. 2806, dated 8 August 2010.** English text retrieved in full at <https://saifulislam.com/wp-content/uploads/2017/05/Dar-Alifta-Al-Misrriyah-2806-Eng.pdf>. This is the strongest primary source found in this pass, and it is worth reading rather than summarising, because it does three things at once.

It affirms estimation as the rule, from the Dajjal hadith and Ibn Taymiyya as quoted above. It then proposes Makkah explicitly:

> We propose to those living in countries of extreme latitudes to fast according to the time of Mecca since Allah designated it as the 'Mother of Villages'; a mother is the source of existence. Moreover, Mecca is the city to turn to, not just for the Qibla [direction of prayer] but also when estimating timings when there are extreme variations in day and night.

And it then rejects aqrab al-bilad, on practical rather than doctrinal grounds:

> Estimating the times for starting and ending a fast based on the nearest country with moderate hours is an extremely confusing matter. Those who are in favor of this method, stipulate knowledge of the precise calculations for starting and terminating the fast in the nearest countries with moderate hours without any difficulty or confusion. From experience and practice, both of these conditions are lacking in the above method of estimation, giving rise to greater confusion.

The fatwa lists a continuous line of Egyptian authority for the Makkah position, which is why it is not a one-off. Muhammad Abduh, the first Grand Mufti of Egypt, as transmitted by Rashid Rida in `Tafsir al-Manar` 2/163. Gad al-Haq Ali Gad al-Haq, fatwa 214 of 1981. Abdul Latif Hamza, fatwa 160 of 1984. Muhammad Sayed Tantawi, fatwa 171 of 1993 and fatwa 579 of 1995. Nasr Farid Wasil, fatwa 438 of 1998. Ali Gomaa, Grand Mufti at the time of the 2806 fatwa. Mahmud Shaltut, former Grand Imam of al-Azhar, is quoted at length in support of the underlying principle.

**AMJA**, fatwa 21730, dated 13 May 2008, <https://www.amjaonline.org/fatwa/en/21730/polar-prayer-times-fasting>. Gives both opinions, prefers aqrab al-bilad, records Makkah and Madinah as the alternative on the grounds that they are the location of the Revelation, and then says the thing that matters most for this library: the issue is one of ijtihad, and the choice between the opinions should be left to the local religious authorities.

## The one place the sources agree

Both opinions are declared valid. This is not a case where one side is tolerating error in the other. Muhammad Abduh, quoted inside the Egyptian fatwa, states it plainly for both:

> Both opinions are permissible; the matter is open to independent reasoning since it is not dealt with in primary texts.

IslamOnline records the same conclusion attributed to Sheikh Muhammad Rida, that both are drawn by personal reasoning not based on religious texts, at <https://fiqh.islamonline.net/en/determining-the-times-of-prayer-in-the-high-latitudes/>.

So the answer to the question this note opened with is: yes, there are fatwas, there are exactly two rules, and the sources themselves say the choice between them is ijtihad rather than a matter of one being correct.

## A distinction that changes the implementation

The Egyptian fatwa does not say to copy Makkah's clock. Gad al-Haq's formulation, quoted inside 2806, is precise about this:

> I call upon Muslims living in Norway and other countries with similar circumstances to fast the same number of hours as Muslims in Mecca or Medina. They are to start their fast at the time of true dawn according to their location and disregard the number of hours for day and night as well as sun set (for breaking the fast)

That is a duration transplanted onto a locally anchored start, not a schedule transplanted wholesale. It is written about fasting, where there is one interval and one anchor, and it does not translate mechanically to five prayer times.

The Alnor Senter Tromso timetable measured in #51 does the other thing. Its August sheet matches libmuslim computing Makkah's own times to the minute across all five prayers, which is a wholesale clock transplant with no local anchoring at all. The comment on #51 records the residual as a single minute across three sampled dates.

These are two different implementations of "follow Makkah", and the mosque's practice is not the one the fatwa's wording describes. Neither is wrong, and the gap is unresolved here. It is flagged because a `HIGHLAT_MAKKAH` value has to pick one, and picking the mosque's version means implementing observed practice while citing a fatwa that says something slightly different. If that is the choice, the header should say so rather than cite 2806 as though it prescribed the transplant.

## What is not established

Recorded so no downstream decision leans on more than was actually found.

- No verbatim official text of the MWL ninth session decree was obtained this pass either. The status is unchanged from the 2026-08-18 note: substance agreed across independent renderings, exact wording not established.
- The ECFR position remains unverified, on a second failed retrieval.
- Fatwa 2806 is titled and framed around fasting. Its reasoning covers prayer explicitly, through Ibn Taymiyya on prayer times and through the Dajjal hadith which is about prayer, and the Dar al-Ifta position is applied to prayer by prayertimes.dk. But the operative proposal in its own closing paragraphs is about the fast. A note that it prescribes prayer times as such would be stronger than the document supports.
- Nothing was found that addresses which of the five prayers a partial substitution should apply to, or what to do when only some signs vanish. Every source treats the polar case as all or nothing.
- Whether any of the 20 silent methods' own authorities have since published something was not re-checked here. This note is about the general fiqh position, not about filling in the method table.

## Consequence for #51

The decision recorded on the issue was to apply a named library convention rather than return unavailable. This note narrows what "named" can honestly mean.

The library can cite aqrab al-bilad and name the Permanent Committee and the MWL Fiqh Council. It can cite the Makkah rule and name Dar al-Ifta and six Egyptian muftis. It cannot claim either is the ruling, because the sources refuse to rank them, and AMJA specifically assigns the choice to local religious authorities, which a header file is not.

That is an argument for the header saying which of the two it implements, saying that the other exists and is equally valid, and saying that the choice is libmuslim's rather than the authority's. It is also an argument for the caller escape hatch at `prayertimes.h:296` surviving the change rather than being replaced by it, since the escape hatch is the only mechanism by which a local religious authority can actually exercise the choice the fatwas assign to it.

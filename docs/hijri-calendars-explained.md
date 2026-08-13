# Why Muslims sometimes start Ramadan on different days

*A plain explanation of the three main Hijri calendar methods. Written for
anyone who has wondered why their family fasted on a different day from the
mosque down the road. No mathematics required.*

## The problem, in one paragraph

A lunar month is the time from one new moon to the next. That is about 29 days,
12 hours and 44 minutes. It is not a whole number of days, so a Hijri month is
sometimes 29 days and sometimes 30, and there is no fixed pattern to it. Every
month, somebody has to decide which one it was.

The traditional answer is to look. If you see the thin crescent after sunset,
tomorrow is the first of the new month. If you do not, you complete 30 days and
start the month after that. The difficulty is that the crescent is extremely
faint, it sits close to the horizon in bright twilight, and it is often
impossible to see even when it is definitely there. So the question becomes:
what counts as the month having begun?

Different authorities answer that differently. All of them are looking at the
same sky.

## What everyone agrees on

Three things are common ground.

**The month can only begin after conjunction.** Conjunction, called *ijtimak*,
is the instant the Moon passes between the Earth and the Sun. Before it, the
crescent you would see is the old moon of the month that is ending. No method
accepts a month beginning before conjunction.

**The day begins at sunset.** The evening of the 29th is when the decision is
made. If the month begins, it begins that evening, and the next daylight is the
first day.

**The Moon must be above the horizon at sunset.** If the Moon has already set
before the Sun, there is nothing to see and no method starts the month.

Every disagreement is about what happens *after* those conditions are met.

## The three methods

### Wujudul Hilal, used by Muhammadiyah

*Wujudul hilal* means "the crescent exists". The rule has two conditions:

1. Conjunction happens before sunset.
2. The Moon is still above the horizon when the Sun sets.

That is all. If the Moon is up, however slightly, the month has begun.

The reasoning is that the crescent is physically present in the sky, whether or
not any human eye could pick it out. Visibility depends on the weather, on
eyesight, and on where you happen to be standing. Existence does not.

The practical consequence is that this calendar can be computed years in
advance and published as a fixed schedule. Muhammadiyah does exactly that, which
is why Indonesian families often know their Eid date long before the
announcement.

### Imkanur Rukyat, used by Kemenag and the MABIMS countries

*Imkanur rukyat* means "the possibility of sighting". This method asks a harder
question: not whether the crescent exists, but whether it could realistically be
seen.

Since 2021, Indonesia, Malaysia, Brunei and Singapore have used two thresholds
together:

- The Moon must be at least **3 degrees** above the horizon at sunset.
- The Moon must be at least **6.4 degrees** away from the Sun in the sky.

Three degrees is about the width of two fingers held at arm's length. The second
number, the *elongation*, matters because a crescent too close to the Sun is
drowned in glare no matter how high it sits.

These thresholds come from studying thousands of records of successful and
failed sightings. They are a summary of what people have actually managed to
see.

In Indonesia the calculation does not decide by itself. Observers are posted at
sites across the country, and the Ministry of Religious Affairs holds a session
called *sidang isbat* that considers the calculation and the observation reports
together before announcing the date. The numbers inform the decision. They are
not the decision.

### Umm al-Qura, used in Saudi Arabia

The Umm al-Qura calendar is a printed table, prepared in advance for
administrative use, and it is the calendar that governs civil life in Saudi
Arabia.

Its stated rule is geometric and evaluated at Mecca: the month begins if
conjunction occurs before sunset and the Moon sets after the Sun. In practice
the published table is authoritative in itself, and it occasionally departs from
its own stated rule.

One important point is widely misunderstood. **Saudi Arabia does not use the
Umm al-Qura table to decide Ramadan, Eid or Hajj.** Those are determined by
actual sighting, reported to and confirmed by the judiciary. The table runs the
civil calendar, not the religious one.

## A worked example: Eid al-Fitr 1447

Here is a real case, coming in 2026. On the evening of Thursday 19 March 2026,
seen from Jakarta:

```
Moon age since conjunction      9.66 hours
Height of the crescent's top    2.469 degrees above the horizon
Height of the crescent's centre 1.634 degrees
Distance from the Sun           5.165 degrees
```

Apply the rules:

**Wujudul hilal.** Conjunction has happened, and the Moon is above the horizon
at 2.469 degrees. Both conditions are met. Friday 20 March is 1 Shawwal, and
Eid is Friday.

**Imkanur rukyat.** The Moon is at 1.634 degrees, short of the 3 degree
threshold, and 5.165 degrees from the Sun, short of the 6.4 degree threshold. It
fails both. Ramadan completes 30 days, and Eid falls on Saturday 21 March.

Two Muslim communities in the same city, celebrating Eid a day apart, and
neither has made an arithmetic error. The crescent really is above the horizon.
It really is too low and too close to the Sun for anyone to see. The methods
disagree because they are answering different questions, and both answers are
true.

*These figures are computed. Kemenag's actual announcement comes from the sidang
isbat and may differ.*

## Why the difference is smaller than it looks

It is worth keeping the scale in mind. The two Indonesian methods agree in the
large majority of months. They diverge only when the crescent lands in a narrow
band, high enough to exist and too low to be seen.

Checking twenty years of new moons as seen from Jakarta, the two rules gave a
different answer four times out of 248, which is about 1.6 percent, or roughly
twice a decade. In all four cases it was Muhammadiyah that began the month a day
earlier. That is the expected direction rather than a coincidence: a rule that
asks only whether the crescent exists is satisfied at or before the moment a
rule that asks whether it could be seen is satisfied.

These counts are approximate, since they come from sampling rather than from
tracing every month, and they apply to Jakarta. Somewhere further west the
crescent sits higher at sunset on the same evening, so the balance shifts.

## A note on time zones

The Hijri date can genuinely differ between countries on the same evening, and
this is not a disagreement about method. Sunset in Jakarta happens hours before
sunset in Mecca. The Moon keeps climbing away from the Sun in the meantime, so
an observer further west sees a higher, more visible crescent on the same
calendar evening. A month can begin in Morocco and not in Indonesia on the very
same night, with both applying identical rules correctly.

This is why criteria are always stated together with a location.

## What this means practically

If your community follows one method and your neighbour follows another, you are
not witnessing a mistake. You are watching two defensible answers to a question
the sky does not answer cleanly by itself.

The scholarly disagreement is old, it is well documented on all sides, and it is
about how to interpret evidence rather than about the evidence itself. Choosing
between them is a religious question and this document does not attempt to
settle it. Follow the authority your community follows.

## Where the numbers here come from

Every figure in this document was computed with this library and checked
against the authorities' own published material:

- **Muhammadiyah:** *Pedoman Hisab Muhammadiyah*, Majelis Tarjih dan Tajdid PP
  Muhammadiyah, 2009.
- **Kemenag:** *Ephemeris Hisab Rukyat 2023*, Direktorat Urusan Agama Islam dan
  Pembinaan Syariah, Kementerian Agama RI, 2022.
- **Umm al-Qura:** the published table, cross-checked against an independent
  astronomical calculation.

For the technical detail behind any of this, including measured accuracy and the
places where the library's calculation does not reproduce an official
announcement, see the notes in `docs/research/`.

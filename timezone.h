/*
 * timezone.h — optional DST-aware timezone helper for libmuslim
 *
 * `prayertimes.h` is pure astronomy: it takes a numeric UTC offset and does
 * math. It does NOT know about DST, because DST is a political rule, not an
 * astronomical one. This header is the optional companion that resolves the
 * correct offset for a given IANA zone and date, with DST and historical zone
 * changes honored by the host operating system's timezone database.
 *
 * Unlike `prayertimes.h` (which depends only on <math.h> and is fully
 * portable), this header touches the OS:
 *   - POSIX  : setenv(TZ) -> tzset() -> localtime_r() -> tm_gmtoff
 *   - Windows: EnumDynamicTimeZoneInformation +
 * SystemTimeToTzSpecificLocalTimeEx Use it only if you want libmuslim to
 * compute the offset for you; otherwise keep supplying the offset to
 * `calculate_prayer_times` yourself.
 *
 * Single-header usage — in exactly ONE translation unit:
 *     #define MUSLIM_TIMEZONE_IMPLEMENTATION
 *     #include "timezone.h"
 * Everywhere else just #include "timezone.h".
 *
 * On glibc, tm_gmtoff requires _GNU_SOURCE / _DEFAULT_SOURCE. The
 * implementation block defines _GNU_SOURCE if it is not already set, so make
 * sure nothing includes <time.h> ahead of the implementation include in that
 * one TU.
 */

#ifndef MUSLIM_TIMEZONE_H
#define MUSLIM_TIMEZONE_H

/* The POSIX implementation reads `struct tm`'s tm_gmtoff field, a BSD/GNU
 * extension that glibc only exposes when a feature-test macro is set BEFORE
 * <time.h> is first included. Define one here so the offset is correct even
 * under -std=c11. This requires timezone.h to be included before any system
 * <time.h> in the translation unit. */
#if !defined(_WIN32) && !defined(_GNU_SOURCE) && !defined(_DEFAULT_SOURCE) &&  \
    !defined(_BSD_SOURCE)
#define _DEFAULT_SOURCE 1
#endif

#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Return the UTC offset, in hours, for the IANA zone `tz_name` at instant
 * `when` (Unix epoch seconds, UTC). DST is already applied: e.g. for
 * "Europe/London" this returns 0.0 in winter and 1.0 during British Summer
 * Time. Pass the result straight into `calculate_prayer_times`.
 *
 * Returns 0.0 if `tz_name` is NULL or cannot be resolved by the host (on
 * Windows, if the zone is outside the bundled IANA<->Windows table).
 */
double parse_timezone_offset(const char *tz_name, time_t when);

/*
 * Write the host system's IANA timezone name (e.g. "Asia/Jakarta") into `buf`.
 * Returns 0 on success, -1 on failure (in which case `buf` is set to "UTC"
 * when there is room). `cap` is the size of `buf` in bytes.
 */
int get_system_timezone(char *buf, size_t cap);

#ifdef __cplusplus
}
#endif

/* ===================================================================== */
/* Implementation                                                        */
/* ===================================================================== */

#ifdef MUSLIM_TIMEZONE_IMPLEMENTATION

#if defined(_WIN32)

/* ---- Windows implementation -------------------------------------------- *
 * Win32 timezone APIs use Windows zone names ("Egypt Standard Time"), not
 * IANA names ("Africa/Cairo"). We translate via a CLDR-derived table. Zones
 * outside this table resolve to 0.0 / "UTC"; extend IANA_TO_WIN as needed.   */

#if !defined(_WIN32_WINNT) || _WIN32_WINNT < 0x0602
#undef _WIN32_WINNT
#define _WIN32_WINNT                                                           \
  0x0602 // Windows 8: EnumDynamicTimeZoneInformation,
         // SystemTimeToTzSpecificLocalTimeEx
#endif

#include <string.h>
#include <wchar.h>
#include <windows.h>

typedef struct {
  const char *iana;
  const wchar_t *win;
} MuslimIanaWinPair;

/* Roughly alphabetical by IANA name (Generated from muslimtify windows cldr).
 * Where multiple IANA names share a Windows zone, the canonical one is
 * listed FIRST so the reverse mapping returns the expected name
 * (e.g. "Asia/Tokyo" rather than "Asia/Jayapura"). */
static const MuslimIanaWinPair MUSLIM_IANA_TO_WIN[] = {
    {"Africa/Cairo", L"Egypt Standard Time"},
    {"Africa/Casablanca", L"Morocco Standard Time"},
    {"Africa/El_Aaiun", L"Morocco Standard Time"},
    {"Africa/Johannesburg", L"South Africa Standard Time"},
    {"Africa/Bujumbura", L"South Africa Standard Time"},
    {"Africa/Gaborone", L"South Africa Standard Time"},
    {"Africa/Lubumbashi", L"South Africa Standard Time"},
    {"Africa/Maseru", L"South Africa Standard Time"},
    {"Africa/Blantyre", L"South Africa Standard Time"},
    {"Africa/Maputo", L"South Africa Standard Time"},
    {"Africa/Kigali", L"South Africa Standard Time"},
    {"Africa/Mbabane", L"South Africa Standard Time"},
    {"Africa/Lusaka", L"South Africa Standard Time"},
    {"Africa/Harare", L"South Africa Standard Time"},
    {"Etc/GMT-2", L"South Africa Standard Time"},
    {"Africa/Juba", L"South Sudan Standard Time"},
    {"Africa/Khartoum", L"Sudan Standard Time"},
    {"Africa/Lagos", L"W. Central Africa Standard Time"},
    {"Africa/Luanda", L"W. Central Africa Standard Time"},
    {"Africa/Porto-Novo", L"W. Central Africa Standard Time"},
    {"Africa/Kinshasa", L"W. Central Africa Standard Time"},
    {"Africa/Bangui", L"W. Central Africa Standard Time"},
    {"Africa/Brazzaville", L"W. Central Africa Standard Time"},
    {"Africa/Douala", L"W. Central Africa Standard Time"},
    {"Africa/Algiers", L"W. Central Africa Standard Time"},
    {"Africa/Libreville", L"W. Central Africa Standard Time"},
    {"Africa/Malabo", L"W. Central Africa Standard Time"},
    {"Africa/Niamey", L"W. Central Africa Standard Time"},
    {"Africa/Ndjamena", L"W. Central Africa Standard Time"},
    {"Africa/Tunis", L"W. Central Africa Standard Time"},
    {"Etc/GMT-1", L"W. Central Africa Standard Time"},
    {"Africa/Nairobi", L"E. Africa Standard Time"},
    {"Antarctica/Syowa", L"E. Africa Standard Time"},
    {"Africa/Djibouti", L"E. Africa Standard Time"},
    {"Africa/Asmera", L"E. Africa Standard Time"},
    {"Africa/Addis_Ababa", L"E. Africa Standard Time"},
    {"Indian/Comoro", L"E. Africa Standard Time"},
    {"Indian/Antananarivo", L"E. Africa Standard Time"},
    {"Africa/Mogadishu", L"E. Africa Standard Time"},
    {"Africa/Dar_es_Salaam", L"E. Africa Standard Time"},
    {"Africa/Kampala", L"E. Africa Standard Time"},
    {"Indian/Mayotte", L"E. Africa Standard Time"},
    {"Etc/GMT-3", L"E. Africa Standard Time"},
    {"Africa/Sao_Tome", L"Sao Tome Standard Time"},
    {"Africa/Tripoli", L"Libya Standard Time"},
    {"Africa/Windhoek", L"Namibia Standard Time"},
    {"America/Adak", L"Aleutian Standard Time"},
    {"America/Anchorage", L"Alaskan Standard Time"},
    {"America/Juneau", L"Alaskan Standard Time"},
    {"America/Metlakatla", L"Alaskan Standard Time"},
    {"America/Nome", L"Alaskan Standard Time"},
    {"America/Sitka", L"Alaskan Standard Time"},
    {"America/Yakutat", L"Alaskan Standard Time"},
    {"America/Araguaina", L"Tocantins Standard Time"},
    {"America/Argentina/Buenos_Aires", L"Argentina Standard Time"},
    {"America/Buenos_Aires", L"Argentina Standard Time"},
    {"America/Argentina/La_Rioja", L"Argentina Standard Time"},
    {"America/Argentina/Rio_Gallegos", L"Argentina Standard Time"},
    {"America/Argentina/Salta", L"Argentina Standard Time"},
    {"America/Argentina/San_Juan", L"Argentina Standard Time"},
    {"America/Argentina/San_Luis", L"Argentina Standard Time"},
    {"America/Argentina/Tucuman", L"Argentina Standard Time"},
    {"America/Argentina/Ushuaia", L"Argentina Standard Time"},
    {"America/Catamarca", L"Argentina Standard Time"},
    {"America/Cordoba", L"Argentina Standard Time"},
    {"America/Jujuy", L"Argentina Standard Time"},
    {"America/Mendoza", L"Argentina Standard Time"},
    {"America/Asuncion", L"Paraguay Standard Time"},
    {"America/Bahia", L"Bahia Standard Time"},
    {"America/Bogota", L"SA Pacific Standard Time"},
    {"America/Rio_Branco", L"SA Pacific Standard Time"},
    {"America/Eirunepe", L"SA Pacific Standard Time"},
    {"America/Coral_Harbour", L"SA Pacific Standard Time"},
    {"America/Guayaquil", L"SA Pacific Standard Time"},
    {"America/Jamaica", L"SA Pacific Standard Time"},
    {"America/Cayman", L"SA Pacific Standard Time"},
    {"America/Panama", L"SA Pacific Standard Time"},
    {"America/Lima", L"SA Pacific Standard Time"},
    {"Etc/GMT+5", L"SA Pacific Standard Time"},
    {"America/Cancun", L"Eastern Standard Time (Mexico)"},
    {"America/Caracas", L"Venezuela Standard Time"},
    {"America/Cayenne", L"SA Eastern Standard Time"},
    {"Antarctica/Rothera", L"SA Eastern Standard Time"},
    {"Antarctica/Palmer", L"SA Eastern Standard Time"},
    {"America/Fortaleza", L"SA Eastern Standard Time"},
    {"America/Belem", L"SA Eastern Standard Time"},
    {"America/Maceio", L"SA Eastern Standard Time"},
    {"America/Recife", L"SA Eastern Standard Time"},
    {"America/Santarem", L"SA Eastern Standard Time"},
    {"Atlantic/Stanley", L"SA Eastern Standard Time"},
    {"America/Paramaribo", L"SA Eastern Standard Time"},
    {"Etc/GMT+3", L"SA Eastern Standard Time"},
    {"America/Chicago", L"Central Standard Time"},
    {"America/Winnipeg", L"Central Standard Time"},
    {"America/Rankin_Inlet", L"Central Standard Time"},
    {"America/Resolute", L"Central Standard Time"},
    {"America/Matamoros", L"Central Standard Time"},
    {"America/Ojinaga", L"Central Standard Time"},
    {"America/Indiana/Knox", L"Central Standard Time"},
    {"America/Indiana/Tell_City", L"Central Standard Time"},
    {"America/Menominee", L"Central Standard Time"},
    {"America/North_Dakota/Beulah", L"Central Standard Time"},
    {"America/North_Dakota/Center", L"Central Standard Time"},
    {"America/North_Dakota/New_Salem", L"Central Standard Time"},
    {"America/Cuiaba", L"Central Brazilian Standard Time"},
    {"America/Campo_Grande", L"Central Brazilian Standard Time"},
    {"America/Denver", L"Mountain Standard Time"},
    {"America/Edmonton", L"Mountain Standard Time"},
    {"America/Cambridge_Bay", L"Mountain Standard Time"},
    {"America/Inuvik", L"Mountain Standard Time"},
    {"America/Ciudad_Juarez", L"Mountain Standard Time"},
    {"America/Boise", L"Mountain Standard Time"},
    {"America/Godthab", L"Greenland Standard Time"},
    {"America/Grand_Turk", L"Turks And Caicos Standard Time"},
    {"America/Guatemala", L"Central America Standard Time"},
    {"America/Belize", L"Central America Standard Time"},
    {"America/Costa_Rica", L"Central America Standard Time"},
    {"Pacific/Galapagos", L"Central America Standard Time"},
    {"America/Tegucigalpa", L"Central America Standard Time"},
    {"America/Managua", L"Central America Standard Time"},
    {"America/El_Salvador", L"Central America Standard Time"},
    {"Etc/GMT+6", L"Central America Standard Time"},
    {"America/Halifax", L"Atlantic Standard Time"},
    {"Atlantic/Bermuda", L"Atlantic Standard Time"},
    {"America/Glace_Bay", L"Atlantic Standard Time"},
    {"America/Goose_Bay", L"Atlantic Standard Time"},
    {"America/Moncton", L"Atlantic Standard Time"},
    {"America/Thule", L"Atlantic Standard Time"},
    {"America/Havana", L"Cuba Standard Time"},
    {"America/Indianapolis", L"US Eastern Standard Time"},
    {"America/Indiana/Marengo", L"US Eastern Standard Time"},
    {"America/Indiana/Vevay", L"US Eastern Standard Time"},
    {"America/La_Paz", L"SA Western Standard Time"},
    {"America/Antigua", L"SA Western Standard Time"},
    {"America/Anguilla", L"SA Western Standard Time"},
    {"America/Aruba", L"SA Western Standard Time"},
    {"America/Barbados", L"SA Western Standard Time"},
    {"America/St_Barthelemy", L"SA Western Standard Time"},
    {"America/Kralendijk", L"SA Western Standard Time"},
    {"America/Manaus", L"SA Western Standard Time"},
    {"America/Boa_Vista", L"SA Western Standard Time"},
    {"America/Porto_Velho", L"SA Western Standard Time"},
    {"America/Blanc-Sablon", L"SA Western Standard Time"},
    {"America/Curacao", L"SA Western Standard Time"},
    {"America/Dominica", L"SA Western Standard Time"},
    {"America/Santo_Domingo", L"SA Western Standard Time"},
    {"America/Grenada", L"SA Western Standard Time"},
    {"America/Guadeloupe", L"SA Western Standard Time"},
    {"America/Guyana", L"SA Western Standard Time"},
    {"America/St_Kitts", L"SA Western Standard Time"},
    {"America/St_Lucia", L"SA Western Standard Time"},
    {"America/Marigot", L"SA Western Standard Time"},
    {"America/Martinique", L"SA Western Standard Time"},
    {"America/Montserrat", L"SA Western Standard Time"},
    {"America/Puerto_Rico", L"SA Western Standard Time"},
    {"America/Lower_Princes", L"SA Western Standard Time"},
    {"America/Port_of_Spain", L"SA Western Standard Time"},
    {"America/St_Vincent", L"SA Western Standard Time"},
    {"America/Tortola", L"SA Western Standard Time"},
    {"America/St_Thomas", L"SA Western Standard Time"},
    {"Etc/GMT+4", L"SA Western Standard Time"},
    {"America/Los_Angeles", L"Pacific Standard Time"},
    {"America/Vancouver", L"Pacific Standard Time"},
    {"America/Mazatlan", L"Mountain Standard Time (Mexico)"},
    {"America/Mexico_City", L"Central Standard Time (Mexico)"},
    {"America/Bahia_Banderas", L"Central Standard Time (Mexico)"},
    {"America/Merida", L"Central Standard Time (Mexico)"},
    {"America/Monterrey", L"Central Standard Time (Mexico)"},
    {"America/Chihuahua", L"Central Standard Time (Mexico)"},
    {"America/Miquelon", L"Saint Pierre Standard Time"},
    {"America/Montevideo", L"Montevideo Standard Time"},
    {"America/New_York", L"Eastern Standard Time"},
    {"America/Nassau", L"Eastern Standard Time"},
    {"America/Toronto", L"Eastern Standard Time"},
    {"America/Iqaluit", L"Eastern Standard Time"},
    {"America/Detroit", L"Eastern Standard Time"},
    {"America/Indiana/Petersburg", L"Eastern Standard Time"},
    {"America/Indiana/Vincennes", L"Eastern Standard Time"},
    {"America/Indiana/Winamac", L"Eastern Standard Time"},
    {"America/Kentucky/Monticello", L"Eastern Standard Time"},
    {"America/Louisville", L"Eastern Standard Time"},
    {"America/Phoenix", L"US Mountain Standard Time"},
    {"America/Creston", L"US Mountain Standard Time"},
    {"America/Dawson_Creek", L"US Mountain Standard Time"},
    {"America/Fort_Nelson", L"US Mountain Standard Time"},
    {"America/Hermosillo", L"US Mountain Standard Time"},
    {"Etc/GMT+7", L"US Mountain Standard Time"},
    {"America/Port-au-Prince", L"Haiti Standard Time"},
    {"America/Punta_Arenas", L"Magallanes Standard Time"},
    {"America/Regina", L"Canada Central Standard Time"},
    {"America/Swift_Current", L"Canada Central Standard Time"},
    {"America/Santiago", L"Pacific SA Standard Time"},
    {"America/Sao_Paulo", L"E. South America Standard Time"},
    {"America/St_Johns", L"Newfoundland Standard Time"},
    {"America/Tijuana", L"Pacific Standard Time (Mexico)"},
    {"America/Whitehorse", L"Yukon Standard Time"},
    {"America/Dawson", L"Yukon Standard Time"},
    {"Asia/Amman", L"Jordan Standard Time"},
    {"Asia/Baghdad", L"Arabic Standard Time"},
    {"Asia/Baku", L"Azerbaijan Standard Time"},
    {"Asia/Barnaul", L"Altai Standard Time"},
    {"Asia/Beirut", L"Middle East Standard Time"},
    {"Asia/Bishkek", L"Central Asia Standard Time"},
    {"Antarctica/Vostok", L"Central Asia Standard Time"},
    {"Asia/Urumqi", L"Central Asia Standard Time"},
    {"Indian/Chagos", L"Central Asia Standard Time"},
    {"Etc/GMT-6", L"Central Asia Standard Time"},
    {"Asia/Chita", L"Transbaikal Standard Time"},
    {"Asia/Colombo", L"Sri Lanka Standard Time"},
    {"Asia/Damascus", L"Syria Standard Time"},
    {"Asia/Dhaka", L"Bangladesh Standard Time"},
    {"Asia/Thimphu", L"Bangladesh Standard Time"},
    {"Asia/Dubai", L"Arabian Standard Time"},
    {"Asia/Muscat", L"Arabian Standard Time"},
    {"Etc/GMT-4", L"Arabian Standard Time"},
    {"Asia/Hebron", L"West Bank Standard Time"},
    {"Asia/Gaza", L"West Bank Standard Time"},
    {"Asia/Hovd", L"W. Mongolia Standard Time"},
    {"Asia/Irkutsk", L"North Asia East Standard Time"},
    {"Asia/Jakarta", L"SE Asia Standard Time"},
    {"Asia/Bangkok", L"SE Asia Standard Time"},
    {"Antarctica/Davis", L"SE Asia Standard Time"},
    {"Indian/Christmas", L"SE Asia Standard Time"},
    {"Asia/Pontianak", L"SE Asia Standard Time"},
    {"Asia/Phnom_Penh", L"SE Asia Standard Time"},
    {"Asia/Vientiane", L"SE Asia Standard Time"},
    {"Asia/Saigon", L"SE Asia Standard Time"},
    {"Etc/GMT-7", L"SE Asia Standard Time"},
    {"Asia/Jerusalem", L"Israel Standard Time"},
    {"Asia/Kabul", L"Afghanistan Standard Time"},
    {"Asia/Kamchatka", L"Russia Time Zone 11"},
    {"Asia/Anadyr", L"Russia Time Zone 11"},
    {"Asia/Karachi", L"Pakistan Standard Time"},
    {"Asia/Kathmandu", L"Nepal Standard Time"},
    {"Asia/Katmandu", L"Nepal Standard Time"},
    {"Asia/Kolkata", L"India Standard Time"},
    {"Asia/Calcutta", L"India Standard Time"},
    {"Asia/Krasnoyarsk", L"North Asia Standard Time"},
    {"Asia/Novokuznetsk", L"North Asia Standard Time"},
    {"Asia/Magadan", L"Magadan Standard Time"},
    {"Asia/Novosibirsk", L"N. Central Asia Standard Time"},
    {"Asia/Omsk", L"Omsk Standard Time"},
    {"Asia/Pyongyang", L"North Korea Standard Time"},
    {"Asia/Qyzylorda", L"Qyzylorda Standard Time"},
    {"Asia/Riyadh", L"Arab Standard Time"},
    {"Asia/Bahrain", L"Arab Standard Time"},
    {"Asia/Kuwait", L"Arab Standard Time"},
    {"Asia/Qatar", L"Arab Standard Time"},
    {"Asia/Aden", L"Arab Standard Time"},
    {"Asia/Sakhalin", L"Sakhalin Standard Time"},
    {"Asia/Seoul", L"Korea Standard Time"},
    {"Asia/Shanghai", L"China Standard Time"},
    {"Asia/Hong_Kong", L"China Standard Time"},
    {"Asia/Macau", L"China Standard Time"},
    {"Asia/Singapore", L"Singapore Standard Time"},
    {"Asia/Brunei", L"Singapore Standard Time"},
    {"Asia/Makassar", L"Singapore Standard Time"},
    {"Asia/Kuala_Lumpur", L"Singapore Standard Time"},
    {"Asia/Kuching", L"Singapore Standard Time"},
    {"Asia/Manila", L"Singapore Standard Time"},
    {"Etc/GMT-8", L"Singapore Standard Time"},
    {"Asia/Srednekolymsk", L"Russia Time Zone 10"},
    {"Asia/Taipei", L"Taipei Standard Time"},
    {"Asia/Tashkent", L"West Asia Standard Time"},
    {"Antarctica/Mawson", L"West Asia Standard Time"},
    {"Asia/Oral", L"West Asia Standard Time"},
    {"Asia/Almaty", L"West Asia Standard Time"},
    {"Asia/Aqtau", L"West Asia Standard Time"},
    {"Asia/Aqtobe", L"West Asia Standard Time"},
    {"Asia/Atyrau", L"West Asia Standard Time"},
    {"Asia/Qostanay", L"West Asia Standard Time"},
    {"Indian/Maldives", L"West Asia Standard Time"},
    {"Indian/Kerguelen", L"West Asia Standard Time"},
    {"Asia/Dushanbe", L"West Asia Standard Time"},
    {"Asia/Ashgabat", L"West Asia Standard Time"},
    {"Asia/Samarkand", L"West Asia Standard Time"},
    {"Etc/GMT-5", L"West Asia Standard Time"},
    {"Asia/Tbilisi", L"Georgian Standard Time"},
    {"Asia/Tehran", L"Iran Standard Time"},
    {"Asia/Tokyo", L"Tokyo Standard Time"},
    {"Asia/Jayapura", L"Tokyo Standard Time"},
    {"Pacific/Palau", L"Tokyo Standard Time"},
    {"Asia/Dili", L"Tokyo Standard Time"},
    {"Etc/GMT-9", L"Tokyo Standard Time"},
    {"Asia/Tomsk", L"Tomsk Standard Time"},
    {"Asia/Ulaanbaatar", L"Ulaanbaatar Standard Time"},
    {"Asia/Vladivostok", L"Vladivostok Standard Time"},
    {"Asia/Ust-Nera", L"Vladivostok Standard Time"},
    {"Asia/Yakutsk", L"Yakutsk Standard Time"},
    {"Asia/Khandyga", L"Yakutsk Standard Time"},
    {"Asia/Yangon", L"Myanmar Standard Time"},
    {"Asia/Rangoon", L"Myanmar Standard Time"},
    {"Indian/Cocos", L"Myanmar Standard Time"},
    {"Asia/Yekaterinburg", L"Ekaterinburg Standard Time"},
    {"Asia/Yerevan", L"Caucasus Standard Time"},
    {"Atlantic/Azores", L"Azores Standard Time"},
    {"America/Scoresbysund", L"Azores Standard Time"},
    {"Atlantic/Cape_Verde", L"Cape Verde Standard Time"},
    {"Etc/GMT+1", L"Cape Verde Standard Time"},
    {"Atlantic/Reykjavik", L"Greenwich Standard Time"},
    {"Africa/Ouagadougou", L"Greenwich Standard Time"},
    {"Africa/Abidjan", L"Greenwich Standard Time"},
    {"Africa/Accra", L"Greenwich Standard Time"},
    {"America/Danmarkshavn", L"Greenwich Standard Time"},
    {"Africa/Banjul", L"Greenwich Standard Time"},
    {"Africa/Conakry", L"Greenwich Standard Time"},
    {"Africa/Bissau", L"Greenwich Standard Time"},
    {"Africa/Monrovia", L"Greenwich Standard Time"},
    {"Africa/Bamako", L"Greenwich Standard Time"},
    {"Africa/Nouakchott", L"Greenwich Standard Time"},
    {"Atlantic/St_Helena", L"Greenwich Standard Time"},
    {"Africa/Freetown", L"Greenwich Standard Time"},
    {"Africa/Dakar", L"Greenwich Standard Time"},
    {"Africa/Lome", L"Greenwich Standard Time"},
    {"Australia/Adelaide", L"Cen. Australia Standard Time"},
    {"Australia/Broken_Hill", L"Cen. Australia Standard Time"},
    {"Australia/Brisbane", L"E. Australia Standard Time"},
    {"Australia/Lindeman", L"E. Australia Standard Time"},
    {"Australia/Darwin", L"AUS Central Standard Time"},
    {"Australia/Eucla", L"Aus Central W. Standard Time"},
    {"Australia/Hobart", L"Tasmania Standard Time"},
    {"Antarctica/Macquarie", L"Tasmania Standard Time"},
    {"Australia/Lord_Howe", L"Lord Howe Standard Time"},
    {"Australia/Perth", L"W. Australia Standard Time"},
    {"Australia/Sydney", L"AUS Eastern Standard Time"},
    {"Australia/Melbourne", L"AUS Eastern Standard Time"},
    {"Etc/GMT+11", L"UTC-11"},
    {"Pacific/Pago_Pago", L"UTC-11"},
    {"Pacific/Niue", L"UTC-11"},
    {"Pacific/Midway", L"UTC-11"},
    {"Etc/GMT+12", L"Dateline Standard Time"},
    {"Etc/GMT+2", L"UTC-02"},
    {"America/Noronha", L"UTC-02"},
    {"Atlantic/South_Georgia", L"UTC-02"},
    {"Etc/GMT+8", L"UTC-08"},
    {"Pacific/Pitcairn", L"UTC-08"},
    {"Etc/GMT+9", L"UTC-09"},
    {"Pacific/Gambier", L"UTC-09"},
    {"Etc/GMT-12", L"UTC+12"},
    {"Pacific/Tarawa", L"UTC+12"},
    {"Pacific/Majuro", L"UTC+12"},
    {"Pacific/Kwajalein", L"UTC+12"},
    {"Pacific/Nauru", L"UTC+12"},
    {"Pacific/Funafuti", L"UTC+12"},
    {"Pacific/Wake", L"UTC+12"},
    {"Pacific/Wallis", L"UTC+12"},
    {"Etc/GMT-13", L"UTC+13"},
    {"Pacific/Enderbury", L"UTC+13"},
    {"Pacific/Fakaofo", L"UTC+13"},
    {"Etc/UTC", L"UTC"},
    {"UTC", L"UTC"},
    {"Etc/GMT", L"UTC"},
    {"Europe/Astrakhan", L"Astrakhan Standard Time"},
    {"Europe/Ulyanovsk", L"Astrakhan Standard Time"},
    {"Europe/Berlin", L"W. Europe Standard Time"},
    {"Europe/Andorra", L"W. Europe Standard Time"},
    {"Europe/Vienna", L"W. Europe Standard Time"},
    {"Europe/Zurich", L"W. Europe Standard Time"},
    {"Europe/Busingen", L"W. Europe Standard Time"},
    {"Europe/Gibraltar", L"W. Europe Standard Time"},
    {"Europe/Rome", L"W. Europe Standard Time"},
    {"Europe/Vaduz", L"W. Europe Standard Time"},
    {"Europe/Luxembourg", L"W. Europe Standard Time"},
    {"Europe/Monaco", L"W. Europe Standard Time"},
    {"Europe/Malta", L"W. Europe Standard Time"},
    {"Europe/Amsterdam", L"W. Europe Standard Time"},
    {"Europe/Oslo", L"W. Europe Standard Time"},
    {"Europe/Stockholm", L"W. Europe Standard Time"},
    {"Arctic/Longyearbyen", L"W. Europe Standard Time"},
    {"Europe/San_Marino", L"W. Europe Standard Time"},
    {"Europe/Vatican", L"W. Europe Standard Time"},
    {"Europe/Bucharest", L"GTB Standard Time"},
    {"Asia/Nicosia", L"GTB Standard Time"},
    {"Asia/Famagusta", L"GTB Standard Time"},
    {"Europe/Athens", L"GTB Standard Time"},
    {"Europe/Budapest", L"Central Europe Standard Time"},
    {"Europe/Tirane", L"Central Europe Standard Time"},
    {"Europe/Prague", L"Central Europe Standard Time"},
    {"Europe/Podgorica", L"Central Europe Standard Time"},
    {"Europe/Belgrade", L"Central Europe Standard Time"},
    {"Europe/Ljubljana", L"Central Europe Standard Time"},
    {"Europe/Bratislava", L"Central Europe Standard Time"},
    {"Europe/Chisinau", L"E. Europe Standard Time"},
    {"Europe/Istanbul", L"Turkey Standard Time"},
    {"Europe/Kaliningrad", L"Kaliningrad Standard Time"},
    {"Europe/Kiev", L"FLE Standard Time"},
    {"Europe/Mariehamn", L"FLE Standard Time"},
    {"Europe/Sofia", L"FLE Standard Time"},
    {"Europe/Tallinn", L"FLE Standard Time"},
    {"Europe/Helsinki", L"FLE Standard Time"},
    {"Europe/Vilnius", L"FLE Standard Time"},
    {"Europe/Riga", L"FLE Standard Time"},
    {"Europe/London", L"GMT Standard Time"},
    {"Atlantic/Canary", L"GMT Standard Time"},
    {"Atlantic/Faeroe", L"GMT Standard Time"},
    {"Europe/Guernsey", L"GMT Standard Time"},
    {"Europe/Dublin", L"GMT Standard Time"},
    {"Europe/Isle_of_Man", L"GMT Standard Time"},
    {"Europe/Jersey", L"GMT Standard Time"},
    {"Europe/Lisbon", L"GMT Standard Time"},
    {"Atlantic/Madeira", L"GMT Standard Time"},
    {"Europe/Minsk", L"Belarus Standard Time"},
    {"Europe/Moscow", L"Russian Standard Time"},
    {"Europe/Kirov", L"Russian Standard Time"},
    {"Europe/Simferopol", L"Russian Standard Time"},
    {"Europe/Paris", L"Romance Standard Time"},
    {"Europe/Brussels", L"Romance Standard Time"},
    {"Europe/Copenhagen", L"Romance Standard Time"},
    {"Europe/Madrid", L"Romance Standard Time"},
    {"Africa/Ceuta", L"Romance Standard Time"},
    {"Europe/Samara", L"Russia Time Zone 3"},
    {"Europe/Saratov", L"Saratov Standard Time"},
    {"Europe/Volgograd", L"Volgograd Standard Time"},
    {"Europe/Warsaw", L"Central European Standard Time"},
    {"Europe/Sarajevo", L"Central European Standard Time"},
    {"Europe/Zagreb", L"Central European Standard Time"},
    {"Europe/Skopje", L"Central European Standard Time"},
    {"Indian/Mauritius", L"Mauritius Standard Time"},
    {"Indian/Reunion", L"Mauritius Standard Time"},
    {"Indian/Mahe", L"Mauritius Standard Time"},
    {"Pacific/Apia", L"Samoa Standard Time"},
    {"Pacific/Auckland", L"New Zealand Standard Time"},
    {"Antarctica/McMurdo", L"New Zealand Standard Time"},
    {"Pacific/Bougainville", L"Bougainville Standard Time"},
    {"Pacific/Chatham", L"Chatham Islands Standard Time"},
    {"Pacific/Easter", L"Easter Island Standard Time"},
    {"Pacific/Fiji", L"Fiji Standard Time"},
    {"Pacific/Guadalcanal", L"Central Pacific Standard Time"},
    {"Antarctica/Casey", L"Central Pacific Standard Time"},
    {"Pacific/Ponape", L"Central Pacific Standard Time"},
    {"Pacific/Kosrae", L"Central Pacific Standard Time"},
    {"Pacific/Noumea", L"Central Pacific Standard Time"},
    {"Pacific/Efate", L"Central Pacific Standard Time"},
    {"Etc/GMT-11", L"Central Pacific Standard Time"},
    {"Pacific/Honolulu", L"Hawaiian Standard Time"},
    {"Pacific/Rarotonga", L"Hawaiian Standard Time"},
    {"Pacific/Tahiti", L"Hawaiian Standard Time"},
    {"Etc/GMT+10", L"Hawaiian Standard Time"},
    {"Pacific/Kiritimati", L"Line Islands Standard Time"},
    {"Etc/GMT-14", L"Line Islands Standard Time"},
    {"Pacific/Marquesas", L"Marquesas Standard Time"},
    {"Pacific/Norfolk", L"Norfolk Standard Time"},
    {"Pacific/Port_Moresby", L"West Pacific Standard Time"},
    {"Antarctica/DumontDUrville", L"West Pacific Standard Time"},
    {"Pacific/Truk", L"West Pacific Standard Time"},
    {"Pacific/Guam", L"West Pacific Standard Time"},
    {"Pacific/Saipan", L"West Pacific Standard Time"},
    {"Etc/GMT-10", L"West Pacific Standard Time"},
    {"Pacific/Tongatapu", L"Tonga Standard Time"},
};

static const wchar_t *muslim_iana_to_windows_zone(const char *tz_name) {
  if (!tz_name)
    return NULL;
  for (size_t i = 0;
       i < sizeof(MUSLIM_IANA_TO_WIN) / sizeof(MUSLIM_IANA_TO_WIN[0]); ++i) {
    if (strcmp(MUSLIM_IANA_TO_WIN[i].iana, tz_name) == 0)
      return MUSLIM_IANA_TO_WIN[i].win;
  }
  return NULL;
}

static const char *muslim_windows_zone_to_iana(const wchar_t *win_zone) {
  if (!win_zone)
    return NULL;
  for (size_t i = 0;
       i < sizeof(MUSLIM_IANA_TO_WIN) / sizeof(MUSLIM_IANA_TO_WIN[0]); ++i) {
    if (wcscmp(MUSLIM_IANA_TO_WIN[i].win, win_zone) == 0)
      return MUSLIM_IANA_TO_WIN[i].iana;
  }
  return NULL;
}

double parse_timezone_offset(const char *tz_name, time_t when) {
  const wchar_t *win_zone = muslim_iana_to_windows_zone(tz_name);
  if (!win_zone)
    return 0.0;

  // Find the DYNAMIC_TIME_ZONE_INFORMATION whose key matches.
  DYNAMIC_TIME_ZONE_INFORMATION dtzi;
  DWORD idx = 0;
  int found = 0;
  while (EnumDynamicTimeZoneInformation(idx++, &dtzi) == ERROR_SUCCESS) {
    if (wcscmp(dtzi.TimeZoneKeyName, win_zone) == 0) {
      found = 1;
      break;
    }
  }
  if (!found)
    return 0.0;

  // time_t (Unix epoch seconds, UTC) -> FILETIME (100ns ticks since
  // 1601-01-01). 11644473600 seconds separate 1601-01-01 from 1970-01-01.
  ULONGLONG ticks = ((ULONGLONG)when + 11644473600ULL) * 10000000ULL;
  FILETIME utc_ft;
  utc_ft.dwLowDateTime = (DWORD)(ticks & 0xFFFFFFFFULL);
  utc_ft.dwHighDateTime = (DWORD)(ticks >> 32);

  SYSTEMTIME utc_st;
  if (!FileTimeToSystemTime(&utc_ft, &utc_st))
    return 0.0;

  SYSTEMTIME local_st;
  if (!SystemTimeToTzSpecificLocalTimeEx(&dtzi, &utc_st, &local_st))
    return 0.0;

  // Treat local_st as if it were UTC to recover a tick count; the delta
  // against utc_ft is exactly the offset (DST already baked in by the API).
  FILETIME local_ft;
  if (!SystemTimeToFileTime(&local_st, &local_ft))
    return 0.0;

  ULONGLONG utc_ticks =
      ((ULONGLONG)utc_ft.dwHighDateTime << 32) | utc_ft.dwLowDateTime;
  ULONGLONG local_ticks =
      ((ULONGLONG)local_ft.dwHighDateTime << 32) | local_ft.dwLowDateTime;
  LONGLONG diff = (LONGLONG)local_ticks - (LONGLONG)utc_ticks;

  // 10^7 ticks/sec * 3600 sec/hr = 3.6 * 10^10 ticks/hr.
  return (double)diff / 36000000000.0;
}

int get_system_timezone(char *buf, size_t cap) {
  if (!buf || cap < 2)
    return -1;

  DYNAMIC_TIME_ZONE_INFORMATION dtzi;
  DWORD rc = GetDynamicTimeZoneInformation(&dtzi);
  if (rc == TIME_ZONE_ID_INVALID) {
    if (cap >= 4)
      memcpy(buf, "UTC", 4);
    else
      buf[0] = '\0';
    return -1;
  }

  const char *iana = muslim_windows_zone_to_iana(dtzi.TimeZoneKeyName);
  if (!iana) {
    if (cap >= 4)
      memcpy(buf, "UTC", 4);
    else
      buf[0] = '\0';
    return -1;
  }

  size_t n = strlen(iana);
  if (n + 1 > cap) {
    buf[0] = '\0';
    return -1;
  }
  memcpy(buf, iana, n + 1);
  return 0;
}

#else /* !_WIN32 */

/* ---- POSIX implementation ---------------------------------------------- *
 * Uses the system tzdb (typically /usr/share/zoneinfo) via libc:
 *   setenv(TZ) -> tzset() -> localtime_r() -> tm_gmtoff.
 * DST and historical zone changes are honored automatically.
 * (The tm_gmtoff feature-test macro is set at the top of this header, before
 * <time.h>, so it is already in effect here.)                                */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

double parse_timezone_offset(const char *tz_name, time_t when) {
  if (!tz_name)
    return 0.0;

  // Save the current TZ so we never leak our setenv to other callers.
  const char *old_tz = getenv("TZ");
  char *saved = old_tz ? strdup(old_tz) : NULL;

  setenv("TZ", tz_name, 1);
  tzset();

  struct tm lt;
  localtime_r(&when, &lt);
  double offset = (double)lt.tm_gmtoff / 3600.0;

  if (saved) {
    setenv("TZ", saved, 1);
    free(saved);
  } else {
    unsetenv("TZ");
  }
  tzset();

  return offset;
}

static int muslim_copy_zone_tail(const char *path, char *buf, size_t cap) {
  // Find the substring "/zoneinfo/" and take everything after it.
  const char *needle = "/zoneinfo/";
  const char *p = strstr(path, needle);
  if (!p)
    return -1;
  p += strlen(needle);
  size_t n = strlen(p);
  if (n == 0 || n + 1 > cap)
    return -1;
  memcpy(buf, p, n + 1);
  return 0;
}

int get_system_timezone(char *buf, size_t cap) {
  if (!buf || cap < 2)
    return -1;

  // Primary: readlink("/etc/localtime") -> /usr/share/zoneinfo/<Area>/<Zone>.
  char link[512];
  ssize_t n = readlink("/etc/localtime", link, sizeof(link) - 1);
  if (n > 0) {
    link[n] = '\0';
    if (muslim_copy_zone_tail(link, buf, cap) == 0)
      return 0;
  }

  // Fallback: /etc/timezone (Debian/Ubuntu) contains "Area/Zone\n".
  FILE *f = fopen("/etc/timezone", "r");
  if (f) {
    char line[128];
    char *got = fgets(line, sizeof(line), f);
    fclose(f);
    if (got) {
      size_t len = strlen(line);
      while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
        line[--len] = '\0';
      if (len > 0 && len + 1 <= cap) {
        memcpy(buf, line, len + 1);
        return 0;
      }
    }
  }

  // Last resort: "UTC".
  if (cap >= 4) {
    memcpy(buf, "UTC", 4);
  } else {
    buf[0] = '\0';
  }
  return -1;
}

#endif /* _WIN32 */

#endif /* MUSLIM_TIMEZONE_IMPLEMENTATION */

#endif /* MUSLIM_TIMEZONE_H */

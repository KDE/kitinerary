/*
 * This code is auto-generated from Wikidata data. Licensed under CC0.
 */

#include "timezonedb_p.h"
#include "timezonedb_data_p.h"

namespace KItinerary {
namespace KnowledgeDb {

// timezone name strings
static const char timezone_names[] =
    "Africa/Abidjan\0"
    "Africa/Accra\0"
    "Africa/Addis_Ababa\0"
    "Africa/Algiers\0"
    "Africa/Asmara\0"
    "Africa/Bamako\0"
    "Africa/Bangui\0"
    "Africa/Banjul\0"
    "Africa/Bissau\0"
    "Africa/Blantyre\0"
    "Africa/Brazzaville\0"
    "Africa/Bujumbura\0"
    "Africa/Cairo\0"
    "Africa/Casablanca\0"
    "Africa/Ceuta\0"
    "Africa/Conakry\0"
    "Africa/Dakar\0"
    "Africa/Dar_es_Salaam\0"
    "Africa/Djibouti\0"
    "Africa/Douala\0"
    "Africa/El_Aaiun\0"
    "Africa/Freetown\0"
    "Africa/Gaborone\0"
    "Africa/Harare\0"
    "Africa/Johannesburg\0"
    "Africa/Juba\0"
    "Africa/Kampala\0"
    "Africa/Khartoum\0"
    "Africa/Kigali\0"
    "Africa/Kinshasa\0"
    "Africa/Lagos\0"
    "Africa/Libreville\0"
    "Africa/Lome\0"
    "Africa/Luanda\0"
    "Africa/Lubumbashi\0"
    "Africa/Lusaka\0"
    "Africa/Malabo\0"
    "Africa/Maputo\0"
    "Africa/Maseru\0"
    "Africa/Mbabane\0"
    "Africa/Mogadishu\0"
    "Africa/Monrovia\0"
    "Africa/Nairobi\0"
    "Africa/Ndjamena\0"
    "Africa/Niamey\0"
    "Africa/Nouakchott\0"
    "Africa/Ouagadougou\0"
    "Africa/Porto-Novo\0"
    "Africa/Sao_Tome\0"
    "Africa/Tripoli\0"
    "Africa/Tunis\0"
    "Africa/Windhoek\0"
    "America/Adak\0"
    "America/Anchorage\0"
    "America/Anguilla\0"
    "America/Antigua\0"
    "America/Araguaina\0"
    "America/Argentina/Buenos_Aires\0"
    "America/Argentina/Catamarca\0"
    "America/Argentina/Cordoba\0"
    "America/Argentina/Jujuy\0"
    "America/Argentina/La_Rioja\0"
    "America/Argentina/Mendoza\0"
    "America/Argentina/Rio_Gallegos\0"
    "America/Argentina/Salta\0"
    "America/Argentina/San_Juan\0"
    "America/Argentina/San_Luis\0"
    "America/Argentina/Tucuman\0"
    "America/Argentina/Ushuaia\0"
    "America/Aruba\0"
    "America/Asuncion\0"
    "America/Atikokan\0"
    "America/Bahia\0"
    "America/Bahia_Banderas\0"
    "America/Barbados\0"
    "America/Belem\0"
    "America/Belize\0"
    "America/Blanc-Sablon\0"
    "America/Boa_Vista\0"
    "America/Bogota\0"
    "America/Boise\0"
    "America/Cambridge_Bay\0"
    "America/Campo_Grande\0"
    "America/Cancun\0"
    "America/Caracas\0"
    "America/Cayenne\0"
    "America/Cayman\0"
    "America/Chicago\0"
    "America/Chihuahua\0"
    "America/Costa_Rica\0"
    "America/Creston\0"
    "America/Cuiaba\0"
    "America/Curacao\0"
    "America/Danmarkshavn\0"
    "America/Dawson\0"
    "America/Dawson_Creek\0"
    "America/Denver\0"
    "America/Detroit\0"
    "America/Dominica\0"
    "America/Edmonton\0"
    "America/Eirunepe\0"
    "America/El_Salvador\0"
    "America/Fort_Nelson\0"
    "America/Fortaleza\0"
    "America/Glace_Bay\0"
    "America/Godthab\0"
    "America/Goose_Bay\0"
    "America/Grand_Turk\0"
    "America/Grenada\0"
    "America/Guadeloupe\0"
    "America/Guatemala\0"
    "America/Guayaquil\0"
    "America/Guyana\0"
    "America/Halifax\0"
    "America/Havana\0"
    "America/Hermosillo\0"
    "America/Indiana/Indianapolis\0"
    "America/Indiana/Knox\0"
    "America/Indiana/Marengo\0"
    "America/Indiana/Petersburg\0"
    "America/Indiana/Tell_City\0"
    "America/Indiana/Vevay\0"
    "America/Indiana/Vincennes\0"
    "America/Indiana/Winamac\0"
    "America/Inuvik\0"
    "America/Iqaluit\0"
    "America/Jamaica\0"
    "America/Juneau\0"
    "America/Kentucky/Louisville\0"
    "America/Kentucky/Monticello\0"
    "America/Kralendijk\0"
    "America/La_Paz\0"
    "America/Lima\0"
    "America/Los_Angeles\0"
    "America/Lower_Princes\0"
    "America/Maceio\0"
    "America/Managua\0"
    "America/Manaus\0"
    "America/Marigot\0"
    "America/Martinique\0"
    "America/Matamoros\0"
    "America/Mazatlan\0"
    "America/Menominee\0"
    "America/Merida\0"
    "America/Metlakatla\0"
    "America/Mexico_City\0"
    "America/Miquelon\0"
    "America/Moncton\0"
    "America/Monterrey\0"
    "America/Montevideo\0"
    "America/Montserrat\0"
    "America/Nassau\0"
    "America/New_York\0"
    "America/Nipigon\0"
    "America/Nome\0"
    "America/Noronha\0"
    "America/North_Dakota/Beulah\0"
    "America/North_Dakota/Center\0"
    "America/North_Dakota/New_Salem\0"
    "America/Ojinaga\0"
    "America/Panama\0"
    "America/Pangnirtung\0"
    "America/Paramaribo\0"
    "America/Phoenix\0"
    "America/Port-au-Prince\0"
    "America/Port_of_Spain\0"
    "America/Porto_Velho\0"
    "America/Puerto_Rico\0"
    "America/Punta_Arenas\0"
    "America/Rainy_River\0"
    "America/Rankin_Inlet\0"
    "America/Recife\0"
    "America/Regina\0"
    "America/Resolute\0"
    "America/Rio_Branco\0"
    "America/Santarem\0"
    "America/Santiago\0"
    "America/Santo_Domingo\0"
    "America/Sao_Paulo\0"
    "America/Scoresbysund\0"
    "America/Sitka\0"
    "America/St_Barthelemy\0"
    "America/St_Johns\0"
    "America/St_Kitts\0"
    "America/St_Lucia\0"
    "America/St_Thomas\0"
    "America/St_Vincent\0"
    "America/Swift_Current\0"
    "America/Tegucigalpa\0"
    "America/Thule\0"
    "America/Thunder_Bay\0"
    "America/Tijuana\0"
    "America/Toronto\0"
    "America/Tortola\0"
    "America/Vancouver\0"
    "America/Whitehorse\0"
    "America/Winnipeg\0"
    "America/Yakutat\0"
    "America/Yellowknife\0"
    "Antarctica/Macquarie\0"
    "Arctic/Longyearbyen\0"
    "Asia/Aden\0"
    "Asia/Almaty\0"
    "Asia/Amman\0"
    "Asia/Anadyr\0"
    "Asia/Aqtau\0"
    "Asia/Aqtobe\0"
    "Asia/Ashgabat\0"
    "Asia/Atyrau\0"
    "Asia/Baghdad\0"
    "Asia/Bahrain\0"
    "Asia/Baku\0"
    "Asia/Bangkok\0"
    "Asia/Barnaul\0"
    "Asia/Beirut\0"
    "Asia/Bishkek\0"
    "Asia/Brunei\0"
    "Asia/Chita\0"
    "Asia/Choibalsan\0"
    "Asia/Colombo\0"
    "Asia/Damascus\0"
    "Asia/Dhaka\0"
    "Asia/Dili\0"
    "Asia/Dubai\0"
    "Asia/Dushanbe\0"
    "Asia/Famagusta\0"
    "Asia/Gaza\0"
    "Asia/Hebron\0"
    "Asia/Ho_Chi_Minh\0"
    "Asia/Hong_Kong\0"
    "Asia/Hovd\0"
    "Asia/Irkutsk\0"
    "Asia/Jakarta\0"
    "Asia/Jayapura\0"
    "Asia/Jerusalem\0"
    "Asia/Kabul\0"
    "Asia/Kamchatka\0"
    "Asia/Karachi\0"
    "Asia/Kathmandu\0"
    "Asia/Khandyga\0"
    "Asia/Kolkata\0"
    "Asia/Krasnoyarsk\0"
    "Asia/Kuala_Lumpur\0"
    "Asia/Kuching\0"
    "Asia/Kuwait\0"
    "Asia/Macau\0"
    "Asia/Magadan\0"
    "Asia/Makassar\0"
    "Asia/Manila\0"
    "Asia/Muscat\0"
    "Asia/Nicosia\0"
    "Asia/Novokuznetsk\0"
    "Asia/Novosibirsk\0"
    "Asia/Omsk\0"
    "Asia/Oral\0"
    "Asia/Phnom_Penh\0"
    "Asia/Pontianak\0"
    "Asia/Pyongyang\0"
    "Asia/Qatar\0"
    "Asia/Qyzylorda\0"
    "Asia/Riyadh\0"
    "Asia/Sakhalin\0"
    "Asia/Samarkand\0"
    "Asia/Seoul\0"
    "Asia/Shanghai\0"
    "Asia/Singapore\0"
    "Asia/Srednekolymsk\0"
    "Asia/Taipei\0"
    "Asia/Tashkent\0"
    "Asia/Tbilisi\0"
    "Asia/Tehran\0"
    "Asia/Thimphu\0"
    "Asia/Tokyo\0"
    "Asia/Tomsk\0"
    "Asia/Ulaanbaatar\0"
    "Asia/Urumqi\0"
    "Asia/Ust-Nera\0"
    "Asia/Vientiane\0"
    "Asia/Vladivostok\0"
    "Asia/Yakutsk\0"
    "Asia/Yangon\0"
    "Asia/Yekaterinburg\0"
    "Asia/Yerevan\0"
    "Atlantic/Azores\0"
    "Atlantic/Bermuda\0"
    "Atlantic/Canary\0"
    "Atlantic/Cape_Verde\0"
    "Atlantic/Faroe\0"
    "Atlantic/Madeira\0"
    "Atlantic/Reykjavik\0"
    "Atlantic/South_Georgia\0"
    "Atlantic/St_Helena\0"
    "Atlantic/Stanley\0"
    "Australia/Adelaide\0"
    "Australia/Brisbane\0"
    "Australia/Broken_Hill\0"
    "Australia/Currie\0"
    "Australia/Darwin\0"
    "Australia/Eucla\0"
    "Australia/Hobart\0"
    "Australia/Lindeman\0"
    "Australia/Lord_Howe\0"
    "Australia/Melbourne\0"
    "Australia/Perth\0"
    "Australia/Sydney\0"
    "Europe/Amsterdam\0"
    "Europe/Andorra\0"
    "Europe/Astrakhan\0"
    "Europe/Athens\0"
    "Europe/Belgrade\0"
    "Europe/Berlin\0"
    "Europe/Bratislava\0"
    "Europe/Brussels\0"
    "Europe/Bucharest\0"
    "Europe/Budapest\0"
    "Europe/Busingen\0"
    "Europe/Chisinau\0"
    "Europe/Copenhagen\0"
    "Europe/Dublin\0"
    "Europe/Gibraltar\0"
    "Europe/Guernsey\0"
    "Europe/Helsinki\0"
    "Europe/Isle_of_Man\0"
    "Europe/Istanbul\0"
    "Europe/Jersey\0"
    "Europe/Kaliningrad\0"
    "Europe/Kiev\0"
    "Europe/Kirov\0"
    "Europe/Lisbon\0"
    "Europe/Ljubljana\0"
    "Europe/London\0"
    "Europe/Luxembourg\0"
    "Europe/Madrid\0"
    "Europe/Malta\0"
    "Europe/Mariehamn\0"
    "Europe/Minsk\0"
    "Europe/Monaco\0"
    "Europe/Moscow\0"
    "Europe/Oslo\0"
    "Europe/Paris\0"
    "Europe/Podgorica\0"
    "Europe/Prague\0"
    "Europe/Riga\0"
    "Europe/Rome\0"
    "Europe/Samara\0"
    "Europe/San_Marino\0"
    "Europe/Sarajevo\0"
    "Europe/Saratov\0"
    "Europe/Simferopol\0"
    "Europe/Skopje\0"
    "Europe/Sofia\0"
    "Europe/Stockholm\0"
    "Europe/Tallinn\0"
    "Europe/Tirane\0"
    "Europe/Ulyanovsk\0"
    "Europe/Uzhgorod\0"
    "Europe/Vaduz\0"
    "Europe/Vatican\0"
    "Europe/Vienna\0"
    "Europe/Vilnius\0"
    "Europe/Volgograd\0"
    "Europe/Warsaw\0"
    "Europe/Zagreb\0"
    "Europe/Zaporozhye\0"
    "Europe/Zurich\0"
    "Indian/Antananarivo\0"
    "Indian/Chagos\0"
    "Indian/Christmas\0"
    "Indian/Cocos\0"
    "Indian/Comoro\0"
    "Indian/Kerguelen\0"
    "Indian/Mahe\0"
    "Indian/Maldives\0"
    "Indian/Mauritius\0"
    "Indian/Mayotte\0"
    "Indian/Reunion\0"
    "Pacific/Apia\0"
    "Pacific/Auckland\0"
    "Pacific/Bougainville\0"
    "Pacific/Chatham\0"
    "Pacific/Chuuk\0"
    "Pacific/Easter\0"
    "Pacific/Efate\0"
    "Pacific/Enderbury\0"
    "Pacific/Fakaofo\0"
    "Pacific/Fiji\0"
    "Pacific/Funafuti\0"
    "Pacific/Galapagos\0"
    "Pacific/Gambier\0"
    "Pacific/Guadalcanal\0"
    "Pacific/Guam\0"
    "Pacific/Honolulu\0"
    "Pacific/Kiritimati\0"
    "Pacific/Kosrae\0"
    "Pacific/Kwajalein\0"
    "Pacific/Majuro\0"
    "Pacific/Marquesas\0"
    "Pacific/Midway\0"
    "Pacific/Nauru\0"
    "Pacific/Niue\0"
    "Pacific/Norfolk\0"
    "Pacific/Noumea\0"
    "Pacific/Pago_Pago\0"
    "Pacific/Palau\0"
    "Pacific/Pitcairn\0"
    "Pacific/Pohnpei\0"
    "Pacific/Port_Moresby\0"
    "Pacific/Rarotonga\0"
    "Pacific/Saipan\0"
    "Pacific/Tahiti\0"
    "Pacific/Tarawa\0"
    "Pacific/Tongatapu\0"
    "Pacific/Wake\0"
    "Pacific/Wallis\0"
;

static constexpr const CountryTimezoneMap country_timezone_map[] = {
    { CountryId{"AD"}, Tz::Europe_Andorra },
    { CountryId{"AE"}, Tz::Asia_Dubai },
    { CountryId{"AF"}, Tz::Asia_Kabul },
    { CountryId{"AG"}, Tz::America_Port_of_Spain },
    { CountryId{"AI"}, Tz::America_Port_of_Spain },
    { CountryId{"AL"}, Tz::Europe_Tirane },
    { CountryId{"AM"}, Tz::Asia_Yerevan },
    { CountryId{"AO"}, Tz::Africa_Lagos },
    { CountryId{"AS"}, Tz::Pacific_Pago_Pago },
    { CountryId{"AT"}, Tz::Europe_Vienna },
    { CountryId{"AW"}, Tz::America_Curacao },
    { CountryId{"AX"}, Tz::Europe_Helsinki },
    { CountryId{"AZ"}, Tz::Asia_Baku },
    { CountryId{"BA"}, Tz::Europe_Belgrade },
    { CountryId{"BB"}, Tz::America_Barbados },
    { CountryId{"BD"}, Tz::Asia_Dhaka },
    { CountryId{"BE"}, Tz::Europe_Brussels },
    { CountryId{"BF"}, Tz::Africa_Abidjan },
    { CountryId{"BG"}, Tz::Europe_Sofia },
    { CountryId{"BH"}, Tz::Asia_Qatar },
    { CountryId{"BI"}, Tz::Africa_Maputo },
    { CountryId{"BJ"}, Tz::Africa_Lagos },
    { CountryId{"BL"}, Tz::America_Port_of_Spain },
    { CountryId{"BM"}, Tz::Atlantic_Bermuda },
    { CountryId{"BN"}, Tz::Asia_Brunei },
    { CountryId{"BO"}, Tz::America_La_Paz },
    { CountryId{"BQ"}, Tz::America_Curacao },
    { CountryId{"BS"}, Tz::America_Nassau },
    { CountryId{"BT"}, Tz::Asia_Thimphu },
    { CountryId{"BW"}, Tz::Africa_Maputo },
    { CountryId{"BY"}, Tz::Europe_Minsk },
    { CountryId{"BZ"}, Tz::America_Belize },
    { CountryId{"CC"}, Tz::Indian_Cocos },
    { CountryId{"CF"}, Tz::Africa_Lagos },
    { CountryId{"CG"}, Tz::Africa_Lagos },
    { CountryId{"CH"}, Tz::Europe_Zurich },
    { CountryId{"CI"}, Tz::Africa_Abidjan },
    { CountryId{"CK"}, Tz::Pacific_Rarotonga },
    { CountryId{"CM"}, Tz::Africa_Lagos },
    { CountryId{"CO"}, Tz::America_Bogota },
    { CountryId{"CR"}, Tz::America_Costa_Rica },
    { CountryId{"CU"}, Tz::America_Havana },
    { CountryId{"CV"}, Tz::Atlantic_Cape_Verde },
    { CountryId{"CW"}, Tz::America_Curacao },
    { CountryId{"CX"}, Tz::Indian_Christmas },
    { CountryId{"CZ"}, Tz::Europe_Prague },
    { CountryId{"DJ"}, Tz::Africa_Nairobi },
    { CountryId{"DK"}, Tz::Europe_Copenhagen },
    { CountryId{"DM"}, Tz::America_Port_of_Spain },
    { CountryId{"DO"}, Tz::America_Santo_Domingo },
    { CountryId{"DZ"}, Tz::Africa_Algiers },
    { CountryId{"EE"}, Tz::Europe_Tallinn },
    { CountryId{"EG"}, Tz::Africa_Cairo },
    { CountryId{"EH"}, Tz::Africa_El_Aaiun },
    { CountryId{"ER"}, Tz::Africa_Nairobi },
    { CountryId{"ET"}, Tz::Africa_Nairobi },
    { CountryId{"FI"}, Tz::Europe_Helsinki },
    { CountryId{"FJ"}, Tz::Pacific_Fiji },
    { CountryId{"FK"}, Tz::Atlantic_Stanley },
    { CountryId{"FO"}, Tz::Atlantic_Faroe },
    { CountryId{"FR"}, Tz::Europe_Paris },
    { CountryId{"GA"}, Tz::Africa_Lagos },
    { CountryId{"GB"}, Tz::Europe_London },
    { CountryId{"GD"}, Tz::America_Port_of_Spain },
    { CountryId{"GE"}, Tz::Asia_Tbilisi },
    { CountryId{"GF"}, Tz::America_Cayenne },
    { CountryId{"GG"}, Tz::Europe_London },
    { CountryId{"GH"}, Tz::Africa_Accra },
    { CountryId{"GI"}, Tz::Europe_Gibraltar },
    { CountryId{"GM"}, Tz::Africa_Abidjan },
    { CountryId{"GN"}, Tz::Africa_Abidjan },
    { CountryId{"GP"}, Tz::America_Port_of_Spain },
    { CountryId{"GQ"}, Tz::Africa_Lagos },
    { CountryId{"GR"}, Tz::Europe_Athens },
    { CountryId{"GS"}, Tz::Atlantic_South_Georgia },
    { CountryId{"GT"}, Tz::America_Guatemala },
    { CountryId{"GU"}, Tz::Pacific_Guam },
    { CountryId{"GW"}, Tz::Africa_Bissau },
    { CountryId{"GY"}, Tz::America_Guyana },
    { CountryId{"HK"}, Tz::Asia_Hong_Kong },
    { CountryId{"HN"}, Tz::America_Tegucigalpa },
    { CountryId{"HR"}, Tz::Europe_Belgrade },
    { CountryId{"HT"}, Tz::America_Port_au_Prince },
    { CountryId{"HU"}, Tz::Europe_Budapest },
    { CountryId{"IE"}, Tz::Europe_Dublin },
    { CountryId{"IL"}, Tz::Asia_Jerusalem },
    { CountryId{"IM"}, Tz::Europe_London },
    { CountryId{"IN"}, Tz::Asia_Kolkata },
    { CountryId{"IO"}, Tz::Indian_Chagos },
    { CountryId{"IQ"}, Tz::Asia_Baghdad },
    { CountryId{"IR"}, Tz::Asia_Tehran },
    { CountryId{"IS"}, Tz::Atlantic_Reykjavik },
    { CountryId{"IT"}, Tz::Europe_Rome },
    { CountryId{"JE"}, Tz::Europe_London },
    { CountryId{"JM"}, Tz::America_Jamaica },
    { CountryId{"JO"}, Tz::Asia_Amman },
    { CountryId{"JP"}, Tz::Asia_Tokyo },
    { CountryId{"KE"}, Tz::Africa_Nairobi },
    { CountryId{"KG"}, Tz::Asia_Bishkek },
    { CountryId{"KH"}, Tz::Asia_Bangkok },
    { CountryId{"KM"}, Tz::Africa_Nairobi },
    { CountryId{"KN"}, Tz::America_Port_of_Spain },
    { CountryId{"KP"}, Tz::Asia_Pyongyang },
    { CountryId{"KR"}, Tz::Asia_Seoul },
    { CountryId{"KW"}, Tz::Asia_Riyadh },
    { CountryId{"KY"}, Tz::America_Panama },
    { CountryId{"LA"}, Tz::Asia_Bangkok },
    { CountryId{"LB"}, Tz::Asia_Beirut },
    { CountryId{"LC"}, Tz::America_Port_of_Spain },
    { CountryId{"LI"}, Tz::Europe_Zurich },
    { CountryId{"LK"}, Tz::Asia_Colombo },
    { CountryId{"LR"}, Tz::Africa_Monrovia },
    { CountryId{"LS"}, Tz::Africa_Johannesburg },
    { CountryId{"LT"}, Tz::Europe_Vilnius },
    { CountryId{"LU"}, Tz::Europe_Luxembourg },
    { CountryId{"LV"}, Tz::Europe_Riga },
    { CountryId{"LY"}, Tz::Africa_Tripoli },
    { CountryId{"MA"}, Tz::Africa_Casablanca },
    { CountryId{"MC"}, Tz::Europe_Monaco },
    { CountryId{"MD"}, Tz::Europe_Chisinau },
    { CountryId{"ME"}, Tz::Europe_Belgrade },
    { CountryId{"MF"}, Tz::America_Port_of_Spain },
    { CountryId{"MG"}, Tz::Africa_Nairobi },
    { CountryId{"MK"}, Tz::Europe_Belgrade },
    { CountryId{"ML"}, Tz::Africa_Abidjan },
    { CountryId{"MM"}, Tz::Asia_Yangon },
    { CountryId{"MO"}, Tz::Asia_Macau },
    { CountryId{"MP"}, Tz::Pacific_Guam },
    { CountryId{"MQ"}, Tz::America_Martinique },
    { CountryId{"MR"}, Tz::Africa_Abidjan },
    { CountryId{"MS"}, Tz::America_Port_of_Spain },
    { CountryId{"MT"}, Tz::Europe_Malta },
    { CountryId{"MU"}, Tz::Indian_Mauritius },
    { CountryId{"MV"}, Tz::Indian_Maldives },
    { CountryId{"MW"}, Tz::Africa_Maputo },
    { CountryId{"MZ"}, Tz::Africa_Maputo },
    { CountryId{"NA"}, Tz::Africa_Windhoek },
    { CountryId{"NC"}, Tz::Pacific_Noumea },
    { CountryId{"NE"}, Tz::Africa_Lagos },
    { CountryId{"NF"}, Tz::Pacific_Norfolk },
    { CountryId{"NG"}, Tz::Africa_Lagos },
    { CountryId{"NI"}, Tz::America_Managua },
    { CountryId{"NL"}, Tz::Europe_Amsterdam },
    { CountryId{"NO"}, Tz::Europe_Oslo },
    { CountryId{"NP"}, Tz::Asia_Kathmandu },
    { CountryId{"NR"}, Tz::Pacific_Nauru },
    { CountryId{"NU"}, Tz::Pacific_Niue },
    { CountryId{"OM"}, Tz::Asia_Dubai },
    { CountryId{"PA"}, Tz::America_Panama },
    { CountryId{"PE"}, Tz::America_Lima },
    { CountryId{"PH"}, Tz::Asia_Manila },
    { CountryId{"PK"}, Tz::Asia_Karachi },
    { CountryId{"PL"}, Tz::Europe_Warsaw },
    { CountryId{"PM"}, Tz::America_Miquelon },
    { CountryId{"PN"}, Tz::Pacific_Pitcairn },
    { CountryId{"PR"}, Tz::America_Puerto_Rico },
    { CountryId{"PW"}, Tz::Pacific_Palau },
    { CountryId{"PY"}, Tz::America_Asuncion },
    { CountryId{"QA"}, Tz::Asia_Qatar },
    { CountryId{"RE"}, Tz::Indian_Reunion },
    { CountryId{"RO"}, Tz::Europe_Bucharest },
    { CountryId{"RS"}, Tz::Europe_Belgrade },
    { CountryId{"RW"}, Tz::Africa_Maputo },
    { CountryId{"SA"}, Tz::Asia_Riyadh },
    { CountryId{"SB"}, Tz::Pacific_Guadalcanal },
    { CountryId{"SC"}, Tz::Indian_Mahe },
    { CountryId{"SD"}, Tz::Africa_Khartoum },
    { CountryId{"SE"}, Tz::Europe_Stockholm },
    { CountryId{"SG"}, Tz::Asia_Singapore },
    { CountryId{"SH"}, Tz::Africa_Abidjan },
    { CountryId{"SI"}, Tz::Europe_Belgrade },
    { CountryId{"SJ"}, Tz::Europe_Oslo },
    { CountryId{"SK"}, Tz::Europe_Prague },
    { CountryId{"SL"}, Tz::Africa_Abidjan },
    { CountryId{"SM"}, Tz::Europe_Rome },
    { CountryId{"SN"}, Tz::Africa_Abidjan },
    { CountryId{"SO"}, Tz::Africa_Nairobi },
    { CountryId{"SR"}, Tz::America_Paramaribo },
    { CountryId{"SS"}, Tz::Africa_Juba },
    { CountryId{"ST"}, Tz::Africa_Sao_Tome },
    { CountryId{"SV"}, Tz::America_El_Salvador },
    { CountryId{"SX"}, Tz::America_Curacao },
    { CountryId{"SY"}, Tz::Asia_Damascus },
    { CountryId{"SZ"}, Tz::Africa_Johannesburg },
    { CountryId{"TC"}, Tz::America_Grand_Turk },
    { CountryId{"TD"}, Tz::Africa_Ndjamena },
    { CountryId{"TG"}, Tz::Africa_Abidjan },
    { CountryId{"TH"}, Tz::Asia_Bangkok },
    { CountryId{"TJ"}, Tz::Asia_Dushanbe },
    { CountryId{"TK"}, Tz::Pacific_Fakaofo },
    { CountryId{"TL"}, Tz::Asia_Dili },
    { CountryId{"TM"}, Tz::Asia_Ashgabat },
    { CountryId{"TN"}, Tz::Africa_Tunis },
    { CountryId{"TO"}, Tz::Pacific_Tongatapu },
    { CountryId{"TR"}, Tz::Europe_Istanbul },
    { CountryId{"TT"}, Tz::America_Port_of_Spain },
    { CountryId{"TV"}, Tz::Pacific_Funafuti },
    { CountryId{"TW"}, Tz::Asia_Taipei },
    { CountryId{"TZ"}, Tz::Africa_Nairobi },
    { CountryId{"UG"}, Tz::Africa_Nairobi },
    { CountryId{"UY"}, Tz::America_Montevideo },
    { CountryId{"VA"}, Tz::Europe_Rome },
    { CountryId{"VC"}, Tz::America_Port_of_Spain },
    { CountryId{"VE"}, Tz::America_Caracas },
    { CountryId{"VG"}, Tz::America_Port_of_Spain },
    { CountryId{"VI"}, Tz::America_Port_of_Spain },
    { CountryId{"VU"}, Tz::Pacific_Efate },
    { CountryId{"WF"}, Tz::Pacific_Wallis },
    { CountryId{"WS"}, Tz::Pacific_Apia },
    { CountryId{"YE"}, Tz::Asia_Riyadh },
    { CountryId{"YT"}, Tz::Africa_Nairobi },
    { CountryId{"ZA"}, Tz::Africa_Johannesburg },
    { CountryId{"ZM"}, Tz::Africa_Maputo },
    { CountryId{"ZW"}, Tz::Africa_Maputo },
};

}
}

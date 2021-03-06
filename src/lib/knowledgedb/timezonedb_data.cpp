/*
 * SPDX-License-Identifier: ODbL-1.0
 * SPDX-FileCopyrightText: OpenStreetMap contributors
 *
 * This code is auto-generated from OpenStreetMap (licensed under ODbL) and Wikidata (licensed under CC0), do not edit!
 */

#include "timezonedb_p.h"
#include "timezonedb_data.h"

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
    "America/Nuuk\0"
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
    "Antarctica/Casey\0"
    "Antarctica/Davis\0"
    "Antarctica/DumontDUrville\0"
    "Antarctica/Macquarie\0"
    "Antarctica/Mawson\0"
    "Antarctica/McMurdo\0"
    "Antarctica/Palmer\0"
    "Antarctica/Rothera\0"
    "Antarctica/Syowa\0"
    "Antarctica/Troll\0"
    "Antarctica/Vostok\0"
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
    "Asia/Qostanay\0"
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

static constexpr const uint16_t timezone_names_offsets[] = {
    14, // Undefined
    0, // Africa/Abidjan
    15, // Africa/Accra
    28, // Africa/Addis_Ababa
    47, // Africa/Algiers
    62, // Africa/Asmara
    76, // Africa/Bamako
    90, // Africa/Bangui
    104, // Africa/Banjul
    118, // Africa/Bissau
    132, // Africa/Blantyre
    148, // Africa/Brazzaville
    167, // Africa/Bujumbura
    184, // Africa/Cairo
    197, // Africa/Casablanca
    215, // Africa/Ceuta
    228, // Africa/Conakry
    243, // Africa/Dakar
    256, // Africa/Dar_es_Salaam
    277, // Africa/Djibouti
    293, // Africa/Douala
    307, // Africa/El_Aaiun
    323, // Africa/Freetown
    339, // Africa/Gaborone
    355, // Africa/Harare
    369, // Africa/Johannesburg
    389, // Africa/Juba
    401, // Africa/Kampala
    416, // Africa/Khartoum
    432, // Africa/Kigali
    446, // Africa/Kinshasa
    462, // Africa/Lagos
    475, // Africa/Libreville
    493, // Africa/Lome
    505, // Africa/Luanda
    519, // Africa/Lubumbashi
    537, // Africa/Lusaka
    551, // Africa/Malabo
    565, // Africa/Maputo
    579, // Africa/Maseru
    593, // Africa/Mbabane
    608, // Africa/Mogadishu
    625, // Africa/Monrovia
    641, // Africa/Nairobi
    656, // Africa/Ndjamena
    672, // Africa/Niamey
    686, // Africa/Nouakchott
    704, // Africa/Ouagadougou
    723, // Africa/Porto-Novo
    741, // Africa/Sao_Tome
    757, // Africa/Tripoli
    772, // Africa/Tunis
    785, // Africa/Windhoek
    801, // America/Adak
    814, // America/Anchorage
    832, // America/Anguilla
    849, // America/Antigua
    865, // America/Araguaina
    883, // America/Argentina/Buenos_Aires
    914, // America/Argentina/Catamarca
    942, // America/Argentina/Cordoba
    968, // America/Argentina/Jujuy
    992, // America/Argentina/La_Rioja
    1019, // America/Argentina/Mendoza
    1045, // America/Argentina/Rio_Gallegos
    1076, // America/Argentina/Salta
    1100, // America/Argentina/San_Juan
    1127, // America/Argentina/San_Luis
    1154, // America/Argentina/Tucuman
    1180, // America/Argentina/Ushuaia
    1206, // America/Aruba
    1220, // America/Asuncion
    1237, // America/Atikokan
    1254, // America/Bahia
    1268, // America/Bahia_Banderas
    1291, // America/Barbados
    1308, // America/Belem
    1322, // America/Belize
    1337, // America/Blanc-Sablon
    1358, // America/Boa_Vista
    1376, // America/Bogota
    1391, // America/Boise
    1405, // America/Cambridge_Bay
    1427, // America/Campo_Grande
    1448, // America/Cancun
    1463, // America/Caracas
    1479, // America/Cayenne
    1495, // America/Cayman
    1510, // America/Chicago
    1526, // America/Chihuahua
    1544, // America/Costa_Rica
    1563, // America/Creston
    1579, // America/Cuiaba
    1594, // America/Curacao
    1610, // America/Danmarkshavn
    1631, // America/Dawson
    1646, // America/Dawson_Creek
    1667, // America/Denver
    1682, // America/Detroit
    1698, // America/Dominica
    1715, // America/Edmonton
    1732, // America/Eirunepe
    1749, // America/El_Salvador
    1769, // America/Fort_Nelson
    1789, // America/Fortaleza
    1807, // America/Glace_Bay
    1825, // America/Goose_Bay
    1843, // America/Grand_Turk
    1862, // America/Grenada
    1878, // America/Guadeloupe
    1897, // America/Guatemala
    1915, // America/Guayaquil
    1933, // America/Guyana
    1948, // America/Halifax
    1964, // America/Havana
    1979, // America/Hermosillo
    1998, // America/Indiana/Indianapolis
    2027, // America/Indiana/Knox
    2048, // America/Indiana/Marengo
    2072, // America/Indiana/Petersburg
    2099, // America/Indiana/Tell_City
    2125, // America/Indiana/Vevay
    2147, // America/Indiana/Vincennes
    2173, // America/Indiana/Winamac
    2197, // America/Inuvik
    2212, // America/Iqaluit
    2228, // America/Jamaica
    2244, // America/Juneau
    2259, // America/Kentucky/Louisville
    2287, // America/Kentucky/Monticello
    2315, // America/Kralendijk
    2334, // America/La_Paz
    2349, // America/Lima
    2362, // America/Los_Angeles
    2382, // America/Lower_Princes
    2404, // America/Maceio
    2419, // America/Managua
    2435, // America/Manaus
    2450, // America/Marigot
    2466, // America/Martinique
    2485, // America/Matamoros
    2503, // America/Mazatlan
    2520, // America/Menominee
    2538, // America/Merida
    2553, // America/Metlakatla
    2572, // America/Mexico_City
    2592, // America/Miquelon
    2609, // America/Moncton
    2625, // America/Monterrey
    2643, // America/Montevideo
    2662, // America/Montserrat
    2681, // America/Nassau
    2696, // America/New_York
    2713, // America/Nipigon
    2729, // America/Nome
    2742, // America/Noronha
    2758, // America/North_Dakota/Beulah
    2786, // America/North_Dakota/Center
    2814, // America/North_Dakota/New_Salem
    2845, // America/Nuuk
    2858, // America/Ojinaga
    2874, // America/Panama
    2889, // America/Pangnirtung
    2909, // America/Paramaribo
    2928, // America/Phoenix
    2944, // America/Port-au-Prince
    2967, // America/Port_of_Spain
    2989, // America/Porto_Velho
    3009, // America/Puerto_Rico
    3029, // America/Punta_Arenas
    3050, // America/Rainy_River
    3070, // America/Rankin_Inlet
    3091, // America/Recife
    3106, // America/Regina
    3121, // America/Resolute
    3138, // America/Rio_Branco
    3157, // America/Santarem
    3174, // America/Santiago
    3191, // America/Santo_Domingo
    3213, // America/Sao_Paulo
    3231, // America/Scoresbysund
    3252, // America/Sitka
    3266, // America/St_Barthelemy
    3288, // America/St_Johns
    3305, // America/St_Kitts
    3322, // America/St_Lucia
    3339, // America/St_Thomas
    3357, // America/St_Vincent
    3376, // America/Swift_Current
    3398, // America/Tegucigalpa
    3418, // America/Thule
    3432, // America/Thunder_Bay
    3452, // America/Tijuana
    3468, // America/Toronto
    3484, // America/Tortola
    3500, // America/Vancouver
    3518, // America/Whitehorse
    3537, // America/Winnipeg
    3554, // America/Yakutat
    3570, // America/Yellowknife
    3590, // Antarctica/Casey
    3607, // Antarctica/Davis
    3624, // Antarctica/DumontDUrville
    3650, // Antarctica/Macquarie
    3671, // Antarctica/Mawson
    3689, // Antarctica/McMurdo
    3708, // Antarctica/Palmer
    3726, // Antarctica/Rothera
    3745, // Antarctica/Syowa
    3762, // Antarctica/Troll
    3779, // Antarctica/Vostok
    3797, // Arctic/Longyearbyen
    3817, // Asia/Aden
    3827, // Asia/Almaty
    3839, // Asia/Amman
    3850, // Asia/Anadyr
    3862, // Asia/Aqtau
    3873, // Asia/Aqtobe
    3885, // Asia/Ashgabat
    3899, // Asia/Atyrau
    3911, // Asia/Baghdad
    3924, // Asia/Bahrain
    3937, // Asia/Baku
    3947, // Asia/Bangkok
    3960, // Asia/Barnaul
    3973, // Asia/Beirut
    3985, // Asia/Bishkek
    3998, // Asia/Brunei
    4010, // Asia/Chita
    4021, // Asia/Choibalsan
    4037, // Asia/Colombo
    4050, // Asia/Damascus
    4064, // Asia/Dhaka
    4075, // Asia/Dili
    4085, // Asia/Dubai
    4096, // Asia/Dushanbe
    4110, // Asia/Famagusta
    4125, // Asia/Gaza
    4135, // Asia/Hebron
    4147, // Asia/Ho_Chi_Minh
    4164, // Asia/Hong_Kong
    4179, // Asia/Hovd
    4189, // Asia/Irkutsk
    4202, // Asia/Jakarta
    4215, // Asia/Jayapura
    4229, // Asia/Jerusalem
    4244, // Asia/Kabul
    4255, // Asia/Kamchatka
    4270, // Asia/Karachi
    4283, // Asia/Kathmandu
    4298, // Asia/Khandyga
    4312, // Asia/Kolkata
    4325, // Asia/Krasnoyarsk
    4342, // Asia/Kuala_Lumpur
    4360, // Asia/Kuching
    4373, // Asia/Kuwait
    4385, // Asia/Macau
    4396, // Asia/Magadan
    4409, // Asia/Makassar
    4423, // Asia/Manila
    4435, // Asia/Muscat
    4447, // Asia/Nicosia
    4460, // Asia/Novokuznetsk
    4478, // Asia/Novosibirsk
    4495, // Asia/Omsk
    4505, // Asia/Oral
    4515, // Asia/Phnom_Penh
    4531, // Asia/Pontianak
    4546, // Asia/Pyongyang
    4561, // Asia/Qatar
    4572, // Asia/Qostanay
    4586, // Asia/Qyzylorda
    4601, // Asia/Riyadh
    4613, // Asia/Sakhalin
    4627, // Asia/Samarkand
    4642, // Asia/Seoul
    4653, // Asia/Shanghai
    4667, // Asia/Singapore
    4682, // Asia/Srednekolymsk
    4701, // Asia/Taipei
    4713, // Asia/Tashkent
    4727, // Asia/Tbilisi
    4740, // Asia/Tehran
    4752, // Asia/Thimphu
    4765, // Asia/Tokyo
    4776, // Asia/Tomsk
    4787, // Asia/Ulaanbaatar
    4804, // Asia/Urumqi
    4816, // Asia/Ust-Nera
    4830, // Asia/Vientiane
    4845, // Asia/Vladivostok
    4862, // Asia/Yakutsk
    4875, // Asia/Yangon
    4887, // Asia/Yekaterinburg
    4906, // Asia/Yerevan
    4919, // Atlantic/Azores
    4935, // Atlantic/Bermuda
    4952, // Atlantic/Canary
    4968, // Atlantic/Cape_Verde
    4988, // Atlantic/Faroe
    5003, // Atlantic/Madeira
    5020, // Atlantic/Reykjavik
    5039, // Atlantic/South_Georgia
    5062, // Atlantic/St_Helena
    5081, // Atlantic/Stanley
    5098, // Australia/Adelaide
    5117, // Australia/Brisbane
    5136, // Australia/Broken_Hill
    5158, // Australia/Currie
    5175, // Australia/Darwin
    5192, // Australia/Eucla
    5208, // Australia/Hobart
    5225, // Australia/Lindeman
    5244, // Australia/Lord_Howe
    5264, // Australia/Melbourne
    5284, // Australia/Perth
    5300, // Australia/Sydney
    5317, // Europe/Amsterdam
    5334, // Europe/Andorra
    5349, // Europe/Astrakhan
    5366, // Europe/Athens
    5380, // Europe/Belgrade
    5396, // Europe/Berlin
    5410, // Europe/Bratislava
    5428, // Europe/Brussels
    5444, // Europe/Bucharest
    5461, // Europe/Budapest
    5477, // Europe/Busingen
    5493, // Europe/Chisinau
    5509, // Europe/Copenhagen
    5527, // Europe/Dublin
    5541, // Europe/Gibraltar
    5558, // Europe/Guernsey
    5574, // Europe/Helsinki
    5590, // Europe/Isle_of_Man
    5609, // Europe/Istanbul
    5625, // Europe/Jersey
    5639, // Europe/Kaliningrad
    5658, // Europe/Kiev
    5670, // Europe/Kirov
    5683, // Europe/Lisbon
    5697, // Europe/Ljubljana
    5714, // Europe/London
    5728, // Europe/Luxembourg
    5746, // Europe/Madrid
    5760, // Europe/Malta
    5773, // Europe/Mariehamn
    5790, // Europe/Minsk
    5803, // Europe/Monaco
    5817, // Europe/Moscow
    5831, // Europe/Oslo
    5843, // Europe/Paris
    5856, // Europe/Podgorica
    5873, // Europe/Prague
    5887, // Europe/Riga
    5899, // Europe/Rome
    5911, // Europe/Samara
    5925, // Europe/San_Marino
    5943, // Europe/Sarajevo
    5959, // Europe/Saratov
    5974, // Europe/Simferopol
    5992, // Europe/Skopje
    6006, // Europe/Sofia
    6019, // Europe/Stockholm
    6036, // Europe/Tallinn
    6051, // Europe/Tirane
    6065, // Europe/Ulyanovsk
    6082, // Europe/Uzhgorod
    6098, // Europe/Vaduz
    6111, // Europe/Vatican
    6126, // Europe/Vienna
    6140, // Europe/Vilnius
    6155, // Europe/Volgograd
    6172, // Europe/Warsaw
    6186, // Europe/Zagreb
    6200, // Europe/Zaporozhye
    6218, // Europe/Zurich
    6232, // Indian/Antananarivo
    6252, // Indian/Chagos
    6266, // Indian/Christmas
    6283, // Indian/Cocos
    6296, // Indian/Comoro
    6310, // Indian/Kerguelen
    6327, // Indian/Mahe
    6339, // Indian/Maldives
    6355, // Indian/Mauritius
    6372, // Indian/Mayotte
    6387, // Indian/Reunion
    6402, // Pacific/Apia
    6415, // Pacific/Auckland
    6432, // Pacific/Bougainville
    6453, // Pacific/Chatham
    6469, // Pacific/Chuuk
    6483, // Pacific/Easter
    6498, // Pacific/Efate
    6512, // Pacific/Enderbury
    6530, // Pacific/Fakaofo
    6546, // Pacific/Fiji
    6559, // Pacific/Funafuti
    6576, // Pacific/Galapagos
    6594, // Pacific/Gambier
    6610, // Pacific/Guadalcanal
    6630, // Pacific/Guam
    6643, // Pacific/Honolulu
    6660, // Pacific/Kiritimati
    6679, // Pacific/Kosrae
    6694, // Pacific/Kwajalein
    6712, // Pacific/Majuro
    6727, // Pacific/Marquesas
    6745, // Pacific/Midway
    6760, // Pacific/Nauru
    6774, // Pacific/Niue
    6787, // Pacific/Norfolk
    6803, // Pacific/Noumea
    6818, // Pacific/Pago_Pago
    6836, // Pacific/Palau
    6850, // Pacific/Pitcairn
    6867, // Pacific/Pohnpei
    6883, // Pacific/Port_Moresby
    6904, // Pacific/Rarotonga
    6922, // Pacific/Saipan
    6937, // Pacific/Tahiti
    6952, // Pacific/Tarawa
    6967, // Pacific/Tongatapu
    6985, // Pacific/Wake
    6998, // Pacific/Wallis
};

static constexpr const CountryTimezoneMap country_timezone_map[] = {
    { CountryId{"AD"}, Tz::Europe_Andorra },
    { CountryId{"AE"}, Tz::Asia_Dubai },
    { CountryId{"AF"}, Tz::Asia_Kabul },
    { CountryId{"AG"}, Tz::America_Antigua },
    { CountryId{"AI"}, Tz::America_Anguilla },
    { CountryId{"AL"}, Tz::Europe_Tirane },
    { CountryId{"AM"}, Tz::Asia_Yerevan },
    { CountryId{"AO"}, Tz::Africa_Luanda },
    { CountryId{"AR"}, Tz::America_Argentina_Buenos_Aires },
    { CountryId{"AS"}, Tz::Pacific_Pago_Pago },
    { CountryId{"AT"}, Tz::Europe_Vienna },
    { CountryId{"AW"}, Tz::America_Aruba },
    { CountryId{"AX"}, Tz::Europe_Mariehamn },
    { CountryId{"AZ"}, Tz::Asia_Baku },
    { CountryId{"BA"}, Tz::Europe_Sarajevo },
    { CountryId{"BB"}, Tz::America_Barbados },
    { CountryId{"BD"}, Tz::Asia_Dhaka },
    { CountryId{"BE"}, Tz::Europe_Brussels },
    { CountryId{"BF"}, Tz::Africa_Ouagadougou },
    { CountryId{"BG"}, Tz::Europe_Sofia },
    { CountryId{"BH"}, Tz::Asia_Bahrain },
    { CountryId{"BI"}, Tz::Africa_Bujumbura },
    { CountryId{"BJ"}, Tz::Africa_Porto_Novo },
    { CountryId{"BL"}, Tz::America_St_Barthelemy },
    { CountryId{"BM"}, Tz::Atlantic_Bermuda },
    { CountryId{"BN"}, Tz::Asia_Brunei },
    { CountryId{"BO"}, Tz::America_La_Paz },
    { CountryId{"BQ"}, Tz::America_Kralendijk },
    { CountryId{"BS"}, Tz::America_Nassau },
    { CountryId{"BT"}, Tz::Asia_Thimphu },
    { CountryId{"BW"}, Tz::Africa_Gaborone },
    { CountryId{"BY"}, Tz::Europe_Minsk },
    { CountryId{"BZ"}, Tz::America_Belize },
    { CountryId{"CC"}, Tz::Indian_Cocos },
    { CountryId{"CF"}, Tz::Africa_Bangui },
    { CountryId{"CG"}, Tz::Africa_Brazzaville },
    { CountryId{"CH"}, Tz::Europe_Zurich },
    { CountryId{"CI"}, Tz::Africa_Abidjan },
    { CountryId{"CK"}, Tz::Pacific_Rarotonga },
    { CountryId{"CM"}, Tz::Africa_Douala },
    { CountryId{"CN"}, Tz::Asia_Shanghai },
    { CountryId{"CO"}, Tz::America_Bogota },
    { CountryId{"CR"}, Tz::America_Costa_Rica },
    { CountryId{"CU"}, Tz::America_Havana },
    { CountryId{"CV"}, Tz::Atlantic_Cape_Verde },
    { CountryId{"CW"}, Tz::America_Curacao },
    { CountryId{"CX"}, Tz::Indian_Christmas },
    { CountryId{"CY"}, Tz::Asia_Nicosia },
    { CountryId{"CZ"}, Tz::Europe_Prague },
    { CountryId{"DE"}, Tz::Europe_Berlin },
    { CountryId{"DJ"}, Tz::Africa_Djibouti },
    { CountryId{"DK"}, Tz::Europe_Copenhagen },
    { CountryId{"DM"}, Tz::America_Dominica },
    { CountryId{"DO"}, Tz::America_Santo_Domingo },
    { CountryId{"DZ"}, Tz::Africa_Algiers },
    { CountryId{"EE"}, Tz::Europe_Tallinn },
    { CountryId{"EG"}, Tz::Africa_Cairo },
    { CountryId{"EH"}, Tz::Africa_El_Aaiun },
    { CountryId{"ER"}, Tz::Africa_Asmara },
    { CountryId{"ET"}, Tz::Africa_Addis_Ababa },
    { CountryId{"FI"}, Tz::Europe_Helsinki },
    { CountryId{"FJ"}, Tz::Pacific_Fiji },
    { CountryId{"FK"}, Tz::Atlantic_Stanley },
    { CountryId{"FO"}, Tz::Atlantic_Faroe },
    { CountryId{"FR"}, Tz::Europe_Paris },
    { CountryId{"GA"}, Tz::Africa_Libreville },
    { CountryId{"GB"}, Tz::Europe_London },
    { CountryId{"GD"}, Tz::America_Grenada },
    { CountryId{"GE"}, Tz::Asia_Tbilisi },
    { CountryId{"GF"}, Tz::America_Cayenne },
    { CountryId{"GG"}, Tz::Europe_Guernsey },
    { CountryId{"GH"}, Tz::Africa_Accra },
    { CountryId{"GI"}, Tz::Europe_Gibraltar },
    { CountryId{"GM"}, Tz::Africa_Banjul },
    { CountryId{"GN"}, Tz::Africa_Conakry },
    { CountryId{"GP"}, Tz::America_Guadeloupe },
    { CountryId{"GQ"}, Tz::Africa_Malabo },
    { CountryId{"GR"}, Tz::Europe_Athens },
    { CountryId{"GS"}, Tz::Atlantic_South_Georgia },
    { CountryId{"GT"}, Tz::America_Guatemala },
    { CountryId{"GU"}, Tz::Pacific_Guam },
    { CountryId{"GW"}, Tz::Africa_Bissau },
    { CountryId{"GY"}, Tz::America_Guyana },
    { CountryId{"HK"}, Tz::Asia_Hong_Kong },
    { CountryId{"HN"}, Tz::America_Tegucigalpa },
    { CountryId{"HR"}, Tz::Europe_Zagreb },
    { CountryId{"HT"}, Tz::America_Port_au_Prince },
    { CountryId{"HU"}, Tz::Europe_Budapest },
    { CountryId{"IE"}, Tz::Europe_Dublin },
    { CountryId{"IL"}, Tz::Asia_Jerusalem },
    { CountryId{"IM"}, Tz::Europe_Isle_of_Man },
    { CountryId{"IN"}, Tz::Asia_Kolkata },
    { CountryId{"IO"}, Tz::Indian_Chagos },
    { CountryId{"IQ"}, Tz::Asia_Baghdad },
    { CountryId{"IR"}, Tz::Asia_Tehran },
    { CountryId{"IS"}, Tz::Atlantic_Reykjavik },
    { CountryId{"IT"}, Tz::Europe_Rome },
    { CountryId{"JE"}, Tz::Europe_Jersey },
    { CountryId{"JM"}, Tz::America_Jamaica },
    { CountryId{"JO"}, Tz::Asia_Amman },
    { CountryId{"JP"}, Tz::Asia_Tokyo },
    { CountryId{"KE"}, Tz::Africa_Nairobi },
    { CountryId{"KG"}, Tz::Asia_Bishkek },
    { CountryId{"KH"}, Tz::Asia_Phnom_Penh },
    { CountryId{"KM"}, Tz::Indian_Comoro },
    { CountryId{"KN"}, Tz::America_St_Kitts },
    { CountryId{"KP"}, Tz::Asia_Pyongyang },
    { CountryId{"KR"}, Tz::Asia_Seoul },
    { CountryId{"KW"}, Tz::Asia_Kuwait },
    { CountryId{"KY"}, Tz::America_Cayman },
    { CountryId{"LA"}, Tz::Asia_Vientiane },
    { CountryId{"LB"}, Tz::Asia_Beirut },
    { CountryId{"LC"}, Tz::America_St_Lucia },
    { CountryId{"LI"}, Tz::Europe_Vaduz },
    { CountryId{"LK"}, Tz::Asia_Colombo },
    { CountryId{"LR"}, Tz::Africa_Monrovia },
    { CountryId{"LS"}, Tz::Africa_Maseru },
    { CountryId{"LT"}, Tz::Europe_Vilnius },
    { CountryId{"LU"}, Tz::Europe_Luxembourg },
    { CountryId{"LV"}, Tz::Europe_Riga },
    { CountryId{"LY"}, Tz::Africa_Tripoli },
    { CountryId{"MA"}, Tz::Africa_Casablanca },
    { CountryId{"MC"}, Tz::Europe_Monaco },
    { CountryId{"MD"}, Tz::Europe_Chisinau },
    { CountryId{"ME"}, Tz::Europe_Podgorica },
    { CountryId{"MF"}, Tz::America_Marigot },
    { CountryId{"MG"}, Tz::Indian_Antananarivo },
    { CountryId{"MK"}, Tz::Europe_Skopje },
    { CountryId{"ML"}, Tz::Africa_Bamako },
    { CountryId{"MM"}, Tz::Asia_Yangon },
    { CountryId{"MO"}, Tz::Asia_Macau },
    { CountryId{"MP"}, Tz::Pacific_Saipan },
    { CountryId{"MQ"}, Tz::America_Martinique },
    { CountryId{"MR"}, Tz::Africa_Nouakchott },
    { CountryId{"MS"}, Tz::America_Montserrat },
    { CountryId{"MT"}, Tz::Europe_Malta },
    { CountryId{"MU"}, Tz::Indian_Mauritius },
    { CountryId{"MV"}, Tz::Indian_Maldives },
    { CountryId{"MW"}, Tz::Africa_Blantyre },
    { CountryId{"MY"}, Tz::Asia_Kuala_Lumpur },
    { CountryId{"MZ"}, Tz::Africa_Maputo },
    { CountryId{"NA"}, Tz::Africa_Windhoek },
    { CountryId{"NC"}, Tz::Pacific_Noumea },
    { CountryId{"NE"}, Tz::Africa_Niamey },
    { CountryId{"NF"}, Tz::Pacific_Norfolk },
    { CountryId{"NG"}, Tz::Africa_Lagos },
    { CountryId{"NI"}, Tz::America_Managua },
    { CountryId{"NL"}, Tz::Europe_Amsterdam },
    { CountryId{"NO"}, Tz::Europe_Oslo },
    { CountryId{"NP"}, Tz::Asia_Kathmandu },
    { CountryId{"NR"}, Tz::Pacific_Nauru },
    { CountryId{"NU"}, Tz::Pacific_Niue },
    { CountryId{"OM"}, Tz::Asia_Muscat },
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
    { CountryId{"RW"}, Tz::Africa_Kigali },
    { CountryId{"SA"}, Tz::Asia_Riyadh },
    { CountryId{"SB"}, Tz::Pacific_Guadalcanal },
    { CountryId{"SC"}, Tz::Indian_Mahe },
    { CountryId{"SD"}, Tz::Africa_Khartoum },
    { CountryId{"SE"}, Tz::Europe_Stockholm },
    { CountryId{"SG"}, Tz::Asia_Singapore },
    { CountryId{"SH"}, Tz::Atlantic_St_Helena },
    { CountryId{"SI"}, Tz::Europe_Ljubljana },
    { CountryId{"SJ"}, Tz::Arctic_Longyearbyen },
    { CountryId{"SK"}, Tz::Europe_Bratislava },
    { CountryId{"SL"}, Tz::Africa_Freetown },
    { CountryId{"SM"}, Tz::Europe_San_Marino },
    { CountryId{"SN"}, Tz::Africa_Dakar },
    { CountryId{"SO"}, Tz::Africa_Mogadishu },
    { CountryId{"SR"}, Tz::America_Paramaribo },
    { CountryId{"SS"}, Tz::Africa_Juba },
    { CountryId{"ST"}, Tz::Africa_Sao_Tome },
    { CountryId{"SV"}, Tz::America_El_Salvador },
    { CountryId{"SX"}, Tz::America_Lower_Princes },
    { CountryId{"SY"}, Tz::Asia_Damascus },
    { CountryId{"SZ"}, Tz::Africa_Mbabane },
    { CountryId{"TC"}, Tz::America_Grand_Turk },
    { CountryId{"TD"}, Tz::Africa_Ndjamena },
    { CountryId{"TF"}, Tz::Indian_Kerguelen },
    { CountryId{"TG"}, Tz::Africa_Lome },
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
    { CountryId{"TZ"}, Tz::Africa_Dar_es_Salaam },
    { CountryId{"UG"}, Tz::Africa_Kampala },
    { CountryId{"UY"}, Tz::America_Montevideo },
    { CountryId{"VA"}, Tz::Europe_Vatican },
    { CountryId{"VC"}, Tz::America_St_Vincent },
    { CountryId{"VE"}, Tz::America_Caracas },
    { CountryId{"VG"}, Tz::America_Tortola },
    { CountryId{"VI"}, Tz::America_St_Thomas },
    { CountryId{"VN"}, Tz::Asia_Ho_Chi_Minh },
    { CountryId{"VU"}, Tz::Pacific_Efate },
    { CountryId{"WF"}, Tz::Pacific_Wallis },
    { CountryId{"WS"}, Tz::Pacific_Apia },
    { CountryId{"YE"}, Tz::Asia_Aden },
    { CountryId{"YT"}, Tz::Indian_Mayotte },
    { CountryId{"ZA"}, Tz::Africa_Johannesburg },
    { CountryId{"ZM"}, Tz::Africa_Lusaka },
    { CountryId{"ZW"}, Tz::Africa_Harare },
};

static constexpr const CountryId timezone_country_map[] = {
    CountryId{}, // Undefined
    CountryId{"CI"}, // Africa/Abidjan
    CountryId{"GH"}, // Africa/Accra
    CountryId{"ET"}, // Africa/Addis_Ababa
    CountryId{"DZ"}, // Africa/Algiers
    CountryId{"ER"}, // Africa/Asmara
    CountryId{"ML"}, // Africa/Bamako
    CountryId{"CF"}, // Africa/Bangui
    CountryId{"GM"}, // Africa/Banjul
    CountryId{"GW"}, // Africa/Bissau
    CountryId{"MW"}, // Africa/Blantyre
    CountryId{"CG"}, // Africa/Brazzaville
    CountryId{"BI"}, // Africa/Bujumbura
    CountryId{"EG"}, // Africa/Cairo
    CountryId{"MA"}, // Africa/Casablanca
    CountryId{"ES"}, // Africa/Ceuta
    CountryId{"GN"}, // Africa/Conakry
    CountryId{"SN"}, // Africa/Dakar
    CountryId{"TZ"}, // Africa/Dar_es_Salaam
    CountryId{"DJ"}, // Africa/Djibouti
    CountryId{"CM"}, // Africa/Douala
    CountryId{"EH"}, // Africa/El_Aaiun
    CountryId{"SL"}, // Africa/Freetown
    CountryId{"BW"}, // Africa/Gaborone
    CountryId{"ZW"}, // Africa/Harare
    CountryId{"ZA"}, // Africa/Johannesburg
    CountryId{"SS"}, // Africa/Juba
    CountryId{"UG"}, // Africa/Kampala
    CountryId{"SD"}, // Africa/Khartoum
    CountryId{"RW"}, // Africa/Kigali
    CountryId{"CD"}, // Africa/Kinshasa
    CountryId{"NG"}, // Africa/Lagos
    CountryId{"GA"}, // Africa/Libreville
    CountryId{"TG"}, // Africa/Lome
    CountryId{"AO"}, // Africa/Luanda
    CountryId{"CD"}, // Africa/Lubumbashi
    CountryId{"ZM"}, // Africa/Lusaka
    CountryId{"GQ"}, // Africa/Malabo
    CountryId{"MZ"}, // Africa/Maputo
    CountryId{"LS"}, // Africa/Maseru
    CountryId{"SZ"}, // Africa/Mbabane
    CountryId{"SO"}, // Africa/Mogadishu
    CountryId{"LR"}, // Africa/Monrovia
    CountryId{"KE"}, // Africa/Nairobi
    CountryId{"TD"}, // Africa/Ndjamena
    CountryId{"NE"}, // Africa/Niamey
    CountryId{"MR"}, // Africa/Nouakchott
    CountryId{"BF"}, // Africa/Ouagadougou
    CountryId{"BJ"}, // Africa/Porto-Novo
    CountryId{"ST"}, // Africa/Sao_Tome
    CountryId{"LY"}, // Africa/Tripoli
    CountryId{"TN"}, // Africa/Tunis
    CountryId{"NA"}, // Africa/Windhoek
    CountryId{"US"}, // America/Adak
    CountryId{"US"}, // America/Anchorage
    CountryId{"AI"}, // America/Anguilla
    CountryId{"AG"}, // America/Antigua
    CountryId{"BR"}, // America/Araguaina
    CountryId{"AR"}, // America/Argentina/Buenos_Aires
    CountryId{"AR"}, // America/Argentina/Catamarca
    CountryId{"AR"}, // America/Argentina/Cordoba
    CountryId{"AR"}, // America/Argentina/Jujuy
    CountryId{"AR"}, // America/Argentina/La_Rioja
    CountryId{"AR"}, // America/Argentina/Mendoza
    CountryId{"AR"}, // America/Argentina/Rio_Gallegos
    CountryId{"AR"}, // America/Argentina/Salta
    CountryId{"AR"}, // America/Argentina/San_Juan
    CountryId{"AR"}, // America/Argentina/San_Luis
    CountryId{"AR"}, // America/Argentina/Tucuman
    CountryId{"AR"}, // America/Argentina/Ushuaia
    CountryId{"AW"}, // America/Aruba
    CountryId{"PY"}, // America/Asuncion
    CountryId{"CA"}, // America/Atikokan
    CountryId{"BR"}, // America/Bahia
    CountryId{"MX"}, // America/Bahia_Banderas
    CountryId{"BB"}, // America/Barbados
    CountryId{"BR"}, // America/Belem
    CountryId{"BZ"}, // America/Belize
    CountryId{"CA"}, // America/Blanc-Sablon
    CountryId{"BR"}, // America/Boa_Vista
    CountryId{"CO"}, // America/Bogota
    CountryId{"US"}, // America/Boise
    CountryId{"CA"}, // America/Cambridge_Bay
    CountryId{"BR"}, // America/Campo_Grande
    CountryId{"MX"}, // America/Cancun
    CountryId{"VE"}, // America/Caracas
    CountryId{"GF"}, // America/Cayenne
    CountryId{"KY"}, // America/Cayman
    CountryId{"US"}, // America/Chicago
    CountryId{"MX"}, // America/Chihuahua
    CountryId{"CR"}, // America/Costa_Rica
    CountryId{"CA"}, // America/Creston
    CountryId{"BR"}, // America/Cuiaba
    CountryId{"CW"}, // America/Curacao
    CountryId{"GL"}, // America/Danmarkshavn
    CountryId{"CA"}, // America/Dawson
    CountryId{"CA"}, // America/Dawson_Creek
    CountryId{"US"}, // America/Denver
    CountryId{"US"}, // America/Detroit
    CountryId{"DM"}, // America/Dominica
    CountryId{"CA"}, // America/Edmonton
    CountryId{"BR"}, // America/Eirunepe
    CountryId{"SV"}, // America/El_Salvador
    CountryId{"CA"}, // America/Fort_Nelson
    CountryId{"BR"}, // America/Fortaleza
    CountryId{"CA"}, // America/Glace_Bay
    CountryId{"CA"}, // America/Goose_Bay
    CountryId{"TC"}, // America/Grand_Turk
    CountryId{"GD"}, // America/Grenada
    CountryId{"GP"}, // America/Guadeloupe
    CountryId{"GT"}, // America/Guatemala
    CountryId{"EC"}, // America/Guayaquil
    CountryId{"GY"}, // America/Guyana
    CountryId{"CA"}, // America/Halifax
    CountryId{"CU"}, // America/Havana
    CountryId{"MX"}, // America/Hermosillo
    CountryId{"US"}, // America/Indiana/Indianapolis
    CountryId{"US"}, // America/Indiana/Knox
    CountryId{"US"}, // America/Indiana/Marengo
    CountryId{"US"}, // America/Indiana/Petersburg
    CountryId{"US"}, // America/Indiana/Tell_City
    CountryId{"US"}, // America/Indiana/Vevay
    CountryId{"US"}, // America/Indiana/Vincennes
    CountryId{"US"}, // America/Indiana/Winamac
    CountryId{"CA"}, // America/Inuvik
    CountryId{"CA"}, // America/Iqaluit
    CountryId{"JM"}, // America/Jamaica
    CountryId{"US"}, // America/Juneau
    CountryId{"US"}, // America/Kentucky/Louisville
    CountryId{"US"}, // America/Kentucky/Monticello
    CountryId{"BQ"}, // America/Kralendijk
    CountryId{"BO"}, // America/La_Paz
    CountryId{"PE"}, // America/Lima
    CountryId{"US"}, // America/Los_Angeles
    CountryId{"SX"}, // America/Lower_Princes
    CountryId{"BR"}, // America/Maceio
    CountryId{"NI"}, // America/Managua
    CountryId{"BR"}, // America/Manaus
    CountryId{"MF"}, // America/Marigot
    CountryId{"MQ"}, // America/Martinique
    CountryId{"MX"}, // America/Matamoros
    CountryId{"MX"}, // America/Mazatlan
    CountryId{"US"}, // America/Menominee
    CountryId{"MX"}, // America/Merida
    CountryId{"US"}, // America/Metlakatla
    CountryId{"MX"}, // America/Mexico_City
    CountryId{"PM"}, // America/Miquelon
    CountryId{"CA"}, // America/Moncton
    CountryId{"MX"}, // America/Monterrey
    CountryId{"UY"}, // America/Montevideo
    CountryId{"MS"}, // America/Montserrat
    CountryId{"BS"}, // America/Nassau
    CountryId{"US"}, // America/New_York
    CountryId{"CA"}, // America/Nipigon
    CountryId{"US"}, // America/Nome
    CountryId{"BR"}, // America/Noronha
    CountryId{"US"}, // America/North_Dakota/Beulah
    CountryId{"US"}, // America/North_Dakota/Center
    CountryId{"US"}, // America/North_Dakota/New_Salem
    CountryId{"GL"}, // America/Nuuk
    CountryId{"MX"}, // America/Ojinaga
    CountryId{"PA"}, // America/Panama
    CountryId{"CA"}, // America/Pangnirtung
    CountryId{"SR"}, // America/Paramaribo
    CountryId{"US"}, // America/Phoenix
    CountryId{"HT"}, // America/Port-au-Prince
    CountryId{"TT"}, // America/Port_of_Spain
    CountryId{"BR"}, // America/Porto_Velho
    CountryId{"PR"}, // America/Puerto_Rico
    CountryId{"CL"}, // America/Punta_Arenas
    CountryId{"CA"}, // America/Rainy_River
    CountryId{"CA"}, // America/Rankin_Inlet
    CountryId{"BR"}, // America/Recife
    CountryId{"CA"}, // America/Regina
    CountryId{"CA"}, // America/Resolute
    CountryId{"BR"}, // America/Rio_Branco
    CountryId{"BR"}, // America/Santarem
    CountryId{"CL"}, // America/Santiago
    CountryId{"DO"}, // America/Santo_Domingo
    CountryId{"BR"}, // America/Sao_Paulo
    CountryId{"GL"}, // America/Scoresbysund
    CountryId{"US"}, // America/Sitka
    CountryId{"BL"}, // America/St_Barthelemy
    CountryId{"CA"}, // America/St_Johns
    CountryId{"KN"}, // America/St_Kitts
    CountryId{"LC"}, // America/St_Lucia
    CountryId{"VI"}, // America/St_Thomas
    CountryId{"VC"}, // America/St_Vincent
    CountryId{"CA"}, // America/Swift_Current
    CountryId{"HN"}, // America/Tegucigalpa
    CountryId{"GL"}, // America/Thule
    CountryId{"CA"}, // America/Thunder_Bay
    CountryId{"MX"}, // America/Tijuana
    CountryId{"CA"}, // America/Toronto
    CountryId{"VG"}, // America/Tortola
    CountryId{"CA"}, // America/Vancouver
    CountryId{"CA"}, // America/Whitehorse
    CountryId{"CA"}, // America/Winnipeg
    CountryId{"US"}, // America/Yakutat
    CountryId{"CA"}, // America/Yellowknife
    CountryId{"AQ"}, // Antarctica/Casey
    CountryId{"AQ"}, // Antarctica/Davis
    CountryId{"AQ"}, // Antarctica/DumontDUrville
    CountryId{"AU"}, // Antarctica/Macquarie
    CountryId{"AQ"}, // Antarctica/Mawson
    CountryId{"AQ"}, // Antarctica/McMurdo
    CountryId{"AQ"}, // Antarctica/Palmer
    CountryId{"AQ"}, // Antarctica/Rothera
    CountryId{"AQ"}, // Antarctica/Syowa
    CountryId{"AQ"}, // Antarctica/Troll
    CountryId{"AQ"}, // Antarctica/Vostok
    CountryId{"SJ"}, // Arctic/Longyearbyen
    CountryId{"YE"}, // Asia/Aden
    CountryId{"KZ"}, // Asia/Almaty
    CountryId{"JO"}, // Asia/Amman
    CountryId{"RU"}, // Asia/Anadyr
    CountryId{"KZ"}, // Asia/Aqtau
    CountryId{"KZ"}, // Asia/Aqtobe
    CountryId{"TM"}, // Asia/Ashgabat
    CountryId{"KZ"}, // Asia/Atyrau
    CountryId{"IQ"}, // Asia/Baghdad
    CountryId{"BH"}, // Asia/Bahrain
    CountryId{"AZ"}, // Asia/Baku
    CountryId{}, // Asia/Bangkok
    CountryId{"RU"}, // Asia/Barnaul
    CountryId{"LB"}, // Asia/Beirut
    CountryId{"KG"}, // Asia/Bishkek
    CountryId{"BN"}, // Asia/Brunei
    CountryId{"RU"}, // Asia/Chita
    CountryId{"MN"}, // Asia/Choibalsan
    CountryId{"LK"}, // Asia/Colombo
    CountryId{"SY"}, // Asia/Damascus
    CountryId{"BD"}, // Asia/Dhaka
    CountryId{"TL"}, // Asia/Dili
    CountryId{"AE"}, // Asia/Dubai
    CountryId{"TJ"}, // Asia/Dushanbe
    CountryId{"CY"}, // Asia/Famagusta
    CountryId{"PS"}, // Asia/Gaza
    CountryId{"PS"}, // Asia/Hebron
    CountryId{"VN"}, // Asia/Ho_Chi_Minh
    CountryId{"HK"}, // Asia/Hong_Kong
    CountryId{"MN"}, // Asia/Hovd
    CountryId{"RU"}, // Asia/Irkutsk
    CountryId{"ID"}, // Asia/Jakarta
    CountryId{"ID"}, // Asia/Jayapura
    CountryId{"IL"}, // Asia/Jerusalem
    CountryId{"AF"}, // Asia/Kabul
    CountryId{"RU"}, // Asia/Kamchatka
    CountryId{"PK"}, // Asia/Karachi
    CountryId{"NP"}, // Asia/Kathmandu
    CountryId{"RU"}, // Asia/Khandyga
    CountryId{"IN"}, // Asia/Kolkata
    CountryId{"RU"}, // Asia/Krasnoyarsk
    CountryId{"MY"}, // Asia/Kuala_Lumpur
    CountryId{"MY"}, // Asia/Kuching
    CountryId{"KW"}, // Asia/Kuwait
    CountryId{"MO"}, // Asia/Macau
    CountryId{"RU"}, // Asia/Magadan
    CountryId{"ID"}, // Asia/Makassar
    CountryId{"PH"}, // Asia/Manila
    CountryId{"OM"}, // Asia/Muscat
    CountryId{"CY"}, // Asia/Nicosia
    CountryId{"RU"}, // Asia/Novokuznetsk
    CountryId{"RU"}, // Asia/Novosibirsk
    CountryId{"RU"}, // Asia/Omsk
    CountryId{"KZ"}, // Asia/Oral
    CountryId{"KH"}, // Asia/Phnom_Penh
    CountryId{"ID"}, // Asia/Pontianak
    CountryId{"KP"}, // Asia/Pyongyang
    CountryId{"QA"}, // Asia/Qatar
    CountryId{"KZ"}, // Asia/Qostanay
    CountryId{"KZ"}, // Asia/Qyzylorda
    CountryId{"SA"}, // Asia/Riyadh
    CountryId{"RU"}, // Asia/Sakhalin
    CountryId{"UZ"}, // Asia/Samarkand
    CountryId{"KR"}, // Asia/Seoul
    CountryId{"CN"}, // Asia/Shanghai
    CountryId{"SG"}, // Asia/Singapore
    CountryId{"RU"}, // Asia/Srednekolymsk
    CountryId{"TW"}, // Asia/Taipei
    CountryId{"UZ"}, // Asia/Tashkent
    CountryId{"GE"}, // Asia/Tbilisi
    CountryId{"IR"}, // Asia/Tehran
    CountryId{"BT"}, // Asia/Thimphu
    CountryId{"JP"}, // Asia/Tokyo
    CountryId{"RU"}, // Asia/Tomsk
    CountryId{"MN"}, // Asia/Ulaanbaatar
    CountryId{"CN"}, // Asia/Urumqi
    CountryId{"RU"}, // Asia/Ust-Nera
    CountryId{"LA"}, // Asia/Vientiane
    CountryId{"RU"}, // Asia/Vladivostok
    CountryId{"RU"}, // Asia/Yakutsk
    CountryId{"MM"}, // Asia/Yangon
    CountryId{"RU"}, // Asia/Yekaterinburg
    CountryId{"AM"}, // Asia/Yerevan
    CountryId{"PT"}, // Atlantic/Azores
    CountryId{"BM"}, // Atlantic/Bermuda
    CountryId{"ES"}, // Atlantic/Canary
    CountryId{"CV"}, // Atlantic/Cape_Verde
    CountryId{"FO"}, // Atlantic/Faroe
    CountryId{"PT"}, // Atlantic/Madeira
    CountryId{"IS"}, // Atlantic/Reykjavik
    CountryId{"GS"}, // Atlantic/South_Georgia
    CountryId{"SH"}, // Atlantic/St_Helena
    CountryId{"FK"}, // Atlantic/Stanley
    CountryId{"AU"}, // Australia/Adelaide
    CountryId{"AU"}, // Australia/Brisbane
    CountryId{"AU"}, // Australia/Broken_Hill
    CountryId{"AU"}, // Australia/Currie
    CountryId{"AU"}, // Australia/Darwin
    CountryId{"AU"}, // Australia/Eucla
    CountryId{"AU"}, // Australia/Hobart
    CountryId{"AU"}, // Australia/Lindeman
    CountryId{"AU"}, // Australia/Lord_Howe
    CountryId{"AU"}, // Australia/Melbourne
    CountryId{"AU"}, // Australia/Perth
    CountryId{"AU"}, // Australia/Sydney
    CountryId{"NL"}, // Europe/Amsterdam
    CountryId{"AD"}, // Europe/Andorra
    CountryId{"RU"}, // Europe/Astrakhan
    CountryId{"GR"}, // Europe/Athens
    CountryId{"RS"}, // Europe/Belgrade
    CountryId{"DE"}, // Europe/Berlin
    CountryId{"SK"}, // Europe/Bratislava
    CountryId{"BE"}, // Europe/Brussels
    CountryId{"RO"}, // Europe/Bucharest
    CountryId{"HU"}, // Europe/Budapest
    CountryId{"DE"}, // Europe/Busingen
    CountryId{"MD"}, // Europe/Chisinau
    CountryId{"DK"}, // Europe/Copenhagen
    CountryId{"IE"}, // Europe/Dublin
    CountryId{"GI"}, // Europe/Gibraltar
    CountryId{"GG"}, // Europe/Guernsey
    CountryId{"FI"}, // Europe/Helsinki
    CountryId{"IM"}, // Europe/Isle_of_Man
    CountryId{"TR"}, // Europe/Istanbul
    CountryId{"JE"}, // Europe/Jersey
    CountryId{"RU"}, // Europe/Kaliningrad
    CountryId{"UA"}, // Europe/Kiev
    CountryId{"RU"}, // Europe/Kirov
    CountryId{"PT"}, // Europe/Lisbon
    CountryId{"SI"}, // Europe/Ljubljana
    CountryId{"GB"}, // Europe/London
    CountryId{"LU"}, // Europe/Luxembourg
    CountryId{"ES"}, // Europe/Madrid
    CountryId{"MT"}, // Europe/Malta
    CountryId{"AX"}, // Europe/Mariehamn
    CountryId{"BY"}, // Europe/Minsk
    CountryId{"MC"}, // Europe/Monaco
    CountryId{"RU"}, // Europe/Moscow
    CountryId{"NO"}, // Europe/Oslo
    CountryId{"FR"}, // Europe/Paris
    CountryId{"ME"}, // Europe/Podgorica
    CountryId{"CZ"}, // Europe/Prague
    CountryId{"LV"}, // Europe/Riga
    CountryId{"IT"}, // Europe/Rome
    CountryId{"RU"}, // Europe/Samara
    CountryId{"SM"}, // Europe/San_Marino
    CountryId{"BA"}, // Europe/Sarajevo
    CountryId{"RU"}, // Europe/Saratov
    CountryId{}, // Europe/Simferopol
    CountryId{"MK"}, // Europe/Skopje
    CountryId{"BG"}, // Europe/Sofia
    CountryId{"SE"}, // Europe/Stockholm
    CountryId{"EE"}, // Europe/Tallinn
    CountryId{"AL"}, // Europe/Tirane
    CountryId{"RU"}, // Europe/Ulyanovsk
    CountryId{"UA"}, // Europe/Uzhgorod
    CountryId{"LI"}, // Europe/Vaduz
    CountryId{"VA"}, // Europe/Vatican
    CountryId{"AT"}, // Europe/Vienna
    CountryId{"LT"}, // Europe/Vilnius
    CountryId{"RU"}, // Europe/Volgograd
    CountryId{"PL"}, // Europe/Warsaw
    CountryId{"HR"}, // Europe/Zagreb
    CountryId{"UA"}, // Europe/Zaporozhye
    CountryId{"CH"}, // Europe/Zurich
    CountryId{"MG"}, // Indian/Antananarivo
    CountryId{"IO"}, // Indian/Chagos
    CountryId{"CX"}, // Indian/Christmas
    CountryId{"CC"}, // Indian/Cocos
    CountryId{"KM"}, // Indian/Comoro
    CountryId{"TF"}, // Indian/Kerguelen
    CountryId{"SC"}, // Indian/Mahe
    CountryId{"MV"}, // Indian/Maldives
    CountryId{"MU"}, // Indian/Mauritius
    CountryId{"YT"}, // Indian/Mayotte
    CountryId{"RE"}, // Indian/Reunion
    CountryId{"WS"}, // Pacific/Apia
    CountryId{"NZ"}, // Pacific/Auckland
    CountryId{"PG"}, // Pacific/Bougainville
    CountryId{"NZ"}, // Pacific/Chatham
    CountryId{"FM"}, // Pacific/Chuuk
    CountryId{"CL"}, // Pacific/Easter
    CountryId{"VU"}, // Pacific/Efate
    CountryId{"KI"}, // Pacific/Enderbury
    CountryId{"TK"}, // Pacific/Fakaofo
    CountryId{"FJ"}, // Pacific/Fiji
    CountryId{"TV"}, // Pacific/Funafuti
    CountryId{"EC"}, // Pacific/Galapagos
    CountryId{"PF"}, // Pacific/Gambier
    CountryId{"SB"}, // Pacific/Guadalcanal
    CountryId{"GU"}, // Pacific/Guam
    CountryId{"US"}, // Pacific/Honolulu
    CountryId{"KI"}, // Pacific/Kiritimati
    CountryId{"FM"}, // Pacific/Kosrae
    CountryId{"MH"}, // Pacific/Kwajalein
    CountryId{"MH"}, // Pacific/Majuro
    CountryId{"PF"}, // Pacific/Marquesas
    CountryId{"UM"}, // Pacific/Midway
    CountryId{"NR"}, // Pacific/Nauru
    CountryId{"NU"}, // Pacific/Niue
    CountryId{"NF"}, // Pacific/Norfolk
    CountryId{"NC"}, // Pacific/Noumea
    CountryId{"AS"}, // Pacific/Pago_Pago
    CountryId{"PW"}, // Pacific/Palau
    CountryId{"PN"}, // Pacific/Pitcairn
    CountryId{"FM"}, // Pacific/Pohnpei
    CountryId{"PG"}, // Pacific/Port_Moresby
    CountryId{"CK"}, // Pacific/Rarotonga
    CountryId{"MP"}, // Pacific/Saipan
    CountryId{"PF"}, // Pacific/Tahiti
    CountryId{"KI"}, // Pacific/Tarawa
    CountryId{"TO"}, // Pacific/Tongatapu
    CountryId{"UM"}, // Pacific/Wake
    CountryId{"WF"}, // Pacific/Wallis
};

}
}

/*
 * SPDX-License-Identifier: ODbL-1.0
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
    1825, // America/Godthab
    1841, // America/Goose_Bay
    1859, // America/Grand_Turk
    1878, // America/Grenada
    1894, // America/Guadeloupe
    1913, // America/Guatemala
    1931, // America/Guayaquil
    1949, // America/Guyana
    1964, // America/Halifax
    1980, // America/Havana
    1995, // America/Hermosillo
    2014, // America/Indiana/Indianapolis
    2043, // America/Indiana/Knox
    2064, // America/Indiana/Marengo
    2088, // America/Indiana/Petersburg
    2115, // America/Indiana/Tell_City
    2141, // America/Indiana/Vevay
    2163, // America/Indiana/Vincennes
    2189, // America/Indiana/Winamac
    2213, // America/Inuvik
    2228, // America/Iqaluit
    2244, // America/Jamaica
    2260, // America/Juneau
    2275, // America/Kentucky/Louisville
    2303, // America/Kentucky/Monticello
    2331, // America/Kralendijk
    2350, // America/La_Paz
    2365, // America/Lima
    2378, // America/Los_Angeles
    2398, // America/Lower_Princes
    2420, // America/Maceio
    2435, // America/Managua
    2451, // America/Manaus
    2466, // America/Marigot
    2482, // America/Martinique
    2501, // America/Matamoros
    2519, // America/Mazatlan
    2536, // America/Menominee
    2554, // America/Merida
    2569, // America/Metlakatla
    2588, // America/Mexico_City
    2608, // America/Miquelon
    2625, // America/Moncton
    2641, // America/Monterrey
    2659, // America/Montevideo
    2678, // America/Montserrat
    2697, // America/Nassau
    2712, // America/New_York
    2729, // America/Nipigon
    2745, // America/Nome
    2758, // America/Noronha
    2774, // America/North_Dakota/Beulah
    2802, // America/North_Dakota/Center
    2830, // America/North_Dakota/New_Salem
    2861, // America/Ojinaga
    2877, // America/Panama
    2892, // America/Pangnirtung
    2912, // America/Paramaribo
    2931, // America/Phoenix
    2947, // America/Port-au-Prince
    2970, // America/Port_of_Spain
    2992, // America/Porto_Velho
    3012, // America/Puerto_Rico
    3032, // America/Punta_Arenas
    3053, // America/Rainy_River
    3073, // America/Rankin_Inlet
    3094, // America/Recife
    3109, // America/Regina
    3124, // America/Resolute
    3141, // America/Rio_Branco
    3160, // America/Santarem
    3177, // America/Santiago
    3194, // America/Santo_Domingo
    3216, // America/Sao_Paulo
    3234, // America/Scoresbysund
    3255, // America/Sitka
    3269, // America/St_Barthelemy
    3291, // America/St_Johns
    3308, // America/St_Kitts
    3325, // America/St_Lucia
    3342, // America/St_Thomas
    3360, // America/St_Vincent
    3379, // America/Swift_Current
    3401, // America/Tegucigalpa
    3421, // America/Thule
    3435, // America/Thunder_Bay
    3455, // America/Tijuana
    3471, // America/Toronto
    3487, // America/Tortola
    3503, // America/Vancouver
    3521, // America/Whitehorse
    3540, // America/Winnipeg
    3557, // America/Yakutat
    3573, // America/Yellowknife
    3593, // Antarctica/Macquarie
    3614, // Arctic/Longyearbyen
    3634, // Asia/Aden
    3644, // Asia/Almaty
    3656, // Asia/Amman
    3667, // Asia/Anadyr
    3679, // Asia/Aqtau
    3690, // Asia/Aqtobe
    3702, // Asia/Ashgabat
    3716, // Asia/Atyrau
    3728, // Asia/Baghdad
    3741, // Asia/Bahrain
    3754, // Asia/Baku
    3764, // Asia/Bangkok
    3777, // Asia/Barnaul
    3790, // Asia/Beirut
    3802, // Asia/Bishkek
    3815, // Asia/Brunei
    3827, // Asia/Chita
    3838, // Asia/Choibalsan
    3854, // Asia/Colombo
    3867, // Asia/Damascus
    3881, // Asia/Dhaka
    3892, // Asia/Dili
    3902, // Asia/Dubai
    3913, // Asia/Dushanbe
    3927, // Asia/Famagusta
    3942, // Asia/Gaza
    3952, // Asia/Hebron
    3964, // Asia/Ho_Chi_Minh
    3981, // Asia/Hong_Kong
    3996, // Asia/Hovd
    4006, // Asia/Irkutsk
    4019, // Asia/Jakarta
    4032, // Asia/Jayapura
    4046, // Asia/Jerusalem
    4061, // Asia/Kabul
    4072, // Asia/Kamchatka
    4087, // Asia/Karachi
    4100, // Asia/Kathmandu
    4115, // Asia/Khandyga
    4129, // Asia/Kolkata
    4142, // Asia/Krasnoyarsk
    4159, // Asia/Kuala_Lumpur
    4177, // Asia/Kuching
    4190, // Asia/Kuwait
    4202, // Asia/Macau
    4213, // Asia/Magadan
    4226, // Asia/Makassar
    4240, // Asia/Manila
    4252, // Asia/Muscat
    4264, // Asia/Nicosia
    4277, // Asia/Novokuznetsk
    4295, // Asia/Novosibirsk
    4312, // Asia/Omsk
    4322, // Asia/Oral
    4332, // Asia/Phnom_Penh
    4348, // Asia/Pontianak
    4363, // Asia/Pyongyang
    4378, // Asia/Qatar
    4389, // Asia/Qyzylorda
    4404, // Asia/Riyadh
    4416, // Asia/Sakhalin
    4430, // Asia/Samarkand
    4445, // Asia/Seoul
    4456, // Asia/Shanghai
    4470, // Asia/Singapore
    4485, // Asia/Srednekolymsk
    4504, // Asia/Taipei
    4516, // Asia/Tashkent
    4530, // Asia/Tbilisi
    4543, // Asia/Tehran
    4555, // Asia/Thimphu
    4568, // Asia/Tokyo
    4579, // Asia/Tomsk
    4590, // Asia/Ulaanbaatar
    4607, // Asia/Urumqi
    4619, // Asia/Ust-Nera
    4633, // Asia/Vientiane
    4648, // Asia/Vladivostok
    4665, // Asia/Yakutsk
    4678, // Asia/Yangon
    4690, // Asia/Yekaterinburg
    4709, // Asia/Yerevan
    4722, // Atlantic/Azores
    4738, // Atlantic/Bermuda
    4755, // Atlantic/Canary
    4771, // Atlantic/Cape_Verde
    4791, // Atlantic/Faroe
    4806, // Atlantic/Madeira
    4823, // Atlantic/Reykjavik
    4842, // Atlantic/South_Georgia
    4865, // Atlantic/St_Helena
    4884, // Atlantic/Stanley
    4901, // Australia/Adelaide
    4920, // Australia/Brisbane
    4939, // Australia/Broken_Hill
    4961, // Australia/Currie
    4978, // Australia/Darwin
    4995, // Australia/Eucla
    5011, // Australia/Hobart
    5028, // Australia/Lindeman
    5047, // Australia/Lord_Howe
    5067, // Australia/Melbourne
    5087, // Australia/Perth
    5103, // Australia/Sydney
    5120, // Europe/Amsterdam
    5137, // Europe/Andorra
    5152, // Europe/Astrakhan
    5169, // Europe/Athens
    5183, // Europe/Belgrade
    5199, // Europe/Berlin
    5213, // Europe/Bratislava
    5231, // Europe/Brussels
    5247, // Europe/Bucharest
    5264, // Europe/Budapest
    5280, // Europe/Busingen
    5296, // Europe/Chisinau
    5312, // Europe/Copenhagen
    5330, // Europe/Dublin
    5344, // Europe/Gibraltar
    5361, // Europe/Guernsey
    5377, // Europe/Helsinki
    5393, // Europe/Isle_of_Man
    5412, // Europe/Istanbul
    5428, // Europe/Jersey
    5442, // Europe/Kaliningrad
    5461, // Europe/Kiev
    5473, // Europe/Kirov
    5486, // Europe/Lisbon
    5500, // Europe/Ljubljana
    5517, // Europe/London
    5531, // Europe/Luxembourg
    5549, // Europe/Madrid
    5563, // Europe/Malta
    5576, // Europe/Mariehamn
    5593, // Europe/Minsk
    5606, // Europe/Monaco
    5620, // Europe/Moscow
    5634, // Europe/Oslo
    5646, // Europe/Paris
    5659, // Europe/Podgorica
    5676, // Europe/Prague
    5690, // Europe/Riga
    5702, // Europe/Rome
    5714, // Europe/Samara
    5728, // Europe/San_Marino
    5746, // Europe/Sarajevo
    5762, // Europe/Saratov
    5777, // Europe/Simferopol
    5795, // Europe/Skopje
    5809, // Europe/Sofia
    5822, // Europe/Stockholm
    5839, // Europe/Tallinn
    5854, // Europe/Tirane
    5868, // Europe/Ulyanovsk
    5885, // Europe/Uzhgorod
    5901, // Europe/Vaduz
    5914, // Europe/Vatican
    5929, // Europe/Vienna
    5943, // Europe/Vilnius
    5958, // Europe/Volgograd
    5975, // Europe/Warsaw
    5989, // Europe/Zagreb
    6003, // Europe/Zaporozhye
    6021, // Europe/Zurich
    6035, // Indian/Antananarivo
    6055, // Indian/Chagos
    6069, // Indian/Christmas
    6086, // Indian/Cocos
    6099, // Indian/Comoro
    6113, // Indian/Kerguelen
    6130, // Indian/Mahe
    6142, // Indian/Maldives
    6158, // Indian/Mauritius
    6175, // Indian/Mayotte
    6190, // Indian/Reunion
    6205, // Pacific/Apia
    6218, // Pacific/Auckland
    6235, // Pacific/Bougainville
    6256, // Pacific/Chatham
    6272, // Pacific/Chuuk
    6286, // Pacific/Easter
    6301, // Pacific/Efate
    6315, // Pacific/Enderbury
    6333, // Pacific/Fakaofo
    6349, // Pacific/Fiji
    6362, // Pacific/Funafuti
    6379, // Pacific/Galapagos
    6397, // Pacific/Gambier
    6413, // Pacific/Guadalcanal
    6433, // Pacific/Guam
    6446, // Pacific/Honolulu
    6463, // Pacific/Kiritimati
    6482, // Pacific/Kosrae
    6497, // Pacific/Kwajalein
    6515, // Pacific/Majuro
    6530, // Pacific/Marquesas
    6548, // Pacific/Midway
    6563, // Pacific/Nauru
    6577, // Pacific/Niue
    6590, // Pacific/Norfolk
    6606, // Pacific/Noumea
    6621, // Pacific/Pago_Pago
    6639, // Pacific/Palau
    6653, // Pacific/Pitcairn
    6670, // Pacific/Pohnpei
    6686, // Pacific/Port_Moresby
    6707, // Pacific/Rarotonga
    6725, // Pacific/Saipan
    6740, // Pacific/Tahiti
    6755, // Pacific/Tarawa
    6770, // Pacific/Tongatapu
    6788, // Pacific/Wake
    6801, // Pacific/Wallis
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

}
}

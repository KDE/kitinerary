/*
 * This code is auto-generated from Wikidata data. Licensed under CC0.
 */

#include "knowledgedb.h"
#include "countrydb_p.h"

namespace KItinerary {
namespace KnowledgeDb {
static const Country country_table[] = {
    {CountryId{"AD"}, DrivingSide::Right, {TypeC|TypeF}}, // Andorra
    {CountryId{"AE"}, DrivingSide::Right, {TypeC|TypeD|TypeG}}, // United Arab Emirates
    {CountryId{"AF"}, DrivingSide::Right, {TypeC|TypeD|TypeF}}, // Afghanistan
    {CountryId{"AG"}, DrivingSide::Left, {TypeA|TypeB}}, // Antigua and Barbuda
    {CountryId{"AL"}, DrivingSide::Right, {TypeC|TypeF}}, // Albania
    {CountryId{"AM"}, DrivingSide::Right, {TypeC|TypeF}}, // Armenia
    {CountryId{"AO"}, DrivingSide::Right, {TypeC}}, // Angola
    {CountryId{"AR"}, DrivingSide::Right, {TypeC|TypeI}}, // Argentina
    {CountryId{"AS"}, DrivingSide::Right, {TypeA|TypeB|TypeF|TypeI}}, // American Samoa
    {CountryId{"AT"}, DrivingSide::Right, {TypeC|TypeF}}, // Austria
    {CountryId{"AU"}, DrivingSide::Left, {TypeI}}, // Australia
    {CountryId{"AW"}, DrivingSide::Right, {TypeA|TypeB|TypeF}}, // Aruba
    {CountryId{"AZ"}, DrivingSide::Right, {TypeC|TypeF}}, // Azerbaijan
    {CountryId{"BA"}, DrivingSide::Right, {TypeC|TypeF}}, // Bosnia and Herzegovina
    {CountryId{"BB"}, DrivingSide::Left, {TypeA|TypeB}}, // Barbados
    {CountryId{"BD"}, DrivingSide::Left, {TypeA|TypeC|TypeD|TypeG|TypeK}}, // Bangladesh
    {CountryId{"BE"}, DrivingSide::Right, {TypeC|TypeE}}, // Belgium
    {CountryId{"BF"}, DrivingSide::Right, {TypeC|TypeE}}, // Burkina Faso
    {CountryId{"BG"}, DrivingSide::Right, {TypeC|TypeF}}, // Bulgaria
    {CountryId{"BH"}, DrivingSide::Right, {TypeG}}, // Bahrain
    {CountryId{"BI"}, DrivingSide::Right, {TypeC|TypeE}}, // Burundi
    {CountryId{"BJ"}, DrivingSide::Right, {TypeC|TypeE}}, // Benin
    {CountryId{"BN"}, DrivingSide::Left, {TypeG}}, // Brunei
    {CountryId{"BO"}, DrivingSide::Right, {TypeA|TypeC}}, // Bolivia
    {CountryId{"BR"}, DrivingSide::Right, {TypeC|TypeN}}, // Brazil
    {CountryId{"BS"}, DrivingSide::Left, {TypeA|TypeB}}, // Bahamas
    {CountryId{"BT"}, DrivingSide::Left, {TypeC|TypeD|TypeF|TypeG|TypeM}}, // Bhutan
    {CountryId{"BW"}, DrivingSide::Left, {TypeD|TypeG|TypeM}}, // Botswana
    {CountryId{"BY"}, DrivingSide::Right, {TypeC|TypeF}}, // Belarus
    {CountryId{"BZ"}, DrivingSide::Right, {TypeA|TypeB|TypeG}}, // Belize
    {CountryId{"CA"}, DrivingSide::Right, {TypeA|TypeB}}, // Canada
    {CountryId{"CD"}, DrivingSide::Right, {TypeC|TypeD|TypeE}}, // Democratic Republic of the Congo
    {CountryId{"CF"}, DrivingSide::Right, {TypeC|TypeE}}, // Central African Republic
    {CountryId{"CG"}, DrivingSide::Right, {TypeC|TypeE}}, // Republic of the Congo
    {CountryId{"CH"}, DrivingSide::Right, {TypeC|TypeJ}}, // Switzerland
    {CountryId{"CI"}, DrivingSide::Right, {TypeC|TypeE}}, // Ivory Coast
    {CountryId{"CK"}, DrivingSide::Left, {TypeI}}, // Cook Islands
    {CountryId{"CL"}, DrivingSide::Right, {TypeC|TypeL}}, // Chile
    {CountryId{"CM"}, DrivingSide::Right, {TypeC|TypeE}}, // Cameroon
    {CountryId{"CN"}, DrivingSide::Right, {TypeA|TypeC|TypeI}}, // People's Republic of China
    {CountryId{"CO"}, DrivingSide::Right, {TypeA|TypeB}}, // Colombia
    {CountryId{"CR"}, DrivingSide::Right, {TypeA|TypeB}}, // Costa Rica
    {CountryId{"CU"}, DrivingSide::Right, {TypeA|TypeB}}, // Cuba
    {CountryId{"CV"}, DrivingSide::Right, {TypeC|TypeF}}, // Cape Verde
    {CountryId{"CW"}, DrivingSide::Right, {TypeA|TypeB}}, // Curaçao
    {CountryId{"CY"}, DrivingSide::Left, {TypeG}}, // Republic of Cyprus
    {CountryId{"CZ"}, DrivingSide::Right, {TypeC|TypeE}}, // Czech Republic
    {CountryId{"DE"}, DrivingSide::Right, {TypeC|TypeF}}, // Germany
    {CountryId{"DJ"}, DrivingSide::Right, {TypeC|TypeE}}, // Djibouti
    {CountryId{"DK"}, DrivingSide::Right, {TypeC|TypeE|TypeF|TypeK}}, // Denmark
    {CountryId{"DM"}, DrivingSide::Left, {TypeD|TypeG}}, // Dominica
    {CountryId{"DO"}, DrivingSide::Right, {TypeA|TypeB}}, // Dominican Republic
    {CountryId{"DZ"}, DrivingSide::Right, {TypeC|TypeE|TypeF}}, // Algeria
    {CountryId{"EC"}, DrivingSide::Right, {TypeA|TypeB}}, // Ecuador
    {CountryId{"EE"}, DrivingSide::Right, {TypeC|TypeF}}, // Estonia
    {CountryId{"EG"}, DrivingSide::Right, {TypeC|TypeF}}, // Egypt
    {CountryId{"EH"}, DrivingSide::Unknown, {}}, // Sahrawi Arab Democratic Republic
    {CountryId{"ER"}, DrivingSide::Right, {TypeC|TypeL}}, // Eritrea
    {CountryId{"ES"}, DrivingSide::Right, {TypeC|TypeF}}, // Spain
    {CountryId{"ET"}, DrivingSide::Right, {TypeC|TypeD|TypeE|TypeF|TypeJ|TypeL}}, // Ethiopia
    {CountryId{"FI"}, DrivingSide::Right, {TypeC|TypeF}}, // Finland
    {CountryId{"FJ"}, DrivingSide::Left, {TypeI}}, // Fiji
    {CountryId{"FM"}, DrivingSide::Right, {TypeA|TypeB}}, // Federated States of Micronesia
    {CountryId{"FO"}, DrivingSide::Right, {TypeC|TypeE|TypeF|TypeK}}, // Faroe Islands
    {CountryId{"FR"}, DrivingSide::Right, {TypeC|TypeE}}, // France
    {CountryId{"GA"}, DrivingSide::Right, {TypeC}}, // Gabon
    {CountryId{"GB"}, DrivingSide::Left, {TypeG}}, // United Kingdom
    {CountryId{"GD"}, DrivingSide::Left, {TypeG}}, // Grenada
    {CountryId{"GE"}, DrivingSide::Right, {TypeC|TypeF}}, // Georgia
    {CountryId{"GH"}, DrivingSide::Right, {TypeD|TypeG}}, // Ghana
    {CountryId{"GL"}, DrivingSide::Right, {TypeC|TypeE|TypeF|TypeK}}, // Greenland
    {CountryId{"GM"}, DrivingSide::Right, {TypeG}}, // Gambia
    {CountryId{"GN"}, DrivingSide::Right, {TypeC|TypeF|TypeK}}, // Guinea
    {CountryId{"GQ"}, DrivingSide::Right, {TypeC|TypeE}}, // Equatorial Guinea
    {CountryId{"GR"}, DrivingSide::Right, {TypeC|TypeF}}, // Greece
    {CountryId{"GT"}, DrivingSide::Right, {TypeA|TypeB}}, // Guatemala
    {CountryId{"GW"}, DrivingSide::Right, {TypeC}}, // Guinea-Bissau
    {CountryId{"GY"}, DrivingSide::Left, {TypeA|TypeB|TypeD|TypeG}}, // Guyana
    {CountryId{"HN"}, DrivingSide::Right, {TypeA|TypeB}}, // Honduras
    {CountryId{"HR"}, DrivingSide::Right, {TypeC|TypeF}}, // Croatia
    {CountryId{"HT"}, DrivingSide::Right, {TypeA|TypeB}}, // Haiti
    {CountryId{"HU"}, DrivingSide::Right, {TypeC|TypeF}}, // Hungary
    {CountryId{"ID"}, DrivingSide::Left, {TypeC|TypeF}}, // Indonesia
    {CountryId{"IE"}, DrivingSide::Left, {TypeG}}, // Ireland
    {CountryId{"IL"}, DrivingSide::Right, {TypeC|TypeH|TypeM}}, // Israel
    {CountryId{"IN"}, DrivingSide::Left, {TypeC|TypeD|TypeM}}, // India
    {CountryId{"IQ"}, DrivingSide::Right, {TypeC|TypeD|TypeG}}, // Iraq
    {CountryId{"IR"}, DrivingSide::Right, {TypeC|TypeF}}, // Iran
    {CountryId{"IS"}, DrivingSide::Right, {TypeC|TypeF}}, // Iceland
    {CountryId{"IT"}, DrivingSide::Right, {TypeC|TypeF|TypeL}}, // Italy
    {CountryId{"JM"}, DrivingSide::Left, {TypeA|TypeB}}, // Jamaica
    {CountryId{"JO"}, DrivingSide::Right, {TypeB|TypeC|TypeD|TypeF|TypeG|TypeJ}}, // Jordan
    {CountryId{"JP"}, DrivingSide::Left, {TypeA|TypeB}}, // Japan
    {CountryId{"KE"}, DrivingSide::Left, {TypeG}}, // Kenya
    {CountryId{"KG"}, DrivingSide::Right, {TypeC|TypeF}}, // Kyrgyzstan
    {CountryId{"KH"}, DrivingSide::Right, {TypeA|TypeC|TypeG}}, // Cambodia
    {CountryId{"KI"}, DrivingSide::Left, {TypeI}}, // Kiribati
    {CountryId{"KM"}, DrivingSide::Right, {TypeC|TypeE}}, // Comoros
    {CountryId{"KN"}, DrivingSide::Left, {TypeA|TypeB|TypeD|TypeG}}, // Saint Kitts and Nevis
    {CountryId{"KP"}, DrivingSide::Right, {TypeA|TypeC|TypeF}}, // North Korea
    {CountryId{"KR"}, DrivingSide::Right, {TypeC|TypeF}}, // South Korea
    {CountryId{"KW"}, DrivingSide::Right, {TypeC|TypeG}}, // Kuwait
    {CountryId{"KZ"}, DrivingSide::Right, {TypeC|TypeF}}, // Kazakhstan
    {CountryId{"LA"}, DrivingSide::Right, {TypeA|TypeB|TypeC|TypeE|TypeF}}, // Laos
    {CountryId{"LB"}, DrivingSide::Right, {TypeA|TypeB|TypeC|TypeD|TypeG}}, // Lebanon
    {CountryId{"LC"}, DrivingSide::Left, {TypeG}}, // Saint Lucia
    {CountryId{"LI"}, DrivingSide::Right, {TypeC|TypeJ}}, // Liechtenstein
    {CountryId{"LK"}, DrivingSide::Left, {TypeD|TypeG|TypeM}}, // Sri Lanka
    {CountryId{"LR"}, DrivingSide::Right, {TypeA|TypeB|TypeC|TypeE|TypeF}}, // Liberia
    {CountryId{"LS"}, DrivingSide::Left, {TypeM}}, // Lesotho
    {CountryId{"LT"}, DrivingSide::Right, {TypeC|TypeF}}, // Lithuania
    {CountryId{"LU"}, DrivingSide::Right, {TypeC|TypeF}}, // Luxembourg
    {CountryId{"LV"}, DrivingSide::Right, {TypeC|TypeF}}, // Latvia
    {CountryId{"LY"}, DrivingSide::Right, {TypeC|TypeD|TypeF|TypeL}}, // Libya
    {CountryId{"MA"}, DrivingSide::Right, {TypeC|TypeE}}, // Morocco
    {CountryId{"MC"}, DrivingSide::Right, {TypeC|TypeD|TypeE|TypeF}}, // Monaco
    {CountryId{"MD"}, DrivingSide::Right, {TypeC|TypeF}}, // Moldova
    {CountryId{"ME"}, DrivingSide::Right, {TypeC|TypeF}}, // Montenegro
    {CountryId{"MG"}, DrivingSide::Right, {TypeC|TypeD|TypeE|TypeJ|TypeK}}, // Madagascar
    {CountryId{"MH"}, DrivingSide::Right, {}}, // Marshall Islands
    {CountryId{"MK"}, DrivingSide::Right, {TypeC|TypeF}}, // North Macedonia
    {CountryId{"ML"}, DrivingSide::Right, {TypeC|TypeE}}, // Mali
    {CountryId{"MM"}, DrivingSide::Right, {TypeC|TypeD|TypeF|TypeG}}, // Myanmar
    {CountryId{"MN"}, DrivingSide::Right, {TypeC|TypeE}}, // Mongolia
    {CountryId{"MP"}, DrivingSide::Unknown, {}}, // Northern Mariana Islands
    {CountryId{"MR"}, DrivingSide::Right, {TypeC}}, // Mauritania
    {CountryId{"MT"}, DrivingSide::Left, {TypeG}}, // Malta
    {CountryId{"MU"}, DrivingSide::Left, {TypeC|TypeG}}, // Mauritius
    {CountryId{"MV"}, DrivingSide::Left, {TypeA|TypeC|TypeD|TypeG|TypeJ|TypeK|TypeL}}, // Maldives
    {CountryId{"MW"}, DrivingSide::Left, {TypeG}}, // Malawi
    {CountryId{"MX"}, DrivingSide::Right, {TypeA|TypeB}}, // Mexico
    {CountryId{"MY"}, DrivingSide::Left, {TypeG}}, // Malaysia
    {CountryId{"MZ"}, DrivingSide::Left, {TypeC|TypeF|TypeM}}, // Mozambique
    {CountryId{"NA"}, DrivingSide::Left, {TypeD|TypeM}}, // Namibia
    {CountryId{"NE"}, DrivingSide::Right, {TypeA|TypeB|TypeC|TypeD|TypeE|TypeF}}, // Niger
    {CountryId{"NG"}, DrivingSide::Right, {TypeG|TypeM}}, // Nigeria
    {CountryId{"NI"}, DrivingSide::Right, {TypeA|TypeB}}, // Nicaragua
    {CountryId{"NL"}, DrivingSide::Right, {TypeC|TypeF}}, // Netherlands
    {CountryId{"NO"}, DrivingSide::Right, {TypeC|TypeF}}, // Norway
    {CountryId{"NP"}, DrivingSide::Left, {TypeC|TypeD|TypeM}}, // Nepal
    {CountryId{"NR"}, DrivingSide::Left, {TypeI}}, // Nauru
    {CountryId{"NU"}, DrivingSide::Left, {TypeI}}, // Niue
    {CountryId{"NZ"}, DrivingSide::Left, {TypeI}}, // New Zealand
    {CountryId{"OM"}, DrivingSide::Right, {TypeC|TypeG}}, // Oman
    {CountryId{"PA"}, DrivingSide::Right, {TypeA|TypeB}}, // Panama
    {CountryId{"PE"}, DrivingSide::Right, {TypeA|TypeB|TypeC}}, // Peru
    {CountryId{"PG"}, DrivingSide::Left, {TypeI}}, // Papua New Guinea
    {CountryId{"PH"}, DrivingSide::Right, {TypeA|TypeB|TypeC}}, // Philippines
    {CountryId{"PK"}, DrivingSide::Left, {TypeC|TypeD|TypeG|TypeM}}, // Pakistan
    {CountryId{"PL"}, DrivingSide::Right, {TypeC|TypeE}}, // Poland
    {CountryId{"PS"}, DrivingSide::Unknown, {}}, // State of Palestine
    {CountryId{"PT"}, DrivingSide::Right, {TypeC|TypeF}}, // Portugal
    {CountryId{"PW"}, DrivingSide::Right, {TypeA|TypeB}}, // Palau
    {CountryId{"PY"}, DrivingSide::Right, {TypeC}}, // Paraguay
    {CountryId{"QA"}, DrivingSide::Right, {TypeD|TypeG}}, // Qatar
    {CountryId{"RO"}, DrivingSide::Right, {TypeC|TypeF}}, // Romania
    {CountryId{"RS"}, DrivingSide::Right, {TypeC|TypeF}}, // Serbia
    {CountryId{"RU"}, DrivingSide::Right, {TypeC|TypeF}}, // Russia
    {CountryId{"RW"}, DrivingSide::Right, {TypeC|TypeJ}}, // Rwanda
    {CountryId{"SA"}, DrivingSide::Right, {TypeG}}, // Saudi Arabia
    {CountryId{"SB"}, DrivingSide::Left, {TypeG|TypeI}}, // Solomon Islands
    {CountryId{"SC"}, DrivingSide::Left, {TypeG}}, // Seychelles
    {CountryId{"SD"}, DrivingSide::Right, {TypeC|TypeD}}, // Sudan
    {CountryId{"SE"}, DrivingSide::Right, {TypeC|TypeF}}, // Sweden
    {CountryId{"SG"}, DrivingSide::Left, {TypeG}}, // Singapore
    {CountryId{"SI"}, DrivingSide::Right, {TypeC|TypeF}}, // Slovenia
    {CountryId{"SK"}, DrivingSide::Right, {TypeC|TypeE}}, // Slovakia
    {CountryId{"SL"}, DrivingSide::Right, {TypeD|TypeG}}, // Sierra Leone
    {CountryId{"SM"}, DrivingSide::Right, {TypeC|TypeF|TypeL}}, // San Marino
    {CountryId{"SN"}, DrivingSide::Right, {TypeC|TypeD|TypeE|TypeK}}, // Senegal
    {CountryId{"SO"}, DrivingSide::Right, {TypeC}}, // Somalia
    {CountryId{"SR"}, DrivingSide::Left, {TypeC|TypeF}}, // Suriname
    {CountryId{"SS"}, DrivingSide::Right, {}}, // South Sudan
    {CountryId{"ST"}, DrivingSide::Right, {TypeC|TypeF}}, // São Tomé and Príncipe
    {CountryId{"SV"}, DrivingSide::Right, {TypeA|TypeB}}, // El Salvador
    {CountryId{"SX"}, DrivingSide::Right, {}}, // Sint Maarten
    {CountryId{"SY"}, DrivingSide::Right, {TypeC|TypeE|TypeL}}, // Syria
    {CountryId{"SZ"}, DrivingSide::Left, {TypeM}}, // Eswatini
    {CountryId{"TD"}, DrivingSide::Right, {TypeC|TypeD|TypeE|TypeF}}, // Chad
    {CountryId{"TG"}, DrivingSide::Right, {TypeC}}, // Togo
    {CountryId{"TH"}, DrivingSide::Left, {TypeA|TypeB|TypeC|TypeF}}, // Thailand
    {CountryId{"TJ"}, DrivingSide::Right, {TypeC|TypeF|TypeI}}, // Tajikistan
    {CountryId{"TL"}, DrivingSide::Left, {TypeC|TypeE|TypeF|TypeI}}, // East Timor
    {CountryId{"TM"}, DrivingSide::Right, {TypeB|TypeC|TypeF}}, // Turkmenistan
    {CountryId{"TN"}, DrivingSide::Right, {TypeC|TypeE}}, // Tunisia
    {CountryId{"TO"}, DrivingSide::Left, {TypeI}}, // Tonga
    {CountryId{"TR"}, DrivingSide::Right, {TypeC|TypeF}}, // Turkey
    {CountryId{"TT"}, DrivingSide::Left, {TypeA|TypeB}}, // Trinidad and Tobago
    {CountryId{"TV"}, DrivingSide::Left, {TypeI}}, // Tuvalu
    {CountryId{"TW"}, DrivingSide::Right, {TypeA|TypeB}}, // Taiwan
    {CountryId{"TZ"}, DrivingSide::Left, {TypeD|TypeG}}, // Tanzania
    {CountryId{"UA"}, DrivingSide::Right, {TypeC|TypeF}}, // Ukraine
    {CountryId{"UG"}, DrivingSide::Left, {TypeG}}, // Uganda
    {CountryId{"US"}, DrivingSide::Right, {TypeA|TypeB}}, // United States of America
    {CountryId{"UY"}, DrivingSide::Right, {TypeC|TypeF|TypeI|TypeL}}, // Uruguay
    {CountryId{"UZ"}, DrivingSide::Right, {TypeC|TypeF|TypeI}}, // Uzbekistan
    {CountryId{"VA"}, DrivingSide::Right, {}}, // Vatican City
    {CountryId{"VC"}, DrivingSide::Left, {TypeA|TypeC|TypeE|TypeG|TypeI|TypeK}}, // Saint Vincent and the Grenadines
    {CountryId{"VE"}, DrivingSide::Right, {TypeA|TypeB}}, // Venezuela
    {CountryId{"VN"}, DrivingSide::Right, {TypeA|TypeC|TypeF|TypeG}}, // Vietnam
    {CountryId{"VU"}, DrivingSide::Right, {TypeC|TypeG|TypeI}}, // Vanuatu
    {CountryId{"WS"}, DrivingSide::Left, {TypeI}}, // Samoa
    {CountryId{"XK"}, DrivingSide::Right, {TypeF}}, // Kosovo
    {CountryId{"YE"}, DrivingSide::Right, {TypeA|TypeD|TypeG}}, // Yemen
    {CountryId{"ZA"}, DrivingSide::Left, {TypeC|TypeD|TypeM|TypeN}}, // South Africa
    {CountryId{"ZM"}, DrivingSide::Left, {TypeC|TypeD|TypeG}}, // Zambia
    {CountryId{"ZW"}, DrivingSide::Left, {TypeD|TypeG}}, // Zimbabwe
};

static const IsoCountryCodeMapping iso_country_code_table[] = {
    { CountryId3{"ABW"}, CountryId{"AW"}},
    { CountryId3{"AFG"}, CountryId{"AF"}},
    { CountryId3{"AGO"}, CountryId{"AO"}},
    { CountryId3{"ALB"}, CountryId{"AL"}},
    { CountryId3{"AND"}, CountryId{"AD"}},
    { CountryId3{"ARE"}, CountryId{"AE"}},
    { CountryId3{"ARG"}, CountryId{"AR"}},
    { CountryId3{"ARM"}, CountryId{"AM"}},
    { CountryId3{"ASM"}, CountryId{"AS"}},
    { CountryId3{"ATG"}, CountryId{"AG"}},
    { CountryId3{"AUS"}, CountryId{"AU"}},
    { CountryId3{"AUT"}, CountryId{"AT"}},
    { CountryId3{"AZE"}, CountryId{"AZ"}},
    { CountryId3{"BDI"}, CountryId{"BI"}},
    { CountryId3{"BEL"}, CountryId{"BE"}},
    { CountryId3{"BEN"}, CountryId{"BJ"}},
    { CountryId3{"BFA"}, CountryId{"BF"}},
    { CountryId3{"BGD"}, CountryId{"BD"}},
    { CountryId3{"BGR"}, CountryId{"BG"}},
    { CountryId3{"BHR"}, CountryId{"BH"}},
    { CountryId3{"BHS"}, CountryId{"BS"}},
    { CountryId3{"BIH"}, CountryId{"BA"}},
    { CountryId3{"BLR"}, CountryId{"BY"}},
    { CountryId3{"BLZ"}, CountryId{"BZ"}},
    { CountryId3{"BOL"}, CountryId{"BO"}},
    { CountryId3{"BRA"}, CountryId{"BR"}},
    { CountryId3{"BRB"}, CountryId{"BB"}},
    { CountryId3{"BRN"}, CountryId{"BN"}},
    { CountryId3{"BTN"}, CountryId{"BT"}},
    { CountryId3{"BWA"}, CountryId{"BW"}},
    { CountryId3{"CAF"}, CountryId{"CF"}},
    { CountryId3{"CAN"}, CountryId{"CA"}},
    { CountryId3{"CHE"}, CountryId{"CH"}},
    { CountryId3{"CHL"}, CountryId{"CL"}},
    { CountryId3{"CHN"}, CountryId{"CN"}},
    { CountryId3{"CIV"}, CountryId{"CI"}},
    { CountryId3{"CMR"}, CountryId{"CM"}},
    { CountryId3{"COD"}, CountryId{"CD"}},
    { CountryId3{"COG"}, CountryId{"CG"}},
    { CountryId3{"COK"}, CountryId{"CK"}},
    { CountryId3{"COL"}, CountryId{"CO"}},
    { CountryId3{"COM"}, CountryId{"KM"}},
    { CountryId3{"CPV"}, CountryId{"CV"}},
    { CountryId3{"CRI"}, CountryId{"CR"}},
    { CountryId3{"CUB"}, CountryId{"CU"}},
    { CountryId3{"CUW"}, CountryId{"CW"}},
    { CountryId3{"CYP"}, CountryId{"CY"}},
    { CountryId3{"CZE"}, CountryId{"CZ"}},
    { CountryId3{"DEU"}, CountryId{"DE"}},
    { CountryId3{"DJI"}, CountryId{"DJ"}},
    { CountryId3{"DMA"}, CountryId{"DM"}},
    { CountryId3{"DNK"}, CountryId{"DK"}},
    { CountryId3{"DOM"}, CountryId{"DO"}},
    { CountryId3{"DZA"}, CountryId{"DZ"}},
    { CountryId3{"ECU"}, CountryId{"EC"}},
    { CountryId3{"EGY"}, CountryId{"EG"}},
    { CountryId3{"ERI"}, CountryId{"ER"}},
    { CountryId3{"ESH"}, CountryId{"EH"}},
    { CountryId3{"ESP"}, CountryId{"ES"}},
    { CountryId3{"EST"}, CountryId{"EE"}},
    { CountryId3{"ETH"}, CountryId{"ET"}},
    { CountryId3{"FIN"}, CountryId{"FI"}},
    { CountryId3{"FJI"}, CountryId{"FJ"}},
    { CountryId3{"FRA"}, CountryId{"FR"}},
    { CountryId3{"FRO"}, CountryId{"FO"}},
    { CountryId3{"FSM"}, CountryId{"FM"}},
    { CountryId3{"GAB"}, CountryId{"GA"}},
    { CountryId3{"GBR"}, CountryId{"GB"}},
    { CountryId3{"GEO"}, CountryId{"GE"}},
    { CountryId3{"GHA"}, CountryId{"GH"}},
    { CountryId3{"GIN"}, CountryId{"GN"}},
    { CountryId3{"GMB"}, CountryId{"GM"}},
    { CountryId3{"GNB"}, CountryId{"GW"}},
    { CountryId3{"GNQ"}, CountryId{"GQ"}},
    { CountryId3{"GRC"}, CountryId{"GR"}},
    { CountryId3{"GRD"}, CountryId{"GD"}},
    { CountryId3{"GRL"}, CountryId{"GL"}},
    { CountryId3{"GTM"}, CountryId{"GT"}},
    { CountryId3{"GUY"}, CountryId{"GY"}},
    { CountryId3{"HND"}, CountryId{"HN"}},
    { CountryId3{"HRV"}, CountryId{"HR"}},
    { CountryId3{"HTI"}, CountryId{"HT"}},
    { CountryId3{"HUN"}, CountryId{"HU"}},
    { CountryId3{"IDN"}, CountryId{"ID"}},
    { CountryId3{"IND"}, CountryId{"IN"}},
    { CountryId3{"IRL"}, CountryId{"IE"}},
    { CountryId3{"IRN"}, CountryId{"IR"}},
    { CountryId3{"IRQ"}, CountryId{"IQ"}},
    { CountryId3{"ISL"}, CountryId{"IS"}},
    { CountryId3{"ISR"}, CountryId{"IL"}},
    { CountryId3{"ITA"}, CountryId{"IT"}},
    { CountryId3{"JAM"}, CountryId{"JM"}},
    { CountryId3{"JOR"}, CountryId{"JO"}},
    { CountryId3{"JPN"}, CountryId{"JP"}},
    { CountryId3{"KAZ"}, CountryId{"KZ"}},
    { CountryId3{"KEN"}, CountryId{"KE"}},
    { CountryId3{"KGZ"}, CountryId{"KG"}},
    { CountryId3{"KHM"}, CountryId{"KH"}},
    { CountryId3{"KIR"}, CountryId{"KI"}},
    { CountryId3{"KNA"}, CountryId{"KN"}},
    { CountryId3{"KOR"}, CountryId{"KR"}},
    { CountryId3{"KWT"}, CountryId{"KW"}},
    { CountryId3{"LAO"}, CountryId{"LA"}},
    { CountryId3{"LBN"}, CountryId{"LB"}},
    { CountryId3{"LBR"}, CountryId{"LR"}},
    { CountryId3{"LBY"}, CountryId{"LY"}},
    { CountryId3{"LCA"}, CountryId{"LC"}},
    { CountryId3{"LIE"}, CountryId{"LI"}},
    { CountryId3{"LKA"}, CountryId{"LK"}},
    { CountryId3{"LSO"}, CountryId{"LS"}},
    { CountryId3{"LTU"}, CountryId{"LT"}},
    { CountryId3{"LUX"}, CountryId{"LU"}},
    { CountryId3{"LVA"}, CountryId{"LV"}},
    { CountryId3{"MAR"}, CountryId{"MA"}},
    { CountryId3{"MCO"}, CountryId{"MC"}},
    { CountryId3{"MDA"}, CountryId{"MD"}},
    { CountryId3{"MDG"}, CountryId{"MG"}},
    { CountryId3{"MDV"}, CountryId{"MV"}},
    { CountryId3{"MEX"}, CountryId{"MX"}},
    { CountryId3{"MHL"}, CountryId{"MH"}},
    { CountryId3{"MKD"}, CountryId{"MK"}},
    { CountryId3{"MLI"}, CountryId{"ML"}},
    { CountryId3{"MLT"}, CountryId{"MT"}},
    { CountryId3{"MMR"}, CountryId{"MM"}},
    { CountryId3{"MNE"}, CountryId{"ME"}},
    { CountryId3{"MNG"}, CountryId{"MN"}},
    { CountryId3{"MNP"}, CountryId{"MP"}},
    { CountryId3{"MOZ"}, CountryId{"MZ"}},
    { CountryId3{"MRT"}, CountryId{"MR"}},
    { CountryId3{"MUS"}, CountryId{"MU"}},
    { CountryId3{"MWI"}, CountryId{"MW"}},
    { CountryId3{"MYS"}, CountryId{"MY"}},
    { CountryId3{"NAM"}, CountryId{"NA"}},
    { CountryId3{"NER"}, CountryId{"NE"}},
    { CountryId3{"NGA"}, CountryId{"NG"}},
    { CountryId3{"NIC"}, CountryId{"NI"}},
    { CountryId3{"NIU"}, CountryId{"NU"}},
    { CountryId3{"NLD"}, CountryId{"NL"}},
    { CountryId3{"NOR"}, CountryId{"NO"}},
    { CountryId3{"NPL"}, CountryId{"NP"}},
    { CountryId3{"NRU"}, CountryId{"NR"}},
    { CountryId3{"NZL"}, CountryId{"NZ"}},
    { CountryId3{"OMN"}, CountryId{"OM"}},
    { CountryId3{"PAK"}, CountryId{"PK"}},
    { CountryId3{"PAN"}, CountryId{"PA"}},
    { CountryId3{"PER"}, CountryId{"PE"}},
    { CountryId3{"PHL"}, CountryId{"PH"}},
    { CountryId3{"PLW"}, CountryId{"PW"}},
    { CountryId3{"PNG"}, CountryId{"PG"}},
    { CountryId3{"POL"}, CountryId{"PL"}},
    { CountryId3{"PRK"}, CountryId{"KP"}},
    { CountryId3{"PRT"}, CountryId{"PT"}},
    { CountryId3{"PRY"}, CountryId{"PY"}},
    { CountryId3{"PSE"}, CountryId{"PS"}},
    { CountryId3{"QAT"}, CountryId{"QA"}},
    { CountryId3{"ROU"}, CountryId{"RO"}},
    { CountryId3{"RUS"}, CountryId{"RU"}},
    { CountryId3{"RWA"}, CountryId{"RW"}},
    { CountryId3{"SAU"}, CountryId{"SA"}},
    { CountryId3{"SDN"}, CountryId{"SD"}},
    { CountryId3{"SEN"}, CountryId{"SN"}},
    { CountryId3{"SGP"}, CountryId{"SG"}},
    { CountryId3{"SLB"}, CountryId{"SB"}},
    { CountryId3{"SLE"}, CountryId{"SL"}},
    { CountryId3{"SLV"}, CountryId{"SV"}},
    { CountryId3{"SMR"}, CountryId{"SM"}},
    { CountryId3{"SOM"}, CountryId{"SO"}},
    { CountryId3{"SRB"}, CountryId{"RS"}},
    { CountryId3{"SSD"}, CountryId{"SS"}},
    { CountryId3{"STP"}, CountryId{"ST"}},
    { CountryId3{"SUR"}, CountryId{"SR"}},
    { CountryId3{"SVK"}, CountryId{"SK"}},
    { CountryId3{"SVN"}, CountryId{"SI"}},
    { CountryId3{"SWE"}, CountryId{"SE"}},
    { CountryId3{"SWZ"}, CountryId{"SZ"}},
    { CountryId3{"SXM"}, CountryId{"SX"}},
    { CountryId3{"SYC"}, CountryId{"SC"}},
    { CountryId3{"SYR"}, CountryId{"SY"}},
    { CountryId3{"TCD"}, CountryId{"TD"}},
    { CountryId3{"TGO"}, CountryId{"TG"}},
    { CountryId3{"THA"}, CountryId{"TH"}},
    { CountryId3{"TJK"}, CountryId{"TJ"}},
    { CountryId3{"TKM"}, CountryId{"TM"}},
    { CountryId3{"TLS"}, CountryId{"TL"}},
    { CountryId3{"TON"}, CountryId{"TO"}},
    { CountryId3{"TTO"}, CountryId{"TT"}},
    { CountryId3{"TUN"}, CountryId{"TN"}},
    { CountryId3{"TUR"}, CountryId{"TR"}},
    { CountryId3{"TUV"}, CountryId{"TV"}},
    { CountryId3{"TWN"}, CountryId{"TW"}},
    { CountryId3{"TZA"}, CountryId{"TZ"}},
    { CountryId3{"UGA"}, CountryId{"UG"}},
    { CountryId3{"UKR"}, CountryId{"UA"}},
    { CountryId3{"URY"}, CountryId{"UY"}},
    { CountryId3{"USA"}, CountryId{"US"}},
    { CountryId3{"UZB"}, CountryId{"UZ"}},
    { CountryId3{"VAT"}, CountryId{"VA"}},
    { CountryId3{"VCT"}, CountryId{"VC"}},
    { CountryId3{"VEN"}, CountryId{"VE"}},
    { CountryId3{"VNM"}, CountryId{"VN"}},
    { CountryId3{"VUT"}, CountryId{"VU"}},
    { CountryId3{"WSM"}, CountryId{"WS"}},
    { CountryId3{"XKS"}, CountryId{"XK"}},
    { CountryId3{"YEM"}, CountryId{"YE"}},
    { CountryId3{"ZAF"}, CountryId{"ZA"}},
    { CountryId3{"ZMB"}, CountryId{"ZM"}},
    { CountryId3{"ZWE"}, CountryId{"ZW"}},
};

static const UicCountryCodeMapping uic_country_code_table[] = {
    {10, CountryId{"FI"}},
    {20, CountryId{"RU"}},
    {21, CountryId{"BY"}},
    {22, CountryId{"UA"}},
    {23, CountryId{"MD"}},
    {24, CountryId{"LT"}},
    {25, CountryId{"LV"}},
    {26, CountryId{"EE"}},
    {27, CountryId{"KZ"}},
    {28, CountryId{"GE"}},
    {29, CountryId{"UZ"}},
    {30, CountryId{"KP"}},
    {31, CountryId{"MN"}},
    {32, CountryId{"VN"}},
    {33, CountryId{"CN"}},
    {40, CountryId{"CU"}},
    {41, CountryId{"AL"}},
    {42, CountryId{"JP"}},
    {44, CountryId{"BA"}},
    {49, CountryId{"BA"}},
    {50, CountryId{"BA"}},
    {51, CountryId{"PL"}},
    {52, CountryId{"BG"}},
    {53, CountryId{"RO"}},
    {54, CountryId{"CZ"}},
    {55, CountryId{"HU"}},
    {56, CountryId{"SK"}},
    {57, CountryId{"AZ"}},
    {58, CountryId{"AM"}},
    {59, CountryId{"KG"}},
    {60, CountryId{"IE"}},
    {61, CountryId{"KR"}},
    {62, CountryId{"ME"}},
    {64, CountryId{"NZ"}},
    {65, CountryId{"MK"}},
    {66, CountryId{"TJ"}},
    {67, CountryId{"TM"}},
    {68, CountryId{"AF"}},
    {70, CountryId{"GB"}},
    {71, CountryId{"ES"}},
    {72, CountryId{"RS"}},
    {73, CountryId{"GR"}},
    {74, CountryId{"SE"}},
    {75, CountryId{"TR"}},
    {76, CountryId{"NO"}},
    {78, CountryId{"HR"}},
    {79, CountryId{"SI"}},
    {80, CountryId{"DE"}},
    {81, CountryId{"AT"}},
    {82, CountryId{"LU"}},
    {83, CountryId{"IT"}},
    {84, CountryId{"NL"}},
    {85, CountryId{"CH"}},
    {86, CountryId{"DK"}},
    {87, CountryId{"FR"}},
    {88, CountryId{"BE"}},
    {90, CountryId{"EG"}},
    {91, CountryId{"TN"}},
    {92, CountryId{"DZ"}},
    {93, CountryId{"MA"}},
    {94, CountryId{"PT"}},
    {95, CountryId{"IL"}},
    {96, CountryId{"IR"}},
    {97, CountryId{"SY"}},
    {98, CountryId{"LB"}},
    {99, CountryId{"IQ"}},
};


}
}

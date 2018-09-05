/*
 * This code is auto-generated from Wikidata data. Licensed under CC0.
 */

#include "knowledgedb.h"
#include "countrydb.h"

namespace KItinerary {
namespace KnowledgeDb {
static const Country country_table[] = {
    {CountryId{"AD"}, DrivingSide::Right, {TypeC|TypeF}}, // Andorra
    {CountryId{"AE"}, DrivingSide::Right, {TypeC|TypeD|TypeG}}, // United Arab Emirates
    {CountryId{"AF"}, DrivingSide::Right, {TypeC|TypeD|TypeF}}, // Afghanistan
    {CountryId{"AG"}, DrivingSide::Left, {TypeA|TypeB}}, // Antigua and Barbuda
    {CountryId{"AL"}, DrivingSide::Right, {TypeC|TypeF}}, // Albania
    {CountryId{"AM"}, DrivingSide::Right, {TypeC|TypeF}}, // Armenia
    {CountryId{"AO"}, DrivingSide::Left, {TypeC}}, // Angola
    {CountryId{"AR"}, DrivingSide::Right, {TypeC|TypeI}}, // Argentina
    {CountryId{"AT"}, DrivingSide::Right, {TypeC|TypeF}}, // Austria
    {CountryId{"AU"}, DrivingSide::Left, {TypeI}}, // Australia
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
    {CountryId{"CK"}, DrivingSide::Unknown, {TypeI}}, // Cook Islands
    {CountryId{"CL"}, DrivingSide::Right, {TypeC|TypeL}}, // Chile
    {CountryId{"CM"}, DrivingSide::Right, {TypeC|TypeE}}, // Cameroon
    {CountryId{"CN"}, DrivingSide::Right, {TypeA|TypeC|TypeI}}, // People's Republic of China
    {CountryId{"CO"}, DrivingSide::Right, {TypeA|TypeB}}, // Colombia
    {CountryId{"CR"}, DrivingSide::Right, {TypeA|TypeB}}, // Costa Rica
    {CountryId{"CU"}, DrivingSide::Right, {TypeA|TypeB}}, // Cuba
    {CountryId{"CV"}, DrivingSide::Right, {TypeC|TypeF}}, // Cape Verde
    {CountryId{"CY"}, DrivingSide::Left, {TypeG}}, // Cyprus
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
    {CountryId{"EH"}, DrivingSide::Unknown, {}}, // Western Sahara
    {CountryId{"ER"}, DrivingSide::Right, {TypeC|TypeL}}, // Eritrea
    {CountryId{"ES"}, DrivingSide::Right, {TypeC|TypeF}}, // Spain
    {CountryId{"ET"}, DrivingSide::Right, {TypeC|TypeD|TypeE|TypeF|TypeJ|TypeL}}, // Ethiopia
    {CountryId{"FI"}, DrivingSide::Right, {TypeC|TypeF}}, // Finland
    {CountryId{"FJ"}, DrivingSide::Left, {TypeI}}, // Fiji
    {CountryId{"FM"}, DrivingSide::Right, {TypeA|TypeB}}, // Federated States of Micronesia
    {CountryId{"FR"}, DrivingSide::Right, {TypeC|TypeE}}, // France
    {CountryId{"GA"}, DrivingSide::Right, {TypeC}}, // Gabon
    {CountryId{"GB"}, DrivingSide::Left, {TypeG}}, // United Kingdom
    {CountryId{"GD"}, DrivingSide::Left, {TypeG}}, // Grenada
    {CountryId{"GE"}, DrivingSide::Right, {TypeC|TypeF}}, // Georgia
    {CountryId{"GH"}, DrivingSide::Right, {TypeD|TypeG}}, // Ghana
    {CountryId{"GM"}, DrivingSide::Right, {TypeG}}, // The Gambia
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
    {CountryId{"JE"}, DrivingSide::Left, {TypeG}}, // Jersey
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
    {CountryId{"MK"}, DrivingSide::Right, {TypeC|TypeF}}, // Macedonia
    {CountryId{"ML"}, DrivingSide::Left, {TypeC|TypeE}}, // Mali
    {CountryId{"MM"}, DrivingSide::Right, {TypeC|TypeD|TypeF|TypeG}}, // Myanmar
    {CountryId{"MN"}, DrivingSide::Right, {TypeC|TypeE}}, // Mongolia
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
    {CountryId{"NU"}, DrivingSide::Left, {}}, // Niue
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
    {CountryId{"SY"}, DrivingSide::Right, {TypeC|TypeE|TypeL}}, // Syria
    {CountryId{"SZ"}, DrivingSide::Left, {TypeM}}, // Swaziland
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
    {CountryId{"YE"}, DrivingSide::Right, {TypeA|TypeD|TypeG}}, // Yemen
    {CountryId{"ZA"}, DrivingSide::Left, {TypeC|TypeD|TypeM|TypeN}}, // South Africa
    {CountryId{"ZM"}, DrivingSide::Left, {TypeC|TypeD|TypeG}}, // Zambia
    {CountryId{"ZW"}, DrivingSide::Left, {TypeD|TypeG}}, // Zimbabwe
};


}
}

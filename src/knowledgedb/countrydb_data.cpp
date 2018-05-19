/*
 * This code is auto-generated from Wikidata data. Licensed under CC0.
 */

#include "knowledgedb.h"
#include "countrydb.h"

namespace KItinerary {
namespace KnowledgeDb {
static const Country country_table[] = {
    {CountryId{"AD"}, DrivingSide::Right}, // Andorra
    {CountryId{"AE"}, DrivingSide::Right}, // United Arab Emirates
    {CountryId{"AF"}, DrivingSide::Right}, // Afghanistan
    {CountryId{"AG"}, DrivingSide::Left}, // Antigua and Barbuda
    {CountryId{"AL"}, DrivingSide::Right}, // Albania
    {CountryId{"AM"}, DrivingSide::Right}, // Armenia
    {CountryId{"AO"}, DrivingSide::Left}, // Angola
    {CountryId{"AR"}, DrivingSide::Right}, // Argentina
    {CountryId{"AT"}, DrivingSide::Right}, // Austria
    {CountryId{"AU"}, DrivingSide::Left}, // Australia
    {CountryId{"AZ"}, DrivingSide::Right}, // Azerbaijan
    {CountryId{"BA"}, DrivingSide::Right}, // Bosnia and Herzegovina
    {CountryId{"BB"}, DrivingSide::Left}, // Barbados
    {CountryId{"BD"}, DrivingSide::Left}, // Bangladesh
    {CountryId{"BE"}, DrivingSide::Right}, // Belgium
    {CountryId{"BF"}, DrivingSide::Right}, // Burkina Faso
    {CountryId{"BG"}, DrivingSide::Right}, // Bulgaria
    {CountryId{"BH"}, DrivingSide::Right}, // Bahrain
    {CountryId{"BI"}, DrivingSide::Right}, // Burundi
    {CountryId{"BJ"}, DrivingSide::Right}, // Benin
    {CountryId{"BN"}, DrivingSide::Left}, // Brunei
    {CountryId{"BO"}, DrivingSide::Right}, // Bolivia
    {CountryId{"BR"}, DrivingSide::Right}, // Brazil
    {CountryId{"BS"}, DrivingSide::Left}, // The Bahamas
    {CountryId{"BT"}, DrivingSide::Left}, // Bhutan
    {CountryId{"BW"}, DrivingSide::Left}, // Botswana
    {CountryId{"BY"}, DrivingSide::Right}, // Belarus
    {CountryId{"BZ"}, DrivingSide::Right}, // Belize
    {CountryId{"CA"}, DrivingSide::Right}, // Canada
    {CountryId{"CD"}, DrivingSide::Right}, // Democratic Republic of the Congo
    {CountryId{"CF"}, DrivingSide::Right}, // Central African Republic
    {CountryId{"CG"}, DrivingSide::Right}, // Republic of the Congo
    {CountryId{"CH"}, DrivingSide::Right}, // Switzerland
    {CountryId{"CI"}, DrivingSide::Right}, // Ivory Coast
    {CountryId{"CK"}, DrivingSide::Unknown}, // Cook Islands
    {CountryId{"CL"}, DrivingSide::Right}, // Chile
    {CountryId{"CM"}, DrivingSide::Right}, // Cameroon
    {CountryId{"CN"}, DrivingSide::Right}, // People's Republic of China
    {CountryId{"CO"}, DrivingSide::Right}, // Colombia
    {CountryId{"CR"}, DrivingSide::Right}, // Costa Rica
    {CountryId{"CU"}, DrivingSide::Right}, // Cuba
    {CountryId{"CV"}, DrivingSide::Right}, // Cape Verde
    {CountryId{"CY"}, DrivingSide::Left}, // Cyprus
    {CountryId{"CZ"}, DrivingSide::Right}, // Czech Republic
    {CountryId{"DE"}, DrivingSide::Right}, // Germany
    {CountryId{"DJ"}, DrivingSide::Right}, // Djibouti
    {CountryId{"DK"}, DrivingSide::Right}, // Denmark
    {CountryId{"DM"}, DrivingSide::Left}, // Dominica
    {CountryId{"DO"}, DrivingSide::Right}, // Dominican Republic
    {CountryId{"DZ"}, DrivingSide::Right}, // Algeria
    {CountryId{"EC"}, DrivingSide::Right}, // Ecuador
    {CountryId{"EE"}, DrivingSide::Right}, // Estonia
    {CountryId{"EG"}, DrivingSide::Right}, // Egypt
    {CountryId{"EH"}, DrivingSide::Unknown}, // Western Sahara
    {CountryId{"ER"}, DrivingSide::Right}, // Eritrea
    {CountryId{"ES"}, DrivingSide::Right}, // Spain
    {CountryId{"ET"}, DrivingSide::Right}, // Ethiopia
    {CountryId{"FI"}, DrivingSide::Right}, // Finland
    {CountryId{"FJ"}, DrivingSide::Left}, // Fiji
    {CountryId{"FM"}, DrivingSide::Right}, // Federated States of Micronesia
    {CountryId{"FR"}, DrivingSide::Right}, // France
    {CountryId{"GA"}, DrivingSide::Right}, // Gabon
    {CountryId{"GB"}, DrivingSide::Left}, // United Kingdom
    {CountryId{"GD"}, DrivingSide::Left}, // Grenada
    {CountryId{"GE"}, DrivingSide::Right}, // Georgia
    {CountryId{"GG"}, DrivingSide::Left}, // Guernsey
    {CountryId{"GH"}, DrivingSide::Right}, // Ghana
    {CountryId{"GM"}, DrivingSide::Right}, // The Gambia
    {CountryId{"GN"}, DrivingSide::Right}, // Guinea
    {CountryId{"GQ"}, DrivingSide::Right}, // Equatorial Guinea
    {CountryId{"GR"}, DrivingSide::Right}, // Greece
    {CountryId{"GT"}, DrivingSide::Right}, // Guatemala
    {CountryId{"GW"}, DrivingSide::Right}, // Guinea-Bissau
    {CountryId{"GY"}, DrivingSide::Left}, // Guyana
    {CountryId{"HN"}, DrivingSide::Right}, // Honduras
    {CountryId{"HR"}, DrivingSide::Right}, // Croatia
    {CountryId{"HT"}, DrivingSide::Right}, // Haiti
    {CountryId{"HU"}, DrivingSide::Right}, // Hungary
    {CountryId{"ID"}, DrivingSide::Left}, // Indonesia
    {CountryId{"IE"}, DrivingSide::Left}, // Ireland
    {CountryId{"IL"}, DrivingSide::Right}, // Israel
    {CountryId{"IM"}, DrivingSide::Left}, // Isle of Man
    {CountryId{"IN"}, DrivingSide::Left}, // India
    {CountryId{"IQ"}, DrivingSide::Right}, // Iraq
    {CountryId{"IR"}, DrivingSide::Right}, // Iran
    {CountryId{"IS"}, DrivingSide::Right}, // Iceland
    {CountryId{"IT"}, DrivingSide::Right}, // Italy
    {CountryId{"JE"}, DrivingSide::Left}, // Jersey
    {CountryId{"JM"}, DrivingSide::Left}, // Jamaica
    {CountryId{"JO"}, DrivingSide::Right}, // Jordan
    {CountryId{"JP"}, DrivingSide::Left}, // Japan
    {CountryId{"KE"}, DrivingSide::Left}, // Kenya
    {CountryId{"KG"}, DrivingSide::Right}, // Kyrgyzstan
    {CountryId{"KH"}, DrivingSide::Right}, // Cambodia
    {CountryId{"KI"}, DrivingSide::Left}, // Kiribati
    {CountryId{"KM"}, DrivingSide::Right}, // Comoros
    {CountryId{"KN"}, DrivingSide::Left}, // Saint Kitts and Nevis
    {CountryId{"KP"}, DrivingSide::Right}, // North Korea
    {CountryId{"KR"}, DrivingSide::Right}, // South Korea
    {CountryId{"KW"}, DrivingSide::Right}, // Kuwait
    {CountryId{"KZ"}, DrivingSide::Right}, // Kazakhstan
    {CountryId{"LA"}, DrivingSide::Right}, // Laos
    {CountryId{"LB"}, DrivingSide::Right}, // Lebanon
    {CountryId{"LC"}, DrivingSide::Left}, // Saint Lucia
    {CountryId{"LI"}, DrivingSide::Right}, // Liechtenstein
    {CountryId{"LK"}, DrivingSide::Left}, // Sri Lanka
    {CountryId{"LR"}, DrivingSide::Right}, // Liberia
    {CountryId{"LS"}, DrivingSide::Left}, // Lesotho
    {CountryId{"LT"}, DrivingSide::Right}, // Lithuania
    {CountryId{"LU"}, DrivingSide::Right}, // Luxembourg
    {CountryId{"LV"}, DrivingSide::Right}, // Latvia
    {CountryId{"LY"}, DrivingSide::Right}, // Libya
    {CountryId{"MA"}, DrivingSide::Right}, // Morocco
    {CountryId{"MC"}, DrivingSide::Right}, // Monaco
    {CountryId{"MD"}, DrivingSide::Right}, // Moldova
    {CountryId{"ME"}, DrivingSide::Right}, // Montenegro
    {CountryId{"MG"}, DrivingSide::Right}, // Madagascar
    {CountryId{"MH"}, DrivingSide::Right}, // Marshall Islands
    {CountryId{"MK"}, DrivingSide::Right}, // Republic of Macedonia
    {CountryId{"ML"}, DrivingSide::Left}, // Mali
    {CountryId{"MM"}, DrivingSide::Right}, // Myanmar
    {CountryId{"MN"}, DrivingSide::Right}, // Mongolia
    {CountryId{"MR"}, DrivingSide::Right}, // Mauritania
    {CountryId{"MT"}, DrivingSide::Left}, // Malta
    {CountryId{"MU"}, DrivingSide::Left}, // Mauritius
    {CountryId{"MV"}, DrivingSide::Left}, // Maldives
    {CountryId{"MW"}, DrivingSide::Left}, // Malawi
    {CountryId{"MX"}, DrivingSide::Right}, // Mexico
    {CountryId{"MY"}, DrivingSide::Left}, // Malaysia
    {CountryId{"MZ"}, DrivingSide::Left}, // Mozambique
    {CountryId{"NA"}, DrivingSide::Left}, // Namibia
    {CountryId{"NE"}, DrivingSide::Right}, // Niger
    {CountryId{"NG"}, DrivingSide::Right}, // Nigeria
    {CountryId{"NI"}, DrivingSide::Right}, // Nicaragua
    {CountryId{"NL"}, DrivingSide::Right}, // Netherlands
    {CountryId{"NO"}, DrivingSide::Right}, // Norway
    {CountryId{"NP"}, DrivingSide::Left}, // Nepal
    {CountryId{"NR"}, DrivingSide::Left}, // Nauru
    {CountryId{"NU"}, DrivingSide::Left}, // Niue
    {CountryId{"NZ"}, DrivingSide::Left}, // New Zealand
    {CountryId{"OM"}, DrivingSide::Right}, // Oman
    {CountryId{"PA"}, DrivingSide::Right}, // Panama
    {CountryId{"PE"}, DrivingSide::Right}, // Peru
    {CountryId{"PG"}, DrivingSide::Left}, // Papua New Guinea
    {CountryId{"PH"}, DrivingSide::Right}, // Philippines
    {CountryId{"PK"}, DrivingSide::Left}, // Pakistan
    {CountryId{"PL"}, DrivingSide::Right}, // Poland
    {CountryId{"PS"}, DrivingSide::Unknown}, // State of Palestine
    {CountryId{"PT"}, DrivingSide::Right}, // Portugal
    {CountryId{"PW"}, DrivingSide::Right}, // Palau
    {CountryId{"PY"}, DrivingSide::Right}, // Paraguay
    {CountryId{"QA"}, DrivingSide::Right}, // Qatar
    {CountryId{"RO"}, DrivingSide::Right}, // Romania
    {CountryId{"RS"}, DrivingSide::Right}, // Serbia
    {CountryId{"RU"}, DrivingSide::Right}, // Russia
    {CountryId{"RW"}, DrivingSide::Right}, // Rwanda
    {CountryId{"SA"}, DrivingSide::Right}, // Saudi Arabia
    {CountryId{"SB"}, DrivingSide::Left}, // Solomon Islands
    {CountryId{"SC"}, DrivingSide::Left}, // Seychelles
    {CountryId{"SD"}, DrivingSide::Right}, // Sudan
    {CountryId{"SE"}, DrivingSide::Right}, // Sweden
    {CountryId{"SG"}, DrivingSide::Left}, // Singapore
    {CountryId{"SI"}, DrivingSide::Right}, // Slovenia
    {CountryId{"SK"}, DrivingSide::Right}, // Slovakia
    {CountryId{"SL"}, DrivingSide::Right}, // Sierra Leone
    {CountryId{"SM"}, DrivingSide::Right}, // San Marino
    {CountryId{"SN"}, DrivingSide::Right}, // Senegal
    {CountryId{"SO"}, DrivingSide::Right}, // Somalia
    {CountryId{"SR"}, DrivingSide::Left}, // Suriname
    {CountryId{"SS"}, DrivingSide::Right}, // South Sudan
    {CountryId{"ST"}, DrivingSide::Right}, // São Tomé and Príncipe
    {CountryId{"SV"}, DrivingSide::Right}, // El Salvador
    {CountryId{"SY"}, DrivingSide::Right}, // Syria
    {CountryId{"SZ"}, DrivingSide::Left}, // Swaziland
    {CountryId{"TD"}, DrivingSide::Right}, // Chad
    {CountryId{"TG"}, DrivingSide::Right}, // Togo
    {CountryId{"TH"}, DrivingSide::Left}, // Thailand
    {CountryId{"TJ"}, DrivingSide::Right}, // Tajikistan
    {CountryId{"TL"}, DrivingSide::Left}, // East Timor
    {CountryId{"TM"}, DrivingSide::Right}, // Turkmenistan
    {CountryId{"TN"}, DrivingSide::Right}, // Tunisia
    {CountryId{"TO"}, DrivingSide::Left}, // Tonga
    {CountryId{"TR"}, DrivingSide::Right}, // Turkey
    {CountryId{"TT"}, DrivingSide::Left}, // Trinidad and Tobago
    {CountryId{"TV"}, DrivingSide::Left}, // Tuvalu
    {CountryId{"TW"}, DrivingSide::Right}, // Taiwan
    {CountryId{"TZ"}, DrivingSide::Left}, // Tanzania
    {CountryId{"UA"}, DrivingSide::Right}, // Ukraine
    {CountryId{"UG"}, DrivingSide::Left}, // Uganda
    {CountryId{"US"}, DrivingSide::Right}, // United States of America
    {CountryId{"UY"}, DrivingSide::Right}, // Uruguay
    {CountryId{"UZ"}, DrivingSide::Right}, // Uzbekistan
    {CountryId{"VA"}, DrivingSide::Right}, // Vatican City
    {CountryId{"VC"}, DrivingSide::Left}, // Saint Vincent and the Grenadines
    {CountryId{"VE"}, DrivingSide::Right}, // Venezuela
    {CountryId{"VN"}, DrivingSide::Right}, // Vietnam
    {CountryId{"VU"}, DrivingSide::Right}, // Vanuatu
    {CountryId{"WS"}, DrivingSide::Left}, // Samoa
    {CountryId{"XK"}, DrivingSide::Right}, // Kosovo
    {CountryId{"YE"}, DrivingSide::Right}, // Yemen
    {CountryId{"ZA"}, DrivingSide::Left}, // South Africa
    {CountryId{"ZM"}, DrivingSide::Left}, // Zambia
    {CountryId{"ZW"}, DrivingSide::Left}, // Zimbabwe
};


}
}

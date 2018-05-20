/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "countrydb.h"
#include "countrydb_data.cpp"

#include <QString>

#include <algorithm>

using namespace KItinerary;
using namespace KItinerary::KnowledgeDb;

static_assert(sizeof(CountryId) <= 2, "CountryId too large");

namespace KItinerary {
namespace KnowledgeDb {

static const auto country_table_size = sizeof(country_table) / sizeof(Country);

const Country* countryTableBegin() { return country_table; }
const Country* countryTableEnd() { return country_table + country_table_size; }

}
}

CountryId::CountryId(const QString& id)
{
    m_id1 = m_id2 = 0;
    m_unused = 0;

    if (id.size() != 2) {
        return;
    }
    if (!id.at(0).isUpper() || !id.at(1).isUpper()) {
        return;
    }
    m_id1 = id.at(0).toLatin1() - '@';
    m_id2 = id.at(1).toLatin1() - '@';
}

Country KnowledgeDb::countryForId(CountryId id)
{
    const auto it = std::lower_bound(countryTableBegin(), countryTableEnd(), id, [](const Country &lhs, CountryId rhs) {
        return lhs.id < rhs;
    });
    if (it == countryTableEnd() || (*it).id != id) {
        return {CountryId{}, DrivingSide::Unknown, Unknown};
    }
    return (*it);
}

QString KnowledgeDb::CountryId::toString() const
{
    QString s;
    s.resize(2);
    s[0] = QLatin1Char('@' + m_id1);
    s[1] = QLatin1Char('@' + m_id2);
    return s;
}

#include "moc_countrydb.cpp"

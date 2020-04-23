/*
    Copyright (C) 2020 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef OSM_XMLPARSER_H
#define OSM_XMLPARSER_H

#include <QString>

class QIODevice;
class QXmlStreamReader;

namespace OSM {

class DataSet;

class XmlParser
{
public:
    explicit XmlParser(DataSet *dataSet);

    void parse(QIODevice *io);
    QString error() const;

private:
    void parseNode(QXmlStreamReader &reader);
    void parseWay(QXmlStreamReader &reader);
    void parseRelation(QXmlStreamReader &reader);
    template <typename T>
    void parseTag(QXmlStreamReader &reader, T &elem);
    template <typename T>
    void parseTagOrBounds(QXmlStreamReader &reader, T&elem);
    template <typename T>
    void parseBounds(QXmlStreamReader &reader, T &elem);

    DataSet *m_dataSet;
    QString m_error;
};

}

#endif // OSM_XMLPARSER_P_H

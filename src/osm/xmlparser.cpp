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

#include "xmlparser.h"
#include "datatypes.h"

#include <QIODevice>
#include <QXmlStreamReader>

using namespace OSM;

XmlParser::XmlParser(DataSet* dataSet)
    : m_dataSet(dataSet)
{
    assert(dataSet);
}

void XmlParser::parse(QIODevice *io)
{
    QXmlStreamReader reader(io);
    while (!reader.atEnd()) {
        const auto token = reader.readNext();
        if (token != QXmlStreamReader::StartElement) {
            continue;
        }

        if (reader.name() == QLatin1String("node")) {
            parseNode(reader);
        }
        else if (reader.name() == QLatin1String("way")) {
            parseWay(reader);
        }
        else if (reader.name() == QLatin1String("relation")) {
            parseRelation(reader);
        } else if (reader.name() == QLatin1String("remark")) {
            m_error = reader.readElementText();
            return;
        }
    }
}

void XmlParser::parseNode(QXmlStreamReader &reader)
{
    Node node;
    node.id = reader.attributes().value(QLatin1String("id")).toLongLong();
    node.coordinate = Coordinate(reader.attributes().value(QLatin1String("lat")).toDouble(), reader.attributes().value(QLatin1String("lon")).toDouble());

    while (!reader.atEnd() && reader.readNext() != QXmlStreamReader::EndElement) {
        if (reader.tokenType() != QXmlStreamReader::StartElement) {
            continue;
        }
        if (reader.name() == QLatin1String("tag")) {
            parseTag(reader, node);
        }
        reader.skipCurrentElement();
    }

    m_dataSet->addNode(std::move(node));
}

void XmlParser::parseWay(QXmlStreamReader &reader)
{
    Way way;
    way.id = reader.attributes().value(QLatin1String("id")).toLongLong();

    while (!reader.atEnd() && reader.readNext() != QXmlStreamReader::EndElement) {
        if (reader.tokenType() != QXmlStreamReader::StartElement) {
            continue;
        }
        if (reader.name() == QLatin1String("nd")) {
            OSM::Id node;
            node = reader.attributes().value(QLatin1String("ref")).toLongLong();
            way.nodes.push_back(node);
        } else if (reader.name() == QLatin1String("tag")) {
            parseTagOrBounds(reader, way);
        } else if (reader.name() == QLatin1String("bounds")) {
            parseBounds(reader, way);
        }
        reader.skipCurrentElement();
    }

    m_dataSet->addWay(std::move(way));
}

void XmlParser::parseRelation(QXmlStreamReader &reader)
{
    Relation rel;
    rel.id = reader.attributes().value(QLatin1String("id")).toLongLong();

    while (!reader.atEnd() && reader.readNext() != QXmlStreamReader::EndElement) {
        if (reader.tokenType() != QXmlStreamReader::StartElement) {
            continue;
        }
        if (reader.name() == QLatin1String("tag")) {
            parseTagOrBounds(reader, rel);
        } else if (reader.name() == QLatin1String("bounds")) { // Overpass style bounding box
            parseBounds(reader, rel);
        } else if (reader.name() == QLatin1String("member")) {
            Member member;
            member.id = reader.attributes().value(QLatin1String("ref")).toLongLong();
            const auto type = reader.attributes().value(QLatin1String("type"));
            if (type == QLatin1String("node")) {
                member.type = Type::Node;
            } else if (type == QLatin1String("way")) {
                member.type = Type::Way;
            } else {
                member.type = Type::Relation;
            }
            member.role = reader.attributes().value(QLatin1String("role")).toString(); // TODO shared value pool for these values
            rel.members.push_back(std::move(member));
        }
        reader.skipCurrentElement();
    }

    m_dataSet->addRelation(std::move(rel));
}

template <typename T>
void XmlParser::parseTag(QXmlStreamReader &reader, T &elem)
{
    OSM::setTagValue(elem, reader.attributes().value(QLatin1String("k")).toString(), reader.attributes().value(QLatin1String("v")).toString());
}

template <typename T>
void XmlParser::parseTagOrBounds(QXmlStreamReader &reader, T &elem)
{
    if (reader.attributes().value(QLatin1String("k")) == QLatin1String("bBox")) { // osmconvert style bounding box
        const auto v = reader.attributes().value(QLatin1String("v")).split(QLatin1Char(','));
        if (v.size() == 4) {
            elem.bbox.min = Coordinate(v[1].toDouble(), v[0].toDouble());
            elem.bbox.max = Coordinate(v[3].toDouble(), v[2].toDouble());
        }
    } else {
        parseTag(reader, elem);
    }
}

template<typename T>
void XmlParser::parseBounds(QXmlStreamReader &reader, T &elem)
{
    // overpass style bounding box
    elem.bbox.min = Coordinate(reader.attributes().value(QLatin1String("minlat")).toDouble(), reader.attributes().value(QLatin1String("minlon")).toDouble());
    elem.bbox.max = Coordinate(reader.attributes().value(QLatin1String("maxlat")).toDouble(), reader.attributes().value(QLatin1String("maxlon")).toDouble());
}

QString XmlParser::error() const
{
    return m_error;
}

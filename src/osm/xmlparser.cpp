/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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

        if (reader.name() == QLatin1StringView("node")) {
          parseNode(reader);
        } else if (reader.name() == QLatin1StringView("way")) {
          parseWay(reader);
        } else if (reader.name() == QLatin1StringView("relation")) {
          parseRelation(reader);
        } else if (reader.name() == QLatin1StringView("remark")) {
          m_error = reader.readElementText();
          return;
        }
    }
}

void XmlParser::parseNode(QXmlStreamReader &reader)
{
    Node node;
    node.id = reader.attributes().value(QLatin1StringView("id")).toLongLong();
    node.coordinate = Coordinate(
        reader.attributes().value(QLatin1StringView("lat")).toDouble(),
        reader.attributes().value(QLatin1StringView("lon")).toDouble());

    while (!reader.atEnd() && reader.readNext() != QXmlStreamReader::EndElement) {
        if (reader.tokenType() != QXmlStreamReader::StartElement) {
            continue;
        }
        if (reader.name() == QLatin1StringView("tag")) {
          parseTag(reader, node);
        }
        reader.skipCurrentElement();
    }

    m_dataSet->addNode(std::move(node));
}

void XmlParser::parseWay(QXmlStreamReader &reader)
{
    Way way;
    way.id = reader.attributes().value(QLatin1StringView("id")).toLongLong();

    while (!reader.atEnd() && reader.readNext() != QXmlStreamReader::EndElement) {
        if (reader.tokenType() != QXmlStreamReader::StartElement) {
            continue;
        }
        if (reader.name() == QLatin1StringView("nd")) {
          OSM::Id node;
          node =
              reader.attributes().value(QLatin1StringView("ref")).toLongLong();
          way.nodes.push_back(node);
        } else if (reader.name() == QLatin1StringView("tag")) {
          parseTagOrBounds(reader, way);
        } else if (reader.name() == QLatin1StringView("bounds")) {
          parseBounds(reader, way);
        }
        reader.skipCurrentElement();
    }

    m_dataSet->addWay(std::move(way));
}

void XmlParser::parseRelation(QXmlStreamReader &reader)
{
    Relation rel;
    rel.id = reader.attributes().value(QLatin1StringView("id")).toLongLong();

    while (!reader.atEnd() && reader.readNext() != QXmlStreamReader::EndElement) {
        if (reader.tokenType() != QXmlStreamReader::StartElement) {
            continue;
        }
        if (reader.name() == QLatin1StringView("tag")) {
          parseTagOrBounds(reader, rel);
        } else if (reader.name() ==
                   QLatin1StringView("bounds")) { // Overpass style bounding box
          parseBounds(reader, rel);
        } else if (reader.name() == QLatin1StringView("member")) {
          Member member;
          member.id =
              reader.attributes().value(QLatin1StringView("ref")).toLongLong();
          const auto type =
              reader.attributes().value(QLatin1StringView("type"));
          if (type == QLatin1StringView("node")) {
            member.type = Type::Node;
          } else if (type == QLatin1StringView("way")) {
            member.type = Type::Way;
          } else {
            member.type = Type::Relation;
          }
          member.role =
              reader.attributes()
                  .value(QLatin1StringView("role"))
                  .toString(); // TODO shared value pool for these values
          rel.members.push_back(std::move(member));
        }
        reader.skipCurrentElement();
    }

    m_dataSet->addRelation(std::move(rel));
}

template <typename T>
void XmlParser::parseTag(QXmlStreamReader &reader, T &elem)
{
  OSM::setTagValue(elem,
                   reader.attributes().value(QLatin1StringView("k")).toString(),
                   reader.attributes().value(QLatin1StringView("v")).toString());
}

template <typename T>
void XmlParser::parseTagOrBounds(QXmlStreamReader &reader, T &elem)
{
  if (reader.attributes().value(QLatin1StringView("k")) ==
      QLatin1StringView("bBox")) { // osmconvert style bounding box
    const auto v = reader.attributes()
                       .value(QLatin1StringView("v"))
                       .split(QLatin1Char(','));
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
    elem.bbox.min = Coordinate(
        reader.attributes().value(QLatin1StringView("minlat")).toDouble(),
        reader.attributes().value(QLatin1StringView("minlon")).toDouble());
    elem.bbox.max = Coordinate(
        reader.attributes().value(QLatin1StringView("maxlat")).toDouble(),
        reader.attributes().value(QLatin1StringView("maxlon")).toDouble());
}

QString XmlParser::error() const
{
    return m_error;
}

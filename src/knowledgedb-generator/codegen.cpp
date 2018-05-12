#include "codegen.h"

#include <QIODevice>

using namespace KItinerary::Generator;

void CodeGen::writeLicenseHeader(QIODevice *out)
{
    out->write(R"(/*
 * This code is auto-generated from Wikidata data. Licensed under CC0.
 */)");
}

void CodeGen::writeCoordinate(QIODevice* out, const KnowledgeDb::Coordinate& coord)
{
    out->write("Coordinate{");
    if (coord.isValid()) {
        out->write(QByteArray::number(coord.longitude));
        out->write(", ");
        out->write(QByteArray::number(coord.latitude));
    }
    out->write("}");
}

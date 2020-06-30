/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef OSM_DATATYPES_H
#define OSM_DATATYPES_H

#include <QDebug>
#include <QString>

#include <cstdint>
#include <vector>

namespace OSM {

/** OSM element identifier. */
typedef int64_t Id;

/** Coordinate, stored as 1e7 * degree to avoid floating point precision issues,
 *  and offset to unsigned values to make the z-order curve work.
 *  Can be in an invalid state with coordinates out of range, see isValid().
 *  @see https://en.wikipedia.org/wiki/Z-order_curve for the z-order curve stuff
 */
class Coordinate {
public:
    Coordinate() = default;
    explicit constexpr Coordinate(double lat, double lon)
        : latitude((lat + 90.0) * 10'000'000)
        , longitude((lon + 180.0) * 10'000'000)
    {}
    explicit constexpr Coordinate(uint32_t lat, uint32_t lon)
        : latitude(lat)
        , longitude(lon)
    {}

    /** Create a coordinate from a z-order curve index. */
    explicit constexpr Coordinate(uint64_t z)
        : latitude(0)
        , longitude(0)
    {
        for (int i = 0; i < 32; ++i) {
            latitude += (z & (1ull << (i * 2))) >> i;
            longitude += (z & (1ull << (1 + i * 2))) >> (i + 1);
        }
    }

    constexpr inline bool isValid() const
    {
        return latitude != std::numeric_limits<uint32_t>::max() && longitude != std::numeric_limits<uint32_t>::max();
    }

    /** Z-order curve value for this coordinate. */
    constexpr inline uint64_t z() const
    {
        uint64_t z = 0;
        for (int i = 0; i < 32; ++i) {
            z += ((uint64_t)latitude & (1 << i)) << i;
            z += ((uint64_t)longitude & (1 << i)) << (i + 1);
        }
        return z;
    }

    constexpr inline double latF() const
    {
        return (latitude / 10'000'000.0) - 90.0;
    }
    constexpr inline double lonF() const
    {
        return (longitude / 10'000'000.0) - 180.0;
    }

    uint32_t latitude = std::numeric_limits<uint32_t>::max();
    uint32_t longitude = std::numeric_limits<uint32_t>::max();
};


/** Bounding box, ie. a pair of coordinates. */
class BoundingBox {
public:
    constexpr BoundingBox() = default;
    constexpr inline BoundingBox(Coordinate c1, Coordinate c2)
        : min(c1)
        , max(c2)
    {}
    constexpr inline bool isValid() const
    {
        return min.isValid() && max.isValid();
    }

    constexpr inline uint32_t width() const
    {
        return max.longitude - min.longitude;
    }
    constexpr inline uint32_t height() const
    {
        return max.latitude - min.latitude;
    }

    constexpr inline Coordinate center() const
    {
        return Coordinate(min.latitude + height() / 2, min.longitude + width() / 2);
    }

    Coordinate min;
    Coordinate max;
};

constexpr inline BoundingBox unite(BoundingBox bbox1, BoundingBox bbox2)
{
    if (!bbox1.isValid()) {
        return bbox2;
    }
    if (!bbox2.isValid()) {
        return bbox1;
    }
    BoundingBox ret;
    ret.min.latitude = std::min(bbox1.min.latitude, bbox2.min.latitude);
    ret.min.longitude = std::min(bbox1.min.longitude, bbox2.min.longitude);
    ret.max.latitude = std::max(bbox1.max.latitude, bbox2.max.latitude);
    ret.max.longitude = std::max(bbox1.max.longitude, bbox2.max.longitude);
    return ret;
}

constexpr inline bool intersects(BoundingBox bbox1, BoundingBox bbox2)
{
    return !(bbox2.min.latitude > bbox1.max.latitude || bbox2.max.latitude < bbox1.min.latitude
        || bbox2.min.longitude > bbox1.max.longitude || bbox2.max.longitude < bbox1.min.longitude);
}

constexpr inline bool contains(BoundingBox bbox, Coordinate coord)
{
    return bbox.min.latitude <= coord.latitude && bbox.max.latitude >= coord.latitude
        && bbox.min.longitude <= coord.longitude && bbox.max.longitude >= coord.longitude;
}

constexpr inline uint32_t latitudeDistance(BoundingBox bbox1, BoundingBox bbox2)
{
    return bbox1.max.latitude < bbox2.min.latitude ? bbox2.min.latitude - bbox1.max.latitude : bbox1.min.latitude - bbox2.max.latitude;
}

constexpr inline uint32_t longitudeDifference(BoundingBox bbox1, BoundingBox bbox2)
{
    return bbox1.max.longitude < bbox2.min.longitude ? bbox2.min.longitude - bbox1.max.longitude : bbox1.min.longitude - bbox2.max.longitude;
}

/** An OSM element tag. */
class Tag {
public:
    inline bool operator<(const Tag &other) const { return key < other.key; }

    QString key;
    QString value;
};

/** An OSM node. */
class Node {
public:
    constexpr inline bool operator<(const Node &other) const { return id < other.id; }

    QString url() const;

    Id id;
    Coordinate coordinate;
    std::vector<Tag> tags;
};

/** An OSM way. */
class Way {
public:
    constexpr inline bool operator<(const Way &other) const { return id < other.id; }

    bool isClosed() const;

    QString url() const;

    Id id;
    mutable BoundingBox bbox;
    std::vector<Id> nodes;
    std::vector<Tag> tags;
};

/** Element type. */
enum class Type : uint8_t {
    Null,
    Node,
    Way,
    Relation
};

/** A member in a relation. */
// TODO this has 7 byte padding, can we make this more efficient?
class Member {
public:
    Id id;
    QString role;
    Type type;
};

/** An OSM relation. */
class Relation {
public:
    constexpr inline bool operator<(const Relation &other) const { return id < other.id; }

    QString url() const;

    Id id;
    mutable BoundingBox bbox;
    std::vector<Member> members;
    std::vector<Tag> tags;
};

/** A set of nodes, ways and relations. */
class DataSet {
public:
    void addNode(Node &&node);
    void addWay(Way &&way);
    void addRelation(Relation &&rel);

    std::vector<Node> nodes;
    std::vector<Way> ways;
    std::vector<Relation> relations;
};

/** Returns the tag value for @p key of @p elem. */
template <typename Elem>
inline QString tagValue(const Elem& elem, const QLatin1String &key)
{
    const auto it = std::lower_bound(elem.tags.begin(), elem.tags.end(), key, [](const auto &lhs, const auto &rhs) { return lhs.key < rhs; });
    if (it != elem.tags.end() && (*it).key == key) {
        return (*it).value;
    }
    return {};
}

/** Inserts a new tag, or replaces an existing one with the same key. */
template <typename Elem>
inline void setTag(Elem &elem, Tag &&tag)
{
    const auto it = std::lower_bound(elem.tags.begin(), elem.tags.end(), tag);
    if (it == elem.tags.end() || (*it).key != tag.key) {
        elem.tags.insert(it, std::move(tag));
    } else {
        (*it) = std::move(tag);
    }
}

/** Inserts a new tag, or updates an existing one. */
template <typename Elem>
inline void setTagValue(Elem &elem, const QString &key, const QString &value)
{
    Tag tag{ key, value };
    setTag(elem, std::move(tag));
}

template <typename Elem>
inline bool operator<(const Elem &elem, Id id)
{
    return elem.id < id;
}

}

QDebug operator<<(QDebug debug, OSM::Coordinate coord);
QDebug operator<<(QDebug debug, OSM::BoundingBox bbox);

#endif // OSM_DATATYPES_H

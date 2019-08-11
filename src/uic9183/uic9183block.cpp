/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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

#include "uic9183block.h"

#include <QByteArray>

using namespace KItinerary;

Uic9183Block::Uic9183Block() = default;
Uic9183Block::Uic9183Block(const Uic9183Block&) = default;
Uic9183Block::Uic9183Block(Uic9183Block&&) = default;
Uic9183Block& Uic9183Block::operator=(const Uic9183Block&) = default;
Uic9183Block& Uic9183Block::operator=(Uic9183Block&&) = default;

Uic9183Block::Uic9183Block(const char* data, int size)
    : m_data(data)
    , m_size(size)
{
}

const char* Uic9183Block::data() const
{
    return m_data;
}

int Uic9183Block::size() const
{
    return m_size;
}

int Uic9183Block::version() const
{
    return QByteArray(m_data + 6, 2).toInt();
}

bool Uic9183Block::isNull() const
{
    return !m_data || m_size <= 12;
}

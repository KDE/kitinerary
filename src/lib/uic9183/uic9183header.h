/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"
#include "uic9183utils.h"

#include <QByteArray>
#include <qobjectdefs.h>

namespace KItinerary {

/** Header of an UIC 918.3 ticket. */
class KITINERARY_EXPORT Uic9183Header
{
    Q_GADGET
    UIC_STR_PROPERTY(messageType, 0, 3)
    UIC_NUM_PROPERTY(version, 3, 2)
    UIC_NUM_PROPERTY(signerCompanyCodeNumeric, 5, 4)
    UIC_STR_PROPERTY(signerCompanyCode, 5, 4)
    UIC_STR_PROPERTY(signatureKeyId, 9, 5)
    Q_PROPERTY(int signatureSize READ signatureSize)
    Q_PROPERTY(int compressedMessageSize READ compressedMessageSize)
public:
    Uic9183Header();
    Uic9183Header(const QByteArray &data);

    bool operator==(const Uic9183Header &other) const;

    bool isValid() const;
    int signatureSize() const;
    int compressedMessageSize() const;
    int compressedMessageOffset() const;
private:
    QByteArray m_data;
};

}


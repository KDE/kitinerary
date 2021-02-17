/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "context.h"

using namespace KItinerary;

QJSValue JsApi::Context::data() const
{
    return m_data;
}

void JsApi::Context::reset()
{
    m_barcode = {};
    m_pdfPageNum = -1;
    m_data = {};
}

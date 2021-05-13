/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseBookingConfirmation(html, node) {
    if (node.result.length != 1) {
        return;
    }
    var res = node.result[0];

    // augment checking times missing from schema.org annotations
    var checkinTimeElems = html.eval('//*[@id="booking-summary-check-in-time"]');
    if (checkinTimeElems.length == 1) {
        var time = checkinTimeElems[0].content.match(/(\d\d).(\d\d)/);
        if (res.checkinTime && res.checkinTime.length <= 10) {
            res.checkinTime = res.checkinTime + "T" + time[1] + ":" + time[2];
        } else if (res.checkinDate) {
            res.checkinTime = res.checkinDate.substr(0, 10) + "T" + time[1] + ":" + time[2];
        }
    }
    var checkoutTimeElems = html.eval('//*[@id="booking-summary-check-out-time"]');
    if (checkoutTimeElems.length == 1) {
        var time = checkoutTimeElems[0].content.match(/(\d\d).(\d\d)/);
        if (res.checkoutTime && res.checkoutTime.length <= 10) {
            res.checkoutTime = res.checkoutTime + "T" + time[1] + ":" + time[2];
        } else if (res.checkoutDate) {
            res.checkoutTime = res.checkoutDate.substr(0, 10) + "T" + time[1] + ":" + time[2];
        }
    }

    return res;
}

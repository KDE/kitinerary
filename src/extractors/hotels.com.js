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

function parseBookingConfirmation(html) {
    if (Context.data.length != 1) {
        return;
    }
    var res = Context.data[0];

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

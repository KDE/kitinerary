/*
   SPDX-FileCopyrightText: 2022 Robin Grindrod <robingrindrod@live.ie>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function main(html) {
    var res = JsonLd.newLodgingReservation();

    res.reservationNumber = html.eval('//div[@class="refnum"]/p')[0].recursiveContent;

    var times = html.eval('/html/body/table/tr/td/table/tr/td/div/div/div/div/table/tr/td/table/tr/td/div/p/b');
    res.checkinTime = JsonLd.toDateTime(times[0].recursiveContent.replace(/\s+/g, ' '), "'From' ha - ddd dd MMM yyyy", 'en');
    res.checkoutTime = JsonLd.toDateTime(times[1].recursiveContent.replace(/\s+/g, ' '), "'Before' ha - ddd dd MMM yyyy", 'en');

    var hotelName = html.eval('/html/body/table/tr/td/table/tr/td/div/div/div/div/table/tr/td/table/tr/td/div/h2')[0].recursiveContent;
    res.reservationFor.name = 'Premier Inn ' + hotelName;

    // Not sure how to split this up generically
    // res.reservationFor.address = html.eval('/html/body/table/tr/td/table/tr/td/div/div/div/div/table/tr/td/table/tr/td/div/table/tbody/tr/td')[1].recursiveContent;

    res.underName.name = html.eval('/html/body/table/tr/td/table/tr/td/div/div/div/div/div/table/tr/td/table/tr/td/div/p')[0].recursiveContent.split('\n')[0];
    
    return res;
}

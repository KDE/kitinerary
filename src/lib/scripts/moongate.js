// SPDX-FileCopyrightText: 2024 David Pilarčík <meow@charliecat.space>
// SPDX-License-Identifier: LGPL-2.0-or-later

function main(content, node) {

    let text = content.root.recursiveContent;
    let res = JsonLd.newEventReservation();

    let moongateEmailInfoRegex = /You have received your ticket to:\n(.*?) ([0-9]+ [A-z]+ [0-9]+ - [0-9]+ [A-z]+ [0-9]+|[0-9]+ [A-z]+ [0-9]+), ([0-9:]+) - ([0-9:]+) (.*?)\nOpen Event Page/gm
    let [_, eventName, eventDate, startTime, endTime, eventLocation] = moongateEmailInfoRegex.exec(text)

    res.reservationFor.name = eventName;

    if (eventDate.includes(' - ')) {
        let dates = eventDate.split(' - ')
        res.reservationFor.startDate = JsonLd.toDateTime(dates[0] + ' ' + startTime, "dd MMM yy hh:mm", "en");
        res.reservationFor.endDate = JsonLd.toDateTime(dates[1] + ' ' + endTime, "dd MMM yy hh:mm", "en");
    } else {
        res.reservationFor.startDate = JsonLd.toDateTime(eventDate + ' ' + startTime, "dd MMM yy hh:mm", "en");
        res.reservationFor.endDate = JsonLd.toDateTime(eventDate + ' ' + endTime, "dd MMM yy hh:mm", "en");
    }

    console.log(eventLocation)

    res.reservationFor.location.name = String(eventLocation);
    
    let images = node.parent.findChildNodes({ mimeType: "internal/qimage", scope: "Descendants" })
    for (image of images) {

        if (!image.childNodes || image.childNodes.length != 1 || image.childNodes[0].mimeType != "text/plain") continue;
        res.reservedTicket.ticketToken = 'qrCode:' + image.childNodes[0].content

    }

    return res;

}

/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function readTime(stream)
{
    let timeStream = stream.readSubMessage();
    return new Date(timeStream.readVarintField());
}

// see https://community.kde.org/KDE_PIM/KItinerary/SBB_Barcode
function parseQrCode(content) {
    let res = JsonLd.newTrainReservation();
    res.reservedTicket.ticketToken = 'qrcodebin:' + ByteArray.toBase64(content);
    res.reservationFor.provider.identifier = 'uic:1085';
    let top = ByteArray.toProtobufStreamReader(content);
    while (!top.atEnd()) {
        switch (top.fieldNumber()) {
            case 1:
                let level1 = top.readSubMessage();
                while (!level1.atEnd()) {
                    switch (level1.fieldNumber()) {
                        case 1:
                            res.reservationNumber = level1.readVarintField() + "";
                            break;
                        case 2:
                            let tripData = level1.readSubMessage();
                            while (!tripData.atEnd()) {
                                switch (tripData.fieldNumber()) {
                                    case 1:
                                        let ticketNameData = tripData.readSubMessage();
                                        while (!ticketNameData.atEnd()) {
                                            switch (ticketNameData.fieldNumber()) {
                                                case 2:
                                                    res.reservedTicket.name = ticketNameData.readString();
                                                    break;
                                                default:
                                                    ticketNameData.skip();
                                            }
                                        }
                                        break;
                                    case 2:
                                        res.reservationFor.departureStation.name = tripData.readString();
                                        break;
                                    case 3:
                                        res.reservationFor.arrivalStation.name = tripData.readString();
                                        break;
                                    case 4:
                                        res.reservedTicket.ticketedSeat.seatingType = tripData.readVarintField() + "";
                                    case 8:
                                        res.reservationFor.departureTime = readTime(tripData);
                                        break;
                                    case 9:
                                        res.reservationFor.arrivalTime = readTime(tripData);
                                        break;
                                    default:
                                        tripData.skip();
                                }
                            }
                            break;
                        case 3:
                            let travelerData = level1.readSubMessage();
                            while (!travelerData.atEnd()) {
                                switch (travelerData.fieldNumber()) {
                                    case 3:
                                        res.underName.givenName = travelerData.readString();
                                        break;
                                    case 4:
                                        res.underName.familyName = travelerData.readString();
                                        break;
                                    case 7:
                                        res.programMembershipUsed.programName = travelerData.readString();
                                    default:
                                        travelerData.skip();
                                }
                            }
                            break;
                        case 8:
                            let trainData = level1.readSubMessage();
                            while (!trainData.atEnd()) {
                                switch (trainData.fieldNumber()) {
                                    case 11:
                                        res.reservationFor.trainNumber = trainData.readString();
                                        break;
                                    default:
                                        trainData.skip();
                                }
                            }
                            break;
                        default:
                            level1.skip();
                    }
                }
                break;
            default:
                top.skip();
        }
    }

    // convert unbound tickets
    if (!res.reservationFor.trainNumber) {
        res.reservedTicket.validFrom = res.reservationFor.departureTime;
        res.reservedTicket.validUntil = res.reservationFor.arrivalTime;
        res.reservationFor.departureDay = res.reservationFor.departureTime;
        res.reservationFor.departureTime = undefined;
        res.reservationFor.arrivalTime = undefined;
    }

    // convert unbound passes to a Ticket
    if (!res.reservationFor.departureStation.name) {
        res.reservedTicket.underName = res.underName;
        res.reservedTicket.ticketNumber = res.reservationNumber;
        return res.reservedTicket;
    }

    return res;
}

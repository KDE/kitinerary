/*
   SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseOnlineTicket(json)
{
    const response = json[0].response;
    let result = [];
    // TODO mark cancelled trips as such
    for (const trips of [response.cancelledTrips, response.passedTrips, response.trips, response.preReservedTrips]) {
        for (const trip of trips) {
            for (const jny of [trip.trip.tripDetails.inwardJourney, trip.trip.tripDetails.outwardJourney]) {
                let jnyResult = [];
                for (const step of jny.timeline.steps) {
                    if (!step['train']) {
                        continue;
                    }
                    const train = step.train;
                    let res = JsonLd.newTrainReservation();
                    res.reservationFor.arrivalStation.name = train.arrival.stationLabel;
                    res.reservationFor.arrivalStation.identifier = 'sncf:' + train.arrival.stationCode;
                    res.reservationFor.arrivalTime = jny.departureDate.substr(0, 11) + train.arrival.timeLabel;
                    res.reservationFor.departureStation.name = train.departure.stationLabel;
                    res.reservationFor.departureStation.identifier = 'sncf:' + train.departure.stationCode;
                    res.reservationFor.departureTime = jny.departureDate.substr(0, 11) + train.departure.timeLabel;
                    res.reservationFor.trainName = train.transporter.description;
                    res.reservationFor.trainNumber = train.transporter.number.match(/\d+/)[0];
                    res.reservedTicket.ticketedSeat.seatingType = train.comfortClass.code;

                    if (train.tripIv.composition) {
                        res.reservationFor.departurePlatform = train.tripIv.composition.positionInformation.title;
                    }
                    jnyResult.push(res);
                }
                for (const jnyRes of jnyResult) {
                    for (const traveler of jny.travelersIdentity) {
                        let res = JsonLd.clone(jnyRes);
                        res.underName.name = traveler.fullName;
                        res.underName.givenName = traveler.firstName;
                        res.underName.familyName = traveler.lastName;
                        for (const fare of jny.travelersFares) {
                            if (fare.travelerDescription != res.underName.name)
                                continue;
                            for (const segFare of fare.segmentFares) {
                                if (segFare.transporterDescription != res.reservationFor.trainNumber)
                                    continue;
                                res.reservedTicket.ticketName = segFare.fareName;
                                break;
                            }
                            break;
                        }
                        result.push(res);
                    }
                }
            }
        }
    }
    return result;
}

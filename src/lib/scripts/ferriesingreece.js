/*
   SPDX-FileCopyrightText: 2023 Ingo Klöcker <kloecker@kde.org>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parseTrip(start, destination, lines) {
    let res = JsonLd.newBoatReservation()
    res.reservationFor.departureBoatTerminal.name = start
    res.reservationFor.arrivalBoatTerminal.name = destination

    // parse a table (header and 1 row) wrapped over multiple lines
    let entries = []
    for (let i = 0; i+1 < lines.length; i += 2) {
        let headerLine = lines[i]
        let rowLine = lines[i+1]
        if (!headerLine.match(/^\s\s\s*/) || !rowLine.match(/^\s\s\s*/)) {
            break
        }
        const headers = headerLine.split(/\s\s\s*/).slice(1)
        const values = rowLine.split(/\s\s\s*/).slice(1)
        if (headers.length != values.length) {
            console.error(`Number of headers (${headers.length}) does not match number of values (${values.length})`)
            console.log(`headers: ${headers}`)
            console.log(`values: ${values}`)
            continue
        }
        for (let j = 0; j < headers.length; ++j) {
            entries.push({key: headers[j], value: values[j]})
        }
    }

    // extract the needed information from the table entries
    for (let i = 0; i < entries.length; ++i) {
        const {key, value} = entries[i]
        if (key == 'Booking Reference Number') {
            res.reservationNumber = value
        } else if (key == 'Company') {
            res.provider = value
        } else if (key == 'Departure') {
            // the following entry should be the departure date and time
            const dateTimeEntry = entries[i+1]
            if (dateTimeEntry && dateTimeEntry.key == 'Date / Time') {
                res.reservationFor.departureTime = JsonLd.toDateTime(dateTimeEntry.value, 'yyyy-MM-dd / hh:mm', 'gr')
                ++i
            }
        } else if (key == 'Arrival') {
            // the following entry should be the arrival date and time
            const dateTimeEntry = entries[i+1]
            if (dateTimeEntry && dateTimeEntry.key == 'Date / Time') {
                res.reservationFor.arrivalTime = JsonLd.toDateTime(dateTimeEntry.value, 'yyyy-MM-dd / hh:mm', 'gr')
                ++i
            }
        } else if (key == 'Name') {
            res.underName.name = value
        }
    }

    return res
}

function parseTicket(pdf, node, triggerNode) {
    const text = pdf.text
    if (!text.match(/FerriesinGreece/))
        return

    let reservations = []
    const lines = text.split('\n')
    for (let i = 0; i < lines.length; ++i) {
        let line = lines[i]
        const trip = line.match(/^([A-Z][A-Z, ]*[A-Z])\s*⟶\s*([A-Z][A-Z, ]*[A-Z])$/)
        if (trip) {
            let res = parseTrip(trip[1], trip[2], lines.slice(i + 1))
            if (res) {
                reservations.push(res)
            }
            continue
        }
    }

    return reservations
}

// Example of extracted table entries
// Booking Reference Number: 123456789
// Company: K/x Anek-superfast
// Company Reference Number: 987654321
// Departure: ITALY, BARI
// Date / Time: 2023-07-12 / 19:30
// Arrival: PATRA
// Date / Time: 2023-07-13 / 13:00
// Vessel: Superfast Ii
// Passengers / Vehicles: 1/0
// Gender: Male
// Name: LASTNAME FIRSTNAME
// Passenger type: Adult
// Seat type: 4 BED CABIN (BUNK BEDS)
// Name: LASTNAME FIRSTNAME <- this duplicate entry is not present in the email
// Birth date: yyyy-MM-dd BirthPlace
// Nationality: Germany
// Passport number: XXXXXXXX yyyy-MM-dd <- expiration date

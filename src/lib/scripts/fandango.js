/*
   SPDX-FileCopyrightText: 2026 David Pilarčík <meow@charliecat.space>
   SPDX-License-Identifier: LGPL-2.0-or-later
*/

function parsePkPass(pass, node) {
	let res = node.result[0];

	res.reservationFor.name = pass.field["event-name"].value
	res.reservationFor.location.name = `${pass.field["event-name"].label}, AUD ${pass.field["auditorium"].value}`
	res.reservationFor.location.address = pass.field["theateraddress"]?.value
	res.reservedTicket.ticketedSeat = JsonLd.newObject("Seat");
	res.reservedTicket.ticketedSeat.seatNumber = pass.field["seats"].value
	res.reservationNumber = pass.field["confirmationnumber"]?.value

	const movieRuntime = pass.field["runtime"]?.value
	if (movieRuntime)
		res.reservationFor.endDate = createEventEndTime(res.reservationFor.startDate, movieRuntime)

    return res;
}

/**
 * @param {Date} startTime 
 * @param {`{number} hr ${number} min` | `${number} min` | `{number} hr`} movieRuntime 
 * @returns {Date}
 */
function createEventEndTime(startTime, movieRuntime) {
	const runtimeParts = movieRuntime.match(/(?:(\d+) hr )?(?:(\d+) min)?/);
	const endTime = new Date(startTime);
	endTime.setHours(endTime.getHours() + (runtimeParts[1] ? parseInt(runtimeParts[1]) : 0));
	endTime.setMinutes(endTime.getMinutes() + (runtimeParts[2] ? parseInt(runtimeParts[2]) : 0));
	return endTime;
}
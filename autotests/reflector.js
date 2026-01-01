/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function dumpValue(value, depth)
{
    if (value === undefined || value === null
        || typeof value == "boolean"
        || typeof value == "number")
    {
        return value;
    } else if (typeof value == "string") {
        if (value.match(/[^"\{\},]\n  +/)) // normalize whitespace variations between different Poppler versions
            value = value.replace(/ +/g, ' ').trim();
        return value.replace(/\r\n/g, '\n').replace(/\n\n/g, '\n');
    } else if (typeof value == "object") {
        if (depth == 0) {
            return "[...]";
        }
        if (value instanceof ArrayBuffer) {
            const view = new DataView(value);
            for (let i = 0; i < view.byteLength; ++i) {
                const c = view.getUInt8(i);
                if (c < 0x20 && c != 0x0A && c != 0x0D) {
                    return "<binary: " + view.byteLength + " bytes>";
                }
            }
            return value;
        }
        if (value.length != undefined) {
            var arr = new Array();
            for (var i = 0; i < value.length; ++i) {
                arr.push(dumpValue(value[i], depth - 1));
            }
            return arr;
        }
        var obj = new Object();
        var empty = true;
        for (var prop in value) {
            if (prop == "parent" || prop == "firstChild" || prop == "nextSibling" || value[prop] === undefined || value[prop] === null || typeof value[prop] == "function") {
                continue;
            }
            const v = dumpValue(value[prop], depth - 1);
            if (v != undefined) { // ### Qt5 behavior compat, remove with Qt6
                obj[prop] = v;
            }
            if (v) { // ### Qt5 behavior compat, remove with Qt6
                empty = false;
            }
        }

        if (empty && Object.keys(obj).length >= 1) {
            return undefined; // ### Qt5 behavior compat, remove with Qt6
        }
        // implicitly convertible (color, date, opaque types, etc)
        if (empty) {
            let s = value.toString();
            if (s.match(/QVariant.*KCalendarCore::Calendar/)) { // normalize opaque shared pointer handle
                return 'KCalendarCore::Calendar';
            }
            return value.toString();
        } else {
            return obj;
        }
    } else {
        console.log("unhandled value type", typeof value);
    }
    return value;
}

function dumpArgs(content, node, triggerNode)
{
    var res = new Array();
    var wrapper = new Object();
    wrapper.content = dumpValue(content, 6);
    res.push(wrapper);
    res.push(dumpValue(node, 6));
    res.push({ triggerNodeEqualsNode: triggerNode == node });
    return res;
}

/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

function dumpValue(value, depth)
{
    if (value === undefined || value === null
        || typeof value == "boolean"
        || typeof value == "number"
        || typeof value == "string")
    {
        return value;
    } else if (typeof value == "object") {
        if (depth == 0) {
            return "[...]";
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
            obj[prop] = dumpValue(value[prop], depth - 1);
            empty = false;
        }

        // implicitly convertible (color, date, etC)
        if (empty) {
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
    res.push(dumpValue(Context, 6));
    return res;
}

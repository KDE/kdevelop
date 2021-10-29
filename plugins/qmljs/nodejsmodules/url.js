/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

exports.parse = function (urlStr, parseQueryString, slashesDenoteHost) { return {href: "", protocol: "", auth: "", hostname: "", port: 1, host: "", pathname: "", search: "", query: new Object(), hash: ""}; };
exports.parse("", true, "");

exports.format = function (urlObj) { return ""; };
exports.format(new Object());

exports.resolve = function (from, to) { return ""; };
exports.resolve("", "");


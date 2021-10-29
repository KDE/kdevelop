/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

exports.stringify = function (object, sep, eq) { return ""; };
exports.stringify(new Object(), "", "");

exports.parse = function (str, sep, eq, options) { return new Object(); };
exports.parse("", "", "", new Object());

exports.escape = new Function();

exports.unescape = new Function();


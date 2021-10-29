/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

exports.format = function (format, args) { return ""; };
exports.format("", _mixed);

exports.debug = function (string) { return ; };
exports.debug("");

exports.error = function (string) { return ; };
exports.error("");

exports.puts = function (string) { return ; };
exports.puts("");

exports.print = function (string) { return ; };
exports.print("");

exports.log = function (string) { return ; };
exports.log("");

exports.inspect = function (object, options) { return ""; };
exports.inspect(new Object(), new Object());

exports.isArray = function (object) { return true; };
exports.isArray([]);

exports.isRegExp = function (object) { return true; };
exports.isRegExp(new RegExp());

exports.isDate = function (object) { return true; };
exports.isDate(new Date());

exports.isError = function (object) { return true; };
exports.isError(new Error());

exports.inherits = function (constructor, superConstructor) { return ; };
exports.inherits(function(){}, function(){});


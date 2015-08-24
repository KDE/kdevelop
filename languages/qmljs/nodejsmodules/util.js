/*
 * This file is part of qmljs, the QML/JS language support plugin for KDevelop
 * Copyright (c) 2014 Denis Steckelmacher <steckdenis@yahoo.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
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


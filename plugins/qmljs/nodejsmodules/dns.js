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
    
exports.lookup = function (domain, family, callback) { return ; };
exports.lookup("", 1, function(){});

exports.resolve = function (domain, rrtype, callback) { return ; };
exports.resolve("", "", function(){});

exports.resolve4 = function (domain, callback) { return ; };
exports.resolve4("", function(){});

exports.resolve6 = function (domain, callback) { return ; };
exports.resolve6("", function(){});

exports.resolveMx = function (domain, callback) { return ; };
exports.resolveMx("", function(){});

exports.resolveTxt = function (domain, callback) { return ; };
exports.resolveTxt("", function(){});

exports.resolveSrv = function (domain, callback) { return ; };
exports.resolveSrv("", function(){});

exports.resolveNs = function (domain, callback) { return ; };
exports.resolveNs("", function(){});

exports.resolveCname = function (domain, callback) { return ; };
exports.resolveCname("", function(){});

exports.reverse = function (ip, callback) { return ; };
exports.reverse("", function(){});

exports.NODATA = 1;

exports.FORMERR = 1;

exports.SERVFAIL = 1;

exports.NOTFOUND = 1;

exports.NOTIMP = 1;

exports.REFUSED = 1;

exports.BADQUERY = 1;

exports.BADNAME = 1;

exports.BADFAMILY = 1;

exports.BADRESP = 1;

exports.CONNREFUSED = 1;

exports.TIMEOUT = 1;

exports.EOF = 1;

exports.FILE = 1;

exports.NOMEM = 1;

exports.DESTRUCTION = 1;

exports.BADSTR = 1;

exports.BADFLAGS = 1;

exports.NONAME = 1;

exports.BADHINTS = 1;

exports.NOTINITIALIZED = 1;

exports.LOADIPHLPAPI = 1;

exports.ADDRGETNETWORKPARAMS = 1;

exports.CANCELLED = 1;


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
    
var tls = require("tls");

var http = require("http");

exports.createServer = function (requestListener) { return new Server(); };
exports.createServer(function(){});

/*
 * exports.Server
 */
function Server() { return ; }
exports.Server = Server;
exports.Server.prototype = tls.Server;

exports.Server.prototype.listen = function (port, hostname, backlog, callback) { return ; };
exports.Server.prototype.listen(1, "", 1, function(){});

exports.Server.prototype.close = function (callback) { return ; };
exports.Server.prototype.close(function(){});


exports.request = function (options, callback) { return new http.ClientRequest(); };
exports.request(new Object(), function(){});

exports.get = function (options, callback) { return new http.ClientRequest(); };
exports.get(new Object(), function(){});

/*
 * exports.Agent
 */
function Agent() { return ; }
exports.Agent = Agent;
exports.Agent.prototype = http.Agent;


exports.globalAgent = new Agent();


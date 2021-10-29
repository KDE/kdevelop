/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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


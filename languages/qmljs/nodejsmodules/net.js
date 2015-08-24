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
    
var event = require("event");

exports.createServer = function (options, connectionListener) { return new Server(); };
exports.createServer(new Object(), function(){});

exports.connect = function (options, connectionListener) { return new Socket(); };
exports.connect(new Object(), function(){});

exports.createConnection = function (options, connectionListener) { return new Socket(); };
exports.createConnection(new Object(), function(){});

/*
 * exports.Server
 */
function Server() { return ; }
exports.Server = Server;
exports.Server.prototype = event.EventEmitter;

exports.Server.prototype.listen = function (port, host, backlog, callback) { return ; };
exports.Server.prototype.listen(1, "", 1, function(){});

exports.Server.prototype.close = function (callback) { return ; };
exports.Server.prototype.close(function(){});

exports.Server.prototype.address = function () { return {port: 1, family: "", address: ""}; };
exports.Server.prototype.address();

exports.Server.prototype.unref = function () { return ; };
exports.Server.prototype.unref();

exports.Server.prototype.ref = function () { return ; };
exports.Server.prototype.ref();

exports.Server.prototype.maxConnections = 1;

exports.Server.prototype.connections = 1;

exports.Server.prototype.getConnections = function (callback) { return ; };
exports.Server.prototype.getConnections(function(){});


/*
 * exports.Socket
 */
function Socket(options) { return ; }
exports.Socket = Socket;
exports.Socket.prototype = event.EventEmitter;

exports.Socket.prototype.connect = function (port, host, connectionListener) { return ; };
exports.Socket.prototype.connect(1, "", function(){});

exports.Socket.prototype.bufferSize = 1;

exports.Socket.prototype.setEncoding = function (encoding) { return ; };
exports.Socket.prototype.setEncoding("");

exports.Socket.prototype.write = function (data, encoding, callback) { return ; };
exports.Socket.prototype.write("", "", function(){});

exports.Socket.prototype.end = function (data, encoding) { return ; };
exports.Socket.prototype.end("", "");

exports.Socket.prototype.destroy = function () { return ; };
exports.Socket.prototype.destroy();

exports.Socket.prototype.pause = function () { return ; };
exports.Socket.prototype.pause();

exports.Socket.prototype.resume = function () { return ; };
exports.Socket.prototype.resume();

exports.Socket.prototype.setTimeout = function (timeout, callback) { return ; };
exports.Socket.prototype.setTimeout(1, function(){});

exports.Socket.prototype.setNoDelay = function (noDelay) { return ; };
exports.Socket.prototype.setNoDelay(true);

exports.Socket.prototype.setKeepAlive = function (enable, initialDelay) { return ; };
exports.Socket.prototype.setKeepAlive(true, 1);

exports.Socket.prototype.address = function () { return {port: 1, family: "", address: ""}; };
exports.Socket.prototype.address();

exports.Socket.prototype.unref = function () { return ; };
exports.Socket.prototype.unref();

exports.Socket.prototype.ref = function () { return ; };
exports.Socket.prototype.ref();

exports.Socket.prototype.remoteAddress = "";

exports.Socket.prototype.remotePort = 1;

exports.Socket.prototype.localAddress = "";

exports.Socket.prototype.localPort = 1;

exports.Socket.prototype.bytesRead = 1;

exports.Socket.prototype.bytesWritten = 1;


exports.isIP = function (input) { return 1; };
exports.isIP("");

exports.isIPv4 = function (input) { return true; };
exports.isIPv4("");

exports.isIPv6 = function (input) { return true; };
exports.isIPv6("");


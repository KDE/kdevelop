/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

var event = require("event");

var buffer = require("buffer");

exports.createSocket = function (type, callback) { return new Socket; };
exports.createSocket("", function(){});

/*
 * exports.Socket
 */
function Socket() { return ; }
exports.Socket = Socket;
exports.Socket.prototype = event.EventEmitter;

exports.Socket.prototype.send = function (buf, offset, length, port, address, callback) { return ; };
exports.Socket.prototype.send(new buffer.Buffer(), 1, 1, 1, "", function(){});

exports.Socket.prototype.bind = function (port, address, callback) { return ; };
exports.Socket.prototype.bind(1, "", function(){});

exports.Socket.prototype.close = function () { return ; };
exports.Socket.prototype.close();

exports.Socket.prototype.address = function () { return {address: "", family: "", port: 1}; };
exports.Socket.prototype.address();

exports.Socket.prototype.setBroadcast = function (flag) { return ; };
exports.Socket.prototype.setBroadcast(true);

exports.Socket.prototype.setTTL = function (ttl) { return ; };
exports.Socket.prototype.setTTL(1);

exports.Socket.prototype.setMulticastTTL = function (ttl) { return ; };
exports.Socket.prototype.setMulticastTTL(1);

exports.Socket.prototype.setMulticastLoopback = function (flag) { return ; };
exports.Socket.prototype.setMulticastLoopback(true);

exports.Socket.prototype.addMembership = function (multicastAddress, multicastInterface) { return ; };
exports.Socket.prototype.addMembership("", "");

exports.Socket.prototype.dropMembership = function (multicastAddress, multicastInterface) { return ; };
exports.Socket.prototype.dropMembership("", "");

exports.Socket.prototype.unref = function () { return ; };
exports.Socket.prototype.unref();

exports.Socket.prototype.ref = function () { return ; };
exports.Socket.prototype.ref();



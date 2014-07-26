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



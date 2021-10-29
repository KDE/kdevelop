/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

var event = require("event");

exports.getCiphers = function () { return []; };
exports.getCiphers();

exports.createServer = function (options, secureConnectionListener) { return new Server(); };
exports.createServer(new Object(), function(){});

exports.SLAB_BUFFER_SIZE = 1;

exports.connect = function (options, callback) { return new CleartextStream(); };
exports.connect(new Object(), function(){});

exports.createSecurePair = function (credentials, isServer, requestCert, rejectUnauthorized) { return new SecurePair(); };
exports.createSecurePair(new Object(), true, true, true);

/*
 * exports.SecurePair
 */
function SecurePair() { return ; }
exports.SecurePair = SecurePair;
exports.SecurePair.prototype = event.EventEmitter;


/*
 * exports.Server
 */
function Server() { return ; }
exports.Server = Server;
exports.Server.prototype = event.EventEmitter;

exports.Server.prototype.listen = function (port, host, callback) { return ; };
exports.Server.prototype.listen(1, "", function(){});

exports.Server.prototype.close = function () { return ; };
exports.Server.prototype.close();

exports.Server.prototype.address = function () { return {port: 1, family: "", address: ""}; };
exports.Server.prototype.address();

exports.Server.prototype.addContext = function (hostname, credentials) { return ; };
exports.Server.prototype.addContext("", new Object());

exports.Server.prototype.maxConnections = 1;

exports.Server.prototype.connections = 1;


/*
 * exports.CryptoStream
 */
function CryptoStream() { return ; }
exports.CryptoStream = CryptoStream;

exports.CryptoStream.prototype.bytesWritten = 1;


/*
 * exports.CleartextStream
 */
function CleartextStream() { return ; }
exports.CleartextStream = CleartextStream;
exports.CleartextStream.prototype = event.EventEmitter;

exports.CleartextStream.prototype.authorized = true;

exports.CleartextStream.prototype.authorizationError = "";

exports.CleartextStream.prototype.getPeerCertificate = function () { return {
    subject: {C: "", ST: "", L: "", O: "", OU: "", CN: ""},
    issuer: {C: "", ST: "", L: "", O: "", OU: "", CN: ""},
    valid_from: "",
    valid_to: "",
    fingerprint: ""}; };
exports.CleartextStream.prototype.getPeerCertificate();

exports.CleartextStream.prototype.getCipher = function () { return {name: "", version: ""}; };
exports.CleartextStream.prototype.getCipher();

exports.CleartextStream.prototype.address = function () { return {port: 1, family: "", address: ""}; };
exports.CleartextStream.prototype.address();

exports.CleartextStream.prototype.remoteAddress = "";

exports.CleartextStream.prototype.remotePort = 1;



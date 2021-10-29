/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

var event = require("event");

/*
 * exports.Readable
 */
function Readable() { return ; }
exports.Readable = Readable;
exports.Readable.prototype = event.EventEmitter;

exports.Readable.prototype.read = function (size) { return ""; };
exports.Readable.prototype.read(1);

exports.Readable.prototype.setEncoding = function (encoding) { return ; };
exports.Readable.prototype.setEncoding("");

exports.Readable.prototype.resume = function () { return ; };
exports.Readable.prototype.resume();

exports.Readable.prototype.pause = function () { return ; };
exports.Readable.prototype.pause();

exports.Readable.prototype.pipe = function (destination, options) { return ; };
exports.Readable.prototype.pipe(new Writable(), new Object());

exports.Readable.prototype.unpipe = function (destination) { return ; };
exports.Readable.prototype.unpipe(new Writable());

exports.Readable.prototype.unshift = function (chunk) { return ; };
exports.Readable.prototype.unshift("");

exports.Readable.prototype.wrap = function (stream) { return new Readable(); };
exports.Readable.prototype.wrap(_mixed);


/*
 * exports.Writable
 */
function Writable() { return ; }
exports.Writable = Writable;
exports.Writable.prototype = event.EventEmitter;

exports.Writable.prototype.write = function (chunk, encoding, callback) { return ; };
exports.Writable.prototype.write("", "", function(){});

exports.Writable.prototype.end = function (chunk, encoding, callback) { return ; };
exports.Writable.prototype.end("", "", function(){});


/*
 * exports.Duplex
 */
function Duplex() { return ; }
exports.Duplex = Duplex;
exports.Duplex.prototype = event.EventEmitter;

exports.Duplex.prototype.read = function (size) { return ""; };
exports.Duplex.prototype.read(1);

exports.Duplex.prototype.setEncoding = function (encoding) { return ; };
exports.Duplex.prototype.setEncoding("");

exports.Duplex.prototype.resume = function () { return ; };
exports.Duplex.prototype.resume();

exports.Duplex.prototype.pause = function () { return ; };
exports.Duplex.prototype.pause();

exports.Duplex.prototype.pipe = function (destination, options) { return ; };
exports.Duplex.prototype.pipe(new Writable(), new Object());

exports.Duplex.prototype.unpipe = function (destination) { return ; };
exports.Duplex.prototype.unpipe(new Writable());

exports.Duplex.prototype.unshift = function (chunk) { return ; };
exports.Duplex.prototype.unshift("");

exports.Duplex.prototype.wrap = function (stream) { return new Readable(); };
exports.Duplex.prototype.wrap(_mixed);

exports.Duplex.prototype.write = function (chunk, encoding, callback) { return ; };
exports.Duplex.prototype.write("", "", function(){});

exports.Duplex.prototype.end = function (chunk, encoding, callback) { return ; };
exports.Duplex.prototype.end("", "", function(){});


/*
 * exports.Transform
 */
function Transform() { return ; }
exports.Transform = Transform;
exports.Transform.prototype = exports.Duplex;



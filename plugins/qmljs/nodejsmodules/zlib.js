/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

var stream = require("stream");

var buffer = require("buffer");

exports.createGzip = function (options) { return new Gzip(); };
exports.createGzip(new Object());

exports.createGunzip = function (options) { return new Gunzip(); };
exports.createGunzip(new Object());

exports.createDeflate = function (options) { return new Deflate(); };
exports.createDeflate(new Object());

exports.createInflate = function (options) { return new Inflate(); };
exports.createInflate(new Object());

exports.createDeflateRaw = function (options) { return new DeflateRaw(); };
exports.createDeflateRaw(new Object());

exports.createInflateRaw = function (options) { return new InflateRaw(); };
exports.createInflateRaw(new Object());

exports.createUnzip = function (options) { return new Unzip(); };
exports.createUnzip(new Object());

/*
 * exports.Zlib
 */
function Zlib() { return ; }
exports.Zlib = Zlib;
exports.Zlib.prototype = stream.Duplex;

exports.Zlib.prototype.flush = function (callback) { return ; };
exports.Zlib.prototype.flush(function(){});

exports.Zlib.prototype.reset = function () { return ; };
exports.Zlib.prototype.reset();


/*
 * exports.Gzip
 */
function Gzip() { return ; }
exports.Gzip = Gzip;
exports.Gzip.prototype = exports.Zlib;


/*
 * exports.Gunzip
 */
function Gunzip() { return ; }
exports.Gunzip = Gunzip;
exports.Gunzip.prototype = exports.Zlib;


/*
 * exports.Deflate
 */
function Deflate() { return ; }
exports.Deflate = Deflate;
exports.Deflate.prototype = exports.Zlib;


/*
 * exports.Inflate
 */
function Inflate() { return ; }
exports.Inflate = Inflate;
exports.Inflate.prototype = exports.Zlib;


/*
 * exports.DeflateRaw
 */
function DeflateRaw() { return ; }
exports.DeflateRaw = DeflateRaw;
exports.DeflateRaw.prototype = exports.Zlib;


/*
 * exports.InflateRaw
 */
function InflateRaw() { return ; }
exports.InflateRaw = InflateRaw;
exports.InflateRaw.prototype = exports.Zlib;


/*
 * exports.Unzip
 */
function Unzip() { return ; }
exports.Unzip = Unzip;
exports.Unzip.prototype = exports.Zlib;


exports.deflate = function (buf, callback) { return ; };
exports.deflate(new buffer.Buffer(), function(){});

exports.deflateRaw = function (buf, callback) { return ; };
exports.deflateRaw(new buffer.Buffer(), function(){});

exports.gzip = function (buf, callback) { return ; };
exports.gzip(new buffer.Buffer(), function(){});

exports.gunzip = function (buf, callback) { return ; };
exports.gunzip(new buffer.Buffer(), function(){});

exports.inflate = function (buf, callback) { return ; };
exports.inflate(new buffer.Buffer(), function(){});

exports.inflateRaw = function (buf, callback) { return ; };
exports.inflateRaw(new buffer.Buffer(), function(){});

exports.unzip = function (buf, callback) { return ; };
exports.unzip(new buffer.Buffer(), function(){});


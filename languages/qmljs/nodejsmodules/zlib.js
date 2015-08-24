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


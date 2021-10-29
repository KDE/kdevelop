/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

var stream = require("stream");

exports.isatty = function (fd) { return true; };
exports.isatty(1);

exports.setRawMode = function (mode) { return ; };
exports.setRawMode(true);

/*
 * exports.ReadStream
 */
function ReadStream() { return ; }
exports.ReadStream = ReadStream;
exports.ReadStream.prototype = stream.Readable;

exports.ReadStream.prototype.isRaw = true;

exports.ReadStream.prototype.setRawMode = function (mode) { return ; };
exports.ReadStream.prototype.setRawMode(true);


/*
 * exports.WriteStream
 */
function WriteStream() { return ; }
exports.WriteStream = WriteStream;
exports.WriteStream.prototype = stream.Writable;

exports.WriteStream.prototype.columns = 1;

exports.WriteStream.prototype.rows = 1;



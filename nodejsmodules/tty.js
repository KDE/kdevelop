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



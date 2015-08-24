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
    
/*
 * exports.Buffer
 */
function Buffer(size) { return ; }
exports.Buffer = Buffer;

exports.Buffer.prototype.isEncoding = function (encoding) { return true; };
exports.Buffer.prototype.isEncoding("");

exports.Buffer.prototype.write = function (string, offset, length, encoding) { return ; };
exports.Buffer.prototype.write("", 1, 1, "");

exports.Buffer.prototype.toJSON = function () { return ""; };
exports.Buffer.prototype.toJSON();

exports.Buffer.prototype.isBuffer = function (obj) { return true; };
exports.Buffer.prototype.isBuffer(new Buffer());

exports.Buffer.prototype.byteLength = function (string, encoding) { return 1; };
exports.Buffer.prototype.byteLength("", "");

exports.Buffer.prototype.concat = function (list, totalLength) { return ; };
exports.Buffer.prototype.concat([], 1);

exports.Buffer.prototype.length = 1;

exports.Buffer.prototype.copy = function (target, targetStart, sourceStart, sourceEnd) { return ; };
exports.Buffer.prototype.copy(new Buffer(), 1, 1, 1);

exports.Buffer.prototype.slice = function (start, end) { return ; };
exports.Buffer.prototype.slice(1, 1);

exports.Buffer.prototype.readUInt8 = function (offset, noAssert) { return 1; };
exports.Buffer.prototype.readUInt8(1, true);

exports.Buffer.prototype.readUInt16LE = function (offset, noAssert) { return 1; };
exports.Buffer.prototype.readUInt16LE(1, true);

exports.Buffer.prototype.readUInt16BE = function (offset, noAssert) { return 1; };
exports.Buffer.prototype.readUInt16BE(1, true);

exports.Buffer.prototype.readUInt32LE = function (offset, noAssert) { return 1; };
exports.Buffer.prototype.readUInt32LE(1, true);

exports.Buffer.prototype.readUInt32BE = function (offset, noAssert) { return 1; };
exports.Buffer.prototype.readUInt32BE(1, true);

exports.Buffer.prototype.readInt8 = function (offset, noAssert) { return 1; };
exports.Buffer.prototype.readInt8(1, true);

exports.Buffer.prototype.readInt16LE = function (offset, noAssert) { return 1; };
exports.Buffer.prototype.readInt16LE(1, true);

exports.Buffer.prototype.readInt16BE = function (offset, noAssert) { return 1; };
exports.Buffer.prototype.readInt16BE(1, true);

exports.Buffer.prototype.readInt32LE = function (offset, noAssert) { return 1; };
exports.Buffer.prototype.readInt32LE(1, true);

exports.Buffer.prototype.readInt32BE = function (offset, noAssert) { return 1; };
exports.Buffer.prototype.readInt32BE(1, true);

exports.Buffer.prototype.readFloatLE = function (offset, noAssert) { return 1.0; };
exports.Buffer.prototype.readFloatLE(1, true);

exports.Buffer.prototype.readFloatBE = function (offset, noAssert) { return 1.0; };
exports.Buffer.prototype.readFloatBE(1, true);

exports.Buffer.prototype.readDoubleLE = function (offset, noAssert) { return 1.0; };
exports.Buffer.prototype.readDoubleLE(1, true);

exports.Buffer.prototype.readDoubleBE = function (offset, noAssert) { return 1.0; };
exports.Buffer.prototype.readDoubleBE(1, true);

exports.Buffer.prototype.writeUInt8 = function (value, offset, noAssert) { return ; };
exports.Buffer.prototype.writeUInt8(1, 1, true);

exports.Buffer.prototype.writeUInt16LE = function (value, offset, noAssert) { return ; };
exports.Buffer.prototype.writeUInt16LE(1, 1, true);

exports.Buffer.prototype.writeUInt16BE = function (value, offset, noAssert) { return ; };
exports.Buffer.prototype.writeUInt16BE(1, 1, true);

exports.Buffer.prototype.writeUInt32LE = function (value, offset, noAssert) { return ; };
exports.Buffer.prototype.writeUInt32LE(1, 1, true);

exports.Buffer.prototype.writeUInt32BE = function (value, offset, noAssert) { return ; };
exports.Buffer.prototype.writeUInt32BE(1, 1, true);

exports.Buffer.prototype.writeInt8 = function (value, offset, noAssert) { return ; };
exports.Buffer.prototype.writeInt8(1, 1, true);

exports.Buffer.prototype.writeInt16LE = function (value, offset, noAssert) { return ; };
exports.Buffer.prototype.writeInt16LE(1, 1, true);

exports.Buffer.prototype.writeInt16BE = function (value, offset, noAssert) { return ; };
exports.Buffer.prototype.writeInt16BE(1, 1, true);

exports.Buffer.prototype.writeInt32LE = function (value, offset, noAssert) { return ; };
exports.Buffer.prototype.writeInt32LE(1, 1, true);

exports.Buffer.prototype.writeInt32BE = function (value, offset, noAssert) { return ; };
exports.Buffer.prototype.writeInt32BE(1, 1, true);

exports.Buffer.prototype.writeFloatLE = function (value, offset, noAssert) { return ; };
exports.Buffer.prototype.writeFloatLE(1.0, 1, true);

exports.Buffer.prototype.writeFloatBE = function (value, offset, noAssert) { return ; };
exports.Buffer.prototype.writeFloatBE(1.0, 1, true);

exports.Buffer.prototype.writeDoubleLE = function (value, offset, noAssert) { return ; };
exports.Buffer.prototype.writeDoubleLE(1.0, 1, true);

exports.Buffer.prototype.writeDoubleBE = function (value, offset, noAssert) { return ; };
exports.Buffer.prototype.writeDoubleBE(1.0, 1, true);

exports.Buffer.prototype.fill = function (value, offset, end) { return ; };
exports.Buffer.prototype.fill(_mixed, 1, 1);

exports.Buffer.prototype.INSPECT_MAX_BYTES = 1;



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

var event = require("event");

exports.createInterface = function (options) { return new Interface(); };
exports.createInterface(new Object());

/*
 * exports.Interface
 */
function Interface() { return ; }
exports.Interface = Interface;
exports.Interface.prototype = event.EventEmitter;

exports.Interface.prototype.setPrompt = function (prompt, length) { return ; };
exports.Interface.prototype.setPrompt("", 1);

exports.Interface.prototype.prompt = function (preserveCursor) { return ; };
exports.Interface.prototype.prompt(true);

exports.Interface.prototype.question = function (query, callback) { return ; };
exports.Interface.prototype.question("", function(){});

exports.Interface.prototype.pause = function () { return ; };
exports.Interface.prototype.pause();

exports.Interface.prototype.resume = function () { return ; };
exports.Interface.prototype.resume();

exports.Interface.prototype.close = function () { return ; };
exports.Interface.prototype.close();

exports.Interface.prototype.write = function (data, key) { return ; };
exports.Interface.prototype.write("", new Object());


exports.cursorTo = function (stream, x, y) { return ; };
exports.cursorTo(new stream.Stream(), 1, 1);

exports.moveCursor = function (stream, dx, dy) { return ; };
exports.moveCursor(new stream.Stream(), 1, 1);

exports.clearLine = function (stream, dir) { return ; };
exports.clearLine(new stream.Stream(), 1);

exports.clearScreenDown = function (stream) { return ; };
exports.clearScreenDown(new stream.Stream());


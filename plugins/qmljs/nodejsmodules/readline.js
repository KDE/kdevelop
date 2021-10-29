/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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


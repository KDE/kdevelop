/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

var event = require("event");

/*
 * exports.ChildProcess
 */
function ChildProcess() { return ; }
exports.ChildProcess = ChildProcess;
exports.ChildProcess.prototype = event.EventEmitter;

exports.ChildProcess.prototype.stdin = new stream.Stream();

exports.ChildProcess.prototype.stdout = new stream.Stream();

exports.ChildProcess.prototype.stderr = new stream.Stream();

exports.ChildProcess.prototype.pid = 1;

exports.ChildProcess.prototype.connected = true;

exports.ChildProcess.prototype.kill = function (signal) { return ; };
exports.ChildProcess.prototype.kill("");

exports.ChildProcess.prototype.send = function (message, sendHandle) { return ; };
exports.ChildProcess.prototype.send(new Object(), new Object());

exports.ChildProcess.prototype.disconnect = function () { return ; };
exports.ChildProcess.prototype.disconnect();


exports.spawn = function (command, args, options) { return new ChildProcess(); };
exports.spawn("", [], new Object());

exports.exec = function (command, args, callback) { return new ChildProcess(); };
exports.exec("", [], function(){});

exports.execFile = function (file, args, options, callback) { return new ChildProcess(); };
exports.execFile("", [], new Object(), function(){});

exports.fork = function (modulePath, args, options) { return new ChildProcess(); };
exports.fork("", [], new Object());


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


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
 * exports.EventEmitter
 */
function EventEmitter() { return ; }
exports.EventEmitter = EventEmitter;

exports.EventEmitter.prototype.addListener = function (event, listener) { return ; };
exports.EventEmitter.prototype.addListener("", function(){});

exports.EventEmitter.prototype.on = function (event, listener) { return ; };
exports.EventEmitter.prototype.on("", function(){});

exports.EventEmitter.prototype.once = function (event, listener) { return ; };
exports.EventEmitter.prototype.once("", function(){});

exports.EventEmitter.prototype.removeListener = function (event, listener) { return ; };
exports.EventEmitter.prototype.removeListener("", function(){});

exports.EventEmitter.prototype.removeAllListeners = function (event) { return ; };
exports.EventEmitter.prototype.removeAllListeners("");

exports.EventEmitter.prototype.setMaxListeners = function (n) { return ; };
exports.EventEmitter.prototype.setMaxListeners(1);

exports.EventEmitter.prototype.listeners = function (event) { return []; };
exports.EventEmitter.prototype.listeners("");

exports.EventEmitter.prototype.emit = function (event, args) { return ; };
exports.EventEmitter.prototype.emit("", _mixed);

exports.EventEmitter.prototype.listenerCount = function (emitter, event) { return 1; };
exports.EventEmitter.prototype.listenerCount(new EventEmitter(), "");



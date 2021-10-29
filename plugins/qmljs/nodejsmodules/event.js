/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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



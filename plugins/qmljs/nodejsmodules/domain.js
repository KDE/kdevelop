/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

var event = require("event");

exports.create = function () { return new Domain(); };
exports.create();

/*
 * exports.Domain
 */
function Domain() { return ; }
exports.Domain = Domain;
exports.Domain.prototype = event.EventEmitter;

exports.Domain.prototype.run = function (fn) { return ; };
exports.Domain.prototype.run(function(){});

exports.Domain.prototype.members = [];

exports.Domain.prototype.add = function (emitter) { return ; };
exports.Domain.prototype.add(new Object());

exports.Domain.prototype.remove = function (emitter) { return ; };
exports.Domain.prototype.remove(new Object());

exports.Domain.prototype.bind = function (callback) { return function(){}; };
exports.Domain.prototype.bind(function(){});

exports.Domain.prototype.intercept = function (callback) { return function(){}; };
exports.Domain.prototype.intercept(function(){});

exports.Domain.prototype.enter = function () { return ; };
exports.Domain.prototype.enter();

exports.Domain.prototype.exit = function () { return ; };
exports.Domain.prototype.exit();

exports.Domain.prototype.dispose = function () { return ; };
exports.Domain.prototype.dispose();



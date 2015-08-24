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



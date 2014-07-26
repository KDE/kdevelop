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
    
exports.runInThisContext = function (code, filename) { return _mixed; };
exports.runInThisContext("", "");

exports.runInNewContext = function (code, sandbox, filename) { return _mixed; };
exports.runInNewContext("", new Object(), "");

exports.runInContext = function (code, context, filename) { return _mixed; };
exports.runInContext("", new Context(), "");

exports.createContext = function (initSandbox) { return new Context(); };
exports.createContext(new Object());

exports.createScript = function (code, filename) { return new Script(); };
exports.createScript("", "");

/*
 * exports.Context
 */
function Context() { return ; }
exports.Context = Context;


/*
 * exports.Script
 */
function Script() { return ; }
exports.Script = Script;

exports.Script.prototype.runInThisContext = function () { return _mixed; };
exports.Script.prototype.runInThisContext();

exports.Script.prototype.runInNewContext = function (sandbox) { return _mixed; };
exports.Script.prototype.runInNewContext(new Object());



/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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



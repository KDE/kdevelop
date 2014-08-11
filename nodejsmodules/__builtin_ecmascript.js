
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
 * exports.Object
 */
function Object() { return ; }
exports.Object = Object;

exports.Object.prototype.assign = function (target, sources) { return new Object(); };
exports.Object.prototype.assign(new Object(), _mixed);

exports.Object.prototype.create = function (O, properties) { return new Object(); };
exports.Object.prototype.create(new Object(), new Object());

exports.Object.prototype.defineProperties = function (O, properties) { return new Object(); };
exports.Object.prototype.defineProperties(new Object(), new Object());

exports.Object.prototype.defineProperty = function (O, property, attributes) { return new Object(); };
exports.Object.prototype.defineProperty(new Object(), "", new Object());

exports.Object.prototype.freeze = function (O) { return new Object(); };
exports.Object.prototype.freeze(new Object());

exports.Object.prototype.getOwnPropertyDescriptor = function (O, property) { return new Object(); };
exports.Object.prototype.getOwnPropertyDescriptor(new Object(), "");

exports.Object.prototype.getOwnPropertyNames = function (O) { return []; };
exports.Object.prototype.getOwnPropertyNames(new Object());

exports.Object.prototype.getOwnPropertySymbols = function (O) { return []; };
exports.Object.prototype.getOwnPropertySymbols(new Object());

exports.Object.prototype.getPrototypeOf = function (O) { return new Object(); };
exports.Object.prototype.getPrototypeOf(new Object());

exports.Object.prototype.is = function (a, b) { return true; };
exports.Object.prototype.is(_mixed, _mixed);

exports.Object.prototype.isExtensible = function (O) { return true; };
exports.Object.prototype.isExtensible(new Object());

exports.Object.prototype.isFrozen = function (O) { return true; };
exports.Object.prototype.isFrozen(new Object());

exports.Object.prototype.isSealed = function (O) { return true; };
exports.Object.prototype.isSealed(new Object());

exports.Object.prototype.keys = function (O) { return {next: function() {}, done: true, value: _mixed}; };
exports.Object.prototype.keys(new Object());

exports.Object.prototype.preventExtensions = function (O) { return new Object(); };
exports.Object.prototype.preventExtensions(new Object());

exports.Object.prototype.seal = function (O) { return new Object(); };
exports.Object.prototype.seal(new Object());

exports.Object.prototype.setPrototypeOf = function (O, proto) { return new Object(); };
exports.Object.prototype.setPrototypeOf(new Object(), new Object());

exports.Object.prototype.constructor = function(){};

exports.Object.prototype.hasOwnProperty = function (property) { return new Object(); };
exports.Object.prototype.hasOwnProperty("");

exports.Object.prototype.isPrototypeOf = function (object) { return true; };
exports.Object.prototype.isPrototypeOf(new Object());

exports.Object.prototype.propertyIsEnumerable = function (property) { return true; };
exports.Object.prototype.propertyIsEnumerable("");

exports.Object.prototype.toLocaleString = function () { return ""; };
exports.Object.prototype.toLocaleString();

exports.Object.prototype.toString = function () { return ""; };
exports.Object.prototype.toString();


/*
 * exports.Function
 */
function Function() { return ; }
exports.Function = Function;

exports.Function.prototype.length = 1;

exports.Function.prototype.apply = function (thisArg, arguments) { return _mixed; };
exports.Function.prototype.apply(new Object(), []);

exports.Function.prototype.bind = function (thisArg, arguments) { return function(){}; };
exports.Function.prototype.bind(new Object(), _mixed);

exports.Function.prototype.call = function (thisArg, arguments) { return _mixed; };
exports.Function.prototype.call(new Object(), _mixed);

exports.Function.prototype.toMethod = function (newHome, methodName) { return function(){}; };
exports.Function.prototype.toMethod(new Object(), "");

exports.Function.prototype.name = "";


/*
 * exports.Boolean
 */
function Boolean(value) { return ; }
exports.Boolean = Boolean;

exports.Boolean.prototype.valueOf = function () { return true; };
exports.Boolean.prototype.valueOf();


/*
 * exports.Error
 */
function Error(message) { return ; }
exports.Error = Error;

exports.Error.prototype.message = "";

exports.Error.prototype.name = "";


/*
 * exports.EvalError
 */
function EvalError() { return ; }
exports.EvalError = EvalError;
exports.EvalError.prototype = Error;


/*
 * exports.RangeError
 */
function RangeError() { return ; }
exports.RangeError = RangeError;
exports.RangeError.prototype = Error;


/*
 * exports.ReferenceError
 */
function ReferenceError() { return ; }
exports.ReferenceError = ReferenceError;
exports.ReferenceError.prototype = Error;


/*
 * exports.SyntaxError
 */
function SyntaxError() { return ; }
exports.SyntaxError = SyntaxError;
exports.SyntaxError.prototype = Error;


/*
 * exports.TypeError
 */
function TypeError() { return ; }
exports.TypeError = TypeError;
exports.TypeError.prototype = Error;


/*
 * exports.URIError
 */
function URIError() { return ; }
exports.URIError = URIError;
exports.URIError.prototype = Error;


/*
 * exports.Number
 */
function Number(value) { return ; }
exports.Number = Number;

exports.Number.prototype.EPSILON = 1.0;

exports.Number.prototype.isFinite = function (number) { return true; };
exports.Number.prototype.isFinite(1.0);

exports.Number.prototype.isInteger = function (number) { return true; };
exports.Number.prototype.isInteger(1);

exports.Number.prototype.isNaN = function (number) { return true; };
exports.Number.prototype.isNaN(1.0);

exports.Number.prototype.isSafeInteger = function (number) { return true; };
exports.Number.prototype.isSafeInteger(1);

exports.Number.prototype.MAX_SAFE_INTEGER = 1;

exports.Number.prototype.MAX_VALUE = 1.0;

exports.Number.prototype.NaN = 1.0;

exports.Number.prototype.NEGATIVE_INFINITY = 1.0;

exports.Number.prototype.MIN_SAFE_INTEGER = 1;

exports.Number.prototype.MIN_VALUE = 1.0;

exports.Number.prototype.parseFloat = function (string) { return 1.0; };
exports.Number.prototype.parseFloat("");

exports.Number.prototype.parseInt = function (string, radix) { return 1; };
exports.Number.prototype.parseInt("", 1);

exports.Number.prototype.POSITIVE_INFINITY = 1.0;

exports.Number.prototype.toExponential = function (fractionDigits) { return ""; };
exports.Number.prototype.toExponential(1);

exports.Number.prototype.toFixed = function (fractionDigits) { return ""; };
exports.Number.prototype.toFixed(1);

exports.Number.prototype.toPrecision = function (precision) { return ""; };
exports.Number.prototype.toPrecision(1);

exports.Number.prototype.valueOf = function () { return 1.0; };
exports.Number.prototype.valueOf();


/*
 * exports.Math
 */
exports.Math = {};

exports.Math.E = 1.0;

exports.Math.LN10 = 1.0;

exports.Math.LOG10E = 1.0;

exports.Math.LN2 = 1.0;

exports.Math.PI = 1.0;

exports.Math.SQRT1_2 = 1.0;

exports.Math.SQRT2 = 1.0;

exports.Math.abs = function (x) { return 1.0; };
exports.Math.abs(1.0);

exports.Math.acos = function (x) { return 1.0; };
exports.Math.acos(1.0);

exports.Math.acosh = function (x) { return 1.0; };
exports.Math.acosh(1.0);

exports.Math.asin = function (x) { return 1.0; };
exports.Math.asin(1.0);

exports.Math.asinh = function (x) { return 1.0; };
exports.Math.asinh(1.0);

exports.Math.atan = function (x) { return 1.0; };
exports.Math.atan(1.0);

exports.Math.atanh = function (x) { return 1.0; };
exports.Math.atanh(1.0);

exports.Math.atan2 = function (x, y) { return 1.0; };
exports.Math.atan2(1.0, 1.0);

exports.Math.cbrt = function (x) { return 1.0; };
exports.Math.cbrt(1.0);

exports.Math.ceil = function (x) { return 1; };
exports.Math.ceil(1.0);

exports.Math.clz32 = function (x) { return 1; };
exports.Math.clz32(1);

exports.Math.cos = function (x) { return 1.0; };
exports.Math.cos(1.0);

exports.Math.cosh = function (x) { return 1.0; };
exports.Math.cosh(1.0);

exports.Math.exp = function (x) { return 1.0; };
exports.Math.exp(1.0);

exports.Math.expm1 = function (x) { return 1.0; };
exports.Math.expm1(1.0);

exports.Math.floor = function (x) { return 1; };
exports.Math.floor(1.0);

exports.Math.fround = function (x) { return 1; };
exports.Math.fround(1.0);

exports.Math.hypot = function (a, b) { return 1.0; };
exports.Math.hypot(1.0, 1.0);

exports.Math.imul = function (x, y) { return 1; };
exports.Math.imul(1, 1);

exports.Math.log = function (x) { return 1.0; };
exports.Math.log(1.0);

exports.Math.log1p = function (x) { return 1.0; };
exports.Math.log1p(1.0);

exports.Math.log10 = function (x) { return 1.0; };
exports.Math.log10(1.0);

exports.Math.log2 = function (x) { return 1.0; };
exports.Math.log2(1.0);

exports.Math.max = function (a, b) { return 1.0; };
exports.Math.max(1.0, 1.0);

exports.Math.min = function (a, b) { return 1.0; };
exports.Math.min(1.0, 1.0);

exports.Math.pow = function (x, y) { return 1.0; };
exports.Math.pow(1.0, 1.0);

exports.Math.random = function () { return 1.0; };
exports.Math.random();

exports.Math.round = function (x) { return 1; };
exports.Math.round(1.0);

exports.Math.sign = function (x) { return 1; };
exports.Math.sign(1.0);

exports.Math.sin = function (x) { return 1.0; };
exports.Math.sin(1.0);

exports.Math.sinh = function (x) { return 1.0; };
exports.Math.sinh(1.0);

exports.Math.sqrt = function (x) { return 1.0; };
exports.Math.sqrt(1.0);

exports.Math.tan = function (x) { return 1.0; };
exports.Math.tan(1.0);

exports.Math.tanh = function (x) { return 1.0; };
exports.Math.tanh(1.0);

exports.Math.trunc = function (x) { return 1; };
exports.Math.trunc(1.0);


/*
 * exports.Date
 */
function Date(year, month, date, hours, minutes, seconds, ms) { return ; }
exports.Date = Date;

exports.Date.prototype.now = function () { return new Date(); };
exports.Date.prototype.now();

exports.Date.prototype.parse = function (string) { return new Date(); };
exports.Date.prototype.parse("");

exports.Date.prototype.UTC = function (year, month, date, hours, minutes, seconds, ms) { return new Date(); };
exports.Date.prototype.UTC(1, 1, 1, 1, 1, 1, 1);

exports.Date.prototype.getDate = function () { return 1; };
exports.Date.prototype.getDate();

exports.Date.prototype.getDay = function () { return 1; };
exports.Date.prototype.getDay();

exports.Date.prototype.getFullYear = function () { return 1; };
exports.Date.prototype.getFullYear();

exports.Date.prototype.getHours = function () { return 1; };
exports.Date.prototype.getHours();

exports.Date.prototype.getMilliseconds = function () { return 1; };
exports.Date.prototype.getMilliseconds();

exports.Date.prototype.getMinutes = function () { return 1; };
exports.Date.prototype.getMinutes();

exports.Date.prototype.getMonth = function () { return 1; };
exports.Date.prototype.getMonth();

exports.Date.prototype.getSeconds = function () { return 1; };
exports.Date.prototype.getSeconds();

exports.Date.prototype.getTime = function () { return 1; };
exports.Date.prototype.getTime();

exports.Date.prototype.getTimezoneOffset = function () { return 1; };
exports.Date.prototype.getTimezoneOffset();

exports.Date.prototype.getUTCDate = function () { return 1; };
exports.Date.prototype.getUTCDate();

exports.Date.prototype.getUTCDay = function () { return 1; };
exports.Date.prototype.getUTCDay();

exports.Date.prototype.getUTCFullYear = function () { return 1; };
exports.Date.prototype.getUTCFullYear();

exports.Date.prototype.getUTCHours = function () { return 1; };
exports.Date.prototype.getUTCHours();

exports.Date.prototype.getUTCMilliseconds = function () { return 1; };
exports.Date.prototype.getUTCMilliseconds();

exports.Date.prototype.getUTCMinutes = function () { return 1; };
exports.Date.prototype.getUTCMinutes();

exports.Date.prototype.getUTCMonth = function () { return 1; };
exports.Date.prototype.getUTCMonth();

exports.Date.prototype.getUTCSeconds = function () { return 1; };
exports.Date.prototype.getUTCSeconds();

exports.Date.prototype.setDate = function (date) { return 1; };
exports.Date.prototype.setDate(1);

exports.Date.prototype.setFullYear = function (year, month, date) { return 1; };
exports.Date.prototype.setFullYear(1, 1, 1);

exports.Date.prototype.setHours = function (hours, minutes, seconds, ms) { return 1; };
exports.Date.prototype.setHours(1, 1, 1, 1);

exports.Date.prototype.setMilliseconds = function (ms) { return 1; };
exports.Date.prototype.setMilliseconds(1);

exports.Date.prototype.setMinutes = function (min, sec, ms) { return 1; };
exports.Date.prototype.setMinutes(1, 1, 1);

exports.Date.prototype.setMonth = function (month, date) { return 1; };
exports.Date.prototype.setMonth(1, 1);

exports.Date.prototype.setSeconds = function (sec, ms) { return 1; };
exports.Date.prototype.setSeconds(1, 1);

exports.Date.prototype.setTime = function (time) { return 1; };
exports.Date.prototype.setTime(1);

exports.Date.prototype.setUTCDate = function (date) { return 1; };
exports.Date.prototype.setUTCDate(1);

exports.Date.prototype.setUTCFullYear = function (year, month, date) { return 1; };
exports.Date.prototype.setUTCFullYear(1, 1, 1);

exports.Date.prototype.setUTCHours = function (hours, minutes, seconds, ms) { return 1; };
exports.Date.prototype.setUTCHours(1, 1, 1, 1);

exports.Date.prototype.setUTCMilliseconds = function (ms) { return 1; };
exports.Date.prototype.setUTCMilliseconds(1);

exports.Date.prototype.setUTCMinutes = function (min, sec, ms) { return 1; };
exports.Date.prototype.setUTCMinutes(1, 1, 1);

exports.Date.prototype.setUTCMonth = function (month, date) { return 1; };
exports.Date.prototype.setUTCMonth(1, 1);

exports.Date.prototype.setUTCSeconds = function (sec, ms) { return 1; };
exports.Date.prototype.setUTCSeconds(1, 1);

exports.Date.prototype.toDateString = function () { return ""; };
exports.Date.prototype.toDateString();

exports.Date.prototype.toISOString = function () { return ""; };
exports.Date.prototype.toISOString();

exports.Date.prototype.toJSON = function (key) { return ""; };
exports.Date.prototype.toJSON("");

exports.Date.prototype.toLocaleDateString = function () { return ""; };
exports.Date.prototype.toLocaleDateString();

exports.Date.prototype.toLocaleTimeString = function () { return ""; };
exports.Date.prototype.toLocaleTimeString();

exports.Date.prototype.toTimeString = function () { return ""; };
exports.Date.prototype.toTimeString();

exports.Date.prototype.toUTCString = function () { return ""; };
exports.Date.prototype.toUTCString();

exports.Date.prototype.valueOf = function () { return 1; };
exports.Date.prototype.valueOf();


/*
 * exports.String
 */
function String(value) { return ; }
exports.String = String;

exports.String.prototype.fromCharCode = function (code) { return ""; };
exports.String.prototype.fromCharCode(1);

exports.String.prototype.fromCodePoint = function (code) { return ""; };
exports.String.prototype.fromCodePoint(1);

exports.String.prototype.raw = function (callSize) { return ""; };
exports.String.prototype.raw(_mixed);

exports.String.prototype.charAt = function (pos) { return ""; };
exports.String.prototype.charAt(1);

exports.String.prototype.charCodeAt = function (pos) { return 1; };
exports.String.prototype.charCodeAt(1);

exports.String.prototype.codePointAt = function (pos) { return 1; };
exports.String.prototype.codePointAt(1);

exports.String.prototype.concat = function (other) { return ""; };
exports.String.prototype.concat("");

exports.String.prototype.contains = function (searchString, position) { return true; };
exports.String.prototype.contains("", 1);

exports.String.prototype.endsWith = function (searchString, endPosition) { return true; };
exports.String.prototype.endsWith("", 1);

exports.String.prototype.indexOf = function (searchString, position) { return 1; };
exports.String.prototype.indexOf("", 1);

exports.String.prototype.lastIndexOf = function (searchString, position) { return 1; };
exports.String.prototype.lastIndexOf("", 1);

exports.String.prototype.localeCompare = function (other) { return 1; };
exports.String.prototype.localeCompare("");

exports.String.prototype.match = function (regexp) { return {index: 1, input: "", length: 1}; };
exports.String.prototype.match(new RegExp());

exports.String.prototype.normalize = function (form) { return ""; };
exports.String.prototype.normalize("");

exports.String.prototype.repeat = function (count) { return ""; };
exports.String.prototype.repeat(1);

exports.String.prototype.replace = function (searchValue, replaceValue) { return ""; };
exports.String.prototype.replace("", "");

exports.String.prototype.search = function (regexp) { return 1; };
exports.String.prototype.search(new RegExp());

exports.String.prototype.slice = function (start, end) { return ""; };
exports.String.prototype.slice(1, 1);

exports.String.prototype.split = function (separator, limit) { return []; };
exports.String.prototype.split("", 1);

exports.String.prototype.startsWith = function (searchString, position) { return true; };
exports.String.prototype.startsWith("", 1);

exports.String.prototype.substring = function (start, end) { return ""; };
exports.String.prototype.substring(1, 1);

exports.String.prototype.toLocaleLowerCase = function () { return ""; };
exports.String.prototype.toLocaleLowerCase();

exports.String.prototype.toLocaleUpperCase = function () { return ""; };
exports.String.prototype.toLocaleUpperCase();

exports.String.prototype.toLowerCase = function () { return ""; };
exports.String.prototype.toLowerCase();

exports.String.prototype.toUpperCase = function () { return ""; };
exports.String.prototype.toUpperCase();

exports.String.prototype.trim = function () { return ""; };
exports.String.prototype.trim();

exports.String.prototype.valueOf = function () { return ""; };
exports.String.prototype.valueOf();

exports.String.prototype.length = 1;


/*
 * exports.RegExp
 */
function RegExp(pattern, flags) { return ; }
exports.RegExp = RegExp;

exports.RegExp.prototype.compile = function () { return ; };
exports.RegExp.prototype.compile();

exports.RegExp.prototype.exec = function (string) { return []; };
exports.RegExp.prototype.exec("");

exports.RegExp.prototype.ingoreCase = true;

exports.RegExp.prototype.match = function (string) { return {index: 1, input: "", length: 1}; };
exports.RegExp.prototype.match("");

exports.RegExp.prototype.multiline = true;

exports.RegExp.prototype.replace = function (string, replaceValue) { return ""; };
exports.RegExp.prototype.replace("", "");

exports.RegExp.prototype.search = function (string) { return 1; };
exports.RegExp.prototype.search("");

exports.RegExp.prototype.source = "";

exports.RegExp.prototype.split = function (string, limit) { return []; };
exports.RegExp.prototype.split("", 1);

exports.RegExp.prototype.sticky = true;

exports.RegExp.prototype.test = function (string) { return true; };
exports.RegExp.prototype.test("");

exports.RegExp.prototype.unicode = true;

exports.RegExp.prototype.lastIndex = 1;

exports.RegExp.prototype.lastMatch = "";


/*
 * exports.Array
 */
function Array(len) { return ; }
exports.Array = Array;

exports.Array.prototype.from = function (arrayLike, mapfn, thisArg) { return []; };
exports.Array.prototype.from(_mixed, function(){}, new Object());

exports.Array.prototype.isArray = function (arg) { return true; };
exports.Array.prototype.isArray(_mixed);

exports.Array.prototype.of = function (items) { return []; };
exports.Array.prototype.of(_mixed);

exports.Array.prototype.concat = function (other) { return []; };
exports.Array.prototype.concat([]);

exports.Array.prototype.copyWithin = function (target, start, end) { return []; };
exports.Array.prototype.copyWithin(1, 1, 1);

exports.Array.prototype.entries = function () { return {next: function() {}, done: true, value: _mixed}; };
exports.Array.prototype.entries();

exports.Array.prototype.every = function (callbackfn, thisArg) { return true; };
exports.Array.prototype.every(function(){}, new Object());

exports.Array.prototype.fill = function (value, start, end) { return []; };
exports.Array.prototype.fill(_mixed, 1, 1);

exports.Array.prototype.filter = function (callbackfn, thisArg) { return []; };
exports.Array.prototype.filter(function(){}, new Object());

exports.Array.prototype.find = function (predicate, thisArg) { return _mixed; };
exports.Array.prototype.find(function(){}, new Object());

exports.Array.prototype.findIndex = function (predicate, thisArg) { return 1; };
exports.Array.prototype.findIndex(function(){}, new Object());

exports.Array.prototype.forEach = function (callbackfn, thisArg) { return ; };
exports.Array.prototype.forEach(function(){}, new Object());

exports.Array.prototype.indexOf = function (searchElement, fromIndex) { return 1; };
exports.Array.prototype.indexOf(_mixed, 1);

exports.Array.prototype.join = function (separator) { return ""; };
exports.Array.prototype.join("");

exports.Array.prototype.lastIndexOf = function (searchElement, fromIndex) { return 1; };
exports.Array.prototype.lastIndexOf(_mixed, 1);

exports.Array.prototype.map = function (callbackfn, thisArg) { return []; };
exports.Array.prototype.map(function(){}, new Object());

exports.Array.prototype.pop = function () { return _mixed; };
exports.Array.prototype.pop();

exports.Array.prototype.push = function (element) { return 1; };
exports.Array.prototype.push(_mixed);

exports.Array.prototype.reduce = function (callbackfn, initialValue) { return _mixed; };
exports.Array.prototype.reduce(function(){}, _mixed);

exports.Array.prototype.reduceRight = function (callbackfn, initialValue) { return _mixed; };
exports.Array.prototype.reduceRight(function(){}, _mixed);

exports.Array.prototype.reverse = function () { return []; };
exports.Array.prototype.reverse();

exports.Array.prototype.shift = function () { return _mixed; };
exports.Array.prototype.shift();

exports.Array.prototype.slice = function (start, end) { return []; };
exports.Array.prototype.slice(1, 1);

exports.Array.prototype.some = function (callbackfn, thisArg) { return true; };
exports.Array.prototype.some(function(){}, new Object());

exports.Array.prototype.sort = function (comparefn) { return []; };
exports.Array.prototype.sort(function(){});

exports.Array.prototype.splice = function (start, deleteCount, items) { return []; };
exports.Array.prototype.splice(1, 1, _mixed);

exports.Array.prototype.substr = function (start, length) { return ""; };
exports.Array.prototype.substr(1, 1);

exports.Array.prototype.unshift = function (items) { return 1; };
exports.Array.prototype.unshift(_mixed);

exports.Array.prototype.values = function () { return {next: function() {}, done: true, value: _mixed}; };
exports.Array.prototype.values();

exports.Array.prototype.length = 1;


/*
 * exports.Map
 */
function Map(iterable) { return ; }
exports.Map = Map;

exports.Map.prototype.clear = function () { return ; };
exports.Map.prototype.clear();

exports.Map.prototype.delete = function (key) { return true; };
exports.Map.prototype.delete(_mixed);

exports.Map.prototype.entries = function () { return {next: function() {}, done: true, value: _mixed}; };
exports.Map.prototype.entries();

exports.Map.prototype.forEach = function (callbackfn, thisArg) { return ; };
exports.Map.prototype.forEach(function(){}, new Object());

exports.Map.prototype.get = function (key) { return _mixed; };
exports.Map.prototype.get(_mixed);

exports.Map.prototype.has = function (key) { return true; };
exports.Map.prototype.has(_mixed);

exports.Map.prototype.set = function (key, value) { return new Map(); };
exports.Map.prototype.set(_mixed, _mixed);

exports.Map.prototype.size = 1;

exports.Map.prototype.values = function () { return {next: function() {}, done: true, value: _mixed}; };
exports.Map.prototype.values();


/*
 * exports.Set
 */
function Set(iterable) { return ; }
exports.Set = Set;

exports.Set.prototype.add = function (value) { return new Set(); };
exports.Set.prototype.add(_mixed);

exports.Set.prototype.clear = function () { return ; };
exports.Set.prototype.clear();

exports.Set.prototype.delete = function (value) { return true; };
exports.Set.prototype.delete(_mixed);

exports.Set.prototype.entries = function () { return {next: function() {}, done: true, value: _mixed}; };
exports.Set.prototype.entries();

exports.Set.prototype.forEach = function (callbackfn, thisArg) { return ; };
exports.Set.prototype.forEach(function(){}, new Object());

exports.Set.prototype.has = function (value) { return true; };
exports.Set.prototype.has(_mixed);

exports.Set.prototype.size = 1;

exports.Set.prototype.values = function () { return {next: function() {}, done: true, value: _mixed}; };
exports.Set.prototype.values();


/*
 * exports.WeakMap
 */
function WeakMap(iterable) { return ; }
exports.WeakMap = WeakMap;

exports.WeakMap.prototype.clear = function () { return ; };
exports.WeakMap.prototype.clear();

exports.WeakMap.prototype.delete = function (key) { return true; };
exports.WeakMap.prototype.delete(_mixed);

exports.WeakMap.prototype.get = function (key) { return _mixed; };
exports.WeakMap.prototype.get(_mixed);

exports.WeakMap.prototype.has = function (key) { return true; };
exports.WeakMap.prototype.has(_mixed);

exports.WeakMap.prototype.set = function (key, value) { return new Map(); };
exports.WeakMap.prototype.set(_mixed, _mixed);


/*
 * exports.WeakSet
 */
function WeakSet(iterable) { return ; }
exports.WeakSet = WeakSet;

exports.WeakSet.prototype.add = function (value) { return new Set(); };
exports.WeakSet.prototype.add(_mixed);

exports.WeakSet.prototype.clear = function () { return ; };
exports.WeakSet.prototype.clear();

exports.WeakSet.prototype.delete = function (value) { return true; };
exports.WeakSet.prototype.delete(_mixed);

exports.WeakSet.prototype.has = function (value) { return true; };
exports.WeakSet.prototype.has(_mixed);


/*
 * exports.ArrayBuffer
 */
function ArrayBuffer(length) { return ; }
exports.ArrayBuffer = ArrayBuffer;

exports.ArrayBuffer.prototype.isView = function (arg) { return true; };
exports.ArrayBuffer.prototype.isView(_mixed);

exports.ArrayBuffer.prototype.byteLength = 1;

exports.ArrayBuffer.prototype.slice = function (start, end) { return new ArrayBuffer(); };
exports.ArrayBuffer.prototype.slice(1, 1);


/*
 * exports.DataView
 */
function DataView(buffer, byteOffset, byteLength) { return ; }
exports.DataView = DataView;

exports.DataView.prototype.buffer = new DataView();

exports.DataView.prototype.byteLength = 1;

exports.DataView.prototype.byteOffset = 1;

exports.DataView.prototype.getFloat32 = function (byteOffset, littleEndian) { return 1.0; };
exports.DataView.prototype.getFloat32(1, true);

exports.DataView.prototype.getFloat64 = function (byteOffset, littleEndian) { return 1.0; };
exports.DataView.prototype.getFloat64(1, true);

exports.DataView.prototype.getInt8 = function (byteOffset) { return 1; };
exports.DataView.prototype.getInt8(1);

exports.DataView.prototype.getInt16 = function (byteOffset, littleEndian) { return 1; };
exports.DataView.prototype.getInt16(1, true);

exports.DataView.prototype.getInt32 = function (byteOffset, littleEndian) { return 1; };
exports.DataView.prototype.getInt32(1, true);

exports.DataView.prototype.getUInt8 = function (byteOffset) { return 1; };
exports.DataView.prototype.getUInt8(1);

exports.DataView.prototype.getUInt16 = function (byteOffset, littleEndian) { return 1; };
exports.DataView.prototype.getUInt16(1, true);

exports.DataView.prototype.getUInt32 = function (byteOffset, littleEndian) { return 1; };
exports.DataView.prototype.getUInt32(1, true);

exports.DataView.prototype.setFloat32 = function (byteOffset, value, littleEndian) { return ; };
exports.DataView.prototype.setFloat32(1, 1.0, true);

exports.DataView.prototype.setFloat64 = function (byteOffset, value, littleEndian) { return ; };
exports.DataView.prototype.setFloat64(1, 1.0, true);

exports.DataView.prototype.setInt8 = function (byteOffset, value) { return ; };
exports.DataView.prototype.setInt8(1, 1);

exports.DataView.prototype.setInt16 = function (byteOffset, value, littleEndian) { return ; };
exports.DataView.prototype.setInt16(1, 1, true);

exports.DataView.prototype.setInt32 = function (byteOffset, value, littleEndian) { return ; };
exports.DataView.prototype.setInt32(1, 1, true);

exports.DataView.prototype.setUInt8 = function (byteOffset, value) { return ; };
exports.DataView.prototype.setUInt8(1, 1);

exports.DataView.prototype.setUInt16 = function (byteOffset, value, littleEndian) { return ; };
exports.DataView.prototype.setUInt16(1, 1, true);

exports.DataView.prototype.setUInt32 = function (byteOffset, value, littleEndian) { return ; };
exports.DataView.prototype.setUInt32(1, 1, true);


/*
 * exports.JSON
 */
exports.JSON = {};

exports.JSON.parse = function (text, reviver) { return _mixed; };
exports.JSON.parse("", function(){});

exports.JSON.stringify = function (value, replacer, space) { return ""; };
exports.JSON.stringify(_mixed, function(){}, "");


/*
 * exports.Promise
 */
function Promise(executor) { return ; }
exports.Promise = Promise;

exports.Promise.prototype.all = function (iterable) { return ; };
exports.Promise.prototype.all([]);

exports.Promise.prototype.race = function (iterable) { return ; };
exports.Promise.prototype.race([]);

exports.Promise.prototype.resolve = function (x) { return new Promise(); };
exports.Promise.prototype.resolve(_mixed);

exports.Promise.prototype.catch = function (onRejected) { return ; };
exports.Promise.prototype.catch(function(){});

exports.Promise.prototype.then = function (onFulfilled, onRejected) { return ; };
exports.Promise.prototype.then(function(){}, function(){});


/*
 * exports.Reflect
 */
exports.Reflect = {};

exports.Reflect.apply = function (target, thisArgument, argumentList) { return _mixed; };
exports.Reflect.apply(function(){}, new Object(), []);

exports.Reflect.construct = function (target, argumentList) { return new Object(); };
exports.Reflect.construct(function(){}, []);

exports.Reflect.deleteProperty = function (target, propertyKey) { return ; };
exports.Reflect.deleteProperty(new Object(), "");

exports.Reflect.enumerate = function (target) { return {next: function() {}, done: true, value: _mixed}; };
exports.Reflect.enumerate(new Object());

exports.Reflect.get = function (target, propertyKey, receiver) { return _mixed; };
exports.Reflect.get(new Object(), "", new Object());

exports.Reflect.has = function (target, propertyKey) { return true; };
exports.Reflect.has(new Object(), "");

exports.Reflect.ownKeys = function (target) { return []; };
exports.Reflect.ownKeys(new Object());

exports.Reflect.set = function (target, propertyKey, value, receiver) { return ; };
exports.Reflect.set(new Object(), "", _mixed, new Object());


/*
 * exports.Int8Array
 */
function Int8Array(length) { return ; }
exports.Int8Array = Int8Array;
exports.Int8Array.prototype = Array;

exports.Int8Array.prototype.buffer = _mixed;

exports.Int8Array.prototype.byteLength = 1;

exports.Int8Array.prototype.byteOffset = 1;

exports.Int8Array.prototype.subarray = function (begin, end) { return []; };
exports.Int8Array.prototype.subarray(1, 1);

exports.Int8Array.prototype.BYTES_PER_ELEMENT = 1;


/*
 * exports.Uint8Array
 */
function Uint8Array(length) { return ; }
exports.Uint8Array = Uint8Array;
exports.Uint8Array.prototype = Array;

exports.Uint8Array.prototype.buffer = _mixed;

exports.Uint8Array.prototype.byteLength = 1;

exports.Uint8Array.prototype.byteOffset = 1;

exports.Uint8Array.prototype.subarray = function (begin, end) { return []; };
exports.Uint8Array.prototype.subarray(1, 1);

exports.Uint8Array.prototype.BYTES_PER_ELEMENT = 1;


/*
 * exports.Uint8ClampedArray
 */
function Uint8ClampedArray(length) { return ; }
exports.Uint8ClampedArray = Uint8ClampedArray;
exports.Uint8ClampedArray.prototype = Array;

exports.Uint8ClampedArray.prototype.buffer = _mixed;

exports.Uint8ClampedArray.prototype.byteLength = 1;

exports.Uint8ClampedArray.prototype.byteOffset = 1;

exports.Uint8ClampedArray.prototype.subarray = function (begin, end) { return []; };
exports.Uint8ClampedArray.prototype.subarray(1, 1);

exports.Uint8ClampedArray.prototype.BYTES_PER_ELEMENT = 1;


/*
 * exports.Int16Array
 */
function Int16Array(length) { return ; }
exports.Int16Array = Int16Array;
exports.Int16Array.prototype = Array;

exports.Int16Array.prototype.buffer = _mixed;

exports.Int16Array.prototype.byteLength = 1;

exports.Int16Array.prototype.byteOffset = 1;

exports.Int16Array.prototype.subarray = function (begin, end) { return []; };
exports.Int16Array.prototype.subarray(1, 1);

exports.Int16Array.prototype.BYTES_PER_ELEMENT = 1;


/*
 * exports.Uint16Array
 */
function Uint16Array(length) { return ; }
exports.Uint16Array = Uint16Array;
exports.Uint16Array.prototype = Array;

exports.Uint16Array.prototype.buffer = _mixed;

exports.Uint16Array.prototype.byteLength = 1;

exports.Uint16Array.prototype.byteOffset = 1;

exports.Uint16Array.prototype.subarray = function (begin, end) { return []; };
exports.Uint16Array.prototype.subarray(1, 1);

exports.Uint16Array.prototype.BYTES_PER_ELEMENT = 1;


/*
 * exports.Int32Array
 */
function Int32Array(length) { return ; }
exports.Int32Array = Int32Array;
exports.Int32Array.prototype = Array;

exports.Int32Array.prototype.buffer = _mixed;

exports.Int32Array.prototype.byteLength = 1;

exports.Int32Array.prototype.byteOffset = 1;

exports.Int32Array.prototype.subarray = function (begin, end) { return []; };
exports.Int32Array.prototype.subarray(1, 1);

exports.Int32Array.prototype.BYTES_PER_ELEMENT = 1;


/*
 * exports.Uint32Array
 */
function Uint32Array(length) { return ; }
exports.Uint32Array = Uint32Array;
exports.Uint32Array.prototype = Array;

exports.Uint32Array.prototype.buffer = _mixed;

exports.Uint32Array.prototype.byteLength = 1;

exports.Uint32Array.prototype.byteOffset = 1;

exports.Uint32Array.prototype.subarray = function (begin, end) { return []; };
exports.Uint32Array.prototype.subarray(1, 1);

exports.Uint32Array.prototype.BYTES_PER_ELEMENT = 1;


/*
 * exports.Float32Array
 */
function Float32Array(length) { return ; }
exports.Float32Array = Float32Array;
exports.Float32Array.prototype = Array;

exports.Float32Array.prototype.buffer = _mixed;

exports.Float32Array.prototype.byteLength = 1;

exports.Float32Array.prototype.byteOffset = 1;

exports.Float32Array.prototype.subarray = function (begin, end) { return []; };
exports.Float32Array.prototype.subarray(1, 1);

exports.Float32Array.prototype.BYTES_PER_ELEMENT = 1;


/*
 * exports.Float64Array
 */
function Float64Array(length) { return ; }
exports.Float64Array = Float64Array;
exports.Float64Array.prototype = Array;

exports.Float64Array.prototype.buffer = _mixed;

exports.Float64Array.prototype.byteLength = 1;

exports.Float64Array.prototype.byteOffset = 1;

exports.Float64Array.prototype.subarray = function (begin, end) { return []; };
exports.Float64Array.prototype.subarray(1, 1);

exports.Float64Array.prototype.BYTES_PER_ELEMENT = 1;



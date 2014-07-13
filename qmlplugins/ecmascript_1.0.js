
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
 * Object
 */
function Object() { return ; }

Object.prototype.assign = function (target, sources) { return new Object(); };
Object.prototype.assign(new Object(), _mixed);

Object.prototype.create = function (O, properties) { return new Object(); };
Object.prototype.create(new Object(), new Object());

Object.prototype.defineProperties = function (O, properties) { return new Object(); };
Object.prototype.defineProperties(new Object(), new Object());

Object.prototype.defineProperty = function (O, property, attributes) { return new Object(); };
Object.prototype.defineProperty(new Object(), "", new Object());

Object.prototype.freeze = function (O) { return new Object(); };
Object.prototype.freeze(new Object());

Object.prototype.getOwnPropertyDescriptor = function (O, property) { return new Object(); };
Object.prototype.getOwnPropertyDescriptor(new Object(), "");

Object.prototype.getOwnPropertyNames = function (O) { return []; };
Object.prototype.getOwnPropertyNames(new Object());

Object.prototype.getOwnPropertySymbols = function (O) { return []; };
Object.prototype.getOwnPropertySymbols(new Object());

Object.prototype.getPrototypeOf = function (O) { return new Object(); };
Object.prototype.getPrototypeOf(new Object());

Object.prototype.is = function (a, b) { return true; };
Object.prototype.is(_mixed, _mixed);

Object.prototype.isExtensible = function (O) { return true; };
Object.prototype.isExtensible(new Object());

Object.prototype.isFrozen = function (O) { return true; };
Object.prototype.isFrozen(new Object());

Object.prototype.isSealed = function (O) { return true; };
Object.prototype.isSealed(new Object());

Object.prototype.keys = function (O) { return []; };
Object.prototype.keys(new Object());

Object.prototype.preventExtensions = function (O) { return new Object(); };
Object.prototype.preventExtensions(new Object());

Object.prototype.seal = function (O) { return new Object(); };
Object.prototype.seal(new Object());

Object.prototype.setPrototypeOf = function (O, proto) { return new Object(); };
Object.prototype.setPrototypeOf(new Object(), new Object());

Object.prototype.constructor = function(){};

Object.prototype.hasOwnProperty = function (property) { return new Object(); };
Object.prototype.hasOwnProperty("");

Object.prototype.isPrototypeOf = function (object) { return true; };
Object.prototype.isPrototypeOf(new Object());

Object.prototype.propertyIsEnumerable = function (property) { return true; };
Object.prototype.propertyIsEnumerable("");

Object.prototype.toLocaleString = function () { return ""; };
Object.prototype.toLocaleString();

Object.prototype.toString = function () { return ""; };
Object.prototype.toString();

Object.prototype.valueOf = function () { return new Object(); };
Object.prototype.valueOf();

/*
 * Function
 */
function Function() { return ; }

Function.prototype.length = 1;

Function.prototype.apply = function (thisArg, arguments) { return _mixed; };
Function.prototype.apply(new Object(), []);

Function.prototype.bind = function (thisArg, arguments) { return function(){}; };
Function.prototype.bind(new Object(), _mixed);

Function.prototype.call = function (thisArg, arguments) { return _mixed; };
Function.prototype.call(new Object(), _mixed);

Function.prototype.constructor = function(){};

Function.prototype.toMethod = function (newHome, methodName) { return function(){}; };
Function.prototype.toMethod(new Object(), "");

Function.prototype.name = "";

/*
 * Boolean
 */
function Boolean(value) { return ; }

Boolean.prototype.valueOf = function () { return true; };
Boolean.prototype.valueOf();

/*
 * Error
 */
function Error(message) { return ; }

Error.prototype.message = "";

Error.prototype.name = "";

/*
 * EvalError
 */
function EvalError() { return ; }

EvalError.prototype = Error


/*
 * RangeError
 */
function RangeError() { return ; }

RangeError.prototype = Error


/*
 * ReferenceError
 */
function ReferenceError() { return ; }

ReferenceError.prototype = Error


/*
 * SyntaxError
 */
function SyntaxError() { return ; }

SyntaxError.prototype = Error


/*
 * TypeError
 */
function TypeError() { return ; }

TypeError.prototype = Error


/*
 * URIError
 */
function URIError() { return ; }

URIError.prototype = Error


/*
 * Math
 */
var Math = {};

Math.E = 1.0;

Math.LN10 = 1.0;

Math.LOG10E = 1.0;

Math.LN2 = 1.0;

Math.PI = 1.0;

Math.SQRT1_2 = 1.0;

Math.SQRT2 = 1.0;

Math.abs = function (x) { return 1.0; };
Math.abs(1.0);

Math.acos = function (x) { return 1.0; };
Math.acos(1.0);

Math.acosh = function (x) { return 1.0; };
Math.acosh(1.0);

Math.asin = function (x) { return 1.0; };
Math.asin(1.0);

Math.asinh = function (x) { return 1.0; };
Math.asinh(1.0);

Math.atan = function (x) { return 1.0; };
Math.atan(1.0);

Math.atanh = function (x) { return 1.0; };
Math.atanh(1.0);

Math.atan2 = function (x, y) { return 1.0; };
Math.atan2(1.0, 1.0);

Math.cbrt = function (x) { return 1.0; };
Math.cbrt(1.0);

Math.ceil = function (x) { return 1; };
Math.ceil(1.0);

Math.clz32 = function (x) { return 1; };
Math.clz32(1);

Math.cos = function (x) { return 1.0; };
Math.cos(1.0);

Math.cosh = function (x) { return 1.0; };
Math.cosh(1.0);

Math.exp = function (x) { return 1.0; };
Math.exp(1.0);

Math.expm1 = function (x) { return 1.0; };
Math.expm1(1.0);

Math.floor = function (x) { return 1; };
Math.floor(1.0);

Math.fround = function (x) { return 1; };
Math.fround(1.0);

Math.hypot = function (a, b) { return 1.0; };
Math.hypot(1.0, 1.0);

Math.imul = function (x, y) { return 1; };
Math.imul(1, 1);

Math.log = function (x) { return 1.0; };
Math.log(1.0);

Math.log1p = function (x) { return 1.0; };
Math.log1p(1.0);

Math.log10 = function (x) { return 1.0; };
Math.log10(1.0);

Math.log2 = function (x) { return 1.0; };
Math.log2(1.0);

Math.max = function (a, b) { return 1.0; };
Math.max(1.0, 1.0);

Math.min = function (a, b) { return 1.0; };
Math.min(1.0, 1.0);

Math.pow = function (x, y) { return 1.0; };
Math.pow(1.0, 1.0);

Math.random = function () { return 1.0; };
Math.random();

Math.round = function (x) { return 1; };
Math.round(1.0);

Math.sign = function (x) { return 1; };
Math.sign(1.0);

Math.sin = function (x) { return 1.0; };
Math.sin(1.0);

Math.sinh = function (x) { return 1.0; };
Math.sinh(1.0);

Math.sqrt = function (x) { return 1.0; };
Math.sqrt(1.0);

Math.tan = function (x) { return 1.0; };
Math.tan(1.0);

Math.tanh = function (x) { return 1.0; };
Math.tanh(1.0);

Math.trunc = function (x) { return 1; };
Math.trunc(1.0);

/*
 * Date
 */
function Date(year, month, date, hours, minutes, seconds, ms) { return ; }

Date.prototype.now = function () { return new Date(); };
Date.prototype.now();

Date.prototype.parse = function (string) { return new Date(); };
Date.prototype.parse("");

Date.prototype.UTC = function (year, month, date, hours, minutes, seconds, ms) { return new Date(); };
Date.prototype.UTC(1, 1, 1, 1, 1, 1, 1);

Date.prototype.getDate = function () { return 1; };
Date.prototype.getDate();

Date.prototype.getDay = function () { return 1; };
Date.prototype.getDay();

Date.prototype.getFullYear = function () { return 1; };
Date.prototype.getFullYear();

Date.prototype.getHours = function () { return 1; };
Date.prototype.getHours();

Date.prototype.getMilliseconds = function () { return 1; };
Date.prototype.getMilliseconds();

Date.prototype.getMinutes = function () { return 1; };
Date.prototype.getMinutes();

Date.prototype.getMonth = function () { return 1; };
Date.prototype.getMonth();

Date.prototype.getSeconds = function () { return 1; };
Date.prototype.getSeconds();

Date.prototype.getTime = function () { return 1; };
Date.prototype.getTime();

Date.prototype.getTimezoneOffset = function () { return 1; };
Date.prototype.getTimezoneOffset();

Date.prototype.getUTCDate = function () { return 1; };
Date.prototype.getUTCDate();

Date.prototype.getUTCDay = function () { return 1; };
Date.prototype.getUTCDay();

Date.prototype.getUTCFullYear = function () { return 1; };
Date.prototype.getUTCFullYear();

Date.prototype.getUTCHours = function () { return 1; };
Date.prototype.getUTCHours();

Date.prototype.getUTCMilliseconds = function () { return 1; };
Date.prototype.getUTCMilliseconds();

Date.prototype.getUTCMinutes = function () { return 1; };
Date.prototype.getUTCMinutes();

Date.prototype.getUTCMonth = function () { return 1; };
Date.prototype.getUTCMonth();

Date.prototype.getUTCSeconds = function () { return 1; };
Date.prototype.getUTCSeconds();

Date.prototype.setDate = function (date) { return 1; };
Date.prototype.setDate(1);

Date.prototype.setFullYear = function (year, month, date) { return 1; };
Date.prototype.setFullYear(1, 1, 1);

Date.prototype.setHours = function (hours, minutes, seconds, ms) { return 1; };
Date.prototype.setHours(1, 1, 1, 1);

Date.prototype.setMilliseconds = function (ms) { return 1; };
Date.prototype.setMilliseconds(1);

Date.prototype.setMinutes = function (min, sec, ms) { return 1; };
Date.prototype.setMinutes(1, 1, 1);

Date.prototype.setMonth = function (month, date) { return 1; };
Date.prototype.setMonth(1, 1);

Date.prototype.setSeconds = function (sec, ms) { return 1; };
Date.prototype.setSeconds(1, 1);

Date.prototype.setTime = function (time) { return 1; };
Date.prototype.setTime(1);

Date.prototype.setUTCDate = function (date) { return 1; };
Date.prototype.setUTCDate(1);

Date.prototype.setUTCFullYear = function (year, month, date) { return 1; };
Date.prototype.setUTCFullYear(1, 1, 1);

Date.prototype.setUTCHours = function (hours, minutes, seconds, ms) { return 1; };
Date.prototype.setUTCHours(1, 1, 1, 1);

Date.prototype.setUTCMilliseconds = function (ms) { return 1; };
Date.prototype.setUTCMilliseconds(1);

Date.prototype.setUTCMinutes = function (min, sec, ms) { return 1; };
Date.prototype.setUTCMinutes(1, 1, 1);

Date.prototype.setUTCMonth = function (month, date) { return 1; };
Date.prototype.setUTCMonth(1, 1);

Date.prototype.setUTCSeconds = function (sec, ms) { return 1; };
Date.prototype.setUTCSeconds(1, 1);

Date.prototype.toDateString = function () { return ""; };
Date.prototype.toDateString();

Date.prototype.toISOString = function () { return ""; };
Date.prototype.toISOString();

Date.prototype.toJSON = function (key) { return ""; };
Date.prototype.toJSON("");

Date.prototype.toLocaleDateString = function () { return ""; };
Date.prototype.toLocaleDateString();

Date.prototype.toLocaleTimeString = function () { return ""; };
Date.prototype.toLocaleTimeString();

Date.prototype.toTimeString = function () { return ""; };
Date.prototype.toTimeString();

Date.prototype.toUTCString = function () { return ""; };
Date.prototype.toUTCString();

Date.prototype.valueOf = function () { return 1; };
Date.prototype.valueOf();

/*
 * String
 */
function String(value) { return ; }

String.prototype.fromCharCode = function (code) { return ""; };
String.prototype.fromCharCode(1);

String.prototype.fromCodePoint = function (code) { return ""; };
String.prototype.fromCodePoint(1);

String.prototype.raw = function (callSize) { return ""; };
String.prototype.raw(_mixed);

String.prototype.charAt = function (pos) { return ""; };
String.prototype.charAt(1);

String.prototype.charCodeAt = function (pos) { return 1; };
String.prototype.charCodeAt(1);

String.prototype.codePointAt = function (pos) { return 1; };
String.prototype.codePointAt(1);

String.prototype.concat = function (other) { return ""; };
String.prototype.concat("");

String.prototype.contains = function (searchString, position) { return true; };
String.prototype.contains("", 1);

String.prototype.endsWith = function (searchString, endPosition) { return true; };
String.prototype.endsWith("", 1);

String.prototype.indexOf = function (searchString, position) { return 1; };
String.prototype.indexOf("", 1);

String.prototype.lastIndexOf = function (searchString, position) { return 1; };
String.prototype.lastIndexOf("", 1);

String.prototype.localeCompare = function (other) { return 1; };
String.prototype.localeCompare("");

String.prototype.match = function (regexp) { return {index: 1, input: "", length: 1}; };
String.prototype.match(new RegExp());

String.prototype.normalize = function (form) { return ""; };
String.prototype.normalize("");

String.prototype.repeat = function (count) { return ""; };
String.prototype.repeat(1);

String.prototype.replace = function (searchValue, replaceValue) { return ""; };
String.prototype.replace("", "");

String.prototype.search = function (regexp) { return 1; };
String.prototype.search(new RegExp());

String.prototype.slice = function (start, end) { return ""; };
String.prototype.slice(1, 1);

String.prototype.split = function (separator, limit) { return []; };
String.prototype.split("", 1);

String.prototype.startsWith = function (searchString, position) { return true; };
String.prototype.startsWith("", 1);

String.prototype.substring = function (start, end) { return ""; };
String.prototype.substring(1, 1);

String.prototype.toLocaleLowerCase = function () { return ""; };
String.prototype.toLocaleLowerCase();

String.prototype.toLocaleUpperCase = function () { return ""; };
String.prototype.toLocaleUpperCase();

String.prototype.toLowerCase = function () { return ""; };
String.prototype.toLowerCase();

String.prototype.toUpperCase = function () { return ""; };
String.prototype.toUpperCase();

String.prototype.trim = function () { return ""; };
String.prototype.trim();

String.prototype.valueOf = function () { return ""; };
String.prototype.valueOf();

String.prototype.length = 1;

/*
 * RegExp
 */
function RegExp(pattern, flags) { return ; }

RegExp.prototype.exec = function (string) { return []; };
RegExp.prototype.exec("");

RegExp.prototype.ingoreCase = true;

RegExp.prototype.match = function (string) { return {index: 1, input: "", length: 1}; };
RegExp.prototype.match("");

RegExp.prototype.multiline = true;

RegExp.prototype.replace = function (string, replaceValue) { return ""; };
RegExp.prototype.replace("", "");

RegExp.prototype.search = function (string) { return 1; };
RegExp.prototype.search("");

RegExp.prototype.source = "";

RegExp.prototype.split = function (string, limit) { return []; };
RegExp.prototype.split("", 1);

RegExp.prototype.sticky = true;

RegExp.prototype.test = function (string) { return true; };
RegExp.prototype.test("");

RegExp.prototype.unicode = true;

RegExp.prototype.lastIndex = 1;

/*
 * Array
 */
function Array(len) { return ; }

Array.prototype.from = function (arrayLike, mapfn, thisArg) { return []; };
Array.prototype.from(_mixed, function(){}, new Object());

Array.prototype.isArray = function (arg) { return true; };
Array.prototype.isArray(_mixed);

Array.prototype.of = function (items) { return []; };
Array.prototype.of(_mixed);

Array.prototype.concat = function (other) { return []; };
Array.prototype.concat([]);

Array.prototype.copyWithin = function (target, start, end) { return []; };
Array.prototype.copyWithin(1, 1, 1);

Array.prototype.entries = function () { return {next: function() {}, done: true, value: _mixed}; };
Array.prototype.entries();

Array.prototype.every = function (callbackfn, thisArg) { return true; };
Array.prototype.every(function(){}, new Object());

Array.prototype.fill = function (value, start, end) { return []; };
Array.prototype.fill(_mixed, 1, 1);

Array.prototype.filter = function (callbackfn, thisArg) { return []; };
Array.prototype.filter(function(){}, new Object());

Array.prototype.find = function (predicate, thisArg) { return _mixed; };
Array.prototype.find(function(){}, new Object());

Array.prototype.findIndex = function (predicate, thisArg) { return 1; };
Array.prototype.findIndex(function(){}, new Object());

Array.prototype.forEach = function (callbackfn, thisArg) { return ; };
Array.prototype.forEach(function(){}, new Object());

Array.prototype.indexOf = function (searchElement, fromIndex) { return 1; };
Array.prototype.indexOf(_mixed, 1);

Array.prototype.join = function (separator) { return ""; };
Array.prototype.join("");

Array.prototype.keys = function () { return {next: function() {}, done: true, value: _mixed}; };
Array.prototype.keys();

Array.prototype.lastIndexOf = function (searchElement, fromIndex) { return 1; };
Array.prototype.lastIndexOf(_mixed, 1);

Array.prototype.map = function (callbackfn, thisArg) { return []; };
Array.prototype.map(function(){}, new Object());

Array.prototype.pop = function () { return _mixed; };
Array.prototype.pop();

Array.prototype.push = function (element) { return 1; };
Array.prototype.push(_mixed);

Array.prototype.reduce = function (callbackfn, initialValue) { return _mixed; };
Array.prototype.reduce(function(){}, _mixed);

Array.prototype.reduceRight = function (callbackfn, initialValue) { return _mixed; };
Array.prototype.reduceRight(function(){}, _mixed);

Array.prototype.reverse = function () { return []; };
Array.prototype.reverse();

Array.prototype.shift = function () { return _mixed; };
Array.prototype.shift();

Array.prototype.slice = function (start, end) { return []; };
Array.prototype.slice(1, 1);

Array.prototype.some = function (callbackfn, thisArg) { return true; };
Array.prototype.some(function(){}, new Object());

Array.prototype.sort = function (comparefn) { return []; };
Array.prototype.sort(function(){});

Array.prototype.splice = function (start, deleteCount, items) { return []; };
Array.prototype.splice(1, 1, _mixed);

Array.prototype.unshift = function (items) { return 1; };
Array.prototype.unshift(_mixed);

Array.prototype.values = function () { return {next: function() {}, done: true, value: _mixed}; };
Array.prototype.values();

Array.prototype.length = 1;

/*
 * Int8Array
 */
function Int8Array(length) { return ; }

Int8Array.prototype.buffer = _mixed;

Int8Array.prototype.byteLength = 1;

Int8Array.prototype.byteOffset = 1;

Int8Array.prototype.subarray = function (begin, end) { return []; };
Int8Array.prototype.subarray(1, 1);

Int8Array.prototype.BYTES_PER_ELEMENT = 1;

Int8Array.prototype = Array


/*
 * Uint8Array
 */
function Uint8Array(length) { return ; }

Uint8Array.prototype.buffer = _mixed;

Uint8Array.prototype.byteLength = 1;

Uint8Array.prototype.byteOffset = 1;

Uint8Array.prototype.subarray = function (begin, end) { return []; };
Uint8Array.prototype.subarray(1, 1);

Uint8Array.prototype.BYTES_PER_ELEMENT = 1;

Uint8Array.prototype = Array


/*
 * Uint8ClampedArray
 */
function Uint8ClampedArray(length) { return ; }

Uint8ClampedArray.prototype.buffer = _mixed;

Uint8ClampedArray.prototype.byteLength = 1;

Uint8ClampedArray.prototype.byteOffset = 1;

Uint8ClampedArray.prototype.subarray = function (begin, end) { return []; };
Uint8ClampedArray.prototype.subarray(1, 1);

Uint8ClampedArray.prototype.BYTES_PER_ELEMENT = 1;

Uint8ClampedArray.prototype = Array


/*
 * Int16Array
 */
function Int16Array(length) { return ; }

Int16Array.prototype.buffer = _mixed;

Int16Array.prototype.byteLength = 1;

Int16Array.prototype.byteOffset = 1;

Int16Array.prototype.subarray = function (begin, end) { return []; };
Int16Array.prototype.subarray(1, 1);

Int16Array.prototype.BYTES_PER_ELEMENT = 1;

Int16Array.prototype = Array


/*
 * Uint16Array
 */
function Uint16Array(length) { return ; }

Uint16Array.prototype.buffer = _mixed;

Uint16Array.prototype.byteLength = 1;

Uint16Array.prototype.byteOffset = 1;

Uint16Array.prototype.subarray = function (begin, end) { return []; };
Uint16Array.prototype.subarray(1, 1);

Uint16Array.prototype.BYTES_PER_ELEMENT = 1;

Uint16Array.prototype = Array


/*
 * Int32Array
 */
function Int32Array(length) { return ; }

Int32Array.prototype.buffer = _mixed;

Int32Array.prototype.byteLength = 1;

Int32Array.prototype.byteOffset = 1;

Int32Array.prototype.subarray = function (begin, end) { return []; };
Int32Array.prototype.subarray(1, 1);

Int32Array.prototype.BYTES_PER_ELEMENT = 1;

Int32Array.prototype = Array


/*
 * Uint32Array
 */
function Uint32Array(length) { return ; }

Uint32Array.prototype.buffer = _mixed;

Uint32Array.prototype.byteLength = 1;

Uint32Array.prototype.byteOffset = 1;

Uint32Array.prototype.subarray = function (begin, end) { return []; };
Uint32Array.prototype.subarray(1, 1);

Uint32Array.prototype.BYTES_PER_ELEMENT = 1;

Uint32Array.prototype = Array


/*
 * Float32Array
 */
function Float32Array(length) { return ; }

Float32Array.prototype.buffer = _mixed;

Float32Array.prototype.byteLength = 1;

Float32Array.prototype.byteOffset = 1;

Float32Array.prototype.subarray = function (begin, end) { return []; };
Float32Array.prototype.subarray(1, 1);

Float32Array.prototype.BYTES_PER_ELEMENT = 1;

Float32Array.prototype = Array


/*
 * Float64Array
 */
function Float64Array(length) { return ; }

Float64Array.prototype.buffer = _mixed;

Float64Array.prototype.byteLength = 1;

Float64Array.prototype.byteOffset = 1;

Float64Array.prototype.subarray = function (begin, end) { return []; };
Float64Array.prototype.subarray(1, 1);

Float64Array.prototype.BYTES_PER_ELEMENT = 1;

Float64Array.prototype = Array


/*
 * Map
 */
function Map(iterable) { return ; }

Map.prototype.clear = function () { return ; };
Map.prototype.clear();

Map.prototype.delete = function (key) { return true; };
Map.prototype.delete(_mixed);

Map.prototype.entries = function () { return {next: function() {}, done: true, value: _mixed}; };
Map.prototype.entries();

Map.prototype.forEach = function (callbackfn, thisArg) { return ; };
Map.prototype.forEach(function(){}, new Object());

Map.prototype.get = function (key) { return _mixed; };
Map.prototype.get(_mixed);

Map.prototype.has = function (key) { return true; };
Map.prototype.has(_mixed);

Map.prototype.keys = function () { return {next: function() {}, done: true, value: _mixed}; };
Map.prototype.keys();

Map.prototype.set = function (key, value) { return new Map(); };
Map.prototype.set(_mixed, _mixed);

Map.prototype.size = 1;

Map.prototype.values = function () { return {next: function() {}, done: true, value: _mixed}; };
Map.prototype.values();

/*
 * Set
 */
function Set(iterable) { return ; }

Set.prototype.add = function (value) { return new Set(); };
Set.prototype.add(_mixed);

Set.prototype.clear = function () { return ; };
Set.prototype.clear();

Set.prototype.delete = function (value) { return true; };
Set.prototype.delete(_mixed);

Set.prototype.entries = function () { return {next: function() {}, done: true, value: _mixed}; };
Set.prototype.entries();

Set.prototype.forEach = function (callbackfn, thisArg) { return ; };
Set.prototype.forEach(function(){}, new Object());

Set.prototype.has = function (value) { return true; };
Set.prototype.has(_mixed);

Set.prototype.keys = function () { return {next: function() {}, done: true, value: _mixed}; };
Set.prototype.keys();

Set.prototype.size = 1;

Set.prototype.values = function () { return {next: function() {}, done: true, value: _mixed}; };
Set.prototype.values();

/*
 * WeakMap
 */
function WeakMap(iterable) { return ; }

WeakMap.prototype.clear = function () { return ; };
WeakMap.prototype.clear();

WeakMap.prototype.delete = function (key) { return true; };
WeakMap.prototype.delete(_mixed);

WeakMap.prototype.get = function (key) { return _mixed; };
WeakMap.prototype.get(_mixed);

WeakMap.prototype.has = function (key) { return true; };
WeakMap.prototype.has(_mixed);

WeakMap.prototype.set = function (key, value) { return new Map(); };
WeakMap.prototype.set(_mixed, _mixed);

/*
 * WeakSet
 */
function WeakSet(iterable) { return ; }

WeakSet.prototype.add = function (value) { return new Set(); };
WeakSet.prototype.add(_mixed);

WeakSet.prototype.clear = function () { return ; };
WeakSet.prototype.clear();

WeakSet.prototype.delete = function (value) { return true; };
WeakSet.prototype.delete(_mixed);

WeakSet.prototype.has = function (value) { return true; };
WeakSet.prototype.has(_mixed);

/*
 * ArrayBuffer
 */
function ArrayBuffer(length) { return ; }

ArrayBuffer.prototype.isView = function (arg) { return true; };
ArrayBuffer.prototype.isView(_mixed);

ArrayBuffer.prototype.byteLength = 1;

ArrayBuffer.prototype.slice = function (start, end) { return new ArrayBuffer(); };
ArrayBuffer.prototype.slice(1, 1);

/*
 * DataView
 */
function DataView(buffer, byteOffset, byteLength) { return ; }

DataView.prototype.buffer = new DataView();

DataView.prototype.byteLength = 1;

DataView.prototype.byteOffset = 1;

DataView.prototype.getFloat32 = function (byteOffset, littleEndian) { return 1.0; };
DataView.prototype.getFloat32(1, true);

DataView.prototype.getFloat64 = function (byteOffset, littleEndian) { return 1.0; };
DataView.prototype.getFloat64(1, true);

DataView.prototype.getInt8 = function (byteOffset) { return 1; };
DataView.prototype.getInt8(1);

DataView.prototype.getInt16 = function (byteOffset, littleEndian) { return 1; };
DataView.prototype.getInt16(1, true);

DataView.prototype.getInt32 = function (byteOffset, littleEndian) { return 1; };
DataView.prototype.getInt32(1, true);

DataView.prototype.getUInt8 = function (byteOffset) { return 1; };
DataView.prototype.getUInt8(1);

DataView.prototype.getUInt16 = function (byteOffset, littleEndian) { return 1; };
DataView.prototype.getUInt16(1, true);

DataView.prototype.getUInt32 = function (byteOffset, littleEndian) { return 1; };
DataView.prototype.getUInt32(1, true);

DataView.prototype.setFloat32 = function (byteOffset, value, littleEndian) { return ; };
DataView.prototype.setFloat32(1, 1.0, true);

DataView.prototype.setFloat64 = function (byteOffset, value, littleEndian) { return ; };
DataView.prototype.setFloat64(1, 1.0, true);

DataView.prototype.setInt8 = function (byteOffset, value) { return ; };
DataView.prototype.setInt8(1, 1);

DataView.prototype.setInt16 = function (byteOffset, value, littleEndian) { return ; };
DataView.prototype.setInt16(1, 1, true);

DataView.prototype.setInt32 = function (byteOffset, value, littleEndian) { return ; };
DataView.prototype.setInt32(1, 1, true);

DataView.prototype.setUInt8 = function (byteOffset, value) { return ; };
DataView.prototype.setUInt8(1, 1);

DataView.prototype.setUInt16 = function (byteOffset, value, littleEndian) { return ; };
DataView.prototype.setUInt16(1, 1, true);

DataView.prototype.setUInt32 = function (byteOffset, value, littleEndian) { return ; };
DataView.prototype.setUInt32(1, 1, true);

/*
 * JSON
 */
var JSON = {};

JSON.parse = function (text, reviver) { return _mixed; };
JSON.parse("", function(){});

JSON.stringify = function (value, replacer, space) { return ""; };
JSON.stringify(_mixed, function(){}, "");

/*
 * Promise
 */
function Promise(executor) { return ; }

Promise.prototype.all = function (iterable) { return ; };
Promise.prototype.all([]);

Promise.prototype.race = function (iterable) { return ; };
Promise.prototype.race([]);

Promise.prototype.resolve = function (x) { return new Promise(); };
Promise.prototype.resolve(_mixed);

Promise.prototype.catch = function (onRejected) { return ; };
Promise.prototype.catch(function(){});

Promise.prototype.then = function (onFulfilled, onRejected) { return ; };
Promise.prototype.then(function(){}, function(){});

/*
 * Reflect
 */
var Reflect = {};

Reflect.apply = function (target, thisArgument, argumentList) { return _mixed; };
Reflect.apply(function(){}, new Object(), []);

Reflect.construct = function (target, argumentList) { return new Object(); };
Reflect.construct(function(){}, []);

Reflect.defineProperty = function (target, propertyKey, attributes) { return ; };
Reflect.defineProperty(new Object(), "", new Object());

Reflect.deleteProperty = function (target, propertyKey) { return ; };
Reflect.deleteProperty(new Object(), "");

Reflect.enumerate = function (target) { return {next: function() {}, done: true, value: _mixed}; };
Reflect.enumerate(new Object());

Reflect.get = function (target, propertyKey, receiver) { return _mixed; };
Reflect.get(new Object(), "", new Object());

Reflect.getOwnPropertyDescriptor = function (target, propertyKey) { return new Object(); };
Reflect.getOwnPropertyDescriptor(new Object(), "");

Reflect.getPrototypeOf = function (target) { return new Object(); };
Reflect.getPrototypeOf(new Object());

Reflect.has = function (target, propertyKey) { return true; };
Reflect.has(new Object(), "");

Reflect.isExtensible = function (target) { return true; };
Reflect.isExtensible(new Object());

Reflect.ownKeys = function (target) { return []; };
Reflect.ownKeys(new Object());

Reflect.preventExtensions = function (target) { return ; };
Reflect.preventExtensions(new Object());

Reflect.set = function (target, propertyKey, value, receiver) { return ; };
Reflect.set(new Object(), "", _mixed, new Object());

Reflect.setPrototypeOf = function (target, proto) { return ; };
Reflect.setPrototypeOf(new Object(), new Object());


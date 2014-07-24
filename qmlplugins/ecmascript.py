#!/usr/bin/python3
# -*- coding: utf-8 -*-
# This file is part of qmljs, the QML/JS language support plugin for KDevelop
# Copyright (c) 2014 Denis Steckelmacher <steckdenis@yahoo.fr>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License or (at your option) version 3 or any later version
# accepted by the membership of KDE e.V. (or its successor approved
# by the membership of KDE e.V.), which shall act as a proxy
# defined in Section 14 of version 3 of the license.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

from jsgenerator import *

# Print the license of the generated file (the same as the one of this file)
print("""
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
""")

_mixed = '_mixed'       # This variable does not exist and will therefore yield the type "mixed"
_string = '""'
_bool = 'true'
_int = '1'
_float = '1.0'
_array = '[]'
_void = ''

def setBaseClass(c, baseClass):
    print('%s.prototype = %s\n\n' % (c, baseClass))

# Fundamental objects (section 19)
_object = 'new Object()'
_function = 'function(){}'
_date = 'new Date()'
_regexp = 'new RegExp()'
_regexp_match = '{index: 1, input: "", length: 1}'
_map = 'new Map()'
_set = 'new Set()'
_arraybuffer = 'new ArrayBuffer()'
_dataview = 'new DataView()'
_iterator = '{next: function() {}, done: true, value: _mixed}'
_promise = 'new Promise()'

Class('Object').members(
    F(_object, 'assign', ('target', _object), ('sources', _mixed)),
    F(_object, 'create', ('O', _object), ('properties', _object)),
    F(_object, 'defineProperties', ('O', _object), ('properties', _object)),
    F(_object, 'defineProperty', ('O', _object), ('property', _string), ('attributes', _object)),
    F(_object, 'freeze', ('O', _object)),
    F(_object, 'getOwnPropertyDescriptor', ('O', _object), ('property', _string)),
    F(_array, 'getOwnPropertyNames', ('O', _object)),
    F(_array, 'getOwnPropertySymbols', ('O', _object)),
    F(_object, 'getPrototypeOf', ('O', _object)),
    F(_bool, 'is', ('a', _mixed), ('b', _mixed)),
    F(_bool, 'isExtensible', ('O', _object)),
    F(_bool, 'isFrozen', ('O', _object)),
    F(_bool, 'isSealed', ('O', _object)),
    F(_iterator, 'keys', ('O', _object)),
    F(_object, 'preventExtensions', ('O', _object)),
    F(_object, 'seal', ('O', _object)),
    F(_object, 'setPrototypeOf', ('O', _object), ('proto', _object)),

    Var(_function, 'constructor'),
    F(_object, 'hasOwnProperty', ('property', _string)),
    F(_bool, 'isPrototypeOf', ('object', _object)),
    F(_bool, 'propertyIsEnumerable', ('property', _string)),
    F(_string, 'toLocaleString'),
    F(_string, 'toString'),
).print()

Class('Function').members(
    Var(_int, 'length'),
    F(_mixed, 'apply', ('thisArg', _object), ('arguments', _array)),
    F(_function, 'bind', ('thisArg', _object), ('arguments', _mixed)),
    F(_mixed, 'call', ('thisArg', _object), ('arguments', _mixed)),
    F(_function, 'toMethod', ('newHome', _object), ('methodName', _string)),
    Var(_string, 'name')
).print()

Class('Boolean', ('value', _bool)).members(
    F(_bool, 'valueOf'),
).print()

# 19.4 (Symbol) skipped for now because it is far too internal
Class('Error', ('message', _string)).members(
    Var(_string, 'message'),
    Var(_string, 'name'),
).print()

for e in ['Eval', 'Range', 'Reference', 'Syntax', 'Type', 'URI']:
    Class(e + 'Error').print()
    setBaseClass(e + 'Error', 'Error')

# 19.5.6 (NativeError) skipped because it seems too internal

# Numbers and dates (section 20)
Class('Number', ('value', _float)).members(
    Var(_float, 'EPSILON'),
    F(_bool, 'isFinite', ('number', _float)),
    F(_bool, 'isInteger', ('number', _int)),
    F(_bool, 'isNaN', ('number', _float)),
    F(_bool, 'isSafeInteger', ('number', _int)),
    Var(_int, 'MAX_SAFE_INTEGER'),
    Var(_float, 'MAX_VALUE'),
    Var(_float, 'NaN'),
    Var(_float, 'NEGATIVE_INFINITY'),
    Var(_int, 'MIN_SAFE_INTEGER'),
    Var(_float, 'MIN_VALUE'),
    F(_float, 'parseFloat', ('string', _string)),
    F(_int, 'parseInt', ('string', _string), ('radix', _int)),
    Var(_float, 'POSITIVE_INFINITY'),
    F(_string, 'toExponential', ('fractionDigits', _int)),
    F(_string, 'toFixed', ('fractionDigits', _int)),
    F(_string, 'toPrecision', ('precision', _int)),
    F(_float, 'valueOf'),
).print()

Struct('Math').members(
    Var(_float, 'E'),
    Var(_float, 'LN10'),
    Var(_float, 'LOG10E'),
    Var(_float, 'LN2'),
    Var(_float, 'PI'),
    Var(_float, 'SQRT1_2'),
    Var(_float, 'SQRT2'),
    F(_float, 'abs', ('x', _float)),
    F(_float, 'acos', ('x', _float)),
    F(_float, 'acosh', ('x', _float)),
    F(_float, 'asin', ('x', _float)),
    F(_float, 'asinh', ('x', _float)),
    F(_float, 'atan', ('x', _float)),
    F(_float, 'atanh', ('x', _float)),
    F(_float, 'atan2', ('x', _float), ('y', _float)),
    F(_float, 'cbrt', ('x', _float)),
    F(_int, 'ceil', ('x', _float)),
    F(_int, 'clz32', ('x', _int)),
    F(_float, 'cos', ('x', _float)),
    F(_float, 'cosh', ('x', _float)),
    F(_float, 'exp', ('x', _float)),
    F(_float, 'expm1', ('x', _float)),
    F(_int, 'floor', ('x', _float)),
    F(_int, 'fround', ('x', _float)),
    F(_float, 'hypot', ('a', _float), ('b', _float)),
    F(_int, 'imul', ('x', _int), ('y', _int)),
    F(_float, 'log', ('x', _float)),
    F(_float, 'log1p', ('x', _float)),
    F(_float, 'log10', ('x', _float)),
    F(_float, 'log2', ('x', _float)),
    F(_float, 'max', ('a', _float), ('b', _float)),
    F(_float, 'min', ('a', _float), ('b', _float)),
    F(_float, 'pow', ('x', _float), ('y', _float)),
    F(_float, 'random'),
    F(_int, 'round', ('x', _float)),
    F(_int, 'sign', ('x', _float)),
    F(_float, 'sin', ('x', _float)),
    F(_float, 'sinh', ('x', _float)),
    F(_float, 'sqrt', ('x', _float)),
    F(_float, 'tan', ('x', _float)),
    F(_float, 'tanh', ('x', _float)),
    F(_int, 'trunc', ('x', _float)),
).print()

Class('Date', ('year', _int), ('month', _int), ('date', _int), ('hours', _int), ('minutes', _int), ('seconds', _int), ('ms', _int)).members(
    F(_date, 'now'),
    F(_date, 'parse', ('string', _string)),
    F(_date, 'UTC', ('year', _int), ('month', _int), ('date', _int), ('hours', _int), ('minutes', _int), ('seconds', _int), ('ms', _int)),
    F(_int, 'getDate'),
    F(_int, 'getDay'),
    F(_int, 'getFullYear'),
    F(_int, 'getHours'),
    F(_int, 'getMilliseconds'),
    F(_int, 'getMinutes'),
    F(_int, 'getMonth'),
    F(_int, 'getSeconds'),
    F(_int, 'getTime'),
    F(_int, 'getTimezoneOffset'),
    F(_int, 'getUTCDate'),
    F(_int, 'getUTCDay'),
    F(_int, 'getUTCFullYear'),
    F(_int, 'getUTCHours'),
    F(_int, 'getUTCMilliseconds'),
    F(_int, 'getUTCMinutes'),
    F(_int, 'getUTCMonth'),
    F(_int, 'getUTCSeconds'),
    F(_int, 'setDate', ('date', _int)),
    F(_int, 'setFullYear', ('year', _int), ('month', _int), ('date', _int)),
    F(_int, 'setHours', ('hours', _int), ('minutes', _int), ('seconds', _int), ('ms', _int)),
    F(_int, 'setMilliseconds', ('ms', _int)),
    F(_int, 'setMinutes', ('min', _int), ('sec', _int), ('ms', _int)),
    F(_int, 'setMonth', ('month', _int), ('date', _int)),
    F(_int, 'setSeconds', ('sec', _int), ('ms', _int)),
    F(_int, 'setTime', ('time', _int)),
    F(_int, 'setUTCDate', ('date', _int)),
    F(_int, 'setUTCFullYear', ('year', _int), ('month', _int), ('date', _int)),
    F(_int, 'setUTCHours', ('hours', _int), ('minutes', _int), ('seconds', _int), ('ms', _int)),
    F(_int, 'setUTCMilliseconds', ('ms', _int)),
    F(_int, 'setUTCMinutes', ('min', _int), ('sec', _int), ('ms', _int)),
    F(_int, 'setUTCMonth', ('month', _int), ('date', _int)),
    F(_int, 'setUTCSeconds', ('sec', _int), ('ms', _int)),
    F(_string, 'toDateString'),
    F(_string, 'toISOString'),
    F(_string, 'toJSON', ('key', _string)),
    F(_string, 'toLocaleDateString'),
    F(_string, 'toLocaleTimeString'),
    F(_string, 'toTimeString'),
    F(_string, 'toUTCString'),
    F(_int, 'valueOf'),
).print()

# Text Processing (section 21)
Class('String', ('value', _mixed)).members(
    F(_string, 'fromCharCode', ('code', _int)),
    F(_string, 'fromCodePoint', ('code', _int)),
    F(_string, 'raw', ('callSize', _mixed)),
    F(_string, 'charAt', ('pos', _int)),
    F(_int, 'charCodeAt', ('pos', _int)),
    F(_int, 'codePointAt', ('pos', _int)),
    F(_string, 'concat', ('other', _string)),
    F(_bool, 'contains', ('searchString', _string), ('position', _int)),
    F(_bool, 'endsWith', ('searchString', _string), ('endPosition', _int)),
    F(_int, 'indexOf', ('searchString', _string), ('position', _int)),
    F(_int, 'lastIndexOf', ('searchString', _string), ('position', _int)),
    F(_int, 'localeCompare', ('other', _string)),
    F(_regexp_match, 'match', ('regexp', _regexp)),
    F(_string, 'normalize', ('form', _string)),
    F(_string, 'repeat', ('count', _int)),
    F(_string, 'replace', ('searchValue', _string), ('replaceValue', _string)),
    F(_int, 'search', ('regexp', _regexp)),
    F(_string, 'slice', ('start', _int), ('end', _int)),
    F(_array, 'split', ('separator', _string), ('limit', _int)),
    F(_bool, 'startsWith', ('searchString', _string), ('position', _int)),
    F(_string, 'substring', ('start', _int), ('end', _int)),
    F(_string, 'toLocaleLowerCase'),
    F(_string, 'toLocaleUpperCase'),
    F(_string, 'toLowerCase'),
    F(_string, 'toUpperCase'),
    F(_string, 'trim'),
    F(_string, 'valueOf'),
    Var(_int, 'length'),
).print()

Class('RegExp', ('pattern', _string), ('flags', _string)).members(
    F(_void, 'compile'),
    F(_array, 'exec', ('string', _string)),
    Var(_bool, 'ingoreCase'),
    F(_regexp_match, 'match', ('string', _string)),
    Var(_bool, 'multiline'),
    F(_string, 'replace', ('string', _string), ('replaceValue', _string)),
    F(_int, 'search', ('string', _string)),
    Var(_string, 'source'),
    F(_array, 'split', ('string', _string), ('limit', _int)),
    Var(_bool, 'sticky'),
    F(_bool, 'test', ('string', _string)),
    Var(_bool, 'unicode'),
    Var(_int, 'lastIndex'),
    Var(_string, 'lastMatch'),
).print()

# Array Objects (section 22)
Class('Array', ('len', _int)).members(
    F(_array, 'from', ('arrayLike', _mixed), ('mapfn', _function), ('thisArg', _object)),
    F(_bool, 'isArray', ('arg', _mixed)),
    F(_array, 'of', ('items', _mixed)),
    F(_array, 'concat', ('other', _array)),
    F(_array, 'copyWithin', ('target', _int), ('start', _int), ('end', _int)),
    F(_iterator, 'entries'),
    F(_bool, 'every', ('callbackfn', _function), ('thisArg', _object)),
    F(_array, 'fill', ('value', _mixed), ('start', _int), ('end', _int)),
    F(_array, 'filter', ('callbackfn', _function), ('thisArg', _object)),
    F(_mixed, 'find', ('predicate', _function), ('thisArg', _object)),
    F(_int, 'findIndex', ('predicate', _function), ('thisArg', _object)),
    F(_void, 'forEach', ('callbackfn', _function), ('thisArg', _object)),
    F(_int, 'indexOf', ('searchElement', _mixed), ('fromIndex', _int)),
    F(_string, 'join', ('separator', _string)),
    F(_int, 'lastIndexOf', ('searchElement', _mixed), ('fromIndex', _int)),
    F(_array, 'map', ('callbackfn', _function), ('thisArg', _object)),
    F(_mixed, 'pop'),
    F(_int, 'push', ('element', _mixed)),
    F(_mixed, 'reduce', ('callbackfn', _function), ('initialValue', _mixed)),
    F(_mixed, 'reduceRight', ('callbackfn', _function), ('initialValue', _mixed)),
    F(_array, 'reverse'),
    F(_mixed, 'shift'),
    F(_array, 'slice', ('start', _int), ('end', _int)),
    F(_bool, 'some', ('callbackfn', _function), ('thisArg', _object)),
    F(_array, 'sort', ('comparefn', _function)),
    F(_array, 'splice', ('start', _int), ('deleteCount', _int), ('items', _mixed)),
    F(_string, 'substr', ('start', _int), ('length', _int)),
    F(_int, 'unshift', ('items', _mixed)),
    F(_iterator, 'values'),
    Var(_int, 'length'),
).print()

for a in ['Int8', 'Uint8', 'Uint8Clamped', 'Int16', 'Uint16', 'Int32', 'Uint32', 'Float32', 'Float64']:
    Class(a + 'Array', ('length', _int)).members(
        Var(_mixed, 'buffer'),
        Var(_int, 'byteLength'),
        Var(_int, 'byteOffset'),
        F(_array, 'subarray', ('begin', _int), ('end', _int)),
        Var(_int, 'BYTES_PER_ELEMENT'),
    ).print()

    setBaseClass(a + 'Array', 'Array')

# Keyed Collection (section 23)
Class('Map', ('iterable', _mixed)).members(
    F(_void, 'clear'),
    F(_bool, 'delete', ('key', _mixed)),
    F(_iterator, 'entries'),
    F(_void, 'forEach', ('callbackfn', _function), ('thisArg', _object)),
    F(_mixed, 'get', ('key', _mixed)),
    F(_bool, 'has', ('key', _mixed)),
    F(_map, 'set', ('key', _mixed), ('value', _mixed)),
    Var(_int, 'size'),
    F(_iterator, 'values'),
).print()

Class('Set', ('iterable', _mixed)).members(
    F(_set, 'add', ('value', _mixed)),
    F(_void, 'clear'),
    F(_bool, 'delete', ('value', _mixed)),
    F(_iterator, 'entries'),
    F(_void, 'forEach', ('callbackfn', _function), ('thisArg', _object)),
    F(_bool, 'has', ('value', _mixed)),
    Var(_int, 'size'),
    F(_iterator, 'values'),
).print()

Class('WeakMap', ('iterable', _mixed)).members(
    F(_void, 'clear'),
    F(_bool, 'delete', ('key', _mixed)),
    F(_mixed, 'get', ('key', _mixed)),
    F(_bool, 'has', ('key', _mixed)),
    F(_map, 'set', ('key', _mixed), ('value', _mixed)),
).print()

Class('WeakSet', ('iterable', _mixed)).members(
    F(_set, 'add', ('value', _mixed)),
    F(_void, 'clear'),
    F(_bool, 'delete', ('value', _mixed)),
    F(_bool, 'has', ('value', _mixed)),
).print()

# Structured Data (section 24)
Class('ArrayBuffer', ('length', _int)).members(
    F(_bool, 'isView', ('arg', _mixed)),
    Var(_int, 'byteLength'),
    F(_arraybuffer, 'slice', ('start', _int), ('end', _int)),
).print()

Class('DataView', ('buffer', _arraybuffer), ('byteOffset', _int), ('byteLength', _int)).members(
    Var(_dataview, 'buffer'),
    Var(_int, 'byteLength'),
    Var(_int, 'byteOffset'),
    F(_float, 'getFloat32', ('byteOffset', _int), ('littleEndian', _bool)),
    F(_float, 'getFloat64', ('byteOffset', _int), ('littleEndian', _bool)),
    F(_int, 'getInt8', ('byteOffset', _int)),
    F(_int, 'getInt16', ('byteOffset', _int), ('littleEndian', _bool)),
    F(_int, 'getInt32', ('byteOffset', _int), ('littleEndian', _bool)),
    F(_int, 'getUInt8', ('byteOffset', _int)),
    F(_int, 'getUInt16', ('byteOffset', _int), ('littleEndian', _bool)),
    F(_int, 'getUInt32', ('byteOffset', _int), ('littleEndian', _bool)),
    F(_void, 'setFloat32', ('byteOffset', _int), ('value', _float), ('littleEndian', _bool)),
    F(_void, 'setFloat64', ('byteOffset', _int), ('value', _float), ('littleEndian', _bool)),
    F(_void, 'setInt8', ('byteOffset', _int), ('value', _int)),
    F(_void, 'setInt16', ('byteOffset', _int), ('value', _int), ('littleEndian', _bool)),
    F(_void, 'setInt32', ('byteOffset', _int), ('value', _int), ('littleEndian', _bool)),
    F(_void, 'setUInt8', ('byteOffset', _int), ('value', _int)),
    F(_void, 'setUInt16', ('byteOffset', _int), ('value', _int), ('littleEndian', _bool)),
    F(_void, 'setUInt32', ('byteOffset', _int), ('value', _int), ('littleEndian', _bool)),
).print()

Struct('JSON').members(
    F(_mixed, 'parse', ('text', _string), ('reviver', _function)),
    F(_string, 'stringify', ('value', _mixed), ('replacer', _function), ('space', _string)),
).print()

# Control Abstraction Objects (section 25)
Class('Promise', ('executor', _function)).members(
    F(_void, 'all', ('iterable', _array)),
    F(_void, 'race', ('iterable', _array)),
    F(_promise, 'resolve', ('x', _mixed)),
    F(_void, 'catch', ('onRejected', _function)),
    F(_void, 'then', ('onFulfilled', _function), ('onRejected', _function)),
).print()

Struct('Reflect').members(
    F(_mixed, 'apply', ('target', _function), ('thisArgument', _object), ('argumentList', _array)),
    F(_object, 'construct', ('target', _function), ('argumentList', _array)),
    F(_void, 'deleteProperty', ('target', _object), ('propertyKey', _string)),
    F(_iterator, 'enumerate', ('target', _object)),
    F(_mixed, 'get', ('target', _object), ('propertyKey', _string), ('receiver', _object)),
    F(_bool, 'has', ('target', _object), ('propertyKey', _string)),
    F(_array, 'ownKeys', ('target', _object)),
    F(_void, 'set', ('target', _object), ('propertyKey', _string), ('value', _mixed), ('receiver', _object)),
).print()

# Realm, Loader and Proxy not decribed for now
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

Class('Object') \
    .member(Function(_object, 'assign', ('target', _object), ('sources', _mixed))) \
    .member(Function(_object, 'create', ('O', _object), ('properties', _object))) \
    .member(Function(_object, 'defineProperties', ('O', _object), ('properties', _object))) \
    .member(Function(_object, 'defineProperty', ('O', _object), ('property', _string), ('attributes', _object))) \
    .member(Function(_object, 'freeze', ('O', _object))) \
    .member(Function(_object, 'getOwnPropertyDescriptor', ('O', _object), ('property', _string))) \
    .member(Function(_array, 'getOwnPropertyNames', ('O', _object))) \
    .member(Function(_array, 'getOwnPropertySymbols', ('O', _object))) \
    .member(Function(_object, 'getPrototypeOf', ('O', _object))) \
    .member(Function(_bool, 'is', ('a', _mixed), ('b', _mixed))) \
    .member(Function(_bool, 'isExtensible', ('O', _object))) \
    .member(Function(_bool, 'isFrozen', ('O', _object))) \
    .member(Function(_bool, 'isSealed', ('O', _object))) \
    .member(Function(_iterator, 'keys', ('O', _object))) \
    .member(Function(_object, 'preventExtensions', ('O', _object))) \
    .member(Function(_object, 'seal', ('O', _object))) \
    .member(Function(_object, 'setPrototypeOf', ('O', _object), ('proto', _object))) \
    \
    .member(Variable(_function, 'constructor')) \
    .member(Function(_object, 'hasOwnProperty', ('property', _string))) \
    .member(Function(_bool, 'isPrototypeOf', ('object', _object))) \
    .member(Function(_bool, 'propertyIsEnumerable', ('property', _string))) \
    .member(Function(_string, 'toLocaleString')) \
    .member(Function(_string, 'toString')) \
    .print()

Class('Function') \
    .member(Variable(_int, 'length')) \
    .member(Function(_mixed, 'apply', ('thisArg', _object), ('arguments', _array))) \
    .member(Function(_function, 'bind', ('thisArg', _object), ('arguments', _mixed))) \
    .member(Function(_mixed, 'call', ('thisArg', _object), ('arguments', _mixed))) \
    .member(Function(_function, 'toMethod', ('newHome', _object), ('methodName', _string))) \
    .member(Variable(_string, 'name')) \
    .print()

Class('Boolean', ('value', _bool)) \
    .member(Function(_bool, 'valueOf')) \
    .print()

# 19.4 (Symbol) skipped for now because it is far too internal
Class('Error', ('message', _string)) \
    .member(Variable(_string, 'message')) \
    .member(Variable(_string, 'name')) \
    .print()

for e in ['Eval', 'Range', 'Reference', 'Syntax', 'Type', 'URI']:
    Class(e + 'Error').print()
    setBaseClass(e + 'Error', 'Error')

# 19.5.6 (NativeError) skipped because it seems too internal

# Numbers and dates (section 20)
Class('Number', ('value', _float)) \
    .member(Variable(_float, 'EPSILON')) \
    .member(Function(_bool, 'isFinite', ('number', _float))) \
    .member(Function(_bool, 'isInteger', ('number', _int))) \
    .member(Function(_bool, 'isNaN', ('number', _float))) \
    .member(Function(_bool, 'isSafeInteger', ('number', _int))) \
    .member(Variable(_int, 'MAX_SAFE_INTEGER')) \
    .member(Variable(_float, 'MAX_VALUE')) \
    .member(Variable(_float, 'NaN')) \
    .member(Variable(_float, 'NEGATIVE_INFINITY')) \
    .member(Variable(_int, 'MIN_SAFE_INTEGER')) \
    .member(Variable(_float, 'MIN_VALUE')) \
    .member(Function(_float, 'parseFloat', ('string', _string))) \
    .member(Function(_int, 'parseInt', ('string', _string), ('radix', _int))) \
    .member(Variable(_float, 'POSITIVE_INFINITY')) \
    .member(Function(_string, 'toExponential', ('fractionDigits', _int))) \
    .member(Function(_string, 'toFixed', ('fractionDigits', _int))) \
    .member(Function(_string, 'toPrecision', ('precision', _int))) \
    .member(Function(_float, 'valueOf')) \
    .print()

Struct('Math') \
    .member(Variable(_float, 'E')) \
    .member(Variable(_float, 'LN10')) \
    .member(Variable(_float, 'LOG10E')) \
    .member(Variable(_float, 'LN2')) \
    .member(Variable(_float, 'PI')) \
    .member(Variable(_float, 'SQRT1_2')) \
    .member(Variable(_float, 'SQRT2')) \
    .member(Function(_float, 'abs', ('x', _float))) \
    .member(Function(_float, 'acos', ('x', _float))) \
    .member(Function(_float, 'acosh', ('x', _float))) \
    .member(Function(_float, 'asin', ('x', _float))) \
    .member(Function(_float, 'asinh', ('x', _float))) \
    .member(Function(_float, 'atan', ('x', _float))) \
    .member(Function(_float, 'atanh', ('x', _float))) \
    .member(Function(_float, 'atan2', ('x', _float), ('y', _float))) \
    .member(Function(_float, 'cbrt', ('x', _float))) \
    .member(Function(_int, 'ceil', ('x', _float))) \
    .member(Function(_int, 'clz32', ('x', _int))) \
    .member(Function(_float, 'cos', ('x', _float))) \
    .member(Function(_float, 'cosh', ('x', _float))) \
    .member(Function(_float, 'exp', ('x', _float))) \
    .member(Function(_float, 'expm1', ('x', _float))) \
    .member(Function(_int, 'floor', ('x', _float))) \
    .member(Function(_int, 'fround', ('x', _float))) \
    .member(Function(_float, 'hypot', ('a', _float), ('b', _float))) \
    .member(Function(_int, 'imul', ('x', _int), ('y', _int))) \
    .member(Function(_float, 'log', ('x', _float))) \
    .member(Function(_float, 'log1p', ('x', _float))) \
    .member(Function(_float, 'log10', ('x', _float))) \
    .member(Function(_float, 'log2', ('x', _float))) \
    .member(Function(_float, 'max', ('a', _float), ('b', _float))) \
    .member(Function(_float, 'min', ('a', _float), ('b', _float))) \
    .member(Function(_float, 'pow', ('x', _float), ('y', _float))) \
    .member(Function(_float, 'random')) \
    .member(Function(_int, 'round', ('x', _float))) \
    .member(Function(_int, 'sign', ('x', _float))) \
    .member(Function(_float, 'sin', ('x', _float))) \
    .member(Function(_float, 'sinh', ('x', _float))) \
    .member(Function(_float, 'sqrt', ('x', _float))) \
    .member(Function(_float, 'tan', ('x', _float))) \
    .member(Function(_float, 'tanh', ('x', _float))) \
    .member(Function(_int, 'trunc', ('x', _float))) \
    .print()

Class('Date', ('year', _int), ('month', _int), ('date', _int), ('hours', _int), ('minutes', _int), ('seconds', _int), ('ms', _int)) \
    .member(Function(_date, 'now')) \
    .member(Function(_date, 'parse', ('string', _string))) \
    .member(Function(_date, 'UTC', ('year', _int), ('month', _int), ('date', _int), ('hours', _int), ('minutes', _int), ('seconds', _int), ('ms', _int))) \
    .member(Function(_int, 'getDate')) \
    .member(Function(_int, 'getDay')) \
    .member(Function(_int, 'getFullYear')) \
    .member(Function(_int, 'getHours')) \
    .member(Function(_int, 'getMilliseconds')) \
    .member(Function(_int, 'getMinutes')) \
    .member(Function(_int, 'getMonth')) \
    .member(Function(_int, 'getSeconds')) \
    .member(Function(_int, 'getTime')) \
    .member(Function(_int, 'getTimezoneOffset')) \
    .member(Function(_int, 'getUTCDate')) \
    .member(Function(_int, 'getUTCDay')) \
    .member(Function(_int, 'getUTCFullYear')) \
    .member(Function(_int, 'getUTCHours')) \
    .member(Function(_int, 'getUTCMilliseconds')) \
    .member(Function(_int, 'getUTCMinutes')) \
    .member(Function(_int, 'getUTCMonth')) \
    .member(Function(_int, 'getUTCSeconds')) \
    .member(Function(_int, 'setDate', ('date', _int))) \
    .member(Function(_int, 'setFullYear', ('year', _int), ('month', _int), ('date', _int))) \
    .member(Function(_int, 'setHours', ('hours', _int), ('minutes', _int), ('seconds', _int), ('ms', _int))) \
    .member(Function(_int, 'setMilliseconds', ('ms', _int))) \
    .member(Function(_int, 'setMinutes', ('min', _int), ('sec', _int), ('ms', _int))) \
    .member(Function(_int, 'setMonth', ('month', _int), ('date', _int))) \
    .member(Function(_int, 'setSeconds', ('sec', _int), ('ms', _int))) \
    .member(Function(_int, 'setTime', ('time', _int))) \
    .member(Function(_int, 'setUTCDate', ('date', _int))) \
    .member(Function(_int, 'setUTCFullYear', ('year', _int), ('month', _int), ('date', _int))) \
    .member(Function(_int, 'setUTCHours', ('hours', _int), ('minutes', _int), ('seconds', _int), ('ms', _int))) \
    .member(Function(_int, 'setUTCMilliseconds', ('ms', _int))) \
    .member(Function(_int, 'setUTCMinutes', ('min', _int), ('sec', _int), ('ms', _int))) \
    .member(Function(_int, 'setUTCMonth', ('month', _int), ('date', _int))) \
    .member(Function(_int, 'setUTCSeconds', ('sec', _int), ('ms', _int))) \
    .member(Function(_string, 'toDateString')) \
    .member(Function(_string, 'toISOString')) \
    .member(Function(_string, 'toJSON', ('key', _string))) \
    .member(Function(_string, 'toLocaleDateString')) \
    .member(Function(_string, 'toLocaleTimeString')) \
    .member(Function(_string, 'toTimeString')) \
    .member(Function(_string, 'toUTCString')) \
    .member(Function(_int, 'valueOf')) \
    .print()

# Text Processing (section 21)
Class('String', ('value', _mixed)) \
    .member(Function(_string, 'fromCharCode', ('code', _int))) \
    .member(Function(_string, 'fromCodePoint', ('code', _int))) \
    .member(Function(_string, 'raw', ('callSize', _mixed))) \
    .member(Function(_string, 'charAt', ('pos', _int))) \
    .member(Function(_int, 'charCodeAt', ('pos', _int))) \
    .member(Function(_int, 'codePointAt', ('pos', _int))) \
    .member(Function(_string, 'concat', ('other', _string))) \
    .member(Function(_bool, 'contains', ('searchString', _string), ('position', _int))) \
    .member(Function(_bool, 'endsWith', ('searchString', _string), ('endPosition', _int))) \
    .member(Function(_int, 'indexOf', ('searchString', _string), ('position', _int))) \
    .member(Function(_int, 'lastIndexOf', ('searchString', _string), ('position', _int))) \
    .member(Function(_int, 'localeCompare', ('other', _string))) \
    .member(Function(_regexp_match, 'match', ('regexp', _regexp))) \
    .member(Function(_string, 'normalize', ('form', _string))) \
    .member(Function(_string, 'repeat', ('count', _int))) \
    .member(Function(_string, 'replace', ('searchValue', _string), ('replaceValue', _string))) \
    .member(Function(_int, 'search', ('regexp', _regexp))) \
    .member(Function(_string, 'slice', ('start', _int), ('end', _int))) \
    .member(Function(_array, 'split', ('separator', _string), ('limit', _int))) \
    .member(Function(_bool, 'startsWith', ('searchString', _string), ('position', _int))) \
    .member(Function(_string, 'substring', ('start', _int), ('end', _int))) \
    .member(Function(_string, 'toLocaleLowerCase')) \
    .member(Function(_string, 'toLocaleUpperCase')) \
    .member(Function(_string, 'toLowerCase')) \
    .member(Function(_string, 'toUpperCase')) \
    .member(Function(_string, 'trim')) \
    .member(Function(_string, 'valueOf')) \
    .member(Variable(_int, 'length')) \
    .print()

Class('RegExp', ('pattern', _string), ('flags', _string)) \
    .member(Function(_void, 'compile')) \
    .member(Function(_array, 'exec', ('string', _string))) \
    .member(Variable(_bool, 'ingoreCase')) \
    .member(Function(_regexp_match, 'match', ('string', _string))) \
    .member(Variable(_bool, 'multiline')) \
    .member(Function(_string, 'replace', ('string', _string), ('replaceValue', _string))) \
    .member(Function(_int, 'search', ('string', _string))) \
    .member(Variable(_string, 'source')) \
    .member(Function(_array, 'split', ('string', _string), ('limit', _int))) \
    .member(Variable(_bool, 'sticky')) \
    .member(Function(_bool, 'test', ('string', _string))) \
    .member(Variable(_bool, 'unicode')) \
    .member(Variable(_int, 'lastIndex')) \
    .member(Variable(_string, 'lastMatch')) \
    .print()

# Array Objects (section 22)
Class('Array', ('len', _int)) \
    .member(Function(_array, 'from', ('arrayLike', _mixed), ('mapfn', _function), ('thisArg', _object))) \
    .member(Function(_bool, 'isArray', ('arg', _mixed))) \
    .member(Function(_array, 'of', ('items', _mixed))) \
    .member(Function(_array, 'concat', ('other', _array))) \
    .member(Function(_array, 'copyWithin', ('target', _int), ('start', _int), ('end', _int))) \
    .member(Function(_iterator, 'entries')) \
    .member(Function(_bool, 'every', ('callbackfn', _function), ('thisArg', _object))) \
    .member(Function(_array, 'fill', ('value', _mixed), ('start', _int), ('end', _int))) \
    .member(Function(_array, 'filter', ('callbackfn', _function), ('thisArg', _object))) \
    .member(Function(_mixed, 'find', ('predicate', _function), ('thisArg', _object))) \
    .member(Function(_int, 'findIndex', ('predicate', _function), ('thisArg', _object))) \
    .member(Function(_void, 'forEach', ('callbackfn', _function), ('thisArg', _object))) \
    .member(Function(_int, 'indexOf', ('searchElement', _mixed), ('fromIndex', _int))) \
    .member(Function(_string, 'join', ('separator', _string))) \
    .member(Function(_int, 'lastIndexOf', ('searchElement', _mixed), ('fromIndex', _int))) \
    .member(Function(_array, 'map', ('callbackfn', _function), ('thisArg', _object))) \
    .member(Function(_mixed, 'pop')) \
    .member(Function(_int, 'push', ('element', _mixed))) \
    .member(Function(_mixed, 'reduce', ('callbackfn', _function), ('initialValue', _mixed))) \
    .member(Function(_mixed, 'reduceRight', ('callbackfn', _function), ('initialValue', _mixed))) \
    .member(Function(_array, 'reverse')) \
    .member(Function(_mixed, 'shift')) \
    .member(Function(_array, 'slice', ('start', _int), ('end', _int))) \
    .member(Function(_bool, 'some', ('callbackfn', _function), ('thisArg', _object))) \
    .member(Function(_array, 'sort', ('comparefn', _function))) \
    .member(Function(_array, 'splice', ('start', _int), ('deleteCount', _int), ('items', _mixed))) \
    .member(Function(_string, 'substr', ('start', _int), ('length', _int))) \
    .member(Function(_int, 'unshift', ('items', _mixed))) \
    .member(Function(_iterator, 'values')) \
    .member(Variable(_int, 'length')) \
    .print()

for a in ['Int8', 'Uint8', 'Uint8Clamped', 'Int16', 'Uint16', 'Int32', 'Uint32', 'Float32', 'Float64']:
    Class(a + 'Array', ('length', _int)) \
        .member(Variable(_mixed, 'buffer')) \
        .member(Variable(_int, 'byteLength')) \
        .member(Variable(_int, 'byteOffset')) \
        .member(Function(_array, 'subarray', ('begin', _int), ('end', _int))) \
        .member(Variable(_int, 'BYTES_PER_ELEMENT')) \
        .print()

    setBaseClass(a + 'Array', 'Array')

# Keyed Collection (section 23)
Class('Map', ('iterable', _mixed)) \
    .member(Function(_void, 'clear')) \
    .member(Function(_bool, 'delete', ('key', _mixed))) \
    .member(Function(_iterator, 'entries')) \
    .member(Function(_void, 'forEach', ('callbackfn', _function), ('thisArg', _object))) \
    .member(Function(_mixed, 'get', ('key', _mixed))) \
    .member(Function(_bool, 'has', ('key', _mixed))) \
    .member(Function(_map, 'set', ('key', _mixed), ('value', _mixed))) \
    .member(Variable(_int, 'size')) \
    .member(Function(_iterator, 'values')) \
    .print()

Class('Set', ('iterable', _mixed)) \
    .member(Function(_set, 'add', ('value', _mixed))) \
    .member(Function(_void, 'clear')) \
    .member(Function(_bool, 'delete', ('value', _mixed))) \
    .member(Function(_iterator, 'entries')) \
    .member(Function(_void, 'forEach', ('callbackfn', _function), ('thisArg', _object))) \
    .member(Function(_bool, 'has', ('value', _mixed))) \
    .member(Variable(_int, 'size')) \
    .member(Function(_iterator, 'values')) \
    .print()

Class('WeakMap', ('iterable', _mixed)) \
    .member(Function(_void, 'clear')) \
    .member(Function(_bool, 'delete', ('key', _mixed))) \
    .member(Function(_mixed, 'get', ('key', _mixed))) \
    .member(Function(_bool, 'has', ('key', _mixed))) \
    .member(Function(_map, 'set', ('key', _mixed), ('value', _mixed))) \
    .print()

Class('WeakSet', ('iterable', _mixed)) \
    .member(Function(_set, 'add', ('value', _mixed))) \
    .member(Function(_void, 'clear')) \
    .member(Function(_bool, 'delete', ('value', _mixed))) \
    .member(Function(_bool, 'has', ('value', _mixed))) \
    .print()

# Structured Data (section 24)
Class('ArrayBuffer', ('length', _int)) \
    .member(Function(_bool, 'isView', ('arg', _mixed))) \
    .member(Variable(_int, 'byteLength')) \
    .member(Function(_arraybuffer, 'slice', ('start', _int), ('end', _int))) \
    .print()

Class('DataView', ('buffer', _arraybuffer), ('byteOffset', _int), ('byteLength', _int)) \
    .member(Variable(_dataview, 'buffer')) \
    .member(Variable(_int, 'byteLength')) \
    .member(Variable(_int, 'byteOffset')) \
    .member(Function(_float, 'getFloat32', ('byteOffset', _int), ('littleEndian', _bool))) \
    .member(Function(_float, 'getFloat64', ('byteOffset', _int), ('littleEndian', _bool))) \
    .member(Function(_int, 'getInt8', ('byteOffset', _int))) \
    .member(Function(_int, 'getInt16', ('byteOffset', _int), ('littleEndian', _bool))) \
    .member(Function(_int, 'getInt32', ('byteOffset', _int), ('littleEndian', _bool))) \
    .member(Function(_int, 'getUInt8', ('byteOffset', _int))) \
    .member(Function(_int, 'getUInt16', ('byteOffset', _int), ('littleEndian', _bool))) \
    .member(Function(_int, 'getUInt32', ('byteOffset', _int), ('littleEndian', _bool))) \
    .member(Function(_void, 'setFloat32', ('byteOffset', _int), ('value', _float), ('littleEndian', _bool))) \
    .member(Function(_void, 'setFloat64', ('byteOffset', _int), ('value', _float), ('littleEndian', _bool))) \
    .member(Function(_void, 'setInt8', ('byteOffset', _int), ('value', _int))) \
    .member(Function(_void, 'setInt16', ('byteOffset', _int), ('value', _int), ('littleEndian', _bool))) \
    .member(Function(_void, 'setInt32', ('byteOffset', _int), ('value', _int), ('littleEndian', _bool))) \
    .member(Function(_void, 'setUInt8', ('byteOffset', _int), ('value', _int))) \
    .member(Function(_void, 'setUInt16', ('byteOffset', _int), ('value', _int), ('littleEndian', _bool))) \
    .member(Function(_void, 'setUInt32', ('byteOffset', _int), ('value', _int), ('littleEndian', _bool))) \
    .print()

Struct('JSON') \
    .member(Function(_mixed, 'parse', ('text', _string), ('reviver', _function))) \
    .member(Function(_string, 'stringify', ('value', _mixed), ('replacer', _function), ('space', _string))) \
    .print()

# Control Abstraction Objects (section 25)
Class('Promise', ('executor', _function)) \
    .member(Function(_void, 'all', ('iterable', _array))) \
    .member(Function(_void, 'race', ('iterable', _array))) \
    .member(Function(_promise, 'resolve', ('x', _mixed))) \
    .member(Function(_void, 'catch', ('onRejected', _function))) \
    .member(Function(_void, 'then', ('onFulfilled', _function), ('onRejected', _function))) \
    .print()

Struct('Reflect') \
    .member(Function(_mixed, 'apply', ('target', _function), ('thisArgument', _object), ('argumentList', _array))) \
    .member(Function(_object, 'construct', ('target', _function), ('argumentList', _array))) \
    .member(Function(_void, 'deleteProperty', ('target', _object), ('propertyKey', _string))) \
    .member(Function(_iterator, 'enumerate', ('target', _object))) \
    .member(Function(_mixed, 'get', ('target', _object), ('propertyKey', _string), ('receiver', _object))) \
    .member(Function(_bool, 'has', ('target', _object), ('propertyKey', _string))) \
    .member(Function(_array, 'ownKeys', ('target', _object))) \
    .member(Function(_void, 'set', ('target', _object), ('propertyKey', _string), ('value', _mixed), ('receiver', _object))) \
    .print()

# Realm, Loader and Proxy not decribed for now
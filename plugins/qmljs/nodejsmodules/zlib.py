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
from common import *

# Print the license of the generated file (the same as the one of this file)
license()
basicTypes(globals())
require('stream')
require('buffer')

_object = 'new Object()'
_function = 'function(){}'
_options = ('options', _object)
_buf = ('buf', 'new buffer.Buffer()')
_callback = ('callback', _function)

Module().members(
    F('new Gzip()', 'createGzip', _options),
    F('new Gunzip()', 'createGunzip', _options),
    F('new Deflate()', 'createDeflate', _options),
    F('new Inflate()', 'createInflate', _options),
    F('new DeflateRaw()', 'createDeflateRaw', _options),
    F('new InflateRaw()', 'createInflateRaw', _options),
    F('new Unzip()', 'createUnzip', _options),
    Class('Zlib').prototype('stream.Duplex').members(
        F(_void, 'flush', ('callback', _function)),
        F(_void, 'reset')
    ),
    Class('Gzip').prototype('exports.Zlib'),
    Class('Gunzip').prototype('exports.Zlib'),
    Class('Deflate').prototype('exports.Zlib'),
    Class('Inflate').prototype('exports.Zlib'),
    Class('DeflateRaw').prototype('exports.Zlib'),
    Class('InflateRaw').prototype('exports.Zlib'),
    Class('Unzip').prototype('exports.Zlib'),
    F(_void, 'deflate', _buf, _callback),
    F(_void, 'deflateRaw', _buf, _callback),
    F(_void, 'gzip', _buf, _callback),
    F(_void, 'gunzip', _buf, _callback),
    F(_void, 'inflate', _buf, _callback),
    F(_void, 'inflateRaw', _buf, _callback),
    F(_void, 'unzip', _buf, _callback)
).print()

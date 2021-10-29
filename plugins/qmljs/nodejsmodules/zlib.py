#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

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

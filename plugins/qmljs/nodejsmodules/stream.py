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
require('event')

_object = 'new Object()'
_function = 'function(){}'

Module().members(
    Class('Readable').prototype('event.EventEmitter').members(
        F(_string, 'read', ('size', _int)),
        F(_void, 'setEncoding', ('encoding', _string)),
        F(_void, 'resume'),
        F(_void, 'pause'),
        F(_void, 'pipe', ('destination', 'new Writable()'), ('options', _object)),
        F(_void, 'unpipe', ('destination', 'new Writable()')),
        F(_void, 'unshift', ('chunk', _string)),
        F('new Readable()', 'wrap', ('stream', _mixed))
    ),
    Class('Writable').prototype('event.EventEmitter').members(
        F(_void, 'write', ('chunk', _string), ('encoding', _string), ('callback', _function)),
        F(_void, 'end', ('chunk', _string), ('encoding', _string), ('callback', _function))
    ),
    Class('Duplex').prototype('event.EventEmitter').members(
        F(_string, 'read', ('size', _int)),
        F(_void, 'setEncoding', ('encoding', _string)),
        F(_void, 'resume'),
        F(_void, 'pause'),
        F(_void, 'pipe', ('destination', 'new Writable()'), ('options', _object)),
        F(_void, 'unpipe', ('destination', 'new Writable()')),
        F(_void, 'unshift', ('chunk', _string)),
        F('new Readable()', 'wrap', ('stream', _mixed)),
        F(_void, 'write', ('chunk', _string), ('encoding', _string), ('callback', _function)),
        F(_void, 'end', ('chunk', _string), ('encoding', _string), ('callback', _function))
    ),
    Class('Transform').prototype('exports.Duplex')
).print()

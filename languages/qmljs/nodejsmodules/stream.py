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

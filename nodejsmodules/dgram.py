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

import sys
sys.path.append('../qmlplugins')

from jsgenerator import *
from common import *

# Print the license of the generated file (the same as the one of this file)
license()
basicTypes(globals())
require('event')
require('buffer')

_object = 'new Object()'
_function = 'function(){}'
_buffer = 'new buffer.Buffer()'
_address = '{address: "", family: "", port: 1}'

Module().members(
    F('new Socket', 'createSocket', ('type', _string), ('callback', _function)),
    Class('Socket').prototype('event.EventEmitter').members(
        F(_void, 'send', ('buf', _buffer), ('offset', _int), ('length', _int), ('port', _int), ('address', _string), ('callback', _function)),
        F(_void, 'bind', ('port', _int), ('address', _string), ('callback', _function)),
        F(_void, 'close'),
        F(_address, 'address'),
        F(_void, 'setBroadcast', ('flag', _bool)),
        F(_void, 'setTTL', ('ttl', _int)),
        F(_void, 'setMulticastTTL', ('ttl', _int)),
        F(_void, 'setMulticastLoopback', ('flag', _bool)),
        F(_void, 'addMembership', ('multicastAddress', _string), ('multicastInterface', _string)),
        F(_void, 'dropMembership', ('multicastAddress', _string), ('multicastInterface', _string)),
        F(_void, 'unref'),
        F(_void, 'ref')
    )
).print()

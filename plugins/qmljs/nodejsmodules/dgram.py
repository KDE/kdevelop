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

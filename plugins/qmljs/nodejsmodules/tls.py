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
_address = '{port: 1, family: "", address: ""}'

_peer_certificate = """{
    subject: {C: "", ST: "", L: "", O: "", OU: "", CN: ""},
    issuer: {C: "", ST: "", L: "", O: "", OU: "", CN: ""},
    valid_from: "",
    valid_to: "",
    fingerprint: ""}"""
_cipher = '{name: "", version: ""}'

Module().members(
    F(_array, 'getCiphers'),
    F('new Server()', 'createServer', ('options', _object), ('secureConnectionListener', _function)),
    Var(_int, 'SLAB_BUFFER_SIZE'),
    F('new CleartextStream()', 'connect', ('options', _object), ('callback', _function)),
    F('new SecurePair()', 'createSecurePair', ('credentials', _object), ('isServer', _bool), ('requestCert', _bool), ('rejectUnauthorized', _bool)),
    Class('SecurePair').prototype('event.EventEmitter'),
    Class('Server').prototype('event.EventEmitter').members(
        F(_void, 'listen', ('port', _int), ('host', _string), ('callback', _function)),
        F(_void, 'close'),
        F(_address, 'address'),
        F(_void, 'addContext', ('hostname', _string), ('credentials', _object)),
        Var(_int, 'maxConnections'),
        Var(_int, 'connections')
    ),
    Class('CryptoStream').members(
        Var(_int, 'bytesWritten')
    ),
    Class('CleartextStream').prototype('event.EventEmitter').members(
        Var(_bool, 'authorized'),
        Var(_string, 'authorizationError'),
        F(_peer_certificate, 'getPeerCertificate'),
        F(_cipher, 'getCipher'),
        F(_address, 'address'),
        Var(_string, 'remoteAddress'),
        Var(_int, 'remotePort')
    )
).print()

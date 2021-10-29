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

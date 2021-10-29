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

_function = 'function(){}'
_object = 'new Object()'
_date = 'new Date()'
_socket = 'new Socket()'
_server = 'new Server()'
_address = '{port: 1, family: "", address: ""}'

Module().members(
    F(_server, 'createServer', ('options', _object), ('connectionListener', _function)),
    F(_socket, 'connect', ('options', _object), ('connectionListener', _function)),
    F(_socket, 'createConnection', ('options', _object), ('connectionListener', _function)),
    Class('Server').prototype('event.EventEmitter').members(
        F(_void, 'listen', ('port', _int), ('host', _string), ('backlog', _int), ('callback', _function)),
        F(_void, 'close', ('callback', _function)),
        F(_address, 'address'),
        F(_void, 'unref'),
        F(_void, 'ref'),
        Var(_int, 'maxConnections'),
        Var(_int, 'connections'),
        F(_void, 'getConnections', ('callback', _function))
    ),
    Class('Socket', ('options', _object)).prototype('event.EventEmitter').members(
        F(_void, 'connect', ('port', _int), ('host', _string), ('connectionListener', _function)),
        Var(_int, 'bufferSize'),
        F(_void, 'setEncoding', ('encoding', _string)),
        F(_void, 'write', ('data', _string), ('encoding', _string), ('callback', _function)),
        F(_void, 'end', ('data', _string), ('encoding', _string)),
        F(_void, 'destroy'),
        F(_void, 'pause'),
        F(_void, 'resume'),
        F(_void, 'setTimeout', ('timeout', _int), ('callback', _function)),
        F(_void, 'setNoDelay', ('noDelay', _bool)),
        F(_void, 'setKeepAlive', ('enable', _bool), ('initialDelay', _int)),
        F(_address, 'address'),
        F(_void, 'unref'),
        F(_void, 'ref'),
        Var(_string, 'remoteAddress'),
        Var(_int, 'remotePort'),
        Var(_string, 'localAddress'),
        Var(_int, 'localPort'),
        Var(_int, 'bytesRead'),
        Var(_int, 'bytesWritten')
    ),
    F(_int, 'isIP', ('input', _string)),
    F(_bool, 'isIPv4', ('input', _string)),
    F(_bool, 'isIPv6', ('input', _string))
).print()

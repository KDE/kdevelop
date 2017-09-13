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

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
require('net')

_function = 'function(){}'
_object = 'new Object()'
_date = 'new Date()'

Module().members(
    Var('{}', 'STATUS_CODES'),
    F('new Server()', 'createServer', ('requestListener', _function)),
    F('new Client()', 'createClient', ('port', _int), ('host', _string)),
    Class('Server').prototype('event.EventEmitter').members(
        F(_void, 'listen', ('port', _int), ('hostname', _string), ('backlog', _int), ('callback', _function)),
        F(_void, 'close', ('callback', _function)),
        Var(_int, 'maxHeadersCount'),
        F(_void, 'setTimeout', ('msecs', _int), ('callback', _function)),
        Var(_int, 'timeout'),
    ),
    Class('ServerResponse').prototype('event.EventEmitter').members(
        F(_void, 'writeContinue'),
        F(_void, 'writeHead', ('statusCode', _int), ('reasonPhrase', _string), ('headers', _object)),
        F(_void, 'setTimeout', ('msecs', _int), ('callback', _function)),
        Var(_int, 'statusCode'),
        F(_void, 'setHeader', ('name', _string), ('value', _string)),
        Var(_bool, 'headersSent'),
        Var(_bool, 'sendDate'),
        F(_string, 'getHeader', ('name', _string)),
        F(_void, 'removeHeader', ('name', _string)),
        F(_bool, 'write', ('chunk', _string), ('encoding', _string)),
        F(_void, 'addTrailers', ('headers', _object)),
        F(_void, 'end', ('data', _string), ('encoding', _string))
    ),
    F('new ClientRequest()', 'request', ('options', _object), ('callback', _function)),
    F('new ClientRequest()', 'get', ('options', _object), ('callback', _function)),
    Class('Agent').members(
        Var(_int, 'maxSockets'),
        Var(_array, 'sockets'),
        Var(_array, 'requests')
    ),
    Var('new Agent()', 'globalAgent'),
    Class('ClientRequest').prototype('event.EventEmitter').members(
        F(_void, 'write', ('chunk', _string), ('encoding', _string)),
        F(_void, 'end', ('data', _string), ('encoding', _string)),
        F(_void, 'abort'),
        F(_void, 'setTimeout', ('msecs', _int), ('callback', _function)),
        F(_void, 'setNoDelay', ('noDelay', _bool)),
        F(_void, 'setSocketKeepAlive', ('enable', _bool), ('initialDelay', _int)),
    ),
    Class('IncomingMessage').prototype('event.EventEmitter').members(
        Var(_string, 'httpVersion'),
        Var(_object, 'headers'),
        Var(_object, 'trailers'),
        F(_void, 'setTimeout', ('msecs', _int), ('callback', _function)),
        Var(_string, 'method'),
        Var(_string, 'url'),
        Var(_int, 'statusCode'),
        Var('new net.Socket()', 'socket')
    )
).print()

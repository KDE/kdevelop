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
require('tls')
require('http')

_function = 'function(){}'
_object = 'new Object()'

Module().members(
    F('new Server()', 'createServer', ('requestListener', _function)),
    Class('Server').prototype('tls.Server').members(
        F(_void, 'listen', ('port', _int), ('hostname', _string), ('backlog', _int), ('callback', _function)),
        F(_void, 'close', ('callback', _function))
    ),
    F('new http.ClientRequest()', 'request', ('options', _object), ('callback', _function)),
    F('new http.ClientRequest()', 'get', ('options', _object), ('callback', _function)),
    Class('Agent').prototype('http.Agent'),
    Var('new Agent()', 'globalAgent')
).print()

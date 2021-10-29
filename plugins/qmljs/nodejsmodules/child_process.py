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
_childprocess = 'new ChildProcess()'
_stream = 'new stream.Stream()'

Module().members(
    Class('ChildProcess').prototype('event.EventEmitter').members(
        Var(_stream, 'stdin'),
        Var(_stream, 'stdout'),
        Var(_stream, 'stderr'),
        Var(_int, 'pid'),
        Var(_bool, 'connected'),
        F(_void, 'kill', ('signal', _string)),
        F(_void, 'send', ('message', _object), ('sendHandle', _object)),
        F(_void, 'disconnect'),
    ),
    F(_childprocess, 'spawn', ('command', _string), ('args', _array), ('options', _object)),
    F(_childprocess, 'exec', ('command', _string), ('args', _array), ('callback', 'function(){}')),
    F(_childprocess, 'execFile', ('file', _string), ('args', _array), ('options', _object), ('callback', 'function(){}')),
    F(_childprocess, 'fork', ('modulePath', _string), ('args', _array), ('options', _object))
).print()

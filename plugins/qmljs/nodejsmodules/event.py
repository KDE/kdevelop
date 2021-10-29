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

_function = 'function(){}'
_object = 'new Object()'

Module().members(
    Class('EventEmitter').members(
        F(_void, 'addListener', ('event', _string), ('listener', _function)),
        F(_void, 'on', ('event', _string), ('listener', _function)),
        F(_void, 'once', ('event', _string), ('listener', _function)),
        F(_void, 'removeListener', ('event', _string), ('listener', _function)),
        F(_void, 'removeAllListeners', ('event', _string)),
        F(_void, 'setMaxListeners', ('n', _int)),
        F(_array, 'listeners', ('event', _string)),
        F(_void, 'emit', ('event', _string), ('args', _mixed)),
        F(_int, 'listenerCount', ('emitter', 'new EventEmitter()'), ('event', _string))
    )
).print()

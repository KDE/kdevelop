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

Module().members(
    F('new Domain()', 'create'),
    Class('Domain').prototype('event.EventEmitter').members(
        F(_void, 'run', ('fn', _function)),
        Var(_array, 'members'),
        F(_void, 'add', ('emitter', _object)),
        F(_void, 'remove', ('emitter', _object)),
        F(_function, 'bind', ('callback', _function)),
        F(_function, 'intercept', ('callback', _function)),
        F(_void, 'enter'),
        F(_void, 'exit'),
        F(_void, 'dispose')
    )
).print()

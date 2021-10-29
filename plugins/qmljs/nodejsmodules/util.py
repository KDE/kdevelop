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

_object = 'new Object()'
_function = 'function(){}'

Module().members(
    F(_string, 'format', ('format', _string), ('args', _mixed)),
    F(_void, 'debug', ('string', _string)),
    F(_void, 'error', ('string', _string)),
    F(_void, 'puts', ('string', _string)),
    F(_void, 'print', ('string', _string)),
    F(_void, 'log', ('string', _string)),
    F(_string, 'inspect', ('object', _object), ('options', _object)),
    F(_bool, 'isArray', ('object', _array)),
    F(_bool, 'isRegExp', ('object', 'new RegExp()')),
    F(_bool, 'isDate', ('object', 'new Date()')),
    F(_bool, 'isError', ('object', 'new Error()')),
    F(_void, 'inherits', ('constructor', _function), ('superConstructor', _function))
).print()

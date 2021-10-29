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
_context = 'new Context()'
_script = 'new Script()'

Module().members(
    F(_mixed, 'runInThisContext', ('code', _string), ('filename', _string)),
    F(_mixed, 'runInNewContext', ('code', _string), ('sandbox', _object), ('filename', _string)),
    F(_mixed, 'runInContext', ('code', _string), ('context', _context), ('filename', _string)),
    F(_context, 'createContext', ('initSandbox', _object)),
    F(_script, 'createScript', ('code', _string), ('filename', _string)),
    Class('Context'),
    Class('Script').members(
        F(_mixed, 'runInThisContext'),
        F(_mixed, 'runInNewContext', ('sandbox', _object))
    )
).print()

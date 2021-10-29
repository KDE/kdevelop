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
_function = 'new Function()'

Module().members(
    F(_string, 'stringify', ('object', _object), ('sep', _string), ('eq', _string)),
    F(_object, 'parse', ('str', _string), ('sep', _string), ('eq', _string), ('options', _object)),
    Var(_function, 'escape'),
    Var(_function, 'unescape')
).print()

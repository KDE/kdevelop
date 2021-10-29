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

Module().members(
    F(_string, 'decode', ('string', _string)),
    F(_string, 'encode', ('string', _string)),
    F(_string, 'toUnicode', ('domain', _string)),
    F(_string, 'toASCII', ('domain', _string)),
    Struct('ucs2').members(
        F(_string, 'decode', ('string', _string)),
        F(_string, 'encode', ('codePoints', _array))
    ),
    Var(_string, 'version')
).print()

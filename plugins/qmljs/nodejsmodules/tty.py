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
require('stream')

_object = 'new Object()'

Module().members(
    F(_bool, 'isatty', ('fd', _int)),
    F(_void, 'setRawMode', ('mode', _bool)),
    Class('ReadStream').prototype('stream.Readable').members(
        Var(_bool, 'isRaw'),
        F(_void, 'setRawMode', ('mode', _bool))
    ),
    Class('WriteStream').prototype('stream.Writable').members(
        Var(_int, 'columns'),
        Var(_int, 'rows')
    )
).print()

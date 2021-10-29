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

Module().members(
    F(_void, 'log', ('data', _mixed)),
    F(_void, 'info', ('data', _mixed)),
    F(_void, 'error', ('data', _mixed)),
    F(_void, 'warn', ('data', _mixed)),
    F(_void, 'dir', ('obj', _object)),
    F(_void, 'time', ('label', _string)),
    F(_void, 'timeEnd', ('label', _string)),
    F(_void, 'trace', ('label', _string)),
    F(_void, 'assert', ('expression', _bool), ('message', _string))
).print()

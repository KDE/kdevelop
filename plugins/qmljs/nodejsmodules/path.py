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
    F(_string, 'normalize', ('p', _string)),
    F(_string, 'join', ('path1', _string), ('path2', _string)),
    F(_string, 'resolve', ('path', _string)),
    F(_string, 'relative', ('from', _string), ('to', _string)),
    F(_string, 'dirname', ('p', _string)),
    F(_string, 'basename', ('p', _string), ('ext', _string)),
    F(_string, 'extname', ('p', _string)),
    Var(_string, 'sep'),
    Var(_string, 'delimiter')
).print()

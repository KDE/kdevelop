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
require('buffer')

Module().members(
    Class('StringDecoder', ('encoding', _string)).members(
        F(_string, 'write', ('buffer', 'new buffer.Buffer()')),
        F(_string, 'end')
    )
).print()

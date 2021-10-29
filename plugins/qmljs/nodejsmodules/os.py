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
    F(_string, 'tmpdir'),
    F(_string, 'endianness'),
    F(_string, 'hostname'),
    F(_string, 'type'),
    F(_string, 'platform'),
    F(_string, 'arch'),
    F(_string, 'release'),
    F(_string, 'uptime'),
    F(_array, 'loadavg'),
    F(_int, 'totalmem'),
    F(_int, 'freemem'),
    F(_array, 'cpus'),
    F(_object, 'networkInterfaces'),
    Var(_string, 'EOL')
).print()

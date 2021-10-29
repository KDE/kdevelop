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
_url = '{href: "", protocol: "", auth: "", hostname: "", port: 1, host: "", pathname: "", search: "", query: new Object(), hash: ""}'

Module().members(
    F(_url, 'parse', ('urlStr', _string), ('parseQueryString', _bool), ('slashesDenoteHost', _string)),
    F(_string, 'format', ('urlObj', _object)),
    F(_string, 'resolve', ('from', _string), ('to', _string))
).print()

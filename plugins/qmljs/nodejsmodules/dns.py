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

_function = 'function(){}'

Module().members(
    F(_void, 'lookup', ('domain', _string), ('family', _int), ('callback', _function)),
    F(_void, 'resolve', ('domain', _string), ('rrtype', _string), ('callback', _function)),
    F(_void, 'resolve4', ('domain', _string), ('callback', _function)),
    F(_void, 'resolve6', ('domain', _string), ('callback', _function)),
    F(_void, 'resolveMx', ('domain', _string), ('callback', _function)),
    F(_void, 'resolveTxt', ('domain', _string), ('callback', _function)),
    F(_void, 'resolveSrv', ('domain', _string), ('callback', _function)),
    F(_void, 'resolveNs', ('domain', _string), ('callback', _function)),
    F(_void, 'resolveCname', ('domain', _string), ('callback', _function)),
    F(_void, 'reverse', ('ip', _string), ('callback', _function)),
    Var(_int, 'NODATA'),
    Var(_int, 'FORMERR'),
    Var(_int, 'SERVFAIL'),
    Var(_int, 'NOTFOUND'),
    Var(_int, 'NOTIMP'),
    Var(_int, 'REFUSED'),
    Var(_int, 'BADQUERY'),
    Var(_int, 'BADNAME'),
    Var(_int, 'BADFAMILY'),
    Var(_int, 'BADRESP'),
    Var(_int, 'CONNREFUSED'),
    Var(_int, 'TIMEOUT'),
    Var(_int, 'EOF'),
    Var(_int, 'FILE'),
    Var(_int, 'NOMEM'),
    Var(_int, 'DESTRUCTION'),
    Var(_int, 'BADSTR'),
    Var(_int, 'BADFLAGS'),
    Var(_int, 'NONAME'),
    Var(_int, 'BADHINTS'),
    Var(_int, 'NOTINITIALIZED'),
    Var(_int, 'LOADIPHLPAPI'),
    Var(_int, 'ADDRGETNETWORKPARAMS'),
    Var(_int, 'CANCELLED')
).print()

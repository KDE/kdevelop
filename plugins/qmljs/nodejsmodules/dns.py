#!/usr/bin/python3
# -*- coding: utf-8 -*-
# This file is part of qmljs, the QML/JS language support plugin for KDevelop
# Copyright (c) 2014 Denis Steckelmacher <steckdenis@yahoo.fr>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License or (at your option) version 3 or any later version
# accepted by the membership of KDE e.V. (or its successor approved
# by the membership of KDE e.V.), which shall act as a proxy
# defined in Section 14 of version 3 of the license.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.




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

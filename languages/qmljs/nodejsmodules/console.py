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

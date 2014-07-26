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

import sys
sys.path.append('../qmlplugins')

from jsgenerator import *
from common import *

# Print the license of the generated file (the same as the one of this file)
license()
basicTypes(globals())

_function = 'function(){}'
_object = 'new Object()'

Module().members(
    Class('EventEmitter').members(
        F(_void, 'addListener', ('event', _string), ('listener', _function)),
        F(_void, 'on', ('event', _string), ('listener', _function)),
        F(_void, 'once', ('event', _string), ('listener', _function)),
        F(_void, 'removeListener', ('event', _string), ('listener', _function)),
        F(_void, 'removeAllListeners', ('event', _string)),
        F(_void, 'setMaxListeners', ('n', _int)),
        F(_array, 'listeners', ('event', _string)),
        F(_void, 'emit', ('event', _string), ('args', _mixed)),
        F(_int, 'listenerCount', ('emitter', 'new EventEmitter()'), ('event', _string))
    )
).print()

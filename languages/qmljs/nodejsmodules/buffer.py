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

_buffer = 'new Buffer()'

Module().members(
    Class('Buffer', ('size', _int)).members(
        F(_bool, 'isEncoding', ('encoding', _string)),
        F(_void, 'write', ('string', _string), ('offset', _int), ('length', _int), ('encoding', _string)),
        F(_string, 'toJSON'),
        F(_bool, 'isBuffer', ('obj', _buffer)),
        F(_int, 'byteLength', ('string', _string), ('encoding', _string)),
        F(_void, 'concat', ('list', _array), ('totalLength', _int)),
        Var(_int, 'length'),
        F(_void, 'copy', ('target', _buffer), ('targetStart', _int), ('sourceStart', _int), ('sourceEnd', _int)),
        F(_void, 'slice', ('start', _int), ('end', _int)),
        F(_int, 'readUInt8', ('offset', _int), ('noAssert', _bool)),
        F(_int, 'readUInt16LE', ('offset', _int), ('noAssert', _bool)),
        F(_int, 'readUInt16BE', ('offset', _int), ('noAssert', _bool)),
        F(_int, 'readUInt32LE', ('offset', _int), ('noAssert', _bool)),
        F(_int, 'readUInt32BE', ('offset', _int), ('noAssert', _bool)),
        F(_int, 'readInt8', ('offset', _int), ('noAssert', _bool)),
        F(_int, 'readInt16LE', ('offset', _int), ('noAssert', _bool)),
        F(_int, 'readInt16BE', ('offset', _int), ('noAssert', _bool)),
        F(_int, 'readInt32LE', ('offset', _int), ('noAssert', _bool)),
        F(_int, 'readInt32BE', ('offset', _int), ('noAssert', _bool)),
        F(_float, 'readFloatLE', ('offset', _int), ('noAssert', _bool)),
        F(_float, 'readFloatBE', ('offset', _int), ('noAssert', _bool)),
        F(_float, 'readDoubleLE', ('offset', _int), ('noAssert', _bool)),
        F(_float, 'readDoubleBE', ('offset', _int), ('noAssert', _bool)),
        F(_void, 'writeUInt8', ('value', _int), ('offset', _int), ('noAssert', _bool)),
        F(_void, 'writeUInt16LE', ('value', _int), ('offset', _int), ('noAssert', _bool)),
        F(_void, 'writeUInt16BE', ('value', _int), ('offset', _int), ('noAssert', _bool)),
        F(_void, 'writeUInt32LE', ('value', _int), ('offset', _int), ('noAssert', _bool)),
        F(_void, 'writeUInt32BE', ('value', _int), ('offset', _int), ('noAssert', _bool)),
        F(_void, 'writeInt8', ('value', _int), ('offset', _int), ('noAssert', _bool)),
        F(_void, 'writeInt16LE', ('value', _int), ('offset', _int), ('noAssert', _bool)),
        F(_void, 'writeInt16BE', ('value', _int), ('offset', _int), ('noAssert', _bool)),
        F(_void, 'writeInt32LE', ('value', _int), ('offset', _int), ('noAssert', _bool)),
        F(_void, 'writeInt32BE', ('value', _int), ('offset', _int), ('noAssert', _bool)),
        F(_void, 'writeFloatLE', ('value', _float), ('offset', _int), ('noAssert', _bool)),
        F(_void, 'writeFloatBE', ('value', _float), ('offset', _int), ('noAssert', _bool)),
        F(_void, 'writeDoubleLE', ('value', _float), ('offset', _int), ('noAssert', _bool)),
        F(_void, 'writeDoubleBE', ('value', _float), ('offset', _int), ('noAssert', _bool)),
        F(_void, 'fill', ('value', _mixed), ('offset', _int), ('end', _int)),
        Var(_int, 'INSPECT_MAX_BYTES')
    )
).print()

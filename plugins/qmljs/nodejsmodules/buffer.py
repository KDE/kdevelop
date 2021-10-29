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

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
require('buffer')

_object = 'new Object()'

Module().members(
    F(_array, 'getCiphers'),
    F(_array, 'getHashes'),
    F(_object, 'createCredentials', ('details', _object)),
    F('new Hash()', 'createHash', ('algorithm', _string)),
    Class('Hash').members(
        F(_void, 'update', ('data', _string), ('input_encoding', _string)),
        F(_string, 'digest', ('encoding', _string))
    ),
    F('new Hmac()', 'createHmac', ('algorithm', _string), ('key', _string)),
    Class('Hmac').members(
        F(_void, 'update', ('data', _string)),
        F(_string, 'digest', ('encoding', _string))
    ),
    F('new Cipher()', 'createCipher', ('algorithm', _string), ('password', _string)),
    F('new Cipher()', 'createCipheriv', ('algorithm', _string), ('password', _string), ('iv', _string)),
    Class('Cipher').members(
        F(_void, 'update', ('data', _string), ('input_encoding', _string), ('output_encoding', _string)),
        F(_string, 'final', ('output_encoding', _string)),
        F(_void, 'setAutoPadding', ('auto_padding', _bool))
    ),
    F('new Decipher()', 'createDecipher', ('algorithm', _string), ('password', _string)),
    F('new Decipher()', 'createDecipheriv', ('algorithm', _string), ('password', _string), ('iv', _string)),
    Class('Decipher').members(
        F(_void, 'update', ('data', _string), ('input_encoding', _string), ('output_encoding', _string)),
        F(_string, 'final', ('output_encoding', _string)),
        F(_void, 'setAutoPadding', ('auto_padding', _bool))
    ),
    F('new Sign()', 'createSign', ('algorithm', _string)),
    Class('Sign').members(
        F(_void, 'update', ('data', _string)),
        F(_string, 'sign', ('private_key', _string), ('output_format', _string))
    ),
    F('new Verify()', 'createVerify', ('algorithm', _string)),
    Class('Verify').members(
        F(_void, 'update', ('data', _string)),
        F(_bool, 'verify', ('object', _string), ('signature', _string), ('signature_format', _string))
    ),
    F('new DiffieHellman()', 'createDiffieHellman', ('prime_length', _int)),
    Class('DiffieHellman').members(
        F(_string, 'generateKeys', ('encoding', _string)),
        F(_string, 'computeSecret', ('other_public_key', _string), ('input_encoding', _string), ('output_encoding', _string)),
        F(_string, 'getPrime', ('encoding', _string)),
        F(_string, 'getGenerator', ('encoding', _string)),
        F(_string, 'getPublicKey', ('encoding', _string)),
        F(_string, 'getPrivateKey', ('encoding', _string)),
        F(_void, 'setPublicKey', ('public_key', _string), ('encoding', _string)),
        F(_void, 'setPrivateKey', ('private_key', _string), ('encoding', _string)),
    ),
    F('new DiffieHellman()', 'getDiffieHellman', ('group_name', _string)),
    F(_void, 'pbkdf2', ('password', _string), ('salt', _string), ('iterations', _int), ('keylen', _int), ('callback', 'function(){}')),
    F(_string, 'pbkdf2Sync', ('password', _string), ('salt', _string), ('iterations', _int), ('keylen', _int)),
    F('new buffer.Buffer()', 'randomBytes', ('size', _int), ('callback', 'function(){}')),
    F('new buffer.Buffer()', 'pseudoRandomBytes', ('size', _int), ('callback', 'function(){}')),
    Var(_string, 'DEFAULT_ENCODING')
).print()

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

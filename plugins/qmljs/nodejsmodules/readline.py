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
require('stream')
require('event')

_object = 'new Object()'
_function = 'function(){}'
_stream = 'new stream.Writable()'

Module().members(
    F('new Interface()', 'createInterface', ('options', _object)),
    Class('Interface').prototype('event.EventEmitter').members(
        F(_void, 'setPrompt', ('prompt', _string), ('length', _int)),
        F(_void, 'prompt', ('preserveCursor', _bool)),
        F(_void, 'question', ('query', _string), ('callback', _function)),
        F(_void, 'pause'),
        F(_void, 'resume'),
        F(_void, 'close'),
        F(_void, 'write', ('data', _string), ('key', _object))
    ),
    F(_void, 'cursorTo', ('stream', _stream), ('x', _int), ('y', _int)),
    F(_void, 'moveCursor', ('stream', _stream), ('dx', _int), ('dy', _int)),
    F(_void, 'clearLine', ('stream', _stream), ('dir', _int)),
    F(_void, 'clearScreenDown', ('stream', _stream))
).print()

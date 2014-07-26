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
require('stream')
require('buffer')
require('event')

_function = 'function(){}'
_object = 'new Object()'
_date = 'new Date()'
_buffer = 'new buffer.Buffer()'
_stats = 'new Stats'
_callback = ('callback', _function)
_fd = ('fd', _int)
_len = ('len', _int)
_path = ('path', _string)
_uid = ('uid', _int)
_gid = ('gid', _int)
_mode = ('mode', _int)
_offset = ('offset', _int)
_length = ('length', _int)
_position = ('position', _int)
_filename = ('filename', _string)
_options = ('options', _object)
_data = ('data', _string)

Module().members(
    F(_void, 'rename', ('oldPath', _string), ('newPath', _string), _callback),
    F(_int, 'renameSync', ('oldPath', _string), ('newPath', _string)),
    F(_void, 'ftruncate', _fd, _len, _callback),
    F(_int, 'ftruncateSync', _fd, _len),
    F(_void, 'truncate', _path, _len, _callback),
    F(_int, 'truncateSync', _path, _len),
    F(_void, 'chown', _path, _uid, _gid, _callback),
    F(_int, 'chownSync', _path, _uid, _gid),
    F(_void, 'fchown', _fd, _uid, _gid, _callback),
    F(_int, 'fchownSync', _fd, _uid, _gid),
    F(_void, 'lchown', _path, _uid, _gid, _callback),
    F(_int, 'lchownSync', _path, _uid, _gid),
    F(_void, 'chmod', _path, _mode, _callback),
    F(_int, 'chmodSync', _path, _mode),
    F(_void, 'fchmod', _fd, _mode, _callback),
    F(_int, 'fchmodSync', _fd, _mode),
    F(_void, 'lchmod', _path, _mode, _callback),
    F(_int, 'lchmodSync', _path, _mode),
    F(_void, 'stat', _path, _callback),
    F(_void, 'lstat', _path, _callback),
    F(_void, 'fstat', _fd, _callback),
    F(_stats, 'statSync', _path),
    F(_stats, 'lstatSync', _path),
    F(_stats, 'fstatSync', _fd),
    F(_void, 'link', ('srcpath', _string), ('dstpath', _string), _callback),
    F(_int, 'linkSync', ('srcpath', _string), ('dstpath', _string)),
    F(_void, 'symlink', ('srcpath', _string), ('dstpath', _string), ('type', _string), _callback),
    F(_int, 'symlinkSync', ('srcpath', _string), ('dstpath', _string), ('type', _string)),
    F(_void, 'readlink', _path, _callback),
    F(_int, 'readlinkSync', _path),
    F(_void, 'realpath', _path, ('cache', _object), _callback),
    F(_string, 'realpathSync', _path, ('cache', _object)),
    F(_void, 'unlink', _path, _callback),
    F(_int, 'unlinkSync', _path),
    F(_void, 'rmdir', _path, _callback),
    F(_int, 'rmdirSync', _path),
    F(_void, 'mkdir', _path, _mode, _callback),
    F(_int, 'mkdirSync', _path, _mode),
    F(_void, 'readdir', _path, _callback),
    F(_array, 'readdirSync', _path),
    F(_void, 'close', _fd, _callback),
    F(_int, 'closeSync', _fd),
    F(_void, 'open', _path, ('flags', _string), _mode, _callback),
    F(_int, 'openSync', _path, ('flags', _string), _mode),
    F(_void, 'utimes', _path, ('atime', _date), ('mtime', _date), _callback),
    F(_int, 'utimesSync', _path, ('atime', _date), ('mtime', _date)),
    F(_void, 'futimes', _fd, ('atime', _date), ('mtime', _date), _callback),
    F(_int, 'futimesSync', _fd, ('atime', _date), ('mtime', _date)),
    F(_void, 'fsync', _fd, _callback),
    F(_int, 'fsyncSync', _fd),
    F(_void, 'write', _fd, _buffer, _offset, _length, _position, _callback),
    F(_int, 'writeSync', _fd, _buffer, _offset, _length, _position),
    F(_void, 'read', _fd, _buffer, _offset, _length, _position, _callback),
    F(_int, 'readSync', _fd, _buffer, _offset, _length, _position),
    F(_void, 'readFile', _filename, _options, _callback),
    F(_int, 'readFileSync', _filename, _options),
    F(_void, 'writeFile', _filename, _data, _options, _callback),
    F(_int, 'writeFileSync', _filename, _data, _options),
    F(_void, 'appendFile', _filename, _data, _options, _callback),
    F(_int, 'appendFileSync', _filename, _data, _options),
    F(_void, 'watchFile', _filename, _options, _callback),
    F(_void, 'unwatchFile', _filename, _callback),
    F('new FSWatcher()', 'watch', _filename, _options, _callback),
    F(_void, 'exists', _path, _callback),
    F(_bool, 'existsSync', _path),
    Class('Stats').members(
        F(_bool, 'isFile'),
        F(_bool, 'isDirectory'),
        F(_bool, 'isBlockDevice'),
        F(_bool, 'isCharacterDevice'),
        F(_bool, 'isSymbolicLink'),
        F(_bool, 'isFIFO'),
        F(_bool, 'isSocket'),
        Var(_int, 'dev'),
        Var(_int, 'ino'),
        Var(_int, 'mode'),
        Var(_int, 'nlink'),
        Var(_int, 'uid'),
        Var(_int, 'gid'),
        Var(_int, 'rdev'),
        Var(_int, 'size'),
        Var(_int, 'blksize'),
        Var(_int, 'blocks'),
        Var(_date, 'atime'),
        Var(_date, 'mtime'),
        Var(_date, 'ctime')
    ),
    F('new ReadStream()', 'createReadStream', _path, _options),
    Class('ReadStream').prototype('stream.Readable'),
    F('new WriteStream()', 'createWriteStream', _path, _options),
    Class('WriteStream').prototype('stream.Writable'),
    Class('FSWatcher').prototype('event.EventEmitter').members(
        F(_void, 'close')
    )
).print()

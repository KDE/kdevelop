# SPDX-FileCopyrightText: 2016 The Qt Company Ltd.
#
# SPDX-License-Identifier: GPL-3.0-only WITH Qt-GPL-exception-1.0

# Modified by David Faure <faure@kde.org> (initially)
# for the needs of qt.py

import gdb

from .dumper import DumperBase

class Dumper(DumperBase):

    def __init__(self):
        DumperBase.__init__(self)

        # KDevelop: the following two non-comment lines were copied verbatim
        #           from Qt Creator's Dumper.fetchVariables().
        #           The packCode field was originally introduced in Qt Creator code to
        #           support debugging core dumps from a system with a different endianness.
        #           On the other hand, using '<' or '>' means unpacking structs
        #           with non-native size and alignment.
        #           If this causes issues, consider using '@' instead of the packCode.
        self.isBigEndian = gdb.execute('show endian', to_string=True).find('big endian') > 0
        self.packCode = '>' if self.isBigEndian else '<'

    def ptrSize(self):
        result = gdb.lookup_type('void').pointer().sizeof
        self.ptrSize = lambda: result
        return result

    def selectedInferior(self):
        try:
            # gdb.Inferior is new in gdb 7.2
            self.cachedInferior = gdb.selected_inferior()
        except:
            # Pre gdb 7.4. Right now we don't have more than one inferior anyway.
            self.cachedInferior = gdb.inferiors()[0]

        # Memoize result.
        self.selectedInferior = lambda: self.cachedInferior
        return self.cachedInferior

    def readRawMemory(self, address, size):
        #self.warn('READ: %s FROM 0x%x' % (size, address))
        if address == 0 or size == 0:
            return bytes()
        res = self.selectedInferior().read_memory(address, size)
        return res

    def extractQtVersion(self):
        try:
            # Only available with Qt 5.3+
            return int(str(gdb.parse_and_eval('((void**)&qtHookData)[2]')), 16)
        except:
            # KDevelop: we do not care about Qt 5.1 and Qt 5.2, so just return 5.0 as fallback
            (major, minor, patch) = (5, 0, 0)
            return 0x10000 * major + 0x100 * minor + patch

    # KDevelop: added this method to override the stub in DumperBase
    def lookupType(self, typename):
        return gdb.lookup_type(typename)

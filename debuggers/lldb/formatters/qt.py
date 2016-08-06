#
# LLDB data formatters for Qt types
# Copyright 2016 Aetf <aetf@unlimitedcodeworks.xyz>
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
#

import lldb

from helpers import *

def __lldb_init_module(debugger, unused):
    debugger.HandleCommand('type synthetic add QString -w kdevelop-qt -l qt.QStringFormatter')
    debugger.HandleCommand('type summary add QString -w kdevelop-qt -F qt.QStringSummaryProvider')

    debugger.HandleCommand('type summary add QChar -w kdevelop-qt -F qt.QCharSummaryProvider')

    debugger.HandleCommand('type synthetic add QByteArray -w kdevelop-qt -l qt.QByteArrayFormatter')
    debugger.HandleCommand('type summary add QByteArray -w kdevelop-qt -e -F qt.QByteArraySummaryProvider')

    debugger.HandleCommand('type synthetic add -x "^QList<.+>$" -w kdevelop-qt -l qt.QListFormatter')
    debugger.HandleCommand('type summary add -x "^QList<.+>$" -w kdevelop-qt -e -s "<size=${svar%#}>"')

    debugger.HandleCommand('type synthetic add QStringList -w kdevelop-qt -l qt.QStringListFormatter')
    debugger.HandleCommand('type summary add QStringList -w kdevelop-qt -e -s "<size=${svar%#}>"')

    debugger.HandleCommand('type synthetic add -x "^QQueue<.+>$" -w kdevelop-qt -l qt.QQueueFormatter')
    debugger.HandleCommand('type summary add -x "^QQueue<.+>$" -w kdevelop-qt -e -s "<size=${svar%#}>"')

    debugger.HandleCommand('type synthetic add -x "^QVector<.+>$" -w kdevelop-qt -l qt.QVectorFormatter')
    debugger.HandleCommand('type summary add -x "^QVector<.+>$" -w kdevelop-qt -e -s "<size=${svar%#}>"')

    debugger.HandleCommand('type synthetic add -x "^QStack<.+>$" -w kdevelop-qt -l qt.QStackFormatter')
    debugger.HandleCommand('type summary add -x "^QStack<.+>$" -w kdevelop-qt -e -s "<size=${svar%#}>"')

def dataForQString(valobj):
    pointer = 0
    length = 0
    if valobj.IsValid():
        d = valobj.GetChildMemberWithName('d')
        data = d.GetChildMemberWithName('data')
        offset = d.GetChildMemberWithName('offset')
        size = d.GetChildMemberWithName('size')

        isQt4 = data.IsValid()
        if isQt4:
            pointer = data.GetValueAsUnsigned(0)
        elif offset.IsValid():
            pointer = d.GetValueAsUnsigned(0) + offset.GetValueAsUnsigned(0)
        else:
            qarraydata_t = valobj.GetTarget().FindFirstType('QArrayData')
            if qarraydata_t.IsValid():
                pointer = d.GetValueAsUnsigned(0) + qarraydata_t.GetByteSize()
            else:
                pointer = d.GetValueAsUnsigned(0) + 24  # Fallback to hardcoded value
        # size in the number of chars, each char is 2 bytes in UTF16
        length = size.GetValueAsUnsigned(0) * 2

    return (pointer, length)


def QStringSummaryProvider(valobj, internal_dict):
    if valobj.IsValid():
        content = valobj.GetChildMemberWithName('content')
        if content.IsValid():
            return valobj.GetChildMemberWithName('content').GetSummary()
        else:
            # No synthetic provider installed, get the content by ourselves
            dataPointer, byteLength = dataForQString(valobj)
            error = lldb.SBError()
            string_data = valobj.process.ReadMemory(dataPointer, byteLength, error)
            # The QString object might be not yet initialized. In this case size is a bogus value,
            # and memory access may fail
            if error.Success():
                return '"{}"'.format(string_data.decode('utf-16').encode('utf-8').replace('"', r'\"'))
    return None


class QStringFormatter(object):
    """A lldb synthetic provider for QString"""

    def __init__(self, valobj, internal_dict):
        self.valobj = valobj
        self._size_member = None
        self._content_member = None
        self._members = []
        self._num_children = 0

        self._qchar_type = valobj.GetTarget().FindFirstType('QChar')
        self._qchar_size = self._qchar_type.GetByteSize()

    def has_children(self):
        return True

    def num_children(self):
        return self._num_children

    def get_child_index(self, name):
        if name == 'size':
            return 0
        elif name == 'content':
            return 1
        else:
            try:
                return int(name.lstrip('[').rstrip(']')) + 2
            except Exception:
                return None

    def get_child_at_index(self, idx):
        if idx < 0 or idx >= self._num_children:
            return None
        if idx == 0:
            return self._size_member
        elif idx == 1:
            return self._content_member
        else:
            return self._members[idx - 2]

    def update(self):
        self._num_children = 2
        self._members = []
        if not self.valobj.IsValid():
            self._size_member = self.valobj.CreateValueFromExpression('size', '(size_t) 0')
            self._content_member = lldb.SBValue()
            return

        dataPointer, byteLength = dataForQString(self.valobj)
        strLength = byteLength / 2
        self._num_children += strLength
        self._size_member = self.valobj.CreateValueFromExpression('size',
                                                                  '(size_t) {}'.format(strLength))

        error = lldb.SBError()
        string_data = self.valobj.process.ReadMemory(dataPointer, byteLength, error)
        # The QString object might be not yet initialized. In this case size is a bogus value,
        # and memory access may fail
        if error.Success():
            string = string_data.decode('utf-16').encode('utf-8')

            for idx in range(0, strLength):
                var = self.valobj.CreateValueFromAddress('[{}]'.format(idx),
                                                         dataPointer + idx * self._qchar_size,
                                                         self._qchar_type)
                self._members.append(var)
            self._content_member = self.valobj.CreateValueFromExpression('content',
                                                                         '(const char*) "{}"'.format(string.replace('"', r'\"')))
        else:
            self._content_member = lldb.SBValue()


def QCharSummaryProvider(valobj, internal_dict):
    if valobj.IsValid():
        ucs = valobj.GetChildMemberWithName('ucs').GetValueAsUnsigned(0)
        return u"'{}'".format(unichr(ucs)).encode('utf-8')
    return ''


def dataForQByteArray(valobj):
    pointer = 0
    length = 0
    if valobj.IsValid():
        d = valobj.GetChildMemberWithName('d')
        data = d.GetChildMemberWithName('data')
        offset = d.GetChildMemberWithName('offset')
        size = d.GetChildMemberWithName('size')

        isQt4 = data.IsValid()
        if isQt4:
            pointer = data.GetValueAsUnsigned(0)
        elif offset.IsValid():
            pointer = d.GetValueAsUnsigned(0) + offset.GetValueAsUnsigned(0)
        else:
            pointer = d.GetValueAsUnsigned(0) + 24  # Fallback to hardcoded value
        length = size.GetValueAsUnsigned(0)

    return (pointer, length)


def QByteArraySummaryProvider(valobj, internal_dict):
    if valobj.IsValid():
        content = valobj.GetChildMemberWithName('content')
        if content.IsValid():
            return valobj.GetChildMemberWithName('content').GetSummary()
        else:
            # No synthetic provider installed, get the content by ourselves
            dataPointer, byteLength = dataForQByteArray(self.valobj)
            error = lldb.SBError()
            string_data = self.valobj.process.ReadMemory(dataPointer, byteLength, error)
            # The object might be not yet initialized. In this case size is a bogus value,
            # and memory access may fail
            if error.Success():
                # replace non-ascii byte with a space and get a printable version
                string = ''.join([i if ord(i) < 128 else ' ' for i in string_data])
                return '"{}"'.format(string)
    return None

class QByteArrayFormatter(object):
    """A lldb synthetic provider for QByteArray"""

    def __init__(self, valobj, internal_dict):
        self.valobj = valobj
        self._content_member = None
        self._members = []
        self._num_children = 0

        self._char_type = valobj.GetType().GetBasicType(lldb.eBasicTypeChar)
        self._char_size = self._char_type.GetByteSize()

    def has_children(self):
        return True

    def num_children(self):
        return self._num_children

    def get_child_index(self, name):
        if name == 'size':
            return 0
        elif name == 'content':
            return 1
        else:
            try:
                return int(name.lstrip('[').rstrip(']')) + 2
            except Exception:
                return None

    def get_child_at_index(self, idx):
        if idx < 0 or idx >= self._num_children:
            return None
        if idx == 0:
            return self._size_member
        elif idx == 1:
            return self._content_member
        else:
            return self._members[idx - 2]

    def update(self):
        self._num_children = 2
        self._members = []
        if not self.valobj.IsValid():
            self._size_member = self.valobj.CreateValueFromExpression('size', '(size_t) 0')
            self._content_member = lldb.SBValue()
            return

        dataPointer, byteLength = dataForQByteArray(self.valobj)
        strLength = byteLength
        self._num_children += strLength
        self._size_member = self.valobj.CreateValueFromExpression('size',
                                                                  '(size_t) {}'.format(strLength))

        error = lldb.SBError()
        string_data = self.valobj.process.ReadMemory(dataPointer, byteLength, error)
        # The object might be not yet initialized. In this case size is a bogus value,
        # and memory access may fail
        if error.Success():
            # replace non-ascii byte with a space and get a printable version
            string = ''.join([i if ord(i) < 128 else ' ' for i in string_data])

            for idx in range(0, strLength):
                var = self.valobj.CreateValueFromAddress('[{}]'.format(idx),
                                                         dataPointer + idx * self._char_size,
                                                         self._char_type)
                self._members.append(var)
            self._content_member = self.valobj.CreateValueFromExpression('content',
                                                                         '(const char*) "{}"'.format(string.replace('"', r'\"')))
        else:
            self._content_member = lldb.SBValue()


class BasicListFormatter(object):
    """A lldb synthetic provider for QList like types"""

    def __init__(self, valobj, internal_dict, item_typename):
        self.valobj = valobj
        self._members = []
        self._num_children = 0

        if item_typename is None:
            self._item_type = valobj.GetType().GetTemplateArgumentType(0)
        else:
            self._item_type = valobj.GetTarget().FindFirstType(item_typename)
        pvoid_type = valobj.GetTarget().GetBasicType(lldb.eBasicTypeVoid).GetPointerType()
        self._pvoid_size = pvoid_type.GetByteSize()

        #from QTypeInfo::isLarge
        isLarge = self._item_type.GetByteSize() > self._pvoid_size

        #unfortunately we can't use QTypeInfo<T>::isStatic as it's all inlined, so use
        #this list of types that use Q_DECLARE_TYPEINFO(T, Q_MOVABLE_TYPE)
        #(obviously it won't work for custom types)
        movableTypes = ['QRect', 'QRectF', 'QString', 'QMargins', 'QLocale', 'QChar', 'QDate',
                        'QTime', 'QDateTime', 'QVector', 'QRegExpr', 'QPoint', 'QPointF', 'QByteArray',
                        'QSize', 'QSizeF', 'QBitArray', 'QLine', 'QLineF', 'QModelIndex',
                        'QPersitentModelIndex', 'QVariant', 'QFileInfo', 'QUrl', 'QXmlStreamAttribute',
                        'QXmlStreamNamespaceDeclaration', 'QXmlStreamNotationDeclaration',
                        'QXmlStreamEntityDeclaration', 'QPair<int, int>']
        movableTypes = [valobj.GetTarget().FindFirstType(t) for t in movableTypes]
        #this list of types that use Q_DECLARE_TYPEINFO(T, Q_PRIMITIVE_TYPE) (from qglobal.h)
        primitiveTypes = ['bool', 'char', 'signed char', 'unsigned char', 'short', 'unsigned short',
                          'int', 'unsigned int', 'long', 'unsigned long', 'long long',
                          'unsigned long long', 'float', 'double']
        primitiveTypes = [valobj.GetTarget().FindFirstType(t) for t in primitiveTypes]

        if self._item_type in movableTypes or self._item_type in primitiveTypes:
            isStatic = False
        else:
            isStatic = not self._item_type.IsPointerType()

        #see QList::Node::t()
        self._externalStorage = isLarge or isStatic
        # If is external storage, then the node (a void*) is a pointer to item
        # else the item is stored inside the node
        if self._externalStorage:
            self._node_type = self._item_type.GetPointerType()
        else:
            self._node_type = self._item_type

    def has_children(self):
        return self._num_children != 0

    def num_children(self):
        return self._num_children

    def get_child_index(self, name):
        try:
            return int(name.lstrip('[').rstrip(']'))
        except Exception:
            return None

    def get_child_at_index(self, idx):
        if idx < 0 or idx >= self._num_children:
            return None
        return self._members[idx]

    def update(self):
        self._num_children = 0
        self._members = []
        if not self.valobj.IsValid():
            return

        d = self.valobj.GetChildMemberWithName('d')
        begin = d.GetChildMemberWithName('begin').GetValueAsUnsigned(0)
        end = d.GetChildMemberWithName('end').GetValueAsUnsigned(0)
        array = d.GetChildMemberWithName('array')

        self._num_children = end - begin

        for idx in range(0, self._num_children):
            pAt = array.GetValueAsUnsigned(0) + (begin + idx) * self._pvoid_size
            name = '[{}]'.format(idx)
            var = array.CreateChildAtOffset(name, pAt, self._node_type)
            if self._externalStorage:
                # can't use var.Dereference() directly, as the returned SBValue has '*' prepended
                # to its name. And SBValue name can't be changed once constructed.
                var = self.valobj.CreateValueFromData(name, var.GetPointeeData(),
                                                      self._item_type)
            self._members.append(var)


class QListFormatter(BasicListFormatter):
    """lldb synthetic provider for QList"""

    def __init__(self, valobj, internal_dict):
        super(QListFormatter, self).__init__(valobj, internal_dict, None)


class QStringListFormatter(BasicListFormatter):
    """lldb synthetic provider for QStringList"""

    def __init__(self, valobj, internal_dict):
        super(QStringListFormatter, self).__init__(valobj, internal_dict, 'QString')


class QQueueFormatter(BasicListFormatter):
    """lldb synthetic provider for QQueue"""

    def __init__(self, valobj, internal_dict):
        super(QQueueFormatter, self).__init__(valobj.GetChildAtIndex(0), internal_dict, None)


class BasicVectorFormatter(object):
    """A lldb synthetic provider for QVector like types"""

    def __init__(self, valobj, internal_dict):
        self.valobj = valobj
        self._members = []
        self._num_children = 0

        self._item_type = valobj.GetType().GetTemplateArgumentType(0)
        self._item_size = self._item_type.GetByteSize()

    def has_children(self):
        return self._num_children != 0

    def num_children(self):
        return self._num_children

    def get_child_index(self, name):
        try:
            return int(name.lstrip('[').rstrip(']'))
        except Exception:
            return None

    def get_child_at_index(self, idx):
        if idx < 0 or idx >= self._num_children:
            return None
        return self._members[idx]

    def update(self):
        self._num_children = 0
        self._members = []
        if not self.valobj.IsValid():
            return

        d = self.valobj.GetChildMemberWithName('p')
        # Qt4 has 'p', Qt5 doesn't
        isQt4 = d.IsValid()
        if isQt4:
            pArray = d.GetChildMemberWithName('array').GetAddress().GetLoadAddress(self.valobj.GetTarget())
        else:
            d = self.valobj.GetChildMemberWithName('d')
            offset = d.GetChildMemberWithName('offset')
            pArray = d.GetValueAsUnsigned(0) + offset.GetValueAsUnsigned(0)

        self._num_children = d.GetChildMemberWithName('size').GetValueAsUnsigned(0)

        for idx in range(0, self._num_children):
            var = self.valobj.CreateValueFromAddress('[{}]'.format(idx),
                                                     pArray + idx * self._item_size,
                                                     self._item_type)
            self._members.append(var)


class QVectorFormatter(BasicVectorFormatter):
    """lldb synthetic provider for QVector"""
    def __init__(self, valobj, internal_dict):
        super(QVectorFormatter, self).__init__(valobj, internal_dict)


class QStackFormatter(BasicVectorFormatter):
    """lldb synthetic provider for QStack"""
    def __init__(self, valobj, internal_dict):
        super(QStackFormatter, self).__init__(valobj.GetChildAtIndex(0), internal_dict)

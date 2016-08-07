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

    debugger.HandleCommand('type synthetic add -x "^QLinkedList<.+>$" -w kdevelop-qt -l qt.QLinkedListFormatter')
    debugger.HandleCommand('type summary add -x "^QLinkedList<.+>$" -w kdevelop-qt -e -s "<size=${svar%#}>"')

    debugger.HandleCommand('type synthetic add -x "^QMapNode<.+>$" -w kdevelop-qt -l qt.KeyValueFormatter')
    debugger.HandleCommand('type summary add -x "^QMapNode<.+>$" -w kdevelop-qt -F qt.KeyValueSummaryProvider')

    debugger.HandleCommand('type synthetic add -x "^QMap<.+>$" -w kdevelop-qt -l qt.QMapFormatter')
    debugger.HandleCommand('type summary add -x "^QMap<.+>$" -w kdevelop-qt -e -s "<size=${svar%#}>"')

    debugger.HandleCommand('type synthetic add -x "^QMultiMap<.+>$" -w kdevelop-qt -l qt.QMultiMapFormatter')
    debugger.HandleCommand('type summary add -x "^QMultiMap<.+>$" -w kdevelop-qt -e -s "<size=${svar%#}>"')

    debugger.HandleCommand('type synthetic add -x "^QHashNode<.+>$" -w kdevelop-qt -l qt.KeyValueFormatter')
    debugger.HandleCommand('type summary add -x "^QHashNode<.+>$" -w kdevelop-qt -F qt.KeyValueSummaryProvider')

    debugger.HandleCommand('type synthetic add -x "^QHash<.+>$" -w kdevelop-qt -l qt.QHashFormatter')
    debugger.HandleCommand('type summary add -x "^QHash<.+>$" -w kdevelop-qt -e -s "<size=${svar%#}>"')

    debugger.HandleCommand('type synthetic add -x "^QMultiHash<.+>$" -w kdevelop-qt -l qt.QMultiHashFormatter')
    debugger.HandleCommand('type summary add -x "^QMultiHash<.+>$" -w kdevelop-qt -e -s "<size=${svar%#}>"')

    debugger.HandleCommand('type category enable kdevelop-qt')


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


class QLinkedListFormatter(object):
    """A lldb synthetic provider for QLinkedList"""

    def __init__(self, valobj, internal_dict):
        self.valobj = valobj
        self._members = []
        self._num_children = 0

        self._item_type = valobj.GetType().GetTemplateArgumentType(0)

    def has_children(self):
        return self._num_children != 0

    def num_children(self):
        return self._num_children

    def get_child_index(self, name):
        try:
            return int(name.lstrip('[').rstrip(']')) + 2
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
        self._num_children = d.GetChildMemberWithName('size').GetValueAsUnsigned(0)

        node = self.valobj.GetChildMemberWithName('e').GetChildMemberWithName('n')

        for idx in range(0, self._num_children):
            var = node.GetChildMemberWithName('t')
            node = node.GetChildMemberWithName('n')

            var = self.valobj.CreateValueFromData('[{}]'.format(idx),
                                                  var.GetData(),
                                                  self._item_type)
            self._members.append(var)


class KeyValueFormatter(object):
    """A lldb synthetic provider for (key,value) pair like types"""

    def __init__(self, valobj, internal_dict):
        self.valobj = valobj
        self._key_item = None
        self._val_item = None

    def num_children(self):
        return 2

    def has_children(self):
        return True

    def get_child_index(self, name):
        if name == 'key':
            return 0
        elif name == 'value':
            return 1
        return None

    def get_child_at_index(self, idx):
        if idx < 0 or idx >= 2:
            return None
        if idx == 0:
            return self._key_item
        elif idx == 1:
            return self._val_item
        return None

    def update(self):
        if not self.valobj.IsValid():
            return
        self._key_item = self.valobj.GetChildMemberWithName('key')
        self._val_item = self.valobj.GetChildMemberWithName('value')


def KeyValueSummaryProvider(valobj, internal_dict):
    if not valobj.IsValid():
        return '<Invalid>'

    key = valobj.GetChildMemberWithName('key')
    value = valobj.GetChildMemberWithName('value')
    return '({}, {})'.format(key.GetSummary(), value.GetValue())


class BasicMapFormatter(object):
    """A lldb synthetic provider for QMap like types"""

    def __init__(self, valobj, internal_dict):
        self.valobj = valobj
        self._members = []
        self._num_children = 0

        self_type = valobj.GetType()
        key_type = self_type.GetTemplateArgumentType(0)
        val_type = self_type.GetTemplateArgumentType(1)
        # the ' ' between two template arguments is significant,
        # otherwise FindFirstType returns None
        node_typename = 'QMapNode<{}, {}>'.format(key_type.GetName(), val_type.GetName())
        self._node_type = valobj.GetTarget().FindFirstType(node_typename)

        e = self.valobj.GetChildMemberWithName('e')
        self.isQt4 = e.IsValid()
        if self.isQt4:
            self._payload_size = self._qt4_calc_payload(key_type, val_type)

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


    def _qt4_calc_payload(self, key_type, val_type):
        """calculate payload size for Qt4"""
        str = lldb.SBStream()
        self.valobj.GetExpressionPath(str, True)
        expr = '{}.payload()'.format(str.GetData())
        ret = lldb.frame.EvaluateExpression(expr).GetValueAsUnsigned(0)
        if ret != 0:
            print 'func call succeed'
            return ret
        else:
            #if the inferior function call didn't work, let's try to calculate ourselves
            target = self.valobj.GetTarget()
            pvoid_type = target.GetBasicType(lldb.eBasicTypeVoid).GetPointerType()
            pvoid_size = pvoid_type.GetByteSize()

            #we can't use QMapPayloadNode as it's inlined
            #as a workaround take the sum of sizeof(members)
            ret = key_type.GetByteSize()
            ret += val_type.GetByteSize()
            ret += pvoid_size

            #but because of data alignment the value can be higher
            #so guess it's aliged by sizeof(void*)
            #TODO: find a real solution for this problem
            ret += ret % pvoid_size

            #for some reason booleans are different
            if val_type == target.GetBasicType(lldb.eBasicTypeBool):
                ret += 2

            ret -= pvoid_size
            return ret


    class _iteratorQt4(Iterator):
        """Map iterator for Qt4"""
        def __init__(self, headerobj, node_type, payload_size):
            self.current = headerobj.GetChildMemberWithName('forward').GetChildAtIndex(0)
            self.header_addr = headerobj.GetValueAsUnsigned(0)
            self.node_type = node_type
            self.payload_size = payload_size

        def __iter__(self):
            return self

        def concrete(self, pdata_node):
            pnode_addr = pdata_node.GetValueAsUnsigned(0)
            pnode_addr -= self.payload_size

            node = self.current.CreateValueFromAddress(None, pnode_addr, self.node_type)
            return node.AddressOf()

        def __next__(self):
            if self.current.GetValueAsUnsigned(0) == self.header_addr:
                raise StopIteration
            pnode = self.concrete(self.current)
            self.current = self.current.GetChildMemberWithName('forward').GetChildAtIndex(0)
            return pnode


    class _iteratorQt5(Iterator):
        """Map iterator for Qt5"""
        def __init__(self, dataobj, pnode_type):
            self.pnode_type = pnode_type
            self.root = dataobj.GetChildMemberWithName('header')
            self.current = lldb.SBValue()

            # We store the path here to avoid keeping re-fetching
            # values from the inferior (also, skip the pointer
            # arithmetic involved in using the parent pointer
            self.path = []

        def __iter__(self):
            return self

        def moveToNextNode(self):
            def isNullPointer(val):
                return not val.IsValid() or val.GetValueAsUnsigned(0) == 0

            if isNullPointer(self.current):
                # find the leftmost node
                left = self.root.GetChildMemberWithName('left')
                if isNullPointer(left):
                    return False
                self.current = self.root
                while not isNullPointer(left):
                    self.path.append(self.current)
                    self.current = left
                    left = self.current.GetChildMemberWithName('left')
            else:
                right = self.current.GetChildMemberWithName('right')
                if not isNullPointer(right):
                    self.path.append(self.current)
                    self.current = right
                    left = self.current.GetChildMemberWithName('left')
                    while not isNullPointer(left):
                        self.path.append(self.current)
                        self.current = left
                        left = self.current.GetChildMemberWithName('left')
                else:
                    last = self.current
                    self.current = self.path.pop()
                    right = self.current.GetChildMemberWithName('right')
                    while right.GetValueAsUnsigned(0) == last.GetValueAsUnsigned(0):
                        last = self.current
                        self.current = self.path.pop()
                        right = self.current.GetChildMemberWithName('right')
                    # if there are no more parents, we are at the root
                    if len(self.path) == 0:
                        return False
            return True

        def __next__(self):
            if not self.moveToNextNode():
                raise StopIteration
            return self.current.Cast(self.pnode_type)

    def update(self):
        #self.valobj.SetPreferSyntheticValue(False)
        self._num_children = 0
        self._members = []
        if not self.valobj.IsValid():
            return

        pnode_type = self._node_type.GetPointerType()
        if self.isQt4:
            e = self.valobj.GetChildMemberWithName('e')
            it = self._iteratorQt4(e, self._node_type, self._payload_size)
        else:
            d = self.valobj.GetChildMemberWithName('d')
            it = self._iteratorQt5(d, pnode_type)

        for pnode in it:
            # dereference node and change to a user friendly name
            name = '[{}]'.format(self._num_children)
            self._num_children += 1
            var = self.valobj.CreateValueFromData(name, pnode.GetPointeeData(),
                                                  self._node_type)
            self._members.append(var)


class QMapFormatter(BasicMapFormatter):
    """lldb synthethic provider for QMap"""

    def __init__(self, valobj, internal_dict):
        super(QMapFormatter, self).__init__(valobj, internal_dict)


class QMultiMapFormatter(BasicMapFormatter):
    """lldb synthethic provider for QMap"""

    def __init__(self, valobj, internal_dict):
        super(QMultiMapFormatter, self).__init__(valobj.GetChildAtIndex(0), internal_dict)


class BasicHashFormatter(object):
    """A lldb synthetic provider for QHash like types"""

    def __init__(self, valobj, internal_dict):
        self.valobj = valobj
        self._members = []
        self._num_children = 0

        self_type = valobj.GetType()
        key_type = self_type.GetTemplateArgumentType(0)
        val_type = self_type.GetTemplateArgumentType(1)
        # the ' ' between two template arguments is significant,
        # otherwise FindFirstType returns None
        node_typename = 'QHashNode<{}, {}>'.format(key_type.GetName(), val_type.GetName())
        self._node_type = valobj.GetTarget().FindFirstType(node_typename)

    def has_children(self):
        return self._num_children != 0

    def num_children(self):
        return self._num_children

    def get_child_index(self, name):
        try:
            return int(name.lstrip('[').rstrip(']')) + 2
        except Exception:
            return None

    def get_child_at_index(self, idx):
        if idx < 0 or idx >= self._num_children:
            return None
        return self._members[idx]

    class _iterator(Iterator):
        """Hash iterator"""
        def __init__(self, valobj, pnode_type):
            d = valobj.GetChildMemberWithName('d')
            self.buckets = d.GetChildMemberWithName('buckets')
            self.null_node = valobj.GetChildMemberWithName('e')
            self.pnode_type = pnode_type

            self.num_buckets = d.GetChildMemberWithName('numBuckets').GetValueAsUnsigned(0)
            self.current = self.firstNode()

        def __iter__(self):
            return self

        def findNode(self, start=0):
            """Iterate through buckets, start at `start`,
               return any bucket the is not the null_node, or the null_node itself if nothing found.
               adapted from QHashData::fisrtNode
            """
            null_node_addr = self.null_node.GetValueAsUnsigned(0)
            for idx in range(start, self.num_buckets):
                # self.buckets has type QHashData::Node**, not an array
                # calling GetChildAtIndex with use_synthetic=True so the pointer is used as an array
                bucket = self.buckets.GetChildAtIndex(idx, lldb.eDynamicCanRunTarget, True)
                if bucket.GetValueAsUnsigned(0) != null_node_addr:
                    # in Qt4, QHashData::Node is incomplete type, but QHashNode is complete,
                    # so always use QHashNode
                    return bucket.Cast(self.pnode_type)
            return self.null_node

        def firstNode(self):
            return self.findNode()

        def moveToNextNode(self):
            """Get the nextNode after the current, see also QHashData::nextNode()."""
            next = self.current.GetChildMemberWithName('next')

            if next.GetValueAsUnsigned(0) != self.null_node.GetValueAsUnsigned(0):
                self.current = next
            else:
                h = self.current.GetChildMemberWithName('h').GetValueAsUnsigned(0)
                start = (h % self.num_buckets) + 1
                self.current = self.findNode(start)

        def __next__(self):
            if self.current.GetValueAsUnsigned(0) == self.null_node.GetValueAsUnsigned(0):
                raise StopIteration
            pnode = self.current
            self.moveToNextNode()
            return pnode

    def update(self):
        self._num_children = 0
        self._members = []
        if not self.valobj.IsValid():
            return

        for pnode in self._iterator(self.valobj, self._node_type.GetPointerType()):
            # dereference node and change to a user friendly name
            name = '[{}]'.format(self._num_children)
            self._num_children += 1
            var = self.valobj.CreateValueFromData(name, pnode.GetPointeeData(),
                                                  self._node_type)
            self._members.append(var)


class QHashFormatter(BasicHashFormatter):
    """lldb synthethic provider for QHash"""

    def __init__(self, valobj, internal_dict):
        super(QHashFormatter, self).__init__(valobj, internal_dict)

class QMultiHashFormatter(BasicHashFormatter):
    """lldb synthethic provider for QHash"""

    def __init__(self, valobj, internal_dict):
        super(QMultiHashFormatter, self).__init__(valobj.GetChildAtIndex(0), internal_dict)
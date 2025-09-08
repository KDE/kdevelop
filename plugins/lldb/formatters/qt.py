#
# LLDB data formatters for Qt types
#
# SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
#

from __future__ import print_function

import time
import datetime as dt
import string
from urllib.parse import urlsplit, urlunsplit

import locale
import lldb

from helpers import (HiddenMemberProvider, quote, unquote, unichr, toSBPointer, Iterator, validAddr,
                     validPointer, invoke, rename, canonicalized_type_name)


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

    debugger.HandleCommand('type synthetic add -x "^QSet<.+>$" -w kdevelop-qt -l qt.QSetFormatter')
    debugger.HandleCommand('type summary add -x "^QSet<.+>$" -w kdevelop-qt -e -s "<size=${svar%#}>"')

    debugger.HandleCommand('type synthetic add QDate -w kdevelop-qt -l qt.QDateFormatter')
    debugger.HandleCommand('type summary add QDate -w kdevelop-qt -e -F qt.QDateSummaryProvider')

    debugger.HandleCommand('type synthetic add QTime -w kdevelop-qt -l qt.QTimeFormatter')
    debugger.HandleCommand('type summary add -x QTime -w kdevelop-qt -e -F qt.QTimeSummaryProvider')

    debugger.HandleCommand('type synthetic add QDateTime -w kdevelop-qt -l qt.QDateTimeFormatter')
    debugger.HandleCommand('type summary add -x QDateTime -w kdevelop-qt -e -F qt.QDateTimeSummaryProvider')

    debugger.HandleCommand('type synthetic add QUrl -w kdevelop-qt -l qt.QUrlFormatter')
    debugger.HandleCommand('type summary add QUrl -w kdevelop-qt -e -F qt.QUrlSummaryProvider')

    debugger.HandleCommand('type synthetic add QUuid -w kdevelop-qt -l qt.QUuidFormatter')
    debugger.HandleCommand('type summary add QUuid -w kdevelop-qt -F qt.QUuidSummaryProvider')

    debugger.HandleCommand('type category enable kdevelop-qt')


def printableQString(valobj):
    pointer = 0
    length = 0
    if valobj.IsValid():
        d = valobj.GetChildMemberWithName('d')
        data = d.GetChildMemberWithName('data')
        offset = d.GetChildMemberWithName('offset')
        size = d.GetChildMemberWithName('size')

        isQt4 = data.IsValid()
        size_val = size.GetValueAsSigned(-1)
        alloc = d.GetChildMemberWithName('alloc').GetValueAsUnsigned(0)
        if isQt4:
            alloc += 1

        # some sanity check to see if we are dealing with garbage
        if size_val < 0 or size_val >= alloc:
            return None, 0, 0

        tooLarge = u''
        if size_val > HiddenMemberProvider._capping_size():
            tooLarge = u'...'
            size_val = HiddenMemberProvider._capping_size()

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
        length = size_val * 2
        if length == 0:
            return u'', pointer, length

        try:
            error = lldb.SBError()
            string_data = valobj.process.ReadMemory(pointer, length, error)
            # The QString object might be not yet initialized. In this case size is a bogus value,
            # and memory access may fail
            if error.Success():
                content = string_data.decode('utf-16')
                return content + tooLarge, pointer, length
        except:
            pass
    return None, 0, 0


def QStringSummaryProvider(valobj, internal_dict):
    if valobj.IsValid():
        # content = valobj.GetChildMemberWithName('(content)')
        # if content.IsValid():
        #     try:
        #         error = lldb.SBError()
        #         rawprintable = content.GetData().GetString(error, 0)
        #         if error.Success():
        #             printable = rawprintable.decode()
        #             return quote(printable)
        #     except:
        #         pass

        # FIXME: there's no reliable way to pass data from formatter to
        # summary provider currently. So directly pull data from inferior

        # Something wrong with synthetic provider, or
        # no synthetic provider installed, get the content by ourselves
        printable, _, _ = printableQString(valobj)
        if printable is not None:
            return quote(printable)
    return '<Invalid>'


class QStringFormatter(HiddenMemberProvider):
    """A lldb synthetic provider for QString"""

    def __init__(self, valobj, internal_dict):
        super(QStringFormatter, self).__init__(valobj, internal_dict)
        self._qchar_type = valobj.GetTarget().FindFirstType('QChar')
        self._qchar_size = self._qchar_type.GetByteSize()

    def _update(self):
        printable, dataPointer, byteLength = printableQString(self.valobj)
        strLength = int(byteLength / 2)

        if printable is not None:
            for idx in range(0, strLength):
                var = self.valobj.CreateValueFromAddress('[{}]'.format(idx),
                                                         dataPointer + idx * self._qchar_size,
                                                         self._qchar_type)
                self._addChild(var)
            self._num_children = strLength


def QCharSummaryProvider(valobj, internal_dict):
    if valobj.IsValid():
        ucs = valobj.GetChildMemberWithName('ucs').GetValueAsUnsigned(0)
        if ucs == 39:
            # for '\'', python returns "'" rather than '\''
            return u"'\\''"
        else:
            return unichr(ucs).__repr__()[1:]
    return None


def printableQByteArray(valobj):
    if valobj.IsValid():
        d = valobj.GetChildMemberWithName('d')
        data = d.GetChildMemberWithName('data')
        offset = d.GetChildMemberWithName('offset')
        size = d.GetChildMemberWithName('size')

        isQt4 = data.IsValid()
        size_val = size.GetValueAsSigned(-1)
        alloc = d.GetChildMemberWithName('alloc').GetValueAsUnsigned(0)
        if isQt4:
            alloc += 1

        # sanity check
        if size_val < 0 or size_val >= alloc:
            return None, 0, 0

        tooLarge = u''
        if size_val > HiddenMemberProvider._capping_size():
            tooLarge = u'...'
            size_val = HiddenMemberProvider._capping_size()

        if isQt4:
            pointer = data.GetValueAsUnsigned(0)
        elif offset.IsValid():
                pointer = d.GetValueAsUnsigned(0) + offset.GetValueAsUnsigned(0)
        else:
            pointer = d.GetValueAsUnsigned(0) + 24  # Fallback to hardcoded value

        length = size_val
        if length == 0:
            return u'', pointer, length

        try:
            error = lldb.SBError()
            string_data = valobj.process.ReadMemory(pointer, length, error)
            # The object might be not yet initialized. In this case size is a bogus value,
            # and memory access may fail
            if error.Success():
                # replace non-ascii byte with a space and get a printable version
                ls = list(string_data)
                for idx in range(length):
                    if ls[idx] in string.printable:
                        if ls[idx] != "'":
                            # convert tab, nl, ..., and '\\' to r'\\'
                            ls[idx] = ls[idx].__repr__()[1:-1]
                    else:
                        ls[idx] = r'\x{:02x}'.format(ord(ls[idx]))
                content = u''.join(ls)
                return content + tooLarge, pointer, length
        except:
            pass
    return None, 0, 0


def QByteArraySummaryProvider(valobj, internal_dict):
    if valobj.IsValid():
        content = valobj.GetChildMemberWithName('(content)')
        if content.IsValid():
            summary = content.GetSummary()
            if summary is not None:
                # unlike QString, we quoted the (content) twice to preserve our own quotation,
                # must undo the quotation done by GetSummary
                return 'b' + unquote(summary)
        # Something wrong with our synthetic provider, get the content by ourselves
        printable, _, _ = printableQByteArray(valobj)
        if printable is not None:
            # first replace " to \", and surround by "", no need to escape other things which
            # are handled in printableQByteArray.
            return 'b"{}"'.format(printable.replace('"', '\\"'))
    return '<Invalid>'


class QByteArrayFormatter(HiddenMemberProvider):
    """A lldb synthetic provider for QByteArray"""

    def __init__(self, valobj, internal_dict):
        super(QByteArrayFormatter, self).__init__(valobj, internal_dict)
        self._char_type = valobj.GetType().GetBasicType(lldb.eBasicTypeChar)
        self._char_size = self._char_type.GetByteSize()

    def _update(self):
        printable, dataPointer, byteLength = printableQByteArray(self.valobj)
        self._num_children = byteLength

        if printable is not None:
            for idx in range(0, self._num_children):
                var = self.valobj.CreateValueFromAddress('[{}]'.format(idx),
                                                         dataPointer + idx * self._char_size,
                                                         self._char_type)
                self._addChild(var)

            # first replace " to \", and surround by "", no need to escape other things which
            # are handled in printableQByteArray.
            printable = b'"{}"'.format(printable.replace(b'"', b'\\"'))
            self._addChild(('(content)', printable), hidden=True)


class BasicListFormatter(HiddenMemberProvider):
    """A lldb synthetic provider for QList like types"""

    def __init__(self, valobj, internal_dict, item_typename):
        super(BasicListFormatter, self).__init__(valobj, internal_dict)
        if item_typename is None:
            self._item_type = valobj.GetType().GetTemplateArgumentType(0)
        else:
            self._item_type = valobj.GetTarget().FindFirstType(item_typename)
        pvoid_type = valobj.GetTarget().GetBasicType(lldb.eBasicTypeVoid).GetPointerType()
        self._pvoid_size = pvoid_type.GetByteSize()

        # from QTypeInfo::isLarge
        isLarge = self._item_type.GetByteSize() > self._pvoid_size

        # unfortunately we can't use QTypeInfo<T>::isStatic as it's all inlined, so use
        # this list of types that use Q_DECLARE_TYPEINFO(T, Q_MOVABLE_TYPE)
        # (obviously it won't work for custom types)
        movableTypes = ['QRect', 'QRectF', 'QString', 'QMargins', 'QLocale', 'QChar', 'QDate',
                        'QTime', 'QDateTime', 'QVector', 'QRegExpr', 'QPoint', 'QPointF', 'QByteArray',
                        'QSize', 'QSizeF', 'QBitArray', 'QLine', 'QLineF', 'QModelIndex',
                        'QPersitentModelIndex', 'QVariant', 'QFileInfo', 'QUrl', 'QXmlStreamAttribute',
                        'QXmlStreamNamespaceDeclaration', 'QXmlStreamNotationDeclaration',
                        'QXmlStreamEntityDeclaration', 'QPair<int, int>']
        movableTypes = [valobj.GetTarget().FindFirstType(t) for t in movableTypes]
        # this list of types that use Q_DECLARE_TYPEINFO(T, Q_PRIMITIVE_TYPE) (from qglobal.h)
        primitiveTypes = ['bool', 'char', 'signed char', 'unsigned char', 'short', 'unsigned short',
                          'int', 'unsigned int', 'long', 'unsigned long', 'long long',
                          'unsigned long long', 'float', 'double']
        primitiveTypes = [valobj.GetTarget().FindFirstType(t) for t in primitiveTypes]

        if self._item_type in movableTypes or self._item_type in primitiveTypes:
            isStatic = False
        else:
            isStatic = not self._item_type.IsPointerType()

        # see QList::Node::t()
        self._externalStorage = isLarge or isStatic
        # If is external storage, then the node (a void*) is a pointer to item
        # else the item is stored inside the node
        if self._externalStorage:
            self._node_type = self._item_type.GetPointerType()
        else:
            self._node_type = self._item_type

    def _update(self):
        d = self.valobj.GetChildMemberWithName('d')
        begin = d.GetChildMemberWithName('begin').GetValueAsSigned(-1)
        end = d.GetChildMemberWithName('end').GetValueAsSigned(-1)
        array = d.GetChildMemberWithName('array')

        # sanity check
        if begin < 0 or end < 0 or end < begin:
            return

        self._num_children = end - begin

        for idx in range(0, self._num_children):
            offset = (begin + idx) * self._pvoid_size
            name = '[{}]'.format(idx)
            var = array.CreateChildAtOffset(name, offset, self._node_type)
            if self._externalStorage:
                # can't use var.Dereference() directly, as the returned SBValue has '*' prepended
                # to its name. And SBValue name can't be changed once constructed.
                var = self.valobj.CreateValueFromData(name, var.GetPointeeData(),
                                                      self._item_type)
            self._addChild(var)


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
        self.actualobj = valobj

    def update(self):
        self.valobj = self.actualobj.GetChildAtIndex(0)
        super(QQueueFormatter, self).update()


class BasicVectorFormatter(HiddenMemberProvider):
    """A lldb synthetic provider for QVector like types"""

    def __init__(self, valobj, internal_dict):
        super(BasicVectorFormatter, self).__init__(valobj, internal_dict)
        self._item_type = valobj.GetType().GetTemplateArgumentType(0)
        self._item_size = self._item_type.GetByteSize()

    def _update(self):
        d = self.valobj.GetChildMemberWithName('p')
        # Qt4 has 'p', Qt5 doesn't
        isQt4 = d.IsValid()
        if isQt4:
            pArray = d.GetChildMemberWithName('array').AddressOf().GetValueAsUnsigned(0)
        else:
            d = self.valobj.GetChildMemberWithName('d')
            offset = d.GetChildMemberWithName('offset')
            pArray = d.GetValueAsUnsigned(0) + offset.GetValueAsUnsigned(0)

        # sanity check
        if not toSBPointer(self.valobj, pArray, self._item_type).IsValid():
            return

        # self._num_children = d.GetChildMemberWithName('size').GetValueAsUnsigned(0)
        self._num_children = d.GetChildMemberWithName('size').GetValueAsSigned(-1)
        if self._num_children < 0:
            return

        if self._num_children > self._capping_size():
            self._num_children = self._capping_size()

        for idx in range(0, self._num_children):
            var = self.valobj.CreateValueFromAddress('[{}]'.format(idx),
                                                     pArray + idx * self._item_size,
                                                     self._item_type)
            self._addChild(var)


class QVectorFormatter(BasicVectorFormatter):
    """lldb synthetic provider for QVector"""
    def __init__(self, valobj, internal_dict):
        super(QVectorFormatter, self).__init__(valobj, internal_dict)


class QStackFormatter(BasicVectorFormatter):
    """lldb synthetic provider for QStack"""

    def __init__(self, valobj, internal_dict):
        super(QStackFormatter, self).__init__(valobj.GetChildAtIndex(0), internal_dict)
        self.actualobj = valobj

    def update(self):
        self.valobj = self.actualobj.GetChildAtIndex(0)
        super(QStackFormatter, self).update()


class QLinkedListFormatter(HiddenMemberProvider):
    """A lldb synthetic provider for QLinkedList"""

    def __init__(self, valobj, internal_dict):
        super(QLinkedListFormatter, self).__init__(valobj, internal_dict)
        self._item_type = valobj.GetType().GetTemplateArgumentType(0)

    def _update(self):
        d = self.valobj.GetChildMemberWithName('d')
        self._num_children = d.GetChildMemberWithName('size').GetValueAsSigned(-1)

        if self._num_children < 0:
            return

        node = self.valobj.GetChildMemberWithName('e').GetChildMemberWithName('n')

        for idx in range(0, self._num_children):
            if not node.IsValid():
                self._members = []
                self._num_children = 0
                return
            var = node.GetChildMemberWithName('t')
            node = node.GetChildMemberWithName('n')

            var = self.valobj.CreateValueFromData('[{}]'.format(idx),
                                                  var.GetData(),
                                                  self._item_type)
            self._addChild(var)


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
        return None

    key = valobj.GetChildMemberWithName('key')
    value = valobj.GetChildMemberWithName('value')
    key_summary = key.GetSummary() or key.GetValue()  # show value if summary is empty or None
    val_summary = value.GetSummary() or value.GetValue()  # show value if summary is empty or None
    return '({}, {})'.format(key_summary, val_summary)


class BasicMapFormatter(HiddenMemberProvider):
    """A lldb synthetic provider for QMap like types"""

    def __init__(self, valobj, internal_dict):
        super(BasicMapFormatter, self).__init__(valobj, internal_dict)
        self_type = valobj.GetType()
        key_type = self_type.GetTemplateArgumentType(0)
        val_type = self_type.GetTemplateArgumentType(1)
        # the ' ' between two template arguments is significant,
        # otherwise FindFirstType returns None
        node_typename = 'QMapNode<{}, {}>'.format(key_type.GetName(), val_type.GetName())
        node_typename = canonicalized_type_name(node_typename)
        self._node_type = valobj.GetTarget().FindFirstType(node_typename)

        e = self.valobj.GetChildMemberWithName('e')
        self.isQt4 = e.IsValid()
        if self.isQt4:
            self._payload_size = self._qt4_calc_payload(key_type, val_type)

    def _qt4_calc_payload(self, key_type, val_type):
        """calculate payload size for Qt4"""
        str = lldb.SBStream()
        self.valobj.GetExpressionPath(str, True)
        expr = '{}.payload()'.format(str.GetData())
        ret = lldb.frame.EvaluateExpression(expr).GetValueAsUnsigned(0)
        if ret != 0:
            return ret
        else:
            # if the inferior function call didn't work, let's try to calculate ourselves
            target = self.valobj.GetTarget()
            pvoid_type = target.GetBasicType(lldb.eBasicTypeVoid).GetPointerType()
            pvoid_size = pvoid_type.GetByteSize()

            # we can't use QMapPayloadNode as it's inlined
            # as a workaround take the sum of sizeof(members)
            ret = key_type.GetByteSize()
            ret += val_type.GetByteSize()
            ret += pvoid_size

            # but because of data alignment the value can be higher
            # so guess it's aligned by sizeof(void*)
            # TODO: find a real solution for this problem
            ret += ret % pvoid_size

            # for some reason booleans are different
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

            # sanity check
            self.is_garbage = False
            if not validAddr(headerobj, self.header_addr):
                self.is_garbage = True
            if not validPointer(self.current):
                self.is_garbage = True

        def __iter__(self):
            return self

        def concrete(self, pdata_node):
            pnode_addr = pdata_node.GetValueAsUnsigned(0)
            pnode_addr -= self.payload_size

            return toSBPointer(self.current, pnode_addr, self.node_type)

        def __next__(self):
            if self.is_garbage:
                raise StopIteration
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

    def _update(self):
        pnode_type = self._node_type.GetPointerType()
        if self.isQt4:
            e = self.valobj.GetChildMemberWithName('e')
            it = self._iteratorQt4(e, self._node_type, self._payload_size)
        else:
            d = self.valobj.GetChildMemberWithName('d')
            it = self._iteratorQt5(d, pnode_type)

        self._num_children = 0
        for pnode in it:
            # dereference node and change to a user friendly name
            name = '[{}]'.format(self._num_children)
            self._num_children += 1
            var = self.valobj.CreateValueFromData(name, pnode.GetPointeeData(),
                                                  self._node_type)
            self._addChild(var)


class QMapFormatter(BasicMapFormatter):
    """lldb synthetic provider for QMap"""

    def __init__(self, valobj, internal_dict):
        super(QMapFormatter, self).__init__(valobj, internal_dict)


class QMultiMapFormatter(BasicMapFormatter):
    """lldb synthetic provider for QMap"""

    def __init__(self, valobj, internal_dict):
        super(QMultiMapFormatter, self).__init__(valobj.GetChildAtIndex(0), internal_dict)
        self.actualobj = valobj

    def update(self):
        self.valobj = self.actualobj.GetChildAtIndex(0)
        super(QMultiMapFormatter, self).update()


class BasicHashFormatter(HiddenMemberProvider):
    """A lldb synthetic provider for QHash like types"""

    def __init__(self, valobj, internal_dict):
        super(BasicHashFormatter, self).__init__(valobj, internal_dict)
        self_type = valobj.GetType()
        self._key_type = self_type.GetTemplateArgumentType(0)
        self._val_type = self_type.GetTemplateArgumentType(1)
        node_typename = 'QHashNode<{}, {}>'.format(self._key_type.GetName(),
                                                   self._val_type.GetName())
        node_typename = canonicalized_type_name(node_typename)

        self._node_type = valobj.GetTarget().FindFirstType(node_typename)

    class _iterator(Iterator):
        """Hash iterator"""
        def __init__(self, valobj, pnode_type):
            d = valobj.GetChildMemberWithName('d')
            self.buckets = d.GetChildMemberWithName('buckets')
            self.null_node = valobj.GetChildMemberWithName('e')
            self.pnode_type = pnode_type

            self.num_buckets = d.GetChildMemberWithName('numBuckets').GetValueAsSigned(-1)

            self.is_garbage = False
            if self.num_buckets < -1:
                self.is_garbage = True
                return
            self.current = self.firstNode()

        def __iter__(self):
            return self

        def findNode(self, start=0):
            """Iterate through buckets, start at `start`,
               return any bucket the is not the null_node, or the null_node itself if nothing found.
               adapted from QHashData::firstNode
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
            if self.is_garbage:
                raise StopIteration
            if self.current.GetValueAsUnsigned(0) == self.null_node.GetValueAsUnsigned(0):
                raise StopIteration
            pnode = self.current
            self.moveToNextNode()
            return pnode

    def _update(self):
        self._num_children = self.valobj.GetChildMemberWithName('d').GetChildMemberWithName('size').GetValueAsSigned(-1)
        if self._num_children < 0:
            return

        idx = 0
        for pnode in self._iterator(self.valobj, self._node_type.GetPointerType()):
            if idx >= self._num_children:
                self._members = []
                self._num_children = 0
                break
            # dereference node and change to a user friendly name
            name = '[{}]'.format(idx)
            idx += 1
            var = self.valobj.CreateValueFromData(name, pnode.GetPointeeData(),
                                                  self._node_type)
            self._addChild(var)
        if idx != self._num_children:
            self._members = []
            self._num_children = 0


class QHashFormatter(BasicHashFormatter):
    """lldb synthetic provider for QHash"""

    def __init__(self, valobj, internal_dict):
        super(QHashFormatter, self).__init__(valobj, internal_dict)


class QMultiHashFormatter(BasicHashFormatter):
    """lldb synthetic provider for QHash"""

    def __init__(self, valobj, internal_dict):
        super(QMultiHashFormatter, self).__init__(valobj.GetChildAtIndex(0), internal_dict)
        self.actualobj = valobj

    def update(self):
        self.valobj = self.actualobj.GetChildAtIndex(0)
        super(QMultiHashFormatter, self).update()


class QSetFormatter(HiddenMemberProvider):
    """lldb synthetic provider for QSet"""

    def __init__(self, valobj, internal_dict):
        super(QSetFormatter, self).__init__(valobj, internal_dict)
        self._hash_formatter = QHashFormatter(valobj.GetChildMemberWithName('q_hash'),
                                              internal_dict)

    def num_children(self):
        return self._num_children

    def _update(self):
        self._hash_formatter.valobj = self.valobj.GetChildMemberWithName('q_hash')
        self._hash_formatter.update()

        self._num_children = 0
        for node in self._hash_formatter._members:
            keydata = node.GetChildMemberWithName('key').GetData()
            name = '[{}]'.format(self._num_children)
            var = self.valobj.CreateValueFromData(name, keydata, self._hash_formatter._key_type)
            self._addChild(var)
            self._num_children += 1


class QDateFormatter(HiddenMemberProvider):
    """lldb synthetic provider for QDate"""
    def __init__(self, valobj, internal_dict):
        super(QDateFormatter, self).__init__(valobj, internal_dict)
        self._add_original = False
        self._qstring_type = valobj.GetTarget().FindFirstType('QString')

    def has_children(self):
        return True

    @staticmethod
    def parse(julianDay):
        """Copied from Qt srources"""
        if julianDay == 0:
            return None
        if julianDay >= 2299161:
            # Gregorian calendar starting from October 15, 1582
            # This algorithm is from Henry F. Fliegel and Thomas C. Van Flandern
            ell = julianDay + 68569
            n = (4 * ell) / 146097
            ell = ell - (146097 * n + 3) / 4
            i = (4000 * (ell + 1)) / 1461001
            ell = ell - (1461 * i) / 4 + 31
            j = (80 * ell) / 2447
            d = ell - (2447 * j) / 80
            ell = j / 11
            m = j + 2 - (12 * ell)
            y = 100 * (n - 49) + i + ell
        else:
            # Julian calendar until October 4, 1582
            # Algorithm from Frequently Asked Questions about Calendars by Claus Toendering
            julianDay += 32082
            dd = (4 * julianDay + 3) / 1461
            ee = julianDay - (1461 * dd) / 4
            mm = ((5 * ee) + 2) / 153
            d = ee - (153 * mm + 2) / 5 + 1
            m = mm + 3 - 12 * (mm / 10)
            y = dd - 4800 + (mm / 10)
            if y <= 0:
                return None
        return dt.date(y, m, d)

    def _update(self):
        # FIXME: Calling functions returns incorrect SBValue for complex type in lldb
        # # toString
        # res = invoke(self.valobj, 'toString', '0')
        # self._addChild(rename('toString', res))

        # jd
        julianDay = self.valobj.GetChildMemberWithName('jd')
        self._addChild(julianDay)

        pydate = self.parse(julianDay.GetValueAsUnsigned(0))
        if pydate is None:
            return
        # (ISO)
        iso_str = pydate.isoformat().decode().__repr__()[2:-1]
        self._addChild(('(ISO)', iso_str))

        # (Locale)
        locale_encoding = [locale.getlocale()[1]]
        if locale_encoding[0] is None:
            locale_encoding = []
        locale_str = pydate.strftime('%x').decode(*locale_encoding).__repr__()[2:-1]
        self._addChild(('(Locale)', locale_str))


def QDateSummaryProvider(valobj, internal_dict):
    if valobj.IsValid():
        content = valobj.GetChildMemberWithName('(Locale)')
        if content.IsValid():
            summary = content.GetSummary()
            if summary is not None:
                return summary
        # No synthetic provider installed, get the content by ourselves
        pydate = QDateFormatter.parse(valobj.GetChildMemberWithName('jd').GetValueAsUnsigned(0))
        if pydate is not None:
            return pydate.isoformat().decode().__repr__()[2:-1]
    return '<Invalid>'


class QTimeFormatter(HiddenMemberProvider):
    """lldb synthetic provider for QTime"""
    def __init__(self, valobj, internal_dict):
        super(QTimeFormatter, self).__init__(valobj, internal_dict)
        self._add_original = False

    def has_children(self):
        return True

    @staticmethod
    def parse(ds):
        if ds < 0:
            return None
        MSECS_PER_HOUR = 3600000
        SECS_PER_MIN = 60
        MSECS_PER_MIN = 60000

        hour = ds / MSECS_PER_HOUR
        minute = (ds % MSECS_PER_HOUR) / MSECS_PER_MIN
        second = (ds / 1000) % SECS_PER_MIN
        msec = ds % 1000
        return dt.time(hour, minute, second, msec)

    def _update(self):
        # FIXME: Calling functions returns incorrect SBValue for complex type in lldb
        # # toString
        # res = invoke(self.valobj, 'toString', '0')
        # self._addChild(rename('toString', res))

        # mds
        mds = self.valobj.GetChildMemberWithName('mds')
        self._addChild(mds)

        pytime = self.parse(mds.GetValueAsUnsigned(0))
        if pytime is None:
            return
        # (ISO)
        iso_str = pytime.isoformat().decode().__repr__()[2:-1]
        self._addChild(('(ISO)', iso_str))

        # (Locale)
        locale_encoding = [locale.getlocale()[1]]
        if locale_encoding[0] is None:
            locale_encoding = []
        locale_str = pytime.strftime('%X').decode(*locale_encoding).__repr__()[2:-1]
        self._addChild(('(Locale)', locale_str))


def QTimeSummaryProvider(valobj, internal_dict):
    if valobj.IsValid():
        content = valobj.GetChildMemberWithName('(Locale)')
        if content.IsValid():
            summary = content.GetSummary()
            if summary is not None:
                return summary
        # No synthetic provider installed, get the content by ourselves
        pytime = QTimeFormatter.parse(valobj.GetChildMemberWithName('mds').GetValueAsUnsigned(0))
        if pytime is not None:
            return pytime.isoformat().decode().__repr__()[2:-1]
    return None


class QDateTimeFormatter(HiddenMemberProvider):
    """lldb synthetic provider for QTime"""
    def __init__(self, valobj, internal_dict):
        super(QDateTimeFormatter, self).__init__(valobj, internal_dict)

    def has_children(self):
        return True

    @staticmethod
    def parse(time_t, utc=False):
        if time_t is None:
            return None
        totuple = time.gmtime if utc else time.localtime
        return totuple(time_t)

    @staticmethod
    def getdata(var):
        # FIXME: data member is in private structure, which has no complete type when no debug info
        # available for Qt.So we can only rely on function call.
        # The comments in Qt source code says data member will be inlined in Qt6,
        res = invoke(var, 'toSecsSinceEpoch')
        return res

    def _update(self):
        time_t = self.getdata(self.valobj)
        if not time_t.IsValid():
            return

        locale_encoding = [locale.getlocale()[1]]
        if locale_encoding[0] is None:
            locale_encoding = []

        # toTime_t
        self._addChild(rename('toTime_t', time_t))

        # time tuple in local time and utc time
        local_tt = self.parse(time_t.GetValueAsUnsigned(0))
        utc_tt = self.parse(time_t.GetValueAsUnsigned(0), utc=True)

        # (ISO)
        formatted = time.strftime('%Y-%m-%d %H:%M:%S', utc_tt).decode(*locale_encoding).__repr__()
        formatted = formatted[2:-1]
        self._addChild(('(ISO)', formatted))

        def locale_fmt(name, tt):
            formatted = time.strftime('%c', tt).decode(*locale_encoding).__repr__()[2:-1]
            self._addChild((name, formatted))

        # (Locale)
        locale_fmt('(Locale)', local_tt)

        # (UTC)
        locale_fmt('(UTC)', utc_tt)

        # FIXME: Calling functions returns incorrect SBValue for complex type in lldb
        # # toString
        # res = invoke(self.valobj, 'toString', '0')
        # print 'tostring', res
        # self._addChild(rename('toString', res))

        # # toLocalTime
        # res = invoke(self.valobj, 'toTimeSpec', '0')  # Qt::LocalTime == 0
        # print 'tolocaltime', res
        # self._addChild(rename('toLocalTime', res))


def QDateTimeSummaryProvider(valobj, internal_dict):
    if valobj.IsValid():
        content = valobj.GetChildMemberWithName('(Locale)')
        if content.IsValid():
            summary = content.GetSummary()
            if summary is not None:
                return summary
        # No synthetic provider installed, get the content by ourselves
        pytime = QDateTimeFormatter.parse(QDateTimeFormatter.getdata(valobj).GetValueAsUnsigned(0))
        if pytime is not None:
            formatted = time.strftime('%Y-%m-%d %H:%M:%S', pytime).decode().__repr__()
            formatted = formatted[2:-1]
            return formatted
    return None


class QUrlFormatter(HiddenMemberProvider):
    """docstring for QUrlFormatter"""
    def __init__(self, valobj, internal_dict):
        super(QUrlFormatter, self).__init__(valobj, internal_dict)

        target = valobj.GetTarget()
        self._int_type = target.GetBasicType(lldb.eBasicTypeInt)
        self._pvoid_type = target.GetBasicType(lldb.eBasicTypeVoid).GetPointerType()
        self._qstring_type = target.FindFirstType('QString')
        self._qbytearray_type = target.FindFirstType('QByteArray')

    def parseQt5Data(self, dataobj):
        def constructEncoded(port, scheme, username, password, host, path, query, fragment):
            netloc = ''
            host_str = printableQString(host)[0]
            if host_str is not None:
                username_str = printableQString(username)[0]
                if username_str is not None:
                    netloc += username_str
                    password_str = printableQString(password)[0]
                    if password_str is not None:
                        netloc += ':' + password_str
                    netloc += "@"
                netloc += host_str
                port_num = port.GetValueAsSigned(-1)
                if port_num != -1:
                    netloc += ":" + str(port_num)

            url = urlunsplit((printableQString(scheme)[0],
                              netloc,
                              printableQString(path)[0],
                              printableQString(query)[0],
                              printableQString(fragment)[0]))
            encoded = None
            if len(url) > 0:
                encoded = ('(encoded)', url)
            return (encoded, port, scheme, username, password, host, path, query, fragment)

        # try if there's debug info available
        port = dataobj.GetChildMemberWithName('port')
        if port.IsValid():
            scheme = dataobj.GetChildMemberWithName('scheme')
            username = dataobj.GetChildMemberWithName('userName')
            password = dataobj.GetChildMemberWithName('password')
            host = dataobj.GetChildMemberWithName('host')
            path = dataobj.GetChildMemberWithName('path')
            query = dataobj.GetChildMemberWithName('query')
            fragment = dataobj.GetChildMemberWithName('fragment')
            return constructEncoded(port, scheme, username, password, host, path, query, fragment)
        # if no debug information is available for Qt, try guessing the correct address
        # problem with this is that if QUrlPrivate members get changed, this fails
        addr = dataobj.GetValueAsUnsigned(0)

        # skip QAtomicInt ref
        addr += self._int_type.GetByteSize()
        # handle int port
        port = dataobj.CreateValueFromAddress('(port)', addr, self._int_type)
        addr += self._int_type.GetByteSize()
        # handle QString scheme
        scheme = dataobj.CreateValueFromAddress('(scheme)', addr, self._qstring_type)
        addr += self._qstring_type.GetByteSize()
        # handle QString username
        username = dataobj.CreateValueFromAddress('(userName)', addr, self._qstring_type)
        addr += self._qstring_type.GetByteSize()
        # handle QString password
        password = dataobj.CreateValueFromAddress('(password)', addr, self._qstring_type)
        addr += self._qstring_type.GetByteSize()
        # handle QString host
        host = dataobj.CreateValueFromAddress('(host)', addr, self._qstring_type)
        addr += self._qstring_type.GetByteSize()
        # handle QString path
        path = dataobj.CreateValueFromAddress('(path)', addr, self._qstring_type)
        addr += self._qstring_type.GetByteSize()
        # handle QString query
        query = dataobj.CreateValueFromAddress('(query)', addr, self._qstring_type)
        addr += self._qstring_type.GetByteSize()
        # handle QString fragment
        fragment = dataobj.CreateValueFromAddress('(fragment)', addr, self._qstring_type)

        return constructEncoded(port, scheme, username, password, host, path, query, fragment)

    def parseQt4Data(self, dataobj):
        def parseComponents(encodedobj):
            url, _, _ = printableQByteArray(encodedobj)
            if url is None:
                return (None,) * 9
            res = urlsplit(url)
            port = dataobj.CreateValueFromExpression('(port)', str(res.port if res.port is not None else -1))
            scheme = ('(scheme)', res.scheme)
            username = ('(username)', res.username if res.username is not None else '')
            password = ('(password)', res.password if res.password is not None else '')
            host = ('(host)', res.hostname if res.hostname is not None else '')
            path = ('(path)', res.path)
            query = ('(query)', res.query)
            fragment = ('(fragment)', res.fragment)
            encoded = ('(encoded)', url)
            return (encoded, port, scheme, username, password, host, path, query, fragment)

        encodedOriginal = dataobj.GetChildMemberWithName('encodedOriginal')
        if encodedOriginal.IsValid():
            return parseComponents(encodedOriginal)

        # if no debug information is available for Qt, try guessing the correct address
        # problem with this is that if QUrlPrivate members get changed, this fails
        addr = dataobj.GetValueAsUnsigned(0)
        if not validAddr(dataobj, addr):
            return (None,) * 9

        # skip QAtomicInt ref
        addr += self._int_type.GetByteSize()
        # alignment,
        # The largest member is QString and QByteArray, which are 8 bytes (one sizeof(void*)),
        # int is aligned to 8 bytes
        addr += self._pvoid_type.GetByteSize() - self._int_type.GetByteSize()
        # These members are always empty: scheme, userName, password, host, path, query (QByteArray), fragment
        addr += self._qstring_type.GetByteSize() * 6
        addr += self._qbytearray_type.GetByteSize()
        # handle QByteArray encodedOriginal
        encoded = dataobj.CreateValueFromAddress('(encoded)', addr, self._qbytearray_type)

        if not encoded.IsValid():
            return (None,) * 9
        return parseComponents(encoded)

    def try_parse(self):
        dataobj = self.valobj.GetChildMemberWithName('d')
        # first try to access Qt4 data
        (encoded, port, scheme,
         username, password, host, path, query, fragment) = self.parseQt4Data(dataobj)
        if encoded is not None:
            return (encoded, port, scheme, username, password, host, path, query, fragment)

        # if this fails, maybe we deal with Qt5
        (encoded, port, scheme,
         username, password, host,
         path, query, fragment) = self.parseQt5Data(dataobj)
        if encoded is not None:
            return (encoded, port, scheme, username, password, host, path, query, fragment)

        # if above fails, try to print directly.
        # But this might not work, and could lead to issues
        # (see http://sourceware-org.1504.n7.nabble.com/help-Calling-malloc-from-a-Python-pretty-printer-td284031.html)
        res = invoke(self.valobj, 'toString', '(QUrl::FormattingOptions)0')  # QUrl::PrettyDecoded == 0
        if res.IsValid():
            return rename('(encoded)', res), None, None, None, None, None, None, None, None
        return None, None, None, None, None, None, None, None, None

    def _update(self):
        (encoded, port, scheme, username,
         password, host, path, query, fragment) = self.try_parse()
        if encoded is not None:
            self._addChild(encoded, hidden=True)
            if port is not None:
                self._addChild(port)
                self._addChild(scheme)
                self._addChild(username)
                self._addChild(password)
                self._addChild(host)
                self._addChild(path)
                self._addChild(query)
                self._addChild(fragment)
            return
        # if everything fails, we have no choice but to show the original member
        self._add_original = False
        self._addChild(self.valobj.GetChildMemberWithName('d'))


def QUrlSummaryProvider(valobj, internal_dict):
    if valobj.IsValid():
        content = valobj.GetChildMemberWithName('(encoded)')
        if content.IsValid():
            summary = content.GetSummary()
            if summary is not None:
                return summary
        # No synthetic provider installed, get the content by ourselves
        encoded = QUrlFormatter(valobj, internal_dict).try_parse()[0][1]
        if encoded is not None:
            return encoded
    return None


class QUuidFormatter(HiddenMemberProvider):
    """A lldb synthetic provider for QUuid"""
    def __init__(self, valobj, internal_dict):
        super(QUuidFormatter, self).__init__(valobj, internal_dict)

    def has_children(self):
        return False


def QUuidSummaryProvider(valobj, internal_dict):
    data = [valobj.GetChildMemberWithName(name).GetValueAsUnsigned(0)
            for name in ['data1', 'data2', 'data3']]
    data += [val.GetValueAsUnsigned(0) for val in valobj.GetChildMemberWithName('data4')]

    return 'QUuid({{{:02x}-{:02x}-{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}}})'.format(*data)

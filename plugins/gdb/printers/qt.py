# -*- coding: iso-8859-1 -*-
# Pretty-printers for Qt types

# SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>
#
# SPDX-License-Identifier: GPL-2.0-or-later

import gdb
import itertools
import re
import struct
import time
from datetime import datetime
from enum import Enum

import qtcreator_debugger as qtcD
from helper import *

# opt-in to new ValuePrinter for collection types to allow direct querying of sizes where appropriate
# see also: https://sourceware.org/gdb/current/onlinedocs/gdb.html/Pretty-Printing-API.html
if hasattr(gdb, 'ValuePrinter'):
    PrinterBaseType = gdb.ValuePrinter
else:
    PrinterBaseType = object

class PrinterForwarder(PrinterBaseType):
    def __init__(self, className):
        self._className = className
        self._underlyingValue = None
        self._printer = None

    def _setUnderlyingValue(self, underlyingValue):
        self._underlyingValue = underlyingValue
        if isinstance(self._underlyingValue, gdb.Value):
            # gdb.default_visualizer(value) returns None if no pretty-printer for the value exists
            self._printer = gdb.default_visualizer(self._underlyingValue)

    def children(self):
        if self._printer is not None and hasattr(self._printer, 'children'):
            return self._printer.children()
        return []

    def num_children(self):
        if self._printer is not None and hasattr(self._printer, 'num_children'):
            return self._printer.num_children()
        return 0

    def child(self, n):
        if self._printer is not None and hasattr(self._printer, 'child'):
            return self._printer.child(n)
        return None

    def display_hint(self):
        if self._printer is not None and hasattr(self._printer, 'display_hint'):
            return self._printer.display_hint()
        return None

    def to_string(self):
        if self._printer is not None:
            return self._printer.to_string()
        if self._underlyingValue is not None: # e.g. for an integer
            return self._underlyingValue
        raise RuntimeError("No underlying value has been set")

dumper = qtcD.Dumper()

def unique_ptr_get(unique_ptrValue):
    if unique_ptrValue.type.sizeof == dumper.ptrSize():
        return unique_ptrValue.cast(gdb.lookup_type('void').pointer())
    raise RuntimeError("A std::unique_ptr with a nonempty deleter is not supported")

def makeQLatin1String(data, size):
    qLatin1StringType = gdb.lookup_type('QLatin1String')
    if dumper.qt6orLater():
        buffer = struct.pack("nP", size, data)
    else: # Qt 5
        buffer = struct.pack("iP", size, data)
    return gdb.Value(buffer, qLatin1StringType)

def makeUtf8String(data, size):
    if dumper.qt6orLater():
        qUtf8StringViewType = gdb.lookup_type('QBasicUtf8StringView<false>')
        buffer = struct.pack("Pn", data, size)
        return gdb.Value(buffer, qUtf8StringViewType)
    else: # Qt 5
        # this is suboptimal, because a Qt5 UTF-8 string has children in KDevelop UI
        return makeQByteArray(data, size)

def makeQString(utf16data, size):
    if dumper.qt6orLater():
        qStringType = gdb.lookup_type('QString')
        buffer = struct.pack("PPn", utf16data, utf16data, size)
    else: # Qt 5
        # creating a QString would require allocating a d pointer,
        # so create a QStringView instead
        qStringType = gdb.lookup_type('QStringView')
        buffer = struct.pack("nP", size, utf16data)
    return gdb.Value(buffer, qStringType)

def makeQByteArray(data, size):
    if dumper.qt6orLater():
        qByteArrayType = gdb.lookup_type('QByteArray')
        buffer = struct.pack("PPn", data, data, size)
        return gdb.Value(buffer, qByteArrayType)
    else: # Qt 5
        # creating a QByteArray would require allocating a d pointer,
        # and there was no QByteArrayView... so just return a char[]
        memBytes = dumper.readMemory(data, size)
        # `size - 1` because the single argument for GDB's function Type.array (n1 [, n2])
        # is the inclusive upper bound of the array (the lower bound is then zero)
        return gdb.Value(memBytes, gdb.lookup_type("char").array(size - 1))

class QStringViewPrinterBase(PrinterBaseType):

    def __init__(self, val, encoding, bytesPerCharacter):
        self._val = val
        self._encoding = encoding
        self._bytesPerCharacter = bytesPerCharacter

    def to_string(self):
        ret = ""

        try:
            size = self._val['m_size']
            if size == 0:
                return ret
            dataAsCharPointer = self._val['m_data'].cast(gdb.lookup_type("char").pointer())
            size *= self._bytesPerCharacter
            ret = dataAsCharPointer.string(encoding = self._encoding, length = size)
        except Exception:
            # swallow the exception and return empty string
            pass
        return ret

    def display_hint (self):
        return 'string'

class QLatin1StringPrinter(QStringViewPrinterBase):

    def __init__(self, val):
        QStringViewPrinterBase.__init__(self, val, 'latin1', 1)

class QUtf8StringViewPrinter(QStringViewPrinterBase):

    def __init__(self, val):
        QStringViewPrinterBase.__init__(self, val, 'UTF-8', 1)

class QStringViewPrinter(QStringViewPrinterBase):

    def __init__(self, val):
        QStringViewPrinterBase.__init__(self, val, 'UTF-16', 2)

class QStringPrinter(PrinterBaseType):

    def __init__(self, val):
        self._val = val

    def to_string(self):
        ret = ""

        # The QString object may not be initialized yet. In this case 'size' is a bogus value
        # or in case of Qt5, 'd' is an invalid pointer and the following lines might throw memory
        # access error. Hence the try/catch.
        try:
            size = self._val['d']['size']
            if size == 0:
                return ret
            isQt6 = has_field(self._val['d'], 'ptr') # Qt6 has d->ptr, Qt5 doesn't.
            if isQt6:
                dataAsCharPointer = self._val['d']['ptr'].cast(gdb.lookup_type("char").pointer())
            else:
                dataAsCharPointer = (self._val['d'] + 1).cast(gdb.lookup_type("char").pointer())
            ret = dataAsCharPointer.string(encoding = 'UTF-16', length = size * 2)
        except Exception:
            # swallow the exception and return empty string
            pass
        return ret

    def num_children(self):
        # The QString object may not be initialized yet. In this case 'size' is a bogus value
        # or in case of Qt5, 'd' is an invalid pointer and the following lines might throw memory
        # access error. Hence the try/catch.
        try:
            return self._val['d']['size']
        except:
            return 0

    def display_hint (self):
        return 'string'

class QByteArrayPrinter(PrinterBaseType):

    def __init__(self, val):
        self._val = val
        self._size = self._val['d']['size']
        # Qt6 has d.ptr, Qt5 doesn't
        self._isQt6 = has_field(self._val['d'], 'ptr')

    class _iterator(Iterator):
        def __init__(self, data, size):
            self.data = data
            self.size = size
            self.count = 0

        def __iter__(self):
            return self

        def __next__(self):
            if self.count >= self.size:
                raise StopIteration
            count = self.count
            self.count = self.count + 1
            return ('[%d]' % count, self.data[count])

    def _stringData(self):
        if self._isQt6:
            return self._val['d']['ptr'].cast(gdb.lookup_type("char").pointer())
        else:
            return self._val['d'].cast(gdb.lookup_type("char").const().pointer()) + self._val['d']['offset']

    def children(self):
        return self._iterator(self._stringData(), self._size)

    def num_children(self):
        return self._size

    def to_string(self):
        data = self._stringData()
        try:
            # Attempt to decode from UTF-8, since many byte arrays are ascii/utf-8 strings
            return data.string(length = self._size)
        except:
            # ... but not all of them
            # Use latin1 to show byte data, as fallback
            # It will show \000 for NUL etc, which is nice.
            return data.string(length = self._size, encoding = 'latin1')

    def display_hint (self):
        return 'string'

class QListPrinter(PrinterBaseType):
    "Print a QList"

    class _iterator(Iterator):
        def __init__(self, nodetype, d, isQt6):
            self.nodetype = nodetype
            self.d = d
            self.count = 0
            self.isQt6 = isQt6

            if not isQt6:
                #from QTypeInfo::isLarge
                isLarge = self.nodetype.sizeof > gdb.lookup_type('void').pointer().sizeof

                isPointer = self.nodetype.code == gdb.TYPE_CODE_PTR

                #unfortunately we can't use QTypeInfo<T>::isStatic as it's all inlined, so use
                #this list of types that use Q_DECLARE_TYPEINFO(T, Q_MOVABLE_TYPE)
                #(obviously it won't work for custom types)
                movableTypes = ['QRect', 'QRectF', 'QString', 'QMargins', 'QLocale', 'QChar', 'QDate', 'QTime', 'QDateTime', 'QVector',
                'QRegExpr', 'QPoint', 'QPointF', 'QByteArray', 'QSize', 'QSizeF', 'QBitArray', 'QLine', 'QLineF', 'QModelIndex', 'QPersitentModelIndex',
                'QVariant', 'QFileInfo', 'QUrl', 'QXmlStreamAttribute', 'QXmlStreamNamespaceDeclaration', 'QXmlStreamNotationDeclaration',
                'QXmlStreamEntityDeclaration', 'QPair<int, int>']
                #this list of types that use Q_DECLARE_TYPEINFO(T, Q_PRIMITIVE_TYPE) (from qglobal.h)
                primitiveTypes = ['bool', 'char', 'signed char', 'unsigned char', 'short', 'unsigned short', 'int', 'unsigned int', 'long', 'unsigned long', 'long long', 'unsigned long long', 'float', 'double']

                if movableTypes.count(self.nodetype.tag) or primitiveTypes.count(str(self.nodetype)):
                    isStatic = False
                else:
                    isStatic = not isPointer

                self.externalStorage = isLarge or isStatic #see QList::Node::t()


        def __iter__(self):
            return self

        def __next__(self):
            if self.isQt6:
                size = self.d['size']
            else:
                size = self.d['end'] - self.d['begin']

            if self.count >= size:
                raise StopIteration
            count = self.count

            if self.isQt6:
                value = self.d['ptr'] + count
            else:
                array = self.d['array'].address + self.d['begin'] + count
                if self.externalStorage:
                    value = array.cast(gdb.lookup_type('QList<%s>::Node' % self.nodetype).pointer())['v']
                else:
                    value = array
            self.count = self.count + 1
            return ('[%d]' % count, value.cast(self.nodetype.pointer()).dereference())

    def __init__(self, val, container, itype):
        self._d = val['d']
        self._container = container
        self._isQt6 = has_field(self._d, 'size')

        if self._isQt6:
            self._size = self._d['size']
        else:
            self._size = self._d['end'] - self._d['begin']

        if itype == None:
            self._itype = val.type.template_argument(0)
        else:
            self._itype = gdb.lookup_type(itype)

    def children(self):
        return self._iterator(self._itype, self._d, self._isQt6)

    def num_children(self):
        return self._size

    def to_string(self):
        return "%s<%s> (size = %s)" % ( self._container, self._itype, self._size )

class QVectorPrinter(PrinterBaseType):
    "Print a QVector"

    class _iterator(Iterator):
        def __init__(self, nodetype, data, size):
            self.nodetype = nodetype
            self.data = data
            self.size = size
            self.count = 0

        def __iter__(self):
            return self

        def __next__(self):
            if self.count >= self.size:
                raise StopIteration
            count = self.count

            self.count = self.count + 1
            return ('[%d]' % count, self.data[count])

    def __init__(self, val, container):
        self._val = val
        self._container = container
        self._itype = self._val.type.template_argument(0)

    def children(self):
        # QVector no longer exists in Qt6, but this printer is still used for QStack
        isQt6 = not has_field(self._val['d'], 'alloc')

        if isQt6:
            listPrinter = QListPrinter(self._val, self._container, None)
            return listPrinter.children()
        else:
            data = self._val['d'].cast(gdb.lookup_type("char").const().pointer()) + self._val['d']['offset']
            return self._iterator(self._itype, data.cast(self._itype.pointer()), self._val['d']['size'])

    def num_children(self):
        return self._val['d']['size']

    def to_string(self):
        return "%s<%s> (size = %s)" % ( self._container, self._itype, self.num_children() )

class QLinkedListPrinter(PrinterBaseType):
    "Print a QLinkedList"

    class _iterator(Iterator):
        def __init__(self, nodetype, begin, size):
            self.nodetype = nodetype
            self.it = begin
            self.pos = 0
            self.size = size

        def __iter__(self):
            return self

        def __next__(self):
            if self.pos >= self.size:
                raise StopIteration

            pos = self.pos
            val = self.it['t']
            self.it = self.it['n']
            self.pos = self.pos + 1
            return ('[%d]' % pos, val)

    def __init__(self, val):
        self._val = val
        self._itype = self._val.type.template_argument(0)

    def children(self):
        return self._iterator(self._itype, self._val['e']['n'], self.num_children())

    def num_children(self):
        return self._val['d']['size']

    def to_string(self):
        return "QLinkedList<%s> (size = %s)" % ( self._itype, self.num_children() )

class QMapPrinter(PrinterBaseType):
    "Print a QMap"

    class _iteratorQt5:
        def __init__(self, val):
            realtype = val.type.strip_typedefs()
            keytype = realtype.template_argument(0)
            valtype = realtype.template_argument(1)
            node_type = gdb.lookup_type('QMapData<' + keytype.name + ',' + valtype.name + '>::Node')
            self.node_p_type = node_type.pointer()
            self.root = val['d']['header']
            self.current = None
            self.next_is_key = True
            self.i = -1
            # we store the path here to avoid keeping re-fetching
            # values from the inferior (also, skips the pointer
            # arithmetic involved in using the parent pointer)
            self.path = []

        def __iter__(self):
            return self

        def moveToNextNode(self):
            if self.current is None:
                # find the leftmost node
                if not self.root['left']:
                    return False
                self.current = self.root
                while self.current['left']:
                    self.path.append(self.current)
                    self.current = self.current['left']
            elif self.current['right']:
                self.path.append(self.current)
                self.current = self.current['right']
                while self.current['left']:
                    self.path.append(self.current)
                    self.current = self.current['left']
            else:
                last = self.current
                self.current = self.path.pop()
                while self.current['right'] == last:
                    last = self.current
                    self.current = self.path.pop()
                # if there are no more parents, we are at the root
                if len(self.path) == 0:
                    return False
            return True

        def __next__(self):
            if self.next_is_key:
                if not self.moveToNextNode():
                    raise StopIteration
                self.current_typed = self.current.reinterpret_cast(self.node_p_type)
                self.next_is_key = False
                self.i += 1
                return (f'[{self.i}].key', self.current_typed['key'])
            else:
                self.next_is_key = True
                return (f'[{self.i}].value', self.current_typed['value'])

        def next(self):
            return self.__next__()

    def __init__(self, val, container):
        self._val = val
        self._container = container
        self._isQt6 = not has_field(self._val['d'], 'size')
        self._qt6StdMapPrinter = None
        if self._isQt6:
            d_d = self._val['d']['d']
            if d_d:
                self._qt6StdMapPrinter = gdb.default_visualizer(d_d['m'])

    def children(self):
        if self._qt6StdMapPrinter:
            return self._qt6StdMapPrinter.children()

        elif self._isQt6:
            # without the std::map printer we cannot pretty print the Qt6 QMap contents
            return []

        else:
            if self._val['d']['size'] == 0:
                return []

            return self._iteratorQt5(self._val)

    def to_string(self):
        num_children = self.num_children()
        if num_children is None:
            # qt6 without std map printer
            return "%s<%s, %s> (size = ?)" % ( self._container, self._val.type.template_argument(0), self._val.type.template_argument(1) )
        return "%s<%s, %s> (size = %s)" % ( self._container, self._val.type.template_argument(0), self._val.type.template_argument(1), int(num_children) // 2 )

    def num_children(self):
        "Return the number of children, that is map.size * 2, because keys and values are separate children"
        if self._isQt6 and not self._val['d']['d']:
            return 0

        if self._qt6StdMapPrinter:
            if hasattr(self._qt6StdMapPrinter, 'num_children'):
                return self._qt6StdMapPrinter.num_children()

            # HACK: let's try to stringify the map and see if we can extract the size from there
            # this is error-prone but faster than a potential O(N) iteration on `children`
            map_str = self._qt6StdMapPrinter.to_string()
            # the regex below supports both libstdc++ and libc++ StdMapPrinter.to_string format
            match = re.compile(r"(?:with (\d+) elements?|is empty)$").search(map_str)
            if match:
                size = match.group(1)
                if not size:
                    return 0
                return int(size) * 2

        if self._isQt6:
            # our heuristics above failed or no pretty printer for std::map is available...
            return None

        return int(self._val['d']['size']) * 2

    def display_hint (self):
        return 'map'

class QHashPrinter(PrinterBaseType):
    "Print a QHash"

    class _iterator_qt6(Iterator):
        """
        Representation Invariants:
            - self.currentNode is valid if self.d is not 0
            - self.chain is valid if self.currentNode is valid and self.isMulti is True
        """
        def __init__(self, val, container):
            self.val = val
            self.d = self.val['d']
            self.bucket = 0
            self.count = 0
            self.isMulti = container == 'QMultiHash'

            keyType = self.val.type.template_argument(0)
            valueType = self.val.type.template_argument(1)
            nodeStruct = 'MultiNode' if self.isMulti else 'Node'
            self.nodeType = f'QHashPrivate::{nodeStruct}<{keyType}, {valueType}>'
            #print("nodeType=%s" % self.nodeType)

            self.firstNode()

        def __iter__(self):
            return self

        def span(self):
            "Python port of iterator::span()"
            return self.bucket >> 7 # SpanConstants::SpanShift

        def index(self):
            "Python port of iterator::index()"
            return self.bucket & 127 # SpanConstants::LocalBucketMask

        def isUnused (self):
            "Python port of iterator::isUnused()"
            # return !d->spans[span()].hasNode(index());
            # where hasNode is return (offsets[i] != SpanConstants::UnusedEntry);
            return self.d['spans'][self.span()]['offsets'][self.index()] == 0xff # SpanConstants::UnusedEntry

        def computeCurrentNode (self):
            "Return the node pointed by the iterator, python port of iterator::node()"
            # return &d->spans[span()].at(index());
            span_index = self.span()
            span = self.d['spans'][span_index]
            # where at() is return entries[offsets[i]].node();
            offset = span['offsets'][self.index()]

            if offset == 0xff: # UnusedEntry, can't happen
                print("Offset points to an unused entry.")
                return None

            #print(f"span() = {span_index}, span = {span}, offset = {offset}")
            entry = span['entries'][offset]

            # where node() is return *reinterpret_cast<Node *>(&storage);
            # where Node is QHashPrivate::(Multi|)Node<Key, T>
            storage_pointer = entry['storage'].address
            return storage_pointer.cast(gdb.lookup_type(self.nodeType).pointer())

        def updateCurrentNode (self):
            "Compute the current node and update the QMultiHash chain"
            self.currentNode = self.computeCurrentNode()
            #print("currentNode=%s" % self.currentNode)
            if self.isMulti:
                # Python port of any of the following two lines in QMultiHash::iterator:
                # e = &it.node()->value;
                # e = i.atEnd() ? nullptr : &i.node()->value;
                # Note that self.currentNode must be valid (not at end) here.
                self.chain = self.currentNode['value']

        def firstNode (self):
            "Go the first node, See Data::begin()."
            self.bucket = 0
            #print("firstNode: if (it.isUnused())")
            if self.isUnused():
                #print("firstNode: ++it;")
                self.nextNode() # calls self.updateCurrentNode() if not empty
            else:
                self.updateCurrentNode()

            #print("firstNode: now at bucket %s" % self.bucket)

        def nextNode (self):
            "Go to the next node, see iterator::operator++()."
            #print("******************************** nextNode")
            #print("nextNode: initial bucket %s" % self.bucket)
            #print("nextNode: numBuckets %s" % self.d['numBuckets'])
            numBuckets = self.d['numBuckets']
            while True:
                self.bucket += 1
                #print("nextNode: in while; bucket %s" % self.bucket)
                if self.bucket == numBuckets:
                    #print("nextNode: in while; end reached")
                    self.d = 0
                    self.bucket = 0
                    return
                #print("nextNode: in while; isUnused %s" % self.isUnused())
                if not self.isUnused():
                    self.updateCurrentNode()
                    #print("not unused, done")
                    return

        def __next__(self):
            "GDB iteration, first call returns key, second value and then jumps to the next chain or hash node."
            if not self.d:
                raise StopIteration

            #print("__next__")

            #print("got node %s" % self.currentNode)

            if self.count % 2 == 0:
                itemType = 'key'
                item = self.currentNode['key']
            else:
                itemType = 'value'
                # QHash stores an element (key and value pair) in each hash node.
                # In contrast, QMultiHash stores a key and a chain (linked list)
                # of values in each hash node.
                if self.isMulti:
                    # Python port of the following line in QMultiHash::iterator:
                    # inline T &value() const noexcept { return (*e)->value; }
                    item = self.chain['value']

                    # Python port of QMultiHash::iterator::operator++()
                    self.chain = self.chain['next']
                    if not self.chain:
                        self.nextNode()
                else:
                    item = self.currentNode['value']
                    self.nextNode()

            result = (f'[{self.count // 2}].{itemType}', item)
            self.count = self.count + 1
            return result

    class _iterator_qt5(Iterator):
        def __init__(self, val):
            self.val = val
            self.d = self.val['d']
            self.ktype = self.val.type.template_argument(0)
            self.vtype = self.val.type.template_argument(1)
            self.end_node = self.d.cast(gdb.lookup_type('QHashData::Node').pointer())
            self.data_node = self.firstNode()
            self.count = 0

        def __iter__(self):
            return self

        def hashNode (self):
            "Casts the current QHashData::Node to a QHashNode and returns the result. See also QHash::concrete()"
            return self.data_node.cast(gdb.lookup_type('QHashNode<%s, %s>' % (self.ktype, self.vtype)).pointer())

        def firstNode (self):
            "Get the first node, See QHashData::firstNode()."
            e = self.d.cast(gdb.lookup_type('QHashData::Node').pointer())
            #print "QHashData::firstNode() e %s" % e
            bucketNum = 0
            bucket = self.d['buckets'][bucketNum]
            #print "QHashData::firstNode() *bucket %s" % bucket
            n = self.d['numBuckets']
            #print "QHashData::firstNode() n %s" % n
            while n:
                #print "QHashData::firstNode() in while, n %s" % n;
                if bucket != e:
                    #print "QHashData::firstNode() in while, return *bucket %s" % bucket
                    return bucket
                bucketNum += 1
                bucket = self.d['buckets'][bucketNum]
                #print "QHashData::firstNode() in while, new bucket %s" % bucket
                n -= 1
            #print "QHashData::firstNode() return e %s" % e
            return e


        def nextNode (self, node):
            "Get the nextNode after the current, see also QHashData::nextNode()."
            #print "******************************** nextNode"
            #print "nextNode: node %s" % node
            next = node['next'].cast(gdb.lookup_type('QHashData::Node').pointer())
            e = next

            #print "nextNode: next %s" % next
            if next['next']:
                #print "nextNode: return next"
                return next

            #print "nextNode: node->h %s" % node['h']
            #print "nextNode: numBuckets %s" % self.d['numBuckets']
            start = (node['h'] % self.d['numBuckets']) + 1
            bucketNum = start
            #print "nextNode: start %s" % start
            bucket = self.d['buckets'][start]
            #print "nextNode: bucket %s" % bucket
            n = self.d['numBuckets'] - start
            #print "nextNode: n %s" % n
            while n:
                #print "nextNode: in while; n %s" % n
                #print "nextNode: in while; e %s" % e
                #print "nextNode: in while; *bucket %s" % bucket
                if bucket != e:
                    #print "nextNode: in while; return bucket %s" % bucket
                    return bucket
                bucketNum += 1
                bucket = self.d['buckets'][bucketNum]
                n -= 1
            #print "nextNode: return e %s" % e
            return e

        def __next__(self):
            "GDB iteration, first call returns key, second value and then jumps to the next hash node."
            if self.data_node == self.end_node:
                raise StopIteration

            node = self.hashNode()

            if self.count % 2 == 0:
                itemType = 'key'
                item = node['key']
            else:
                itemType = 'value'
                item = node['value']
                self.data_node = self.nextNode(self.data_node)

            result = (f'[{self.count // 2}].{itemType}', item)
            self.count = self.count + 1
            return result

    def __init__(self, val, container):
        self._val = val
        self._container = container

    def children(self):
        d = self._val['d']
        if not d:
            return []
        isQt5 = has_field(d, 'buckets') # Qt5 has 'buckets', Qt6 doesn't
        if isQt5:
            return self._iterator_qt5(self._val)
        else:
            return self._iterator_qt6(self._val, self._container)

    def num_children(self):
        "Return the number of children, that is hash.size * 2, because keys and values are separate children"
        if has_field(self._val, 'm_size'):
            size = self._val['m_size'] # only Qt6 QMultiHash has m_size
        else:
            d = self._val['d']
            size = d['size'] if d else 0
        return int(size) * 2

    def to_string(self):
        return "%s<%s, %s> (size = %s)" % ( self._container, self._val.type.template_argument(0), self._val.type.template_argument(1), int(self.num_children()) // 2 )

    def display_hint (self):
        return 'map'

class QDatePrinter:

    def __init__(self, val):
        self.val = val

    def to_string(self):
        julianDay = self.val['jd']

        if julianDay == 0:
            return "invalid QDate"

        # Copied from Qt sources
        if julianDay >= 2299161:
            # Gregorian calendar starting from October 15, 1582
            # This algorithm is from Henry F. Fliegel and Thomas C. Van Flandern
            ell = julianDay + 68569;
            n = (4 * ell) / 146097;
            ell = ell - (146097 * n + 3) / 4;
            i = (4000 * (ell + 1)) / 1461001;
            ell = ell - (1461 * i) / 4 + 31;
            j = (80 * ell) / 2447;
            d = ell - (2447 * j) / 80;
            ell = j / 11;
            m = j + 2 - (12 * ell);
            y = 100 * (n - 49) + i + ell;
        else:
            # Julian calendar until October 4, 1582
            # Algorithm from Frequently Asked Questions about Calendars by Claus Toendering
            julianDay += 32082;
            dd = (4 * julianDay + 3) / 1461;
            ee = julianDay - (1461 * dd) / 4;
            mm = ((5 * ee) + 2) / 153;
            d = ee - (153 * mm + 2) / 5 + 1;
            m = mm + 3 - 12 * (mm / 10);
            y = dd - 4800 + (mm / 10);
            if y <= 0:
                --y;
        return "%d-%02d-%02d" % (y, m, d)

class QTimePrinter:

    def __init__(self, val):
        self.val = val

    def to_string(self):
        ds = self.val['mds']

        if ds == -1:
            return "invalid QTime"

        MSECS_PER_HOUR = 3600000
        SECS_PER_MIN = 60
        MSECS_PER_MIN = 60000

        hour = ds / MSECS_PER_HOUR
        minute = (ds % MSECS_PER_HOUR) / MSECS_PER_MIN
        second = (ds / 1000)%SECS_PER_MIN
        msec = ds % 1000
        return "%02d:%02d:%02d.%03d" % (hour, minute, second, msec)

class TimeSpec(Enum): # enum Qt::TimeSpec
    LocalTime = 0
    UTC = 1
    OffsetFromUTC = 2
    TimeZone = 3

class QTimeZonePrinter(PrinterBaseType):

    INVALID = "<invalid>"

    def timeZoneId(spec, offsetFromUtc):
        if spec == TimeSpec.LocalTime.value:
            return 'Local'
        if spec == TimeSpec.UTC.value:
            return 'UTC'
        if spec == TimeSpec.OffsetFromUTC.value:
            sign = '-' if offsetFromUtc < 0 else '+'
            hours = abs(offsetFromUtc) // 3600
            minutes = (abs(offsetFromUtc) % 3600) // 60
            return f"UTC{sign}{hours:02}:{minutes:02}"
        # ShortData(Qt::TimeZone) has mode == 0, in which case Data is *not* short (and Data::d is nullptr).
        # QTimeZonePrinter.timeZoneId() is not called if Data is not short, so the final return statement
        # below should never be reached (unless the QTimeZone object is uninitialized?).
        return f'<error: unhandled time spec {spec}>'

    def __init__(self, val):
        self._val = val

    def to_string(self):
        d = self._val['d'] # QTimeZone::Data
        isShort = d.cast(gdb.lookup_type('long long')) & 3 # QTimeZone::Data::isShort (Qt6-only)
        if isShort:
            mode = d['s']['mode']
            spec = (mode + 3) & 3 # QTimeZone::ShortData::spec()
            offsetFromUtc = d['s']['offset']
            return QTimeZonePrinter.timeZoneId(spec, int(offsetFromUtc))
        else:
            # QTimeZonePrivate contains:
            # - QSharedData (int) (plus 4 bytes of padding in case of a 64-bit architecture)
            # - vtable for QTimeZonePrivate
            # - QByteArray m_id
            qByteArrayPointerType = gdb.lookup_type('QByteArray').pointer()
            address = d.cast(qByteArrayPointerType.pointer()) # address of QTimeZonePrivate as QByteArray**
            if address == 0:
                # QTimeZone::isValid(), if not short, returns d.d && d->isValid()
                return QTimeZonePrinter.INVALID
            address += 2 # skip the first two hidden, pointer-sized data members

            tzId = QByteArrayPrinter(address.cast(qByteArrayPointerType).dereference()).to_string()
            # QTimeZonePrivate::isValid() returns !m_id.isEmpty()
            return tzId if tzId else QTimeZonePrinter.INVALID

class QDateTimePrinter(PrinterBaseType):

    TimeSpecShift = 4
    TimeSpecMask  = 0x30
    def extractTimeSpec(status):
        return (status & QDateTimePrinter.TimeSpecMask) >> QDateTimePrinter.TimeSpecShift

    def timeZoneAbbreviation(spec, offsetFromUtc, qTimeZoneData):
        if spec == TimeSpec.TimeZone.value:
            # Create a pointer-size buffer and pack the QTimeZone::Data
            buffer = struct.pack("P", qTimeZoneData)
            qTimeZone = gdb.Value(buffer, gdb.lookup_type('QTimeZone'))
            return str(QTimeZonePrinter(qTimeZone).to_string())
        return QTimeZonePrinter.timeZoneId(spec, offsetFromUtc)

    def __init__(self, val):
        self._val = val

    def to_string(self):
        d = self._val['d'] # QDateTime::Data
        if d.cast(gdb.lookup_type('long long')) & 1: # QDateTime::Data::isShort
            msecs = d['data']['msecs']
            status = d['data']['status']
            offsetFromUtc = 0
            qTimeZoneData = 0
        else:
            priv = d['d'] # QDateTimePrivate
            # QDateTimePrivate contains
            # - QSharedData (int)
            # - (int) StatusFlags m_status
            # - qint64 m_msecs
            # - int m_offsetFromUtc (plus 4 bytes of padding)
            # - QTimeZone m_timeZone (i.e. QTimeZone::Data object, 8 bytes union)
            (_, status, msecs, offsetFromUtc, _, qTimeZoneData) = dumper.createValue(int(priv), '').split('iIqiiq')

        spec = QDateTimePrinter.extractTimeSpec(status)
        tz = ' ' + QDateTimePrinter.timeZoneAbbreviation(spec, offsetFromUtc, qTimeZoneData)
        return datetime.utcfromtimestamp(int(msecs) / 1000.0).strftime('%Y-%m-%d %H:%M:%S.%f')[:-3] + tz

class QUrlPrinter:

    def __init__(self, val):
        self.val = val

    def to_string(self):
        try:
            int_type = gdb.lookup_type('int')
            string_type = gdb.lookup_type('QString')
            string_pointer = string_type.pointer()

            addr = self.val['d'].cast(gdb.lookup_type('char').pointer())
            if not addr:
                return "<invalid>"
            # skip QAtomicInt ref
            addr += int_type.sizeof
            # handle int port
            port = addr.cast(int_type.pointer()).dereference()
            addr += int_type.sizeof
            # handle QString scheme
            scheme = QStringPrinter(addr.cast(string_pointer).dereference()).to_string()
            addr += string_type.sizeof
            # handle QString username
            username = QStringPrinter(addr.cast(string_pointer).dereference()).to_string()
            addr += string_type.sizeof
            # skip QString password
            addr += string_type.sizeof
            # handle QString host
            host = QStringPrinter(addr.cast(string_pointer).dereference()).to_string()
            addr += string_type.sizeof
            # handle QString path
            path = QStringPrinter(addr.cast(string_pointer).dereference()).to_string()
            addr += string_type.sizeof
            # handle QString query
            query = QStringPrinter(addr.cast(string_pointer).dereference()).to_string()
            addr += string_type.sizeof
            # handle QString fragment
            fragment = QStringPrinter(addr.cast(string_pointer).dereference()).to_string()

            url = ""
            if len(scheme) > 0:
                # TODO: always adding // is apparently not compliant in all cases
                url += scheme + "://"
            if len(host) > 0:
                if len(username) > 0:
                    url += username + "@"
                url += host
                if port != -1:
                    url += ":" + str(port)
            url += path
            if len(query) > 0:
                url += "?" + query
            if len(fragment) > 0:
                url += "#" + fragment

            return url
        except:
            pass
        # then try to print directly, but that might lead to issues (see http://sourceware-org.1504.n7.nabble.com/help-Calling-malloc-from-a-Python-pretty-printer-td284031.html)
        try:
            return gdb.parse_and_eval("reinterpret_cast<const QUrl*>(%s)->toString((QUrl::FormattingOptions)QUrl::PrettyDecoded)" % self.val.address)
        except:
            pass
        return "<uninitialized>"

class QSetPrinter(PrinterBaseType):
    "Print a QSet"

    def __init__(self, val):
        self._val = val

    class _iterator_qt6(Iterator):
        def __init__(self, hashIterator):
            self.hashIterator = hashIterator
            self.count = 0

        def __iter__(self):
            return self

        def __next__(self):
            if not self.hashIterator.d:
                raise StopIteration

            item = self.hashIterator.currentNode['key']
            self.hashIterator.nextNode()

            self.count = self.count + 1
            return ('[%d]' % (self.count-1), item)

    class _iterator_qt5(Iterator):
        def __init__(self, hashIterator):
            self.hashIterator = hashIterator
            self.count = 0

        def __iter__(self):
            return self

        def __next__(self):
            if self.hashIterator.data_node == self.hashIterator.end_node:
                raise StopIteration

            node = self.hashIterator.hashNode()

            item = node['key']
            self.hashIterator.data_node = self.hashIterator.nextNode(self.hashIterator.data_node)

            self.count = self.count + 1
            return ('[%d]' % (self.count-1), item)

    def children(self):
        qhash = self._val['q_hash']
        d = qhash['d']
        if not d:
            return []

        hashPrinter = QHashPrinter(qhash, None)
        hashIterator = hashPrinter.children()

        isQt5 = has_field(d, 'buckets') # Qt5 has 'buckets', Qt6 doesn't
        if isQt5:
            return self._iterator_qt5(hashIterator)
        else:
            return self._iterator_qt6(hashIterator)

    def num_children(self):
        d = self._val['q_hash']['d']
        return d['size'] if d else 0

    def to_string(self):
        return "QSet<%s> (size = %s)" % ( self._val.type.template_argument(0), self.num_children() )


class QCharPrinter:

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return unichr(self.val['ucs'])

    def display_hint (self):
        return 'string'

class QPersistentModelIndexPrinter:

    def __init__(self, val):
        self.val = val

    def to_string(self):
        # Do not check d = self.val['d'] and return str(d['index']) if d, because the type of d
        # is class QPersistentModelIndexData, which is defined in the private Qt header
        # qabstractitemmodel_p.h, so printing d's data member requires installing QtCore debug symbols.
        modelIndex = gdb.parse_and_eval("reinterpret_cast<const QPersistentModelIndex*>(%s)->operator QModelIndex()" % self.val.address)
        return str(modelIndex)

class QUuidPrinter:

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return "QUuid({%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x})" % (int(self.val['data1']), int(self.val['data2']), int(self.val['data3']),
                                            int(self.val['data4'][0]), int(self.val['data4'][1]),
                                            int(self.val['data4'][2]), int(self.val['data4'][3]),
                                            int(self.val['data4'][4]), int(self.val['data4'][5]),
                                            int(self.val['data4'][6]), int(self.val['data4'][7]))

class CborValueType(Enum):
    # converted from qcborvalue.h
    Integer         = 0x00
    ByteArray       = 0x40
    String          = 0x60
    Array           = 0x80
    Map             = 0xa0
    Tag             = 0xc0

    # range 0x100 - 0x1ff for Simple Types
    SimpleType      = 0x100
    FalseValue      = SimpleType + 20
    TrueValue       = SimpleType + 21
    Null            = SimpleType + 22
    Undefined       = SimpleType + 23

    Double          = 0x202

    # extended (tagged) types
    DateTime        = 0x10000
    Url             = 0x10020
    RegularExpression = 0x10023
    Uuid            = 0x10025

    Invalid         = -1

class CborOrJsonValueData:

    def __init__(self, item_data, container_ptr, item_type, is_cbor):
        self.item_data = item_data
        self.container_ptr = container_ptr
        self.item_type = item_type
        self.is_cbor = is_cbor

    def createCborOrJsonContainer(self, containerClassName):
        if dumper.qt6orLater() or self.is_cbor:
            # Create an 8-byte buffer and pack the address as a pointer
            buffer = struct.pack("P", self.container_ptr)
        else: # Qt 5.15's QJsonArray and QJsonObject had a dead pointer first
            buffer = struct.pack("PP", 0, self.container_ptr)
        # This will trigger {containerClassName}Printer
        return gdb.Value(buffer, containerClassName)

    def toCborOrJsonGdbValue(self):
        valueType = gdb.lookup_type('QCborValue' if self.is_cbor else 'QJsonValue')
        # n, container, t, padding
        buffer = struct.pack("qPii", self.item_data, self.container_ptr, self.item_type, 0)
        return gdb.Value(buffer, valueType)

    def toGdbValueString(self, element_index, is_bytes):
        bytedata_data, bytedata_len, element_flags = qtcD.qdumpHelper_QCbor_string(
                                                        dumper, self.container_ptr, element_index)
        if is_bytes:
            return makeQByteArray(bytedata_data, bytedata_len)
        if element_flags & 8: # QtCbor::Element::StringIsAscii
            return makeQLatin1String(bytedata_data, bytedata_len)
        if element_flags & 4: # QtCbor::Element::StringIsUtf16
            return makeQString(bytedata_data, int(bytedata_len) // 2)
        return makeUtf8String(bytedata_data, bytedata_len)

    def toPythonBytes(self, element_index):
        "A variant of toGdbValueString(), which returns a Python bytes buffer"
        bytedata_data, bytedata_len, element_flags = qtcD.qdumpHelper_QCbor_string(
                                                        dumper, self.container_ptr, element_index)
        buffer = dumper.readMemory(bytedata_data, bytedata_len)
        return (buffer, bytedata_len, element_flags)

    def toPythonString(self, element_index):
        "A variant of toGdbValueString(), which returns a Python string instead"
        buffer, bytedata_len, element_flags = self.toPythonBytes(element_index)
        enc = 'utf8'
        if element_flags & 8: # QtCbor::Element::StringIsAscii
            enc = 'latin1'
        elif (element_flags & 4): # QtCbor::Element::StringIsUtf16
            enc = 'utf16'
        return str(buffer[0:bytedata_len], enc)

    def inspect(self):
        "This function corresponds to Qt Creator's qdump__QCborValue_proxy(), but the implementation has diverged significantly"
        item_data = self.item_data
        item_type = self.item_type
        is_cbor = self.is_cbor

        if item_type == CborValueType.Integer.value:
            return item_data

        elif item_type == CborValueType.FalseValue.value:
            return False

        elif item_type == CborValueType.TrueValue.value:
            return True

        elif item_type in {CborValueType.Invalid.value,
                           CborValueType.Null.value,
                           CborValueType.Undefined.value,
                           CborValueType.DateTime.value,
                           CborValueType.Url.value,
                           CborValueType.RegularExpression.value,
                           CborValueType.Tag.value}:
            # forward to QCborValuePrinterBase so that the Type column shows QCborValue or QJsonValue, not char[]
            return self.toCborOrJsonGdbValue()

        elif item_type == CborValueType.Double.value:
            val, = struct.unpack('d', struct.pack('q', item_data))
            return float(val)

        elif item_type == CborValueType.ByteArray.value:
            return self.toGdbValueString(item_data, True)

        elif item_type == CborValueType.String.value:
            return self.toGdbValueString(item_data, False)

        elif item_type == CborValueType.Array.value:
            arrayType = gdb.lookup_type('QCborArray' if is_cbor else 'QJsonArray')
            return self.createCborOrJsonContainer(arrayType)

        elif item_type == CborValueType.Map.value:
            mapType = gdb.lookup_type('QCborMap' if is_cbor else 'QJsonObject')
            return self.createCborOrJsonContainer(mapType)

        elif item_type == CborValueType.Uuid.value:
            butesBuffer, _, _ = self.toPythonBytes(1)
            # QUuid format: uint, ushort, ushort, uchar[8] in big endian
            data1, data2, data3, data4 = struct.unpack('!IHH8s', butesBuffer) # cbor is in network (big-endian) byte order
            butesBuffer = struct.pack('@IHH8s', data1, data2, data3, data4) # convert to native ordering for QUuid members
            return gdb.Value(butesBuffer, gdb.lookup_type('QUuid'))

        elif (int(item_type) >> 8) == (int(CborValueType.SimpleType.value) >> 8): # isSimpleType()
            # QCborSimpleType is just an enum
            simpleType = item_type & 0xFF
            buffer = struct.pack("B", simpleType) # ... of size 1 byte
            return gdb.Value(buffer, gdb.lookup_type('QCborSimpleType'))

        else:
            return f'<Unknown type> (0x{item_type:x}): {item_data}'

def qCborContainerValueAt(container_ptr, elements_data_ptr, idx, bytedata, is_cbor):
    return CborOrJsonValueData(*qtcD.qdumpHelper_QCborArray_valueAt(
                                    dumper, container_ptr, elements_data_ptr, idx, bytedata, is_cbor))

class QCborContainerPrivateIterator:
    def __init__(self, container_ptr, containerClassName, childName = None):
        self.container_ptr = container_ptr
        self.is_cbor = 'QCbor' in containerClassName
        if childName is None:
            self.isArray = 'Array' in containerClassName
        self.childName = childName

        self.data_pos, self.elements_data_ptr, self.size = qtcD.parseQCborContainer(dumper, container_ptr)
        self.bytedata = None
        self.index = 0

    def __iter__(self):
        return self

    def valueAt(self, index):
        if self.bytedata is None:
            self.bytedata = qtcD.qCborContainerBytedata(dumper, self.data_pos)
        return qCborContainerValueAt(self.container_ptr, self.elements_data_ptr, index, self.bytedata, self.is_cbor)

    def __next__(self):
        if self.index >= self.size:
            raise StopIteration

        item = self.valueAt(self.index).inspect()

        if self.childName is not None:
            result = (self.childName, item)
        elif self.isArray:
            result = (f'[{self.index}]', item)
        else:
            if self.index % 2 == 0:
                itemType = 'key'
            else:
                itemType = 'value'
            result = (f'[{self.index // 2}].{itemType}', item)

        self.index += 1
        return result

class QCborContainerPrinterBase(PrinterBaseType):
    def __init__(self, container_ptr, containerClassName):
        self._container_ptr = container_ptr
        self._containerClassName = containerClassName

        if container_ptr:
            _, _, elements_size = qtcD.parseQCborContainer(dumper, container_ptr)
            self._size = int(elements_size)
        else:
            self._size = 0

        if 'Array' in containerClassName:
            self.display_hint = lambda : 'array'
        else:
            self._size //= 2
            self.display_hint = lambda : 'map'

    def children(self):
        if self._size == 0:
            return []
        return QCborContainerPrivateIterator(self._container_ptr, self._containerClassName)

    def to_string(self):
        return f"{self._containerClassName} (size = {self._size})"

class QCborArrayPrinter(QCborContainerPrinterBase):

    def __init__(self, val):
        container_ptr = int(val['d']['d'])
        super().__init__(container_ptr, 'QCborArray')

class QCborMapPrinter(QCborContainerPrinterBase):

    def __init__(self, val):
        container_ptr = int(val['d']['d'])
        super().__init__(container_ptr, 'QCborMap')

class QJsonArrayPrinter(QCborContainerPrinterBase):

    def __init__(self, val):
        if dumper.qtVersionAtLeast(0x050f00): # also works in Qt6
            container_ptr = int(val['a']['d'])
        else:
            raise RuntimeError("Qt version too old for inspecting QJsonArray")
        super().__init__(container_ptr, 'QJsonArray')

class QJsonObjectPrinter(QCborContainerPrinterBase):

    def __init__(self, val):
        if dumper.qtVersionAtLeast(0x050f00): # also works in Qt6
            container_ptr = int(val['o']['d'])
        else:
            raise RuntimeError("Qt version too old for inspecting QJsonObject")
        super().__init__(container_ptr, 'QJsonObject')

class QCborValuePrinterBase(PrinterForwarder):

    def _initFromFields(self, item_data, container_ptr, item_type):
        valueData = CborOrJsonValueData(item_data, container_ptr, item_type, 'QCbor' in self._className)
        self._initFromValueData(valueData)

    def _initFromValueData(self, valueData):
        item_type = valueData.item_type
        if item_type == CborValueType.Invalid.value:
            self._setUnderlyingValue('<Invalid>')
        elif item_type == CborValueType.Null.value:
            self._setUnderlyingValue('<Null>')
        elif item_type == CborValueType.Undefined.value:
            self._setUnderlyingValue('<Undefined>')
        elif item_type == CborValueType.DateTime.value: # stored as string
            self._setUnderlyingValue(valueData.toPythonString(1))
        elif item_type == CborValueType.Url.value:
            self._setUnderlyingValue('Url(%s)' % valueData.toPythonString(1))
        elif item_type == CborValueType.RegularExpression.value:
            self._setUnderlyingValue('RegularExpression(%s)' % valueData.toPythonString(1))
        elif item_type == CborValueType.Tag.value:
            container_ptr = valueData.container_ptr
            _, elements_data_ptr, elements_size = qtcD.parseQCborContainer(dumper, container_ptr)
            if elements_size == 2:
                tag = dumper.extractInt64(elements_data_ptr)
                self._setUnderlyingValue(f'Tag({tag})')
                self.children = lambda : self._tagIterator(container_ptr, self._className)
                self.num_children = lambda : 1
            else:
                self._setUnderlyingValue('<Invalid Tag>')
        else:
            self._setUnderlyingValue(valueData.inspect())
            if item_type == CborValueType.ByteArray.value or item_type == CborValueType.String.value:
                self.display_hint = lambda : 'string'

    class _tagIterator(QCborContainerPrivateIterator):
        "Iterate over the single tagged value - the second element of the size=2 Tag container"
        def __init__(self, container_ptr, className):
            super().__init__(container_ptr, className, "value")
            self.index = 1

class QJsonDocumentPrinter(QCborValuePrinterBase):

    def __init__(self, val):
        # QJsonDocument has a single data member: std::unique_ptr<QJsonDocumentPrivate> d;
        d = unique_ptr_get(val['d'])
        super().__init__('QJsonDocument')
        if d:
            # the first data member of QJsonDocumentPrivate is QCborValue value;
            # QCborValue has 3 data members of types qint64, QCborContainerPrivate*, enum Type : int;
            # create a DumperBase.Value and unpack the QCborValue like Qt Creator's qdump__QCborValue() does:
            # item_data, container_ptr, item_type = value.split('qpi')
            item_data, container_ptr, item_type = dumper.createValue(int(d), '').split('qpi')
            self._initFromFields(item_data, container_ptr, item_type)
        else:
            self._setUnderlyingValue("<empty>")

class QCborValuePrinter(QCborValuePrinterBase):

    def __init__(self, val):
        item_data = int(val['n'])
        container_ptr = int(val['container'])
        item_type = int(val['t'])
        super().__init__('QCborValue')
        self._initFromFields(item_data, container_ptr, item_type)

class QJsonValuePrinter(QCborValuePrinterBase):

    def __init__(self, val):
        if dumper.qt6orLater():
            value = val['value']
            container_ptr = int(value['container'])
        elif dumper.qtVersionAtLeast(0x050f00):
            value = val
            container_ptr = int(value['d']['d'])
        else:
            raise RuntimeError("Qt version too old for inspecting QJsonValue")
        item_data = int(value['n'])
        item_type = int(value['t'])
        super().__init__('QJsonValue')
        self._initFromFields(item_data, container_ptr, item_type)

class QCborValueConstRefPrinterBase(QCborValuePrinterBase):
    def __init__(self, className, container_ptr, itemIndex):
        # array or map makes no difference to QCborContainerPrivateIterator.valueAt(),
        # so only the presence of 'QCbor' in `className` matters
        it = QCborContainerPrivateIterator(container_ptr, className)
        valueData = it.valueAt(itemIndex)
        super().__init__(className)
        self._initFromValueData(valueData)

class QCborValueConstRefPrinter(QCborValueConstRefPrinterBase):

    def __init__(self, val):
        container_ptr = int(val['d'])
        itemIndex = int(val['i'])
        super().__init__('QCborValue', container_ptr, itemIndex)

class QJsonValueConstRefPrinter(QCborValueConstRefPrinterBase):

    def __init__(self, val):
        arrayOrMap = int(val['o'])
        isObject = int(val['is_object'])
        itemIndex = int(val['index'])
        if isObject:
            itemIndex = itemIndex * 2 + 1 # see QJsonPrivate::Value::indexHelper()
        if dumper.qt6orLater():
            container_ptr = dumper.extractPointer(arrayOrMap)
        elif dumper.qtVersionAtLeast(0x050f00):
            container_ptr = dumper.extractPointer(arrayOrMap + dumper.ptrSize())
        super().__init__('QJsonValue', container_ptr, itemIndex)

class QCborSimpleTypePrinter(PrinterBaseType):

    def __init__(self, val):
        self._val = int(val) # QCborSimpleType is just an enum

    def to_string(self):
        return f'QCborSimpleType(0x{self._val:02x})'

class QVariantPrinter:

    def __init__(self, val):
        self.val = val

    def to_string(self):
        d = self.val['d']

        if d['is_null']:
            return "QVariant(NULL)"

        # Qt5 has 'type', Qt6 has 'packedType'
        isQt6 =  has_field(d, 'packedType')
        if isQt6:
            return self.to_string_qt6()
        else:
            return self.to_string_qt5()

    def to_string_qt6(self):
        d = self.val['d']

        #inline const QtPrivate::QMetaTypeInterface *typeInterface() const
        #{
        #    return reinterpret_cast<const QtPrivate::QMetaTypeInterface *>(packedType << 2);
        #}
        data_type = d['packedType'] << 2
        metatype_interface = data_type.cast(gdb.lookup_type("QtPrivate::QMetaTypeInterface").pointer())
        type_str = ""
        try:
            typeAsCharPointer = metatype_interface['name']
            if typeAsCharPointer:
                type_str = typeAsCharPointer.string(encoding = 'UTF-8')
        except Exception as e:
            pass

        data = d['data']
        is_shared = d['is_shared']
        value_str = ""
        if is_shared:
            private_shared = data['shared'].dereference()
            value_str = "PrivateShared(%s)" % hex(private_shared['data'])
        else:
            if type_str.endswith('*'):
                value_ptr = data['data'].reinterpret_cast(gdb.lookup_type('void').pointer().pointer())
                value_str = str(value_ptr.dereference())
            else:
                type_obj = None
                try:
                    type_obj = gdb.lookup_type(type_str)
                except Exception as e:
                    # Looking up type_str failed... falling back to printing out data raw:
                    value_str = str(data['data'])

                if type_obj:
                    value_ptr = data['data'].reinterpret_cast(type_obj.pointer())
                    value_str = str(value_ptr.dereference())

        return "QVariant(%s, %s)" % (type_str, value_str)

    def to_string_qt5(self):
        d = self.val['d']
        data_type = d['type']
        type_str = ("type = %d" % data_type)
        try:
            typeAsCharPointer = (gdb.parse_and_eval("QVariant::typeToName(%d)" % data_type).cast(gdb.lookup_type("char").pointer()))
            if typeAsCharPointer:
                type_str = typeAsCharPointer.string(encoding = 'UTF-8')
        except Exception as e:
            pass

        data = d['data']
        is_shared = d['is_shared']
        value_str = ""
        if is_shared:
            private_shared = data['shared'].dereference()
            value_str = "PrivateShared(%s)" % hex(private_shared['ptr'])
        elif type_str.startswith("type = "):
            value_str = str(data['ptr'])
        else:
            type_obj = None
            try:
                type_obj = gdb.lookup_type(type_str)
            except Exception as e:
                value_str = str(data['ptr'])
            if type_obj:
                if type_obj.sizeof > type_obj.pointer().sizeof:
                    value_ptr = data['ptr'].reinterpret_cast(type_obj.const().pointer())
                    value_str = str(value_ptr.dereference())
                else:
                    value_ptr = data['c'].address.reinterpret_cast(type_obj.const().pointer())
                    value_str = str(value_ptr.dereference())

        return "QVariant(%s, %s)" % (type_str, value_str)

pretty_printers_dict = {}

def register_qt_printers (obj):
    if obj == None:
        obj = gdb

    obj.pretty_printers.append(FunctionLookup(gdb, pretty_printers_dict))

# Note: dumper.qtVersionAtLeast() cannot be used in this function, because the Qt version isn't known before starting the app
def build_dictionary ():
    pretty_printers_dict[re.compile('^QLatin1String$')] = lambda val: QLatin1StringPrinter(val)
    pretty_printers_dict[re.compile('^QBasicUtf8StringView<(?:true|false)>$')] = lambda val: QUtf8StringViewPrinter(val)
    pretty_printers_dict[re.compile('^QStringView$')] = lambda val: QStringViewPrinter(val)
    pretty_printers_dict[re.compile('^QString$')] = lambda val: QStringPrinter(val)
    pretty_printers_dict[re.compile('^QByteArray$')] = lambda val: QByteArrayPrinter(val)
    pretty_printers_dict[re.compile('^QList<.*>$')] = lambda val: QListPrinter(val, 'QList', None)
    pretty_printers_dict[re.compile('^QStringList$')] = lambda val: QListPrinter(val, 'QStringList', 'QString')
    pretty_printers_dict[re.compile('^QQueue<.*>$')] = lambda val: QListPrinter(val, 'QQueue', None)
    pretty_printers_dict[re.compile('^QVector<.*>$')] = lambda val: QVectorPrinter(val, 'QVector')
    pretty_printers_dict[re.compile('^QStack<.*>$')] = lambda val: QVectorPrinter(val, 'QStack')
    pretty_printers_dict[re.compile('^QLinkedList<.*>$')] = lambda val: QLinkedListPrinter(val)
    pretty_printers_dict[re.compile('^QMap<.*>$')] = lambda val: QMapPrinter(val, 'QMap')
    pretty_printers_dict[re.compile('^QMultiMap<.*>$')] = lambda val: QMapPrinter(val, 'QMultiMap')
    pretty_printers_dict[re.compile('^QHash<.*>$')] = lambda val: QHashPrinter(val, 'QHash')
    pretty_printers_dict[re.compile('^QMultiHash<.*>$')] = lambda val: QHashPrinter(val, 'QMultiHash')
    pretty_printers_dict[re.compile('^QDate$')] = lambda val: QDatePrinter(val)
    pretty_printers_dict[re.compile('^QTime$')] = lambda val: QTimePrinter(val)
    pretty_printers_dict[re.compile('^QDateTime$')] = lambda val: QDateTimePrinter(val)
    pretty_printers_dict[re.compile('^QTimeZone$')] = lambda val: QTimeZonePrinter(val)
    pretty_printers_dict[re.compile('^QUrl$')] = lambda val: QUrlPrinter(val)
    pretty_printers_dict[re.compile('^QSet<.*>$')] = lambda val: QSetPrinter(val)
    pretty_printers_dict[re.compile('^QChar$')] = lambda val: QCharPrinter(val)
    pretty_printers_dict[re.compile('^QUuid$')] = lambda val: QUuidPrinter(val)
    pretty_printers_dict[re.compile('^QVariant$')] = lambda val: QVariantPrinter(val)
    pretty_printers_dict[re.compile('^QPersistentModelIndex$')] = lambda val: QPersistentModelIndexPrinter(val)
    pretty_printers_dict[re.compile('^QCborArray$')] = lambda val: QCborArrayPrinter(val)
    pretty_printers_dict[re.compile('^QCborMap$')] = lambda val: QCborMapPrinter(val)
    pretty_printers_dict[re.compile('^QCborValue$')] = lambda val: QCborValuePrinter(val)
    pretty_printers_dict[re.compile('^QCborValue(Const|)Ref$')] = lambda val: QCborValueConstRefPrinter(val)
    pretty_printers_dict[re.compile('^QCborSimpleType$')] = lambda val: QCborSimpleTypePrinter(val)
    pretty_printers_dict[re.compile('^QJsonArray$')] = lambda val: QJsonArrayPrinter(val)
    pretty_printers_dict[re.compile('^QJsonObject$')] = lambda val: QJsonObjectPrinter(val)
    pretty_printers_dict[re.compile('^QJsonDocument$')] = lambda val: QJsonDocumentPrinter(val)
    pretty_printers_dict[re.compile('^QJsonValue$')] = lambda val: QJsonValuePrinter(val)
    pretty_printers_dict[re.compile('^QJsonValue(Const|)Ref$')] = lambda val: QJsonValueConstRefPrinter(val)


build_dictionary ()

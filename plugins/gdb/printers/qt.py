# -*- coding: iso-8859-1 -*-
# Pretty-printers for Qt types

# SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>
#
# SPDX-License-Identifier: GPL-2.0-or-later

import gdb
import itertools
import re
import time

from helper import *

# opt-in to new ValuePrinter for collection types to allow direct querying of sizes where appropriate
# see also: https://sourceware.org/gdb/current/onlinedocs/gdb.html/Pretty-Printing-API.html
if hasattr(gdb, 'ValuePrinter'):
    PrinterBaseType = gdb.ValuePrinter
else:
    PrinterBaseType = object

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
            isQt4 = has_field(self._val['d'], 'data') # Qt4 has d->data, Qt5 doesn't.
            isQt6 = has_field(self._val['d'], 'ptr') # Qt6 has d->ptr, Qt5 doesn't.
            if isQt4:
                dataAsCharPointer = self._val['d']['data'].cast(gdb.lookup_type("char").pointer())
            elif isQt6:
                dataAsCharPointer = self._val['d']['ptr'].cast(gdb.lookup_type("char").pointer())
            else:
                dataAsCharPointer = (self._val['d'] + 1).cast(gdb.lookup_type("char").pointer())
            ret = dataAsCharPointer.string(encoding = 'UTF-16', length = size * 2)
        except Exception:
            # swallow the exception and return empty string
            pass
        return ret

    def display_hint (self):
        return 'string'

class QByteArrayPrinter(PrinterBaseType):

    def __init__(self, val):
        self._val = val
        self._size = self._val['d']['size']
        # Qt4 has 'data', Qt5 doesn't
        self._isQt4 = has_field(self._val['d'], 'data')
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
        if self._isQt4:
            return self._val['d']['data']
        elif self._isQt6:
            return self._val['d']['ptr'].cast(gdb.lookup_type("char").pointer())
        else:
            return self._val['d'].cast(gdb.lookup_type("char").const().pointer()) + self._val['d']['offset']

    def children(self):
        return self._iterator(self._stringData(), self._size)

    def num_children(self):
        return self._size

    def to_string(self):
        #todo: handle charset correctly
        return self._stringData().string(length = self._size)

    def display_hint (self):
        return 'string'

class QListPrinter(PrinterBaseType):
    "Print a QList"

    class _iterator(Iterator):
        def __init__(self, nodetype, d):
            self.nodetype = nodetype
            self.d = d
            self.count = 0

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
            isQt6 = has_field(self.d, 'size')
            if isQt6:
                size = self.d['size']
            else:
                size = self.d['end'] - self.d['begin']

            if self.count >= size:
                raise StopIteration
            count = self.count

            if isQt6:
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
        return self._iterator(self._itype, self._d)

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
        isQt4 = has_field(self._val['d'], 'p') # Qt4 has 'p', Qt5/Qt6 don't
        # QVector no longer exists in Qt6, but this printer is still used for QStack
        isQt6 = not has_field(self._val['d'], 'alloc')

        if isQt4:
            return self._iterator(self._itype, self._val['p']['array'], self._val['p']['size'])
        elif isQt6:
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

    class _iteratorQt4(Iterator):
        def __init__(self, val):
            self.val = val
            self.ktype = self.val.type.template_argument(0)
            self.vtype = self.val.type.template_argument(1)
            self.data_node = self.val['e']['forward'][0]
            self.count = 0

        def __iter__(self):
            return self

        def payload (self):
            if gdb.parse_and_eval:
                ret = int(gdb.parse_and_eval('QMap<%s, %s>::payload()' % (self.ktype, self.vtype)))
                if (ret): return ret;

            #if the inferior function call didn't work, let's try to calculate ourselves

            #we can't use QMapPayloadNode as it's inlined
            #as a workaround take the sum of sizeof(members)
            ret = self.ktype.sizeof
            ret += self.vtype.sizeof
            ret += gdb.lookup_type('void').pointer().sizeof

            #but because of data alignment the value can be higher
            #so guess it's aliged by sizeof(void*)
            #TODO: find a real solution for this problem
            ret += ret % gdb.lookup_type('void').pointer().sizeof

            #for some reason booleans are different
            if str(self.vtype) == 'bool':
                ret += 2

            ret -= gdb.lookup_type('void').pointer().sizeof

            return ret

        def concrete (self, data_node):
            node_type = gdb.lookup_type('QMapNode<%s, %s>' % (self.ktype, self.vtype)).pointer()
            return (data_node.cast(gdb.lookup_type('char').pointer()) - self.payload()).cast(node_type)

        def __next__(self):
            if self.data_node == self.val['e']:
                raise StopIteration
            node = self.concrete(self.data_node).dereference()
            if self.count % 2 == 0:
                item = node['key']
            else:
                item = node['value']
                self.data_node = node['forward'][0]

            result = ('[%d]' % self.count, item)
            self.count = self.count + 1
            return result

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
                return ('key' + str(self.i), self.current_typed['key'])
            else:
                self.next_is_key = True
                return ('value' + str(self.i), self.current_typed['value'])

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

            isQt4 = has_field(self._val, 'e') # Qt4 has 'e', Qt5 doesn't
            if isQt4:
                return self._iteratorQt4(self._val)
            else:
                return self._iteratorQt5(self._val)

    def to_string(self):
        num_children = self.num_children()
        if num_children is None:
            # qt6 without std map printer
            return "%s<%s, %s> (size = ?)" % ( self._container, self._val.type.template_argument(0), self._val.type.template_argument(1) )
        return "%s<%s, %s> (size = %s)" % ( self._container, self._val.type.template_argument(0), self._val.type.template_argument(1), num_children )

    def num_children(self):
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
                return int(size)

        if self._isQt6:
            # our heuristics above failed or no pretty printer for std::map is available...
            return None

        return self._val['d']['size']

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
                item = self.currentNode['key']
            else:
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

            self.count = self.count + 1

            return ('[%d]' % self.count, item)

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
                item = node['key']
            else:
                item = node['value']
                self.data_node = self.nextNode(self.data_node)

            self.count = self.count + 1
            return ('[%d]' % self.count, item)

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
        if has_field(self._val, 'm_size'):
            return self._val['m_size'] # only Qt6 QMultiHash has m_size
        d = self._val['d']
        return d['size'] if d else 0

    def to_string(self):
        return "%s<%s, %s> (size = %s)" % ( self._container, self._val.type.template_argument(0), self._val.type.template_argument(1), self.num_children() )

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

class QDateTimePrinter:

    def __init__(self, val):
        self.val = val

    def to_string(self):
        time_t = gdb.parse_and_eval("reinterpret_cast<const QDateTime*>(%s)->toSecsSinceEpoch()" % self.val.address)
        return time.ctime(int(time_t))

class QUrlPrinter:

    def __init__(self, val):
        self.val = val

    def to_string(self):
        # first try to access the Qt 5 data
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
        # if everything fails, maybe we deal with Qt 4 code
        try:
            return self.val['d']['encodedOriginal']
        except RuntimeError:
            #if no debug information is available for Qt, try guessing the correct address for encodedOriginal
            #problem with this is that if QUrlPrivate members get changed, this fails
            offset = gdb.lookup_type('int').sizeof
            offset += offset % gdb.lookup_type('void').pointer().sizeof #alignment
            offset += gdb.lookup_type('QString').sizeof * 6
            offset += gdb.lookup_type('QByteArray').sizeof
            encodedOriginal = self.val['d'].cast(gdb.lookup_type('char').pointer());
            encodedOriginal += offset
            encodedOriginal = encodedOriginal.cast(gdb.lookup_type('QByteArray').pointer()).dereference();
            encodedOriginal = encodedOriginal['d']['data'].string()
            return encodedOriginal

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

    def display_hint (self):
        return 'string'

class QVariantPrinter:

    def __init__(self, val):
        self.val = val

    def to_string(self):
        d = self.val['d']

        if d['is_null']:
            return "QVariant(NULL)"

        # Qt4/Qt5 has 'type', Qt6 has 'packedType'
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

def build_dictionary ():
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
    pretty_printers_dict[re.compile('^QUrl$')] = lambda val: QUrlPrinter(val)
    pretty_printers_dict[re.compile('^QSet<.*>$')] = lambda val: QSetPrinter(val)
    pretty_printers_dict[re.compile('^QChar$')] = lambda val: QCharPrinter(val)
    pretty_printers_dict[re.compile('^QUuid$')] = lambda val: QUuidPrinter(val)
    pretty_printers_dict[re.compile('^QVariant$')] = lambda val: QVariantPrinter(val)
    pretty_printers_dict[re.compile('^QPersistentModelIndex$')] = lambda val: QPersistentModelIndexPrinter(val)


build_dictionary ()

# SPDX-FileCopyrightText: 2016 The Qt Company Ltd.
#
# SPDX-License-Identifier: GPL-3.0-only WITH Qt-GPL-exception-1.0

# Modified by David Faure <faure@kde.org> (initially)
# for the needs of qt.py

import gdb
import functools
import struct

# similar to qtcreator's API
class DumperBase():

    @staticmethod
    def warn(message):
        print(message)

    def __init__(self):
        self.qtversion = None

    def qtVersion(self):
        if self.qtversion:
            return self.qtversion

        #self.warn("ACCESSING UNKNOWN QT VERSION")
        self.qtversion = self.extractQtVersion()
        if self.qtversion:
            return self.qtversion

        #self.warn("EXTRACTING QT VERSION FAILED. GUESSING NOW.")
        return 0x060602

    def qtVersionAtLeast(self, version):
        return self.qtVersion() >= version

    def lookupType(self, typename):
        return gdb.lookup_type(typename)

    def vectorData(self, value):
        if self.qtVersionAtLeast(0x060000):
            data, length, alloc = self.qArrayData(value)
        elif self.qtVersionAtLeast(0x050000):
            vector_data_ptr = self.extractPointer(value)
            if self.ptrSize() == 4:
                (ref, length, alloc, offset) = self.split('IIIp', vector_data_ptr)
            else:
                (ref, length, alloc, pad, offset) = self.split('IIIIp', vector_data_ptr)
            alloc = alloc & 0x7ffffff
            data = vector_data_ptr + offset
        else:
            vector_data_ptr = self.extractPointer(value)
            (ref, alloc, length) = self.split('III', vector_data_ptr)
            data = vector_data_ptr + 16
        self.check(0 <= length and length <= alloc and alloc <= 1000 * 1000 * 1000)
        return data, length

    def qArrayData(self, value):
        if self.qtVersionAtLeast(0x60000):
            dd, data, length = self.split('ppp', value)
            if dd:
                _, _, alloc = self.split('iip', dd)
            else: # fromRawData
                alloc = length
            return data, length, alloc
        return self.qArrayDataHelper(self.extractPointer(value))

    def qArrayDataHelper(self, array_data_ptr): # Qt <= 5
        # array_data_ptr is what is e.g. stored in a QByteArray's d_ptr.
        if self.qtVersionAtLeast(0x050000):
            # QTypedArray:
            # - QtPrivate::RefCount ref
            # - int length
            # - uint alloc : 31, capacityReserved : 1
            # - qptrdiff offset
            (ref, length, alloc, offset) = self.split('IIpp', array_data_ptr)
            alloc = alloc & 0x7ffffff
            data = array_data_ptr + offset
            if self.ptrSize() == 4:
                data = data & 0xffffffff
            else:
                data = data & 0xffffffffffffffff
        elif self.qtVersionAtLeast(0x040000):
            # Data:
            # - QBasicAtomicInt ref;
            # - int alloc, length;
            # - [padding]
            # - char *data;
            if self.ptrSize() == 4:
                (ref, alloc, length, data) = self.split('IIIp', array_data_ptr)
            else:
                (ref, alloc, length, pad, data) = self.split('IIIIp', array_data_ptr)
        else:
            # Data:
            # - QShared count;
            # - QChar *unicode
            # - char *ascii
            # - uint len: 30
            (dummy, dummy, dummy, length) = self.split('IIIp', array_data_ptr)
            length = self.extractInt(array_data_ptr + 3 * self.ptrSize()) & 0x3ffffff
            alloc = length  # pretend.
            data = self.extract_pointer_at_address(array_data_ptr + self.ptrSize())
        return data, length, alloc

    # Unlike QtCreator, we don't hexencode in base64
    def readMemory(self, addr, size):
        return bytes(self.readRawMemory(addr, size))

    def check(self, exp):
        if not exp:
            self.warn('Check failed: %s' % exp)
            #self.dump_location()
            raise RuntimeError('Check failed: %s' % exp)

    def extract_pointer_at_address(self, address):
        blob = self.value_data_from_address(address, self.ptrSize())
        return int.from_bytes(blob, byteorder='little')

    def value_extract_something(self, valuish, size, signed=False):
        if isinstance(valuish, int):
            blob = self.value_data_from_address(valuish, size)
        elif isinstance(valuish, self.Value):
            blob = self.value_data(valuish, size)
        else:
            raise RuntimeError('CANT EXTRACT FROM %s' % type(valuish))
        res = int.from_bytes(blob, byteorder='little', signed=signed)
        #self.warn("EXTRACTED %s SIZE %s FROM %s" % (res, size, blob))
        return res

    def extractPointer(self, value):
        return self.value_extract_something(value, self.ptrSize())

    def extractInt64(self, value):
        return self.value_extract_something(value, 8, True)

    def extractUInt64(self, value):
        return self.value_extract_something(value, 8)

    def extractInt(self, value):
        return self.value_extract_something(value, 4, True)

    def extractUInt(self, value):
        return self.value_extract_something(value, 4)

    def extractShort(self, value):
        return self.value_extract_something(value, 2, True)

    def extractUShort(self, value):
        return self.value_extract_something(value, 2)

    def extractByte(self, value):
        return self.value_extract_something(value, 1)

    class Value():
        def __init__(self, dumper):
            # This can be helpful to track down from where a Value was created
            #self._stack = inspect.stack()
            self.dumper = dumper
            self.ldata = None        # Target address in case of references and pointers.
            self.laddress = None     # Own address.

        def split(self, pattern):
            return self.dumper.value_split(self, pattern)

    class Field:
        __slots__ = ['name', 'typeid', 'bitsize', 'bitpos', 'is_struct', 'is_artificial', 'is_base_class']

        def __init__(self, name=None, typeid=None, bitsize=None, bitpos=None,
                    extractor=None, is_struct=False, is_artificial=False, is_base_class=False):
            self.name = name
            self.typeid = typeid
            self.bitsize = bitsize
            self.bitpos = bitpos
            self.is_struct = is_struct
            self.is_base_class = is_base_class

    def ptrCode(self):
        return 'I' if self.ptrSize() == 4 else 'Q'

    def createValueFromAddress(self, address, typish):
        val = self.Value(self)
        #val.typeid = self.create_typeid(typish)
        #self.warn('CREATING %s AT 0x%x' % (val.type.name, address))
        val.laddress = address
        #if self.useDynamicType:
        #    val.typeid = self.dynamic_typeid_at_address(val.typeid, address)
        return val

    def createValueFromData(self, data, typish):
        val = self.Value(self)
        #val.typeid = self.create_typeid(typish)
        #self.warn('CREATING %s WITH DATA %s' % (val.type.name, self.hexencode(data)))
        val.ldata = data
        #val.check()
        return val

    def createValue(self, datish, typish):
        if isinstance(datish, int):  # Used as address.
            return self.createValueFromAddress(datish, typish)
        if isinstance(datish, bytes):
            return self.createValueFromData(datish, typish)
        raise RuntimeError('EXPECTING ADDRESS OR BYTES, GOT %s' % type(datish))

    class StructBuilder():
        def __init__(self, dumper):
            self.dumper = dumper
            self.pattern = ''
            self.current_size = 0
            self.fields = []
            self.autoPadNext = False
            self.maxAlign = 1

        def add_field(self, field_size, field_code=None, field_is_struct=False,
                      field_name=None, field_typeid=0, field_align=1):

            if field_code is None:
                field_code = '%ss' % field_size

            #self.dumper.warn("FIELD SIZE: %s %s %s " % (field_name, field_size, str(field_align)))

            if self.autoPadNext:
                padding = (field_align - self.current_size) % field_align
                #self.warn('AUTO PADDING AT %s BITS BY %s BYTES' % (self.current_size, padding))
                field = self.dumper.Field(self.dumper, bitpos=self.current_size * 8,
                                          bitsize=padding * 8)
                self.pattern += '%ds' % padding
                self.current_size += padding
                self.fields.append(field)
                self.autoPadNext = False

            if field_align > self.maxAlign:
                self.maxAlign = field_align

            #self.warn("MAX ALIGN: %s" % self.maxAlign)

            field = self.dumper.Field(name=field_name, typeid=field_typeid,
                                      is_struct=field_is_struct, bitpos=self.current_size *8,
                                      bitsize=field_size * 8)

            self.pattern += field_code
            self.current_size += field_size
            self.fields.append(field)

    @functools.lru_cache(maxsize = None)
    def describeStruct(self, pattern):
        ptrSize = self.ptrSize()
        builder = self.StructBuilder(self)
        n = None
        typename = ''
        readingTypeName = False
        #self.warn("PATTERN: %s" % pattern)
        for c in pattern:
            #self.warn("PAT CODE: %s %s" % (c, str(n)))
            if readingTypeName:
                if c == '}':
                    readingTypeName = False
                    n, field_typeid = self.describe_struct_member(typename)

                    field_align = self.type_alignment(field_typeid)
                    builder.add_field(n,
                                      field_is_struct=True,
                                      field_typeid=field_typeid,
                                      field_align=field_align)
                    typename = None
                    n = None
                else:
                    typename += c
            elif c == 't':  # size_t
                builder.add_field(ptrSize, self.ptrCode(), field_align=ptrSize)
            elif c == 'p':  # Pointer as int
                builder.add_field(ptrSize, self.ptrCode(), field_align=ptrSize)
            elif c == 'P':  # Pointer as Value
                builder.add_field(ptrSize, '%ss' % ptrSize, field_align=ptrSize)
            elif c in ('d'):
                builder.add_field(8, c, field_align=ptrSize)  # field_type = 'double' ?
            elif c in ('q', 'Q'):
                builder.add_field(8, c, field_align=ptrSize)
            elif c in ('i', 'I', 'f'):
                builder.add_field(4, c, field_align=4)
            elif c in ('h', 'H'):
                builder.add_field(2, c, field_align=2)
            elif c in ('b', 'B', 'c'):
                builder.add_field(1, c, field_align=1)
            elif c >= '0' and c <= '9':
                if n is None:
                    n = ''
                n += c
            elif c == 's':
                builder.add_field(int(n), field_align=1)
                n = None
            elif c == '{':
                readingTypeName = True
                typename = ''
            elif c == '@':
                if n is None:
                    # Automatic padding depending on next item
                    builder.autoPadNext = True
                else:
                    # Explicit padding.
                    padding = (int(n) - builder.current_size) % int(n)
                    field = self.Field(self)
                    builder.pattern += '%ds' % padding
                    builder.current_size += padding
                    builder.fields.append(field)
                    n = None
            else:
                raise RuntimeError('UNKNOWN STRUCT CODE: %s' % c)
        pp = builder.pattern
        size = builder.current_size
        fields = builder.fields
        tailPad = (builder.maxAlign - size) % builder.maxAlign
        size += tailPad
        #self.warn("FIELDS: %s" % ((pp, size, fields),))
        return (pp, size, fields)

    def value_data(self, value, size):
        if value.ldata is not None:
            return value.ldata[:size]
        if value.laddress is not None:
            return self.value_data_from_address(value.laddress, size)
        raise RuntimeError('CANNOT CONVERT TO BYTES: %s' % value)

    def value_data_from_address(self, address, size):
        if not isinstance(address, int):
            raise RuntimeError('ADDRESS WRONG TYPE: %s' % type(address))
        if not isinstance(size, int):
            raise RuntimeError('SIZE WRONG TYPE: %s' % type(size))
        if size <= 0:
            raise RuntimeError('SIZE WRONG VALUE: %s' % size)
        res = self.readRawMemory(address, size)
        if len(res) > 0:
            return res
        raise RuntimeError('CANNOT READ %d BYTES FROM ADDRESS: %s' % (size, address))

    def split(self, pattern, value_or_address):
        #if isinstance(value_or_address, self.Value):
        #    return self.value_split(value_or_address, pattern)
        if isinstance(value_or_address, int):
            val = self.Value(self)
            val.laddress = value_or_address
            return self.value_split(val, pattern)
        raise RuntimeError('CANNOT EXTRACT STRUCT FROM %s' % type(value_or_address))

    def value_split(self, value, pattern):
        #self.warn('EXTRACT STRUCT FROM: %s' % self.type)
        (pp, size, fields) = self.describeStruct(pattern)
        #self.warn('SIZE: %s ' % size)

        blob = self.value_data(value, size)
        address = value.laddress

        parts = struct.unpack_from(self.packCode + pp, blob)

        def fix_struct(field, part):
            #self.warn('STRUCT MEMBER: %s' % type(part))
            if field.is_struct:
                res = self.Value(self)
                res.typeid = field.typeid
                res.ldata = part
                if address is not None:
                    res.laddress = address + field.bitpos // 8
                return res
            return part

        if len(fields) != len(parts):
            raise RuntimeError('STRUCT ERROR: %s %s' % (fields, parts))
        return tuple(map(fix_struct, fields, parts))


# similar to qtcreator's API
class Dumper(DumperBase): # gdb-specific

    def __init__(self):
        DumperBase.__init__(self)

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
            # We don't care for 5.0-5.2 so just return 4.8.x as fallback
            (major, minor, patch) = (4, 8, 0)
            return 0x10000 * major + 0x100 * minor + patch

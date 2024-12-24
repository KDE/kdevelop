# SPDX-FileCopyrightText: 2016 The Qt Company Ltd.
#
# SPDX-License-Identifier: GPL-3.0-only WITH Qt-GPL-exception-1.0

# Modified by David Faure <faure@kde.org> (initially)
# for the needs of qt.py

import functools
import struct

class DumperBase():
    @staticmethod
    def warn(message):
        print(message) # KDevelop: simplified - no replacements

    def __init__(self):
        self.qtversion = None

    def extractQtVersion(self):
        # can be overridden in bridges
        pass

    def qtVersion(self):
        if self.qtversion:
            return self.qtversion

        #self.warn("ACCESSING UNKNOWN QT VERSION")
        self.qtversion = self.extractQtVersion()
        if self.qtversion:
            return self.qtversion

        #self.warn("EXTRACTING QT VERSION FAILED. GUESSING NOW.")
        # KDevelop: simplified - no qtversionAtLeast6
        return 0x060602

    def qtVersionAtLeast(self, version):
        # KDevelop: simplified - no qtversionAtLeast6
        return self.qtVersion() >= version

    # KDevelop: added this convenience function
    def qt6orLater(self):
        return self.qtVersionAtLeast(0x060000)

    def lookupType(self, typename):
        # KDevelop: simplified - must be overridden in bridges
        pass

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
            # KDevelop: removed Qt 4 support
            warn("unsupported Qt version < 5.0")
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

    def qArrayDataHelper(self, array_data_ptr):
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
        else:
            # KDevelop: removed Qt 4 and earlier support
            warn("unsupported Qt version < 5.0")
        return data, length, alloc

    def readMemory(self, addr, size):
        # KDevelop: unlike QtCreator, we don't hexencode in base64
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

    # KDevelop: simplified - removed almost everything
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

    # KDevelop: commented out all val.typeid assignments,
    #           because we have not copied Value.typeid for simplicity

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
        # KDevelop: commented out, because we have not copied Value.check() for simplicity
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
        # KDevelop: removed support for the `{type}` syntax in the pattern of split()
        #           to avoid copying functions used only in the '}' branch from Qt Creator
        #self.warn("PATTERN: %s" % pattern)
        for c in pattern:
            #self.warn("PAT CODE: %s %s" % (c, str(n)))
            if False:
                pass
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
        if isinstance(value_or_address, self.Value):
            return self.value_split(value_or_address, pattern)
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

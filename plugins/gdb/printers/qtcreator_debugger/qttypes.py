# SPDX-FileCopyrightText: 2016 The Qt Company Ltd.
#
# SPDX-License-Identifier: GPL-3.0-only WITH Qt-GPL-exception-1.0

# Modified by Igor Kushnir <igorkuo@gmail.com> (initially)
# for the needs of qt.py

# KDevelop: the bodies of the functions parseQCborContainer() and qCborContainerBytedata()
#           (except for the last `return` line) were extracted from Qt Creator's function
#           qdumpHelper_QCbor_string(). The same code is duplicated in two other
#           Qt Creator's functions - qdumpHelper_QCbor_array() and qdumpHelper_QCbor_map().

def parseQCborContainer(d, container_ptr):
    # d.split('i@{@QByteArray::size_type}pp', container_ptr) doesn't work with CDB,
    # so be explicit:
    data_pos = container_ptr + (2 * d.ptrSize() if d.qtVersionAtLeast(0x060000) else 8)
    elements_pos = data_pos + (3 * d.ptrSize() if d.qtVersionAtLeast(0x060000) else d.ptrSize())
    elements_data_ptr, elements_size = d.vectorData(elements_pos)
    return (data_pos, elements_data_ptr, elements_size)

def qCborContainerBytedata(d, data_pos):
    bytedata, _, _ = d.qArrayData(data_pos)
    return bytedata

# KDevelop: removed unneeded parameter `is_bytes`
def qdumpHelper_QCbor_string(d, container_ptr, element_index):
    # KDevelop: extracted the helper functions parseQCborContainer()
    #           and qCborContainerBytedata() for reuse elsewhere
    data_pos, elements_data_ptr, _ = parseQCborContainer(d, container_ptr)
    element_at_n_addr = elements_data_ptr + element_index * 16 # sizeof(QtCbor::Element) == 16
    element_value, _, element_flags = d.split('qII', element_at_n_addr)
    # KDevelop: removed unneeded variable `enc`
    bytedata = qCborContainerBytedata(d, data_pos)
    bytedata += element_value
    if d.qtVersionAtLeast(0x060000):
        bytedata_len = d.extractInt64(bytedata)
        bytedata_data = bytedata + 8
    else:
        bytedata_len = d.extractInt(bytedata)
        bytedata_data = bytedata + 4 # sizeof(QtCbor::ByteData) header part
    # KDevelop: return the values instead of dumping them
    return (bytedata_data, bytedata_len, element_flags)

def qdumpHelper_QCborArray_valueAt(d, container_ptr, elements_data_ptr, idx, bytedata, is_cbor):
    element_at_n_addr = elements_data_ptr + idx * 16 # sizeof(QtCbor::Element) == 15
    element_value, element_type, element_flags = d.split('qII', element_at_n_addr)
    element_container, _, _ = d.split('pII', element_at_n_addr)
    # KDevelop: return a tuple instead of wrapping the values in d.createProxyValue()
    if element_flags & 1: # QtCbor::Element::IsContainer
        return (-1, element_container, element_type, is_cbor)
    if element_flags & 2: # QtCbor::Element::HasByteData
        return (idx, container_ptr, element_type, is_cbor)
    return (element_value, 0, element_type, is_cbor)

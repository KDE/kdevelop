#
# LLDB data formatter helpers
#
# SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
#

# BEGIN: Utilities for wrapping differences of Python 2.x and Python 3
# Inspired by https://pythonhosted.org/six/
from __future__ import print_function
import sys
import lldb
# Useful for very coarse version differentiation.
PY2 = sys.version_info[0] == 2
PY3 = sys.version_info[0] == 3

# create Python 2.x & 3.x compatible iterator base
if PY3:
    Iterator = object
else:
    class Iterator(object):

        def next(self):
            return type(self).__next__(self)
if PY3:
    unichr = chr
    unicode = str
else:
    unichr = unichr
# END


def canonicalized_type_name(name):
    """Canonicalize the type name for FindFirstType usage.
        + 1 space between template arguments (after comma)
        + no space before pointer *
    otherwise FindFirstType returns None
    """
    return name.replace(' ', '').replace(',', ', ')


def quote(string, quote='"'):
    """Quote a string so it's suitable to be used in quote"""
    if isinstance(string, unicode):
        ls = []
        for uch in string:
            code = ord(uch)
            if code > 255:
                ls += '\\u{:04x}'.format(code)
            elif code >= 127:
                ls += '\\x{:02x}'.format(code)
            elif uch == quote or uch == '\\':
                ls += '\\' + chr(code)
            elif code == 0:
                ls += '\\x00'
            else:
                ls += chr(code)
        return quote + ''.join(ls) + quote
    else:
        return '{q}{s}{q}'.format(s=string.replace('\\', '\\\\').replace(quote, '\\' + quote),
                                  q=quote)


def unquote(data, quote='"'):
    """Unquote a string"""
    if data.startswith(quote) and data.endswith(quote):
        data = data[1:-1]
        ls = []
        esc = False
        for ch in data:
            if esc:
                ls.append(ch)
                esc = False
            else:
                if ch == '\\':
                    esc = True
                else:
                    ls.append(ch)
        if esc:
            print('WARNING: unpaired escape')
        data = ''.join(ls)
    return data


def invoke(val, method, args=''):
    """Try to invoke a method on val, args are passed in as an expression string"""
    # first try to get a valid frame
    frame = None
    for f in [val.frame, lldb.frame, val.process.selected_thread.GetFrameAtIndex(0)]:
        if f.IsValid():
            frame = f
            break
    if frame is None:
        return lldb.SBValue()

    # second try to get a pointer to val
    if val.GetType().IsPointerType():
        ptype = val.GetType()
        addr = val.GetValueAsUnsigned(0)
    else:
        ptype = val.GetType().GetPointerType()
        addr = val.AddressOf().GetValueAsUnsigned(0)

    # third, build expression
    expr = 'reinterpret_cast<const {}>({})->{}({})'.format(ptype.GetName(), addr, method, args)
    res = frame.EvaluateExpression(expr)
    # if not res.IsValid():
    #     print 'Expr {} on value {} failed'.format(expr, val.GetName())
    return res


def rename(name, val):
    """Rename a SBValue"""
    return val.CreateValueFromData(name, val.GetData(), val.GetType())


def toSBPointer(valobj, addr, pointee_type):
    """Convert a addr integer to SBValue"""
    addr = addr & 0xFFFFFFFFFFFFFFFF  # force unsigned
    return valobj.CreateValueFromAddress(None, addr, pointee_type).AddressOf()


def validAddr(valobj, addr):
    """Test if a address is valid"""
    return toSBPointer(valobj, addr,
                       valobj.GetType().GetBasicType(lldb.eBasicTypeVoid).GetPointerType()).IsValid()


def validPointer(pointer):
    """Test if a SBValue pointer is valid"""
    if not pointer.IsValid():
        return False
    if pointer.GetValueAsUnsigned(0) == 0:
        return False
    return toSBPointer(pointer, pointer.GetValueAsUnsigned(0), pointer.GetType().GetPointeeType()).IsValid()


class AutoCacheValue(object):
    """An object that can create itself when needed and cache the result"""
    def __init__(self, creator):
        super(AutoCacheValue, self).__init__()
        self.creator = creator
        self.cache = None
        self.cached = False

    def get(self):
        if not self.cached:
            self.cache = self.creator()
            self.cached = True
        return self.cache


class HiddenMemberProvider(object):
    """A lldb synthetic provider that can provide hidden children.
       Original children is exposed in this way"""

    @staticmethod
    def _capping_size():
        return 255

    def __init__(self, valobj, internal_dict):
        self.valobj = valobj
        # number of normally visible children
        self._num_children = 0
        # cache for visible children
        self._members = []
        # cache for hidden children
        self._hiddens = []
        # child name to index
        self._name2idx = {}
        # whether to add original children
        self._add_original = True
        # some useful info
        process = self.valobj.GetProcess()
        self._endianness = process.GetByteOrder()
        self._pointer_size = process.GetAddressByteSize()
        self._char_type = valobj.GetType().GetBasicType(lldb.eBasicTypeChar)

    def has_children(self):
        return self._num_children != 0

    def num_children(self):
        return self._num_children

    def get_child_index(self, name):
        if name in self._name2idx:
            return self._name2idx[name]
        return None

    def get_child_at_index(self, idx):
        if not self.valobj.IsValid():
            return None
        if idx < 0:
            return None
        elif idx < self._num_children:
            child = self._members[idx]
        # These are hidden children, which won't be queried by lldb, but we know
        # they are there, so we can use them in summary provider, to avoid another
        # fetch from the inferior, and don't shadow original children
        elif idx < self._num_children + len(self._hiddens):
            child = self._hiddens[idx - self._num_children]
        else:
            return None

        if isinstance(child, AutoCacheValue):
            child = child.get()

        return child

    @staticmethod
    def _getName(var):
        if isinstance(var, lldb.SBValue):
            return var.GetName()
        else:
            return var[0]

    def update(self):
        self._num_children = -1
        self._members = []
        self._hiddens = []
        self._name2idx = {}

        if not self.valobj.IsValid():
            return

        # call _update on subclass
        self._update()

        # add valobj's original children as hidden children,
        # must be called after self._update, so subclass has chance
        # to disable it.
        if self._add_original:
            for v in self.valobj:
                self._addChild(v, hidden=True)

        # update num_children
        if self._num_children < 0:
            self._num_children = len(self._members)

        # build name to index lookup, hidden value first, so normal value takes precedence
        self._name2idx = {
            self._getName(self._hiddens[idx]): idx + self._num_children
            for idx in range(0, len(self._hiddens))
        }
        self._name2idx.update({
            self._getName(self._members[idx]): idx
            for idx in range(0, self._num_children)
        })

    def _update(self):
        """override in subclass"""
        pass

    def _addChild(self, var, hidden=False):
        if not isinstance(var, lldb.SBValue):
            # special handling for (name, expr) tuple of string constants
            if len(var) != 2:
                print('error, const char[] value should be a tuple with two elements, it is', var)
            name, content = var

            if isinstance(content, unicode):
                content = content.encode()

            try:
                char_arr_type = self._char_type.GetArrayType(len(content));
                strdata = lldb.SBData.CreateDataFromCString(self._endianness, self._pointer_size, content)
                var = self.valobj.CreateValueFromData(name, strdata, char_arr_type)
            except:
                pass

        cache = self._hiddens if hidden else self._members
        cache.append(var)
